/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           RvSipAddress.c                                   *
 *                                                                            *
 * The file defines the methods of the address object: construct, destruct,   *
 * copy, encode, parse and the ability to access and change it's parameters.  *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "AddrUrl.h"
#include "AddrAbsUri.h"
#include "AddrTelUri.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "AddrDiameterUri.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#include "_SipAddress.h"
#include "RvSipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipMsg.h"
#include "_SipCommonUtils.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvBool IsUrlAddrType(MsgAddress* pAddr);

#ifdef RV_SIP_JSR32_SUPPORT
static RvStatus RVCALLCONV  EncodeDisplayName(
								IN    MsgAddress*        pAddr,
								IN    HRPOOL             hPool,
								IN    HPAGE              hPage,
								INOUT RvUint32*          length);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

static RvStatus RVCALLCONV  EncodeBracket(
								IN  MsgAddress*        pAddr,
								IN  RvBool             bIsOpening,
								IN  HRPOOL             hPool,
								IN  HPAGE              hPage,
								OUT RvUint32*          length);


/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/
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
                                            OUT RvSipAddressHandle* hSipAddr)
{
    MsgMessage*   msg;
    RvStatus   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    msg = (MsgMessage*)hSipMsg;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                               msg->hPool, msg->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* associate the url to the requestLine */
    msg->startLine.requestLine.hRequestUri = *hSipAddr;

    return RV_OK;
}

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
                                          OUT RvSipAddressHandle*    hSipAddr)
{
    MsgPartyHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPartyHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the party header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

/***************************************************************************
 * RvSipAddrConstructInRouteHopHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given RouteHop
 *          header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Route header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInRouteHopHeader(
                                          IN  RvSipRouteHopHeaderHandle hHeader,
                                          IN  RvSipAddressType       eAddrType,
                                          OUT RvSipAddressHandle*    hSipAddr)
{
    MsgRouteHopHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgRouteHopHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the RouteHop header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

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
                                          OUT RvSipAddressHandle*      hSipAddr)
{
    MsgReferToHeader*   pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgReferToHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the Refer-To header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_SUBS_ON
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
                                        OUT RvSipAddressHandle*         hSipAddr)
{
    MsgReferredByHeader*   pHeader;
    RvStatus              stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgReferredByHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the Refer-To header according to the given entity */
    pHeader->hReferrerAddrSpec = *hSipAddr;

    return RV_OK;
}
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
                                          IN  RvSipAuthorizationHeaderHandle hHeader,
                                          IN  RvSipAddressType               eAddrType,
                                          OUT  RvSipAddressHandle*           hSipAddr)
{
    MsgAuthorizationHeader* pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgAuthorizationHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the Authorization header */
    pHeader->hDigestUri = *hSipAddr;

    return RV_OK;
}
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
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInContactHeader(
                                          IN  RvSipContactHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr)
{
    MsgContactHeader* pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgContactHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the contact header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

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
                                          IN  RvSipInfoHeaderHandle    hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT RvSipAddressHandle*      hAddr)
{
	MsgInfoHeader* pHeader;
    RvStatus          stat;
	
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hAddr == NULL)
        return RV_ERROR_NULLPTR;
	
    *hAddr=NULL;
	
    pHeader = (MsgInfoHeader*)hHeader;
	
    /* Creating the new uri object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
							  pHeader->hPool, pHeader->hPage, 
							  eAddrType, hAddr);
	
    if(stat != RV_OK)
    {
        return stat;
    }
	
    /* Associating the new url to the contact header */
    pHeader->hAddrSpec = *hAddr;
	
    return RV_OK;
}

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
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInReplyToHeader(
                                          IN   RvSipReplyToHeaderHandle hHeader,
                                          IN   RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*      hAddr)
{
	MsgReplyToHeader* pHeader;
    RvStatus          stat;
	
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hAddr == NULL)
        return RV_ERROR_NULLPTR;
	
    *hAddr=NULL;
	
    pHeader = (MsgReplyToHeader*)hHeader;
	
    /* Creating the new uri object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
							  pHeader->hPool, pHeader->hPage, 
							  eAddrType, hAddr);
	
    if(stat != RV_OK)
    {
        return stat;
    }
	
    /* Associating the new url to the contact header */
    pHeader->hAddrSpec = *hAddr;
	
    return RV_OK;
}

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
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPUriHeader(
                                          IN  RvSipPUriHeaderHandle hHeader,
                                          IN  RvSipAddressType         eAddrType,
                                          OUT  RvSipAddressHandle*     hSipAddr)
{
    MsgPUriHeader* pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPUriHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the PUriHeader header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

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
                                          OUT  RvSipAddressHandle*     hSipAddr)
{
    MsgPProfileKeyHeader* pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPProfileKeyHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the PProfileKeyHeader header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

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
                                          OUT  RvSipAddressHandle*     hSipAddr)
{
    MsgPUserDatabaseHeader* pHeader;
    RvStatus                stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPUserDatabaseHeader*)hHeader;

    /* Make sure the address type is of DiameterURI */
    if(eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipAddrConstructInPUserDatabaseHeader - The given address is not a DIAMETER URI"));
        return RV_ERROR_BADPARAM;
    }
    
    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the PUserDatabaseHeader header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

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
                                          OUT  RvSipAddressHandle*     hSipAddr)
{
    MsgPServedUserHeader* pHeader;
    RvStatus           stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPServedUserHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the PServedUserHeader header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}
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
                                         OUT  RvSipAddressHandle*				hSipAddr)
{
    MsgPDCSTracePartyIDHeader*  pHeader;
    RvStatus					stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPDCSTracePartyIDHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the PDCSTracePartyIDHeader header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}

/***************************************************************************
 * RvSipAddrConstructInPDCSBillingInfoHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given PDCSBillingInfo
 *			header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the PDCSBillingInfo header.
 *         eAddrType - The type of address to construct.
 * output: hAddr     - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInPDCSBillingInfoHeader(
                                          IN  RvSipPDCSBillingInfoHeaderHandle	hHeader,
                                          IN  RvSipAddressType					eAddrType,
										  IN  RvSipPDCSBillingInfoAddressType	ePDCSBillingInfoAddrType,
                                         OUT  RvSipAddressHandle*				hSipAddr)
{
    MsgPDCSBillingInfoHeader*	pHeader;
    RvStatus					stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPDCSBillingInfoHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Put the new url in the correct field of the PDCSBillingInfo header */
    switch(ePDCSBillingInfoAddrType)
	{
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE:
		pHeader->hChargeAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING:
		pHeader->hCallingAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED:
		pHeader->hCalledAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING:
		pHeader->hRoutingAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE:
		pHeader->hLocRouteAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_UNDEFINED:
	default:
		return RV_ERROR_BADPARAM;
	}

    return RV_OK;
}

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
                                 OUT  RvSipAddressHandle*			hSipAddr)
{
    MsgPDCSRedirectHeader*	pHeader;
    RvStatus				stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgPDCSRedirectHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Put the new url in the correct field of the PDCSRedirect header */
    switch(ePDCSRedirectAddrType)
	{
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID:
		pHeader->hCalledIDAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR:
		pHeader->hRedirectorAddr = *hSipAddr;
		break;
	case RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_UNDEFINED:
	default:
		return RV_ERROR_BADPARAM;
	}

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 


#ifndef RV_SIP_PRIMITIVES

/***************************************************************************
 * RvSipAddrConstructInContentTypeHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given 
 *          Content-Type header. The address handle is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hHeader   - Handle to the Content-Type header.
 *         eAddrType - The type of address to construct-In this version, only URL.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructInContentTypeHeader(
                                          IN  RvSipContentTypeHeaderHandle hHeader,
                                          IN  RvSipAddressType			   eAddrType,
                                          OUT RvSipAddressHandle		  *hSipAddr)
{
    MsgContentTypeHeader *pHeader;
    RvStatus              stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgContentTypeHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the ContentType header */
    pHeader->hStartAddrSpec = *hSipAddr;

    return RV_OK;
}


/***************************************************************************
 * RvSipAddrConstructInContentIDHeader
 * ------------------------------------------------------------------------
 * General: Constructs the address-spec address object inside a given 
 *          Content-ID header. The address handle is returned.
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
                                          OUT RvSipAddressHandle           *hSipAddr)
{
    MsgContentIDHeader  *pHeader;
    RvStatus             stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pHeader = (MsgContentIDHeader*)hHeader;

    /* Creating the new url object */
    stat = RvSipAddrConstruct((RvSipMsgMgrHandle)pHeader->pMsgMgr,
                               pHeader->hPool, pHeader->hPage, eAddrType, hSipAddr);

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Associating the new url to the ContentID header */
    pHeader->hAddrSpec = *hSipAddr;

    return RV_OK;
}
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
                                              OUT RvSipAddressHandle* hSipAddr)
{
    MsgAddress* pAddr;
    MsgMgr*     pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    *hSipAddr=NULL;

    pAddr = (MsgAddress*)hSipAddr;

    pAddr = (MsgAddress*)SipMsgUtilsAllocAlign(pMsgMgr,
                                               hPool,
                                               hPage,
                                               sizeof(MsgAddress),
                                               RV_TRUE);
    if(pAddr == NULL)
    {
        *hSipAddr = NULL;

        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipAddrConstruct - Failed. No resources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));

        return RV_ERROR_OUTOFRESOURCES;
    }

    pAddr->pMsgMgr = pMsgMgr;
    pAddr->hPool   = hPool;
    pAddr->hPage   = hPage;
#ifdef RV_SIP_JSR32_SUPPORT
	pAddr->strDisplayName = UNDEFINED;
#endif

    switch (eAddrType)
    {
        case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
        case RVSIP_ADDRTYPE_IM:
        case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
        {
            pAddr->eAddrType = eAddrType;

            if(AddrUrlConstruct((RvSipMsgMgrHandle)pAddr->pMsgMgr,
                                hPool, hPage, &(pAddr->uAddrBody.hUrl)) != RV_OK)
            {
                return RV_ERROR_OUTOFRESOURCES;
            }
            else
            {
                *hSipAddr = (RvSipAddressHandle)pAddr;
            }

            break;
        }
        case RVSIP_ADDRTYPE_ABS_URI:
        {
            pAddr->eAddrType = eAddrType;

            if(AddrAbsUriConstruct((RvSipMsgMgrHandle)pAddr->pMsgMgr,
                                   hPool, hPage, &(pAddr->uAddrBody.pAbsUri)) != RV_OK)
            {
                return RV_ERROR_OUTOFRESOURCES;
            }
            else
            {
                *hSipAddr = (RvSipAddressHandle)pAddr;
            }

            break;
        }
#ifdef RV_SIP_TEL_URI_SUPPORT
		case RVSIP_ADDRTYPE_TEL:
			{
				pAddr->eAddrType = eAddrType;

				if(AddrTelUriConstruct((RvSipMsgMgrHandle)pAddr->pMsgMgr,
					hPool, hPage, &(pAddr->uAddrBody.pTelUri)) != RV_OK)
				{
					return RV_ERROR_OUTOFRESOURCES;
				}
				else
				{
					*hSipAddr = (RvSipAddressHandle)pAddr;
				}

				break;
			}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_ADDRTYPE_DIAMETER:
			{
				pAddr->eAddrType = eAddrType;

				if(AddrDiameterUriConstruct((RvSipMsgMgrHandle)pAddr->pMsgMgr,
					hPool, hPage, &(pAddr->uAddrBody.pDiameterUri)) != RV_OK)
				{
					return RV_ERROR_OUTOFRESOURCES;
				}
				else
				{
					*hSipAddr = (RvSipAddressHandle)pAddr;
				}

				break;
			}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
        default:
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipAddrConstruct - Failed. Unknown address type. eAddrType is %d",
                eAddrType));
            return RV_ERROR_BADPARAM;
        }
    }

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
        "RvSipAddrConstruct - Address was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
        hPool, hPage, pAddr));
    return RV_OK;
}


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
RVAPI RvStatus RVCALLCONV RvSipAddrCopy(INOUT RvSipAddressHandle hDestination,
                                        IN	  RvSipAddressHandle hSource)
{
    MsgAddress* dest;
    MsgAddress* source;

    if((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest   = (MsgAddress*)hDestination;
    source = (MsgAddress*)hSource;

    if (dest->eAddrType != source->eAddrType)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAddrCopy - Failed. dest eAddrType %d is different than source eAddrType %d",
             dest->eAddrType, source->eAddrType));
        return RV_ERROR_INVALID_HANDLE;
    }

#ifdef RV_SIP_JSR32_SUPPORT
	/* copy display name */
    if(source->strDisplayName > UNDEFINED)
    {
        dest->strDisplayName = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
															dest->hPool,
															dest->hPage,
															source->hPool,
															source->hPage,
															source->strDisplayName);
        if(dest->strDisplayName == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
						"RvSipAddrCopy - Failed to copy strDisplayName"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pDisplayName = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strDisplayName);
#endif
    }
    else
    {
        dest->strDisplayName = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDisplayName = NULL;
#endif
    }
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    switch(source->eAddrType)
    {
    case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
    case RVSIP_ADDRTYPE_IM:
    case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
        return AddrUrlCopy(hDestination, hSource);
    case RVSIP_ADDRTYPE_ABS_URI:
        return AddrAbsUriCopy(hDestination, hSource);
#ifdef RV_SIP_TEL_URI_SUPPORT
    case RVSIP_ADDRTYPE_TEL:
        return AddrTelUriCopy(hDestination, hSource);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    case RVSIP_ADDRTYPE_DIAMETER:
        return AddrDiameterUriCopy(hDestination, hSource);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    default:
        RvLogExcep(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
            "RvSipAddrCopy - Failed. eAddrType is unknown %d", source->eAddrType));
        return RV_ERROR_BADPARAM;
    }
}

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
                                           OUT RvUint32*         length)
{
    RvStatus stat;
    MsgAddress* pAddr ;

    pAddr = (MsgAddress*)hSipAddr;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;
    if ((hPool == NULL)||(hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hSipAddr is 0x%p",
                hPool, hSipAddr));
        return stat;
    }
    else
    {
        RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrEncode - Got new page 0x%p on pool 0x%p. hSipAddr is 0x%p",
                *hPage, hPool, hSipAddr));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = AddrEncode(hSipAddr, hPool, *hPage, RV_FALSE, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrEncode - Failed, free page 0x%p on pool 0x%p. AddrEncode fail",
                *hPage, hPool));
    }
    return stat;
}
/***************************************************************************
 * SipAddrEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *            Url as encoded buffer (string) onto it.
 *            The user should check the return code.  If the size of
 *            the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *            and the user should send bigger buffer for the encoded message.
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
					   OUT RvUint32*          length)
{
    if (length == NULL)
        return RV_ERROR_NULLPTR;

    return (AddrEncode(hSipAddr, hPool, hPage, RV_FALSE, bAddBrackets, length));
}


/***************************************************************************
 * AddrEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *            Url as encoded buffer (string) onto it.
 *            The user should check the return code.  If the size of
 *            the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *            and the user should send bigger buffer for the encoded message.
 * Return Value: RV_OK, RV_ERROR_INSUFFICIENT_BUFFER, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle of the url address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bAddBrackets - Indicates whether the address should be wrapped by
 *                       brackets (<>). If the address has a display name (jsr32)
 *                       then it will use brackets anyway.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  AddrEncode(
								IN  RvSipAddressHandle hSipAddr,
								IN  HRPOOL             hPool,
								IN  HPAGE              hPage,
								IN  RvBool             bInUrlHeaders,
								IN  RvBool             bAddBrackets,
								OUT RvUint32*          length)
{
    MsgAddress* pAddr;
	RvStatus    stat;

    pAddr = (MsgAddress*)hSipAddr;
    if((hSipAddr == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_INVALID_HANDLE;

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "AddrEncode - Encoding Address. hSipAddr 0x%p, hPool 0x%p, hPage 0x%p",
             hSipAddr, hPool, hPage));

#ifdef RV_SIP_JSR32_SUPPORT
	/* encode display name */
    if(pAddr->strDisplayName > UNDEFINED)
    {
        stat = EncodeDisplayName(pAddr, hPool, hPage, length);
		if (RV_OK != stat)
		{
			return stat;
		}

		bAddBrackets = RV_TRUE;
    }
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

	/* set "<" */
	if (bAddBrackets == RV_TRUE)
	{
		stat = EncodeBracket(pAddr, RV_TRUE, hPool, hPage, length);
		if (RV_OK != stat)
		{
			return stat;
		}
	}
    stat = EncodeAddrSpec(pAddr, hPool, hPage, bInUrlHeaders, length);
    if (stat != RV_OK)
	{
		return stat;
	}


	if (bAddBrackets == RV_TRUE)
	{
		/* set ">" */
		stat = EncodeBracket(pAddr, RV_FALSE, hPool, hPage, length);
		if (RV_OK != stat)
		{
            return stat;
		}
	}

	return RV_OK;
}

/***************************************************************************
 * EncodeAddrSpec
 * ------------------------------------------------------------------------
 * General: Encodes the addr-spec of an address object on a given pool and page
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pAddr    - Pointer to the address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * Inout: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  EncodeAddrSpec(
								IN    MsgAddress*        pAddr,
								IN    HRPOOL             hPool,
								IN    HPAGE              hPage,
								IN    RvBool             bInUrlHeaders,
								INOUT RvUint32*          length)
{
    RvStatus    stat = RV_OK;

    switch(pAddr->eAddrType)
    {
        case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
        case RVSIP_ADDRTYPE_IM:
        case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
            stat = AddrUrlEncode((RvSipAddressHandle)pAddr,
                                 hPool,
                                 hPage,
                                 bInUrlHeaders,
                                 length);
			break;
        case RVSIP_ADDRTYPE_ABS_URI:
            stat = AddrAbsUriEncode((RvSipAddressHandle)pAddr,
                                    hPool,
                                    hPage,
                                    bInUrlHeaders,
                                    length);
			break;
#ifdef RV_SIP_TEL_URI_SUPPORT
		case RVSIP_ADDRTYPE_TEL:
            stat = AddrTelUriEncode((RvSipAddressHandle)pAddr,
									hPool,
									hPage,
									bInUrlHeaders,
									length);
			break;
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_ADDRTYPE_DIAMETER:
            stat = AddrDiameterUriEncode((RvSipAddressHandle)pAddr,
									hPool,
									hPage,
									bInUrlHeaders,
									length);
			break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
        default:
        {
            RvLogExcep(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                   "EncodeAddrSpec - eAddrType is unknown %d",
                   pAddr->eAddrType));
            return RV_ERROR_BADPARAM;
        }
    }

	if (RV_OK != stat)
	{
		RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
			       "EncodeAddrSpec - Failed to encode address spec. address=%x",
			       pAddr));
	}

	return stat;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddrEncodeWithReplaces
 * ------------------------------------------------------------------------
 * General: This function encodes an address object similarly to AddrEncode(),
 *          with the addition of a Replaces parameter that is received from
 *          the caller of this function and is encoded as part of the address
 *          object. Notice: the Replaces parameter is used for URL addresses.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle of the url address object.
 *        hReplaces - Handle of the replaces parameter.
 *        hPool     - Handle of the pool of pages
 *        hPage     - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bAddBrackets - Indicates whether the address should be wrapped by
 *                       brackets (<>). If the address has a display name (jsr32)
 *                       then it will use brackets anyway.
 * output: length   - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  AddrEncodeWithReplaces(
                     IN  RvSipAddressHandle        hSipAddr,
					 IN  RvSipReplacesHeaderHandle hReplaces,
                     IN  HRPOOL                    hPool,
                     IN  HPAGE                     hPage,
                     IN  RvBool                    bInUrlHeaders,
					 IN  RvBool                    bAddBrackets,
                     OUT RvUint32*                 length)
{
	MsgAddress* pAddr;
	RvStatus    stat;

    pAddr = (MsgAddress*)hSipAddr;
    if((hSipAddr == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_INVALID_HANDLE;

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
				"AddrEncodeWithReplaces - Encoding Address. hSipAddr 0x%p, hPool 0x%p, hPage 0x%p",
				hSipAddr, hPool, hPage));

#ifdef RV_SIP_JSR32_SUPPORT
	/* encode display name */
    if(pAddr->strDisplayName > UNDEFINED)
    {
        stat = EncodeDisplayName(pAddr, hPool, hPage, length);
		if (RV_OK != stat)
		{
			return stat;
		}

		bAddBrackets = RV_TRUE;
    }
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

	/* set "<" */
	if (bAddBrackets == RV_TRUE)
	{
		stat = EncodeBracket(pAddr, RV_TRUE, hPool, hPage, length);
		if (RV_OK != stat)
		{
			return stat;
		}
	}

    stat = EncodeAddrSpec(pAddr, hPool, hPage, bInUrlHeaders, length);
	if (stat != RV_OK)
	{
		return stat;
	}

	if(hReplaces != NULL)
	{
		MsgAddrUrl *pUrl;

		pUrl = (MsgAddrUrl *)pAddr->uAddrBody.hUrl;

		if (pAddr->eAddrType != RVSIP_ADDRTYPE_URL)
		{
			RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
					   "AddrEncodeWithReplaces: Failed to encode Replaces - This is not a SIP URL. hAddr 0x%p",
						pAddr));
			return RV_ERROR_UNKNOWN;
		}
		if(pUrl->strHeaders != UNDEFINED)
		{
			/* set "&" because it is not the first header in this url. */
			stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "&", length);
		}
		else
		{
			/* set "?" because it is the first header in url. */
			stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "?", length);
		}
		if (stat != RV_OK)
		{
			RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
					"AddrEncodeWithReplaces: Failed to encode Replaces. hAddr 0x%p",
					pAddr));
			return stat;
		}

		stat = ReplacesHeaderEncode(hReplaces, hPool, hPage, RV_TRUE, length);
		if (stat != RV_OK)
		{
			RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
				"AddrEncodeWithReplaces: Failed to encode Replaces. hAddr 0x%p",
				pAddr));
			return stat;
		}
	}

	if (bAddBrackets == RV_TRUE)
	{
		/* set ">" */
		stat = EncodeBracket(pAddr, RV_FALSE, hPool, hPage, length);
		if (RV_OK != stat)
		{
			return stat;
		}
	}

	return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * RvSipAddrParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual Address object, such as, "sip:bob@radvision.com", into an
 *         Address object . All the textual fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A handle to the Address object.
 *    buffer    - Buffer containing a textual Address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrParse(
                                     IN    RvSipAddressHandle hSipAddr,
                                     IN    RvChar*           buffer)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if((hSipAddr == NULL) || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "RvSipAddrParse: Parsing address. hSipAddr 0x%p, buffer 0x%p",
             hSipAddr, buffer));

    switch(((MsgAddress*)hSipAddr)->eAddrType)
    {
        case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
        case RVSIP_ADDRTYPE_IM:
        case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
            return AddrUrlParse(hSipAddr, buffer);

        case RVSIP_ADDRTYPE_ABS_URI:
            return AddrAbsUriParse(hSipAddr, buffer);

#ifdef RV_SIP_TEL_URI_SUPPORT
		case RVSIP_ADDRTYPE_TEL:
            return AddrTelUriParse(hSipAddr, buffer);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
        case RVSIP_ADDRTYPE_DIAMETER:
            return AddrDiameterUriParse(hSipAddr, buffer);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
        default:
        {
            RvLogExcep(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                   "RvSipAddrParse - eAddrType is unknown %d",
                   ((MsgAddress*)hSipAddr)->eAddrType));
            return RV_ERROR_BADPARAM;
        }
    }
}


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
 *         hPool     - Handle to the memory pool. that the object uses.
 *         hPage     - Handle to the memory page that the object uses.
 *         buffer    - Buffer containing a textual Address.
 * output: hSipAddr  - Handle to the newly constructed address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrConstructAndParse(
											  IN  RvSipMsgMgrHandle   hMsgMgr,
                                              IN  HRPOOL              hPool,
                                              IN  HPAGE               hPage,
                                              IN  RvChar*             buffer,
                                              OUT RvSipAddressHandle* hSipAddr)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
	MsgMgr*            pMsgMgr  = (MsgMgr*)hMsgMgr;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    RvSipAddressType   eAddrType;
	RvStatus           rv;
	
    if (hSipAddr == NULL || (hMsgMgr == NULL))
        return RV_ERROR_INVALID_HANDLE;
	if (buffer == NULL)
		return RV_ERROR_BADPARAM;
	
	eAddrType = SipAddrGetAddressTypeFromString(hMsgMgr, buffer);

	rv = RvSipAddrConstruct(hMsgMgr, hPool, hPage, eAddrType, hSipAddr);
	if (RV_OK != rv)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
			"RvSipAddrConstructAndParse: Failed in RvSipAddrConstruct. rv=%d",
			rv));
		return rv;
	}

	rv = RvSipAddrParse(*hSipAddr, buffer);
	if (RV_OK != rv)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
			"RvSipAddrConstructAndParse: Failed in RvSipAddrParse. hAddr=0x%p, rv=%d",
			*hSipAddr, rv));
		return rv;
	}

	return RV_OK;
}


/***************************************************************************
 * SipAddrGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the address object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipAddrGetPool(RvSipAddressHandle hAddr)
{
    return ((MsgAddress*)hAddr)->hPool;
}

/***************************************************************************
 * SipAddrGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the address object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipAddrGetPage(RvSipAddressHandle hAddr)
{
    return ((MsgAddress*)hAddr)->hPage;
}

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
 *    hSipAddr   - Handle to the Address object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipAddrGetStringLength(
                                      IN  RvSipAddressHandle     hSipAddr,
                                      IN  RvSipAddressStringName stringName)
{
    RvInt32 strOffset;
    MsgAddress*  pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return 0;

#ifdef RV_SIP_JSR32_SUPPORT
	if (stringName == RVSIP_ADDRESS_DISPLAY_NAME)
	{
		strOffset = SipAddrGetDisplayName(hSipAddr);
	}
    else
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

	if(IsUrlAddrType(pAddr) == RV_TRUE)
    {
        switch (stringName)
        {
        case RVSIP_ADDRESS_USER:
            strOffset = SipAddrUrlGetUser(hSipAddr);
            break;
        case RVSIP_ADDRESS_HOST:
            strOffset = SipAddrUrlGetHost(hSipAddr);
            break;
        case RVSIP_ADDRESS_TRANSPORT:
            strOffset = SipAddrUrlGetStrTransport(hSipAddr);
            break;
        case RVSIP_ADDRESS_MADDR_PARAM:
            strOffset = SipAddrUrlGetMaddrParam(hSipAddr);
            break;
        case RVSIP_ADDRESS_USER_PARAM:
            strOffset = SipAddrUrlGetStrUserParam(hSipAddr);
            break;
		case RVSIP_ADDRESS_TOKENIZED_BY_PARAM:
            strOffset = SipAddrUrlGetTokenizedByParam(hSipAddr);
            break;
        case RVSIP_ADDRESS_URL_OTHER_PARAMS:
            strOffset = SipAddrUrlGetUrlParams(hSipAddr);
            break;
        case RVSIP_ADDRESS_HEADERS:
            strOffset = SipAddrUrlGetHeaders(hSipAddr);
            break;
        case RVSIP_ADDRESS_METHOD:
            strOffset = SipAddrUrlGetStrMethod(hSipAddr);
            break;
        case RVSIP_ADDRESS_COMP_PARAM:
            strOffset = SipAddrUrlGetStrCompParam(hSipAddr);
            break;
		case RVSIP_ADDRESS_SIGCOMPID_PARAM:
			strOffset = SipAddrUrlGetSigCompIdParam(hSipAddr);
			break;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
        case RVSIP_ADDRESS_CPC:
            strOffset = SipAddrUrlGetStrCPC(hSipAddr);
            break;
        case RVSIP_ADDRESS_GR:
            strOffset = SipAddrUrlGetStrGr(hSipAddr);
            break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    else if(pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
    {
        switch (stringName)
        {
        case RVSIP_ADDRESS_ABS_URI_SCHEME:
            strOffset = SipAddrAbsUriGetScheme(hSipAddr);
            break;
        case RVSIP_ADDRESS_ABS_URI_IDENTIFIER:
            strOffset = SipAddrAbsUriGetIdentifier(hSipAddr);
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
#ifdef RV_SIP_TEL_URI_SUPPORT
	else if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
    {
		switch (stringName)
        {
        case RVSIP_ADDRESS_TEL_URI_NUMBER:
            strOffset = SipAddrTelUriGetPhoneNumber(hSipAddr);
            break;
        case RVSIP_ADDRESS_TEL_URI_EXTENSION:
            strOffset = SipAddrTelUriGetExtension(hSipAddr);
            break;
		case RVSIP_ADDRESS_TEL_URI_CONTEXT:
            strOffset = SipAddrTelUriGetContext(hSipAddr);
            break;
		case RVSIP_ADDRESS_TEL_URI_ISDN_SUBS:
            strOffset = SipAddrTelUriGetIsdnSubAddr(hSipAddr);
            break;
		case RVSIP_ADDRESS_TEL_URI_POST_DIAL:
            strOffset = SipAddrTelUriGetPostDial(hSipAddr);
            break;
		case RVSIP_ADDRESS_TEL_URI_OTHER_PARAMS:
            strOffset = SipAddrTelUriGetOtherParams(hSipAddr);
            break;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
        case RVSIP_ADDRESS_TEL_URI_CPC:
            strOffset = SipAddrTelUriGetStrCPC(hSipAddr);
            break;
        case RVSIP_ADDRESS_TEL_URI_RN:
            strOffset = SipAddrTelUriGetRn(hSipAddr);
            break;
        case RVSIP_ADDRESS_TEL_URI_RN_CONTEXT:
            strOffset = SipAddrTelUriGetRnContext(hSipAddr);
            break;
        case RVSIP_ADDRESS_TEL_URI_CIC:
            strOffset = SipAddrTelUriGetCic(hSipAddr);
            break;
        case RVSIP_ADDRESS_TEL_URI_CIC_CONTEXT:
            strOffset = SipAddrTelUriGetCicContext(hSipAddr);
            break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */             

        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	else if(pAddr->eAddrType == RVSIP_ADDRTYPE_DIAMETER)
    {
		switch (stringName)
        {
        case RVSIP_ADDRESS_DIAMETER_URI_HOST:
            strOffset = SipAddrDiameterUriGetHost(hSipAddr);
            break;
        case RVSIP_ADDRESS_DIAMETER_URI_TRANSPORT:
            strOffset = SipAddrDiameterUriGetStrTransport(hSipAddr);
            break;
		case RVSIP_ADDRESS_DIAMETER_URI_OTHER_PARAMS:
            strOffset = SipAddrDiameterUriGetOtherParams(hSipAddr);
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                 "RvSipAddrGetStringLength - Unknown address type %d", stringName));
        return 0;
    }

    if(strOffset != UNDEFINED)
        return (RPOOL_Strlen(pAddr->hPool, pAddr->hPage, strOffset)+ 1);
                                                                /* +1 for null in the end */
    else
        return 0;
}
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
RVAPI RvSipAddressType RVCALLCONV RvSipAddrGetAddrType(IN RvSipAddressHandle hSipAddr)
{
    if (hSipAddr == NULL)
        return RVSIP_ADDRTYPE_UNDEFINED;

    return ((MsgAddress*)hSipAddr)->eAddrType;
}

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
RvStatus RVCALLCONV SipAddrConvertOtherAddrToUrl(IN RvSipAddressHandle hSipAddr)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if (hSipAddr == NULL)
        return RVSIP_ADDRTYPE_UNDEFINED;

     if(pAddr->eAddrType == RVSIP_ADDRTYPE_IM || pAddr->eAddrType == RVSIP_ADDRTYPE_PRES)
     {
         pAddr->eAddrType = RVSIP_ADDRTYPE_URL;
     }
     else
     {
         return RV_ERROR_ILLEGAL_ACTION;
     }
     return RV_OK;
}
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

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
                             INOUT RPOOL_Ptr                 *pRpoolPtr)
{
    MsgAddress* pAddr;
    MsgAddrUrl* pUrl = NULL;
    RvInt32    requestedParamOffset;

    if(hAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pAddr = (MsgAddress*)hAddr;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

#ifdef RV_SIP_JSR32_SUPPORT
	if (eStringName == RVSIP_ADDRESS_DISPLAY_NAME)
	{
		requestedParamOffset = pAddr->strDisplayName;
	}
	else
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
    {
        switch(eStringName)
        {
        case RVSIP_ADDRESS_ABS_URI_SCHEME:
            requestedParamOffset = pAddr->uAddrBody.pAbsUri->strScheme;
            break;
        case RVSIP_ADDRESS_ABS_URI_IDENTIFIER:
            requestedParamOffset = pAddr->uAddrBody.pAbsUri->strIdentifier;
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
    }
    else if(IsUrlAddrType(pAddr) == RV_TRUE)
    {
        pUrl = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;
        switch(eStringName)
        {
        case RVSIP_ADDRESS_USER:
            requestedParamOffset = pUrl->strUser;
            break;
        case RVSIP_ADDRESS_HOST:
            requestedParamOffset = pUrl->strHost;
            break;
        case RVSIP_ADDRESS_MADDR_PARAM:
            requestedParamOffset = pUrl->strMaddrParam;
            break;
        case RVSIP_ADDRESS_URL_OTHER_PARAMS:
            requestedParamOffset = pUrl->strUrlParams;
            break;
        case RVSIP_ADDRESS_HEADERS:
            requestedParamOffset = pUrl->strHeaders;
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
    }
#ifdef RV_SIP_TEL_URI_SUPPORT
	else if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
    {
		switch (eStringName)
        {
        case RVSIP_ADDRESS_TEL_URI_NUMBER:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strPhoneNumber;
            break;
        case RVSIP_ADDRESS_TEL_URI_EXTENSION:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strExtension;
            break;
		case RVSIP_ADDRESS_TEL_URI_CONTEXT:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strContext;
            break;
		case RVSIP_ADDRESS_TEL_URI_ISDN_SUBS:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strIsdnSubAddr;
            break;
		case RVSIP_ADDRESS_TEL_URI_POST_DIAL:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strPostDial;
            break;
		case RVSIP_ADDRESS_TEL_URI_OTHER_PARAMS:
            requestedParamOffset = pAddr->uAddrBody.pTelUri->strOtherParams;
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
	}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
    #ifdef RV_SIP_IMS_HEADER_SUPPORT
	else if(pAddr->eAddrType == RVSIP_ADDRTYPE_DIAMETER)
    {
		switch (eStringName)
        {
        case RVSIP_ADDRESS_DIAMETER_URI_HOST:
            requestedParamOffset = pAddr->uAddrBody.pDiameterUri->strHost;
            break;
        case RVSIP_ADDRESS_DIAMETER_URI_TRANSPORT:
            requestedParamOffset = pAddr->uAddrBody.pDiameterUri->strTransport;
            break;
		case RVSIP_ADDRESS_DIAMETER_URI_OTHER_PARAMS:
            requestedParamOffset = pAddr->uAddrBody.pDiameterUri->strOtherParams;
            break;
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrGetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    else
    {
        RvLogExcep(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrGetRpoolString - Failed, Address type is not defined"));
        return RV_ERROR_BADPARAM;

    }

    /* the parameter does not exist in the header - return UNDEFINED*/
    if(requestedParamOffset == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(
                                     pAddr->pMsgMgr,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pAddr->hPool,
                                     pAddr->hPage,
                                     requestedParamOffset);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipAddrSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the address
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hAddr - Handle to the address object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrSetRpoolString(
                             IN    RvSipAddressHandle      hAddr,
                             IN    RvSipAddressStringName  eStringName,
                             IN    RPOOL_Ptr               *pRpoolPtr)
{
    MsgAddress* pAddr;

    if(hAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pAddr = (MsgAddress*)hAddr;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

#ifdef RV_SIP_JSR32_SUPPORT
	if (eStringName == RVSIP_ADDRESS_DISPLAY_NAME)
	{
		return SipAddrSetDisplayName(hAddr,NULL,
									 pRpoolPtr->hPool,
									 pRpoolPtr->hPage,
									 pRpoolPtr->offset);
	}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
    {
        switch(eStringName)
        {
        case RVSIP_ADDRESS_ABS_URI_SCHEME:
            return SipAddrAbsUriSetScheme(hAddr,NULL,
                                          pRpoolPtr->hPool,
                                          pRpoolPtr->hPage,
                                          pRpoolPtr->offset);

        case RVSIP_ADDRESS_ABS_URI_IDENTIFIER:
            return SipAddrAbsUriSetIdentifier(hAddr,NULL,
                                          pRpoolPtr->hPool,
                                          pRpoolPtr->hPage,
                                          pRpoolPtr->offset);
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrSetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
    }
    else if(IsUrlAddrType(pAddr)==RV_TRUE)
    {
        switch(eStringName)
        {
        case RVSIP_ADDRESS_USER:
            return SipAddrUrlSetUser(hAddr,NULL,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);
        case RVSIP_ADDRESS_HOST:
            return SipAddrUrlSetHost(hAddr,NULL,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);
        case RVSIP_ADDRESS_MADDR_PARAM:
            return SipAddrUrlSetMaddrParam(hAddr,NULL,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);
        case RVSIP_ADDRESS_URL_OTHER_PARAMS:
            return SipAddrUrlSetUrlParams(hAddr,NULL,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);
        case RVSIP_ADDRESS_HEADERS:
            return SipAddrUrlSetHeaders(hAddr,NULL,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pRpoolPtr->offset);
        default:
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrSetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
    }
#ifdef RV_SIP_TEL_URI_SUPPORT
	else if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
    {
		switch (eStringName)
        {
        case RVSIP_ADDRESS_TEL_URI_NUMBER:
            return SipAddrTelUriSetPhoneNumber(hAddr,NULL,
												pRpoolPtr->hPool,
												pRpoolPtr->hPage,
												pRpoolPtr->offset);
        case RVSIP_ADDRESS_TEL_URI_EXTENSION:
            return SipAddrTelUriSetExtension(hAddr,NULL,
											pRpoolPtr->hPool,
											pRpoolPtr->hPage,
											pRpoolPtr->offset);
		case RVSIP_ADDRESS_TEL_URI_CONTEXT:
            return SipAddrTelUriSetContext(hAddr,NULL,
											pRpoolPtr->hPool,
											pRpoolPtr->hPage,
											pRpoolPtr->offset);
		case RVSIP_ADDRESS_TEL_URI_ISDN_SUBS:
            return SipAddrTelUriSetIsdnSubAddr(hAddr,NULL,
												pRpoolPtr->hPool,
												pRpoolPtr->hPage,
												pRpoolPtr->offset);
		case RVSIP_ADDRESS_TEL_URI_POST_DIAL:
            return SipAddrTelUriSetPostDial(hAddr,NULL,
											pRpoolPtr->hPool,
											pRpoolPtr->hPage,
											pRpoolPtr->offset);
		case RVSIP_ADDRESS_TEL_URI_OTHER_PARAMS:
            return SipAddrTelUriSetOtherParams(hAddr,NULL,
												pRpoolPtr->hPool,
												pRpoolPtr->hPage,
												pRpoolPtr->offset);
        default:
           RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrSetRpoolString - Failed, the requested parameter is not supported by this function"));
            return RV_ERROR_BADPARAM;
        }
    }
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
    else
    {
        RvLogExcep(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrSetRpoolString - Failed, Address type is not defined"));
        return RV_ERROR_BADPARAM;

    }
}

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
                                          IN RvSipAddressHandle hOtherAddress)
{
    MsgAddress*   pAddr;
    MsgAddress*   pOtherAddr;

    if((hAddress == NULL) || (hOtherAddress == NULL))
        return RV_FALSE;

    pAddr = (MsgAddress*)hAddress;
    pOtherAddr = (MsgAddress*)hOtherAddress;

    if(pAddr->eAddrType != pOtherAddr->eAddrType)
    {
        return RV_FALSE;
    }

    switch(pAddr->eAddrType)
    {
    case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
    case RVSIP_ADDRTYPE_IM:
    case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
        return AddrUrlIsEqual(hAddress, hOtherAddress);
    case RVSIP_ADDRTYPE_ABS_URI:
        return AddrAbsUriIsEqual(hAddress, hOtherAddress);
#ifdef RV_SIP_TEL_URI_SUPPORT
    case RVSIP_ADDRTYPE_TEL:
        return AddrTelUriIsEqual(hAddress, hOtherAddress);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    case RVSIP_ADDRTYPE_DIAMETER:
        return AddrDiameterUriIsEqual(hAddress, hOtherAddress);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    default:
        return RV_FALSE;
    }
}


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
                                          IN RvSipAddressHandle hOtherAddress)
{
    MsgAddress*   pAddr;
    MsgAddress*   pOtherAddr;

    if((hAddress == NULL) || (hOtherAddress == NULL))
        return RV_FALSE;

    pAddr = (MsgAddress*)hAddress;
    pOtherAddr = (MsgAddress*)hOtherAddress;

    if(pAddr->eAddrType != pOtherAddr->eAddrType)
    {
        return RV_FALSE;
    }

    switch(pAddr->eAddrType)
    {
    case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
    case RVSIP_ADDRTYPE_IM:
    case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
        return AddrUrl3261IsEqual(hAddress, hOtherAddress);
    case RVSIP_ADDRTYPE_ABS_URI:
        return AddrAbsUriIsEqual(hAddress, hOtherAddress);
#ifdef RV_SIP_TEL_URI_SUPPORT
    case RVSIP_ADDRTYPE_TEL:
        return AddrTelUriIsEqual(hAddress, hOtherAddress);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
    default:
        return RV_FALSE;
    }
}

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
                                             IN RvSipAddressHandle hOtherHeader)
{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    return ((RvSipAddrUrlCompare(hHeader, hOtherHeader) == 0)? RV_TRUE : RV_FALSE);
#else
    return RvSipAddrIsEqual(hHeader, hOtherHeader);
#endif
/* SPIRENT_END */

}

/* SPIRENT_BEGIN */
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
									    	 IN RvSipAddressHandle hOtherHeader)
{
    MsgAddress*   pAddr;
    MsgAddress*   pOtherAddr;

    if (hHeader == NULL)
        return ((RvInt8) -1);
    else if (hOtherHeader == NULL)
        return ((RvInt8) 1);

    pAddr = (MsgAddress*)hHeader;
    pOtherAddr = (MsgAddress*)hOtherHeader;

    if(pAddr->eAddrType != pOtherAddr->eAddrType)
    {
        if (pAddr->eAddrType != RVSIP_ADDRTYPE_URL)
            return ((RvInt8) -1);
        else
            return ((RvInt8) 1);
    }
    else if(pAddr->eAddrType == RVSIP_ADDRTYPE_URL)
        return AddrUrlCompare(hHeader, hOtherHeader);
#ifndef EXPRESS_EXTRA_LEAN
    else if (pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
        return AddrAbsUriCompare(hHeader, hOtherHeader);
#endif /* EXPRESS_EXTRA_LEAN */
    else
        return ((RvInt8) -1);
}
#endif
/* SPIRENT_END */


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
RvInt32 SipAddrGetDisplayName(IN RvSipAddressHandle hAddr)
{
    MsgAddress* pAddr = (MsgAddress*)hAddr;

    return pAddr->strDisplayName;
}

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
                                               OUT RvUint*           actualLen)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
               "RvSipAddrGetDisplayName - Getting display-name of address 0x%p",hAddr));


    return MsgUtilsFillUserBuffer(pAddr->hPool,
                                  pAddr->hPage,
                                   SipAddrGetDisplayName(hAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                               IN  RvInt32            strDisplayNameOffset)
{
    RvInt32        newStrOffset;
    MsgAddress*    pAddr;
    RvStatus       retStatus;

    if(hAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hAddr;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage,
								  strDisplayNameOffset, strDisplayName,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrSetDisplayName - Failed to set display-name"));
        return retStatus;
    }
    pAddr->strDisplayName = newStrOffset;
#ifdef SIP_DEBUG
    pAddr->pDisplayName = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												 pAddr->strDisplayName);
#endif

    return RV_OK;
}

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
                                               IN RvChar*            strDisplayName)
{
    if((hAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    return SipAddrSetDisplayName(hAddr, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                  SIP URL - GET AND SET FUNCTIONS                      */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipAddrUrlGetUser
 * ------------------------------------------------------------------------
 * General:  This method return the user field from the URL object.
 * Return Value: user name or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetUser(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;

    if(hSipAddr == NULL)
        return UNDEFINED;

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strUser;
}

/***************************************************************************
 * RvSipAddrUrlGetUser
 * ------------------------------------------------------------------------
 * General:  Copies the user field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains
 *           the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL character at the end
 *                    of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetUser(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

/* SPIRENT_BEGIN -- if logSrc doesn't exist, the following line causes a crash --> comment it out! 
    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetUser - Getting user of address 0x%p",hSipAddr));
SPIRENT_END */

    if(IsUrlAddrType((MsgAddress*)hSipAddr) == RV_FALSE)
    {
/* SPIRENT_BEGIN -- if logSrc doesn't exist, the following line causes a crash --> comment it out! 
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetUser - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
SPIRENT_END */
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrUrlGetUser(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

/***************************************************************************
 * RvSipAddrUrlSetUser
 * ------------------------------------------------------------------------
 * General: Sets the user string value in the Address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the Address object.
 *    strUser  - The user value to be set in the object. If NULL is supplied, the existing strUser
 *             is removed from the Address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetUser(IN    RvSipAddressHandle hSipAddr,
                                               IN    RvChar*           strUser)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetUser -  illegal address type %d for this function",
            pAddr->eAddrType));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetUser(hSipAddr, strUser, NULL, NULL_PAGE, UNDEFINED);
    }

}

/***************************************************************************
 * SipAddrUrlGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the name of the host, from the URL object.
 * Return Value: host name or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetHost(IN RvSipAddressHandle hSipAddr)
{


    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strHost;
}

/***************************************************************************
 * RvSipAddrUrlGetHost
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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHost(IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType((MsgAddress*)hSipAddr)== RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetHost - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetHost(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}
/***************************************************************************
 * RvSipAddrUrlSetHost
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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHost(
                                           IN    RvSipAddressHandle hSipAddr,
                                           IN    RvChar*           strHost)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetHost -  illegal address type %d for this function",pAddr->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetHost(hSipAddr, strHost, NULL, NULL_PAGE, UNDEFINED);
    }

}

/***************************************************************************
 * RvSipAddrUrlGetPortNum
 * ------------------------------------------------------------------------
 * General:Gets the port number from the URL object.
 * Return Value: Returns the port number, or UNDEFINED if the port is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAddrUrlGetPortNum(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return UNDEFINED;

    if(IsUrlAddrType((MsgAddress*)hSipAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetPortNum - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return UNDEFINED;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->portNum;
}


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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetPortNum(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvInt32           portNum)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*  pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetPortNum - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        hUrl = pAddr->uAddrBody.hUrl;
        ((MsgAddrUrl*)hUrl)->portNum = portNum;
        return RV_OK;
    }
}

/***************************************************************************
 * RvSipAddrUrlGetTtlNum
 * ------------------------------------------------------------------------
 * General: Gets the ttl number from the URL object.
 * Return Value: Returns the ttl number, or UNDEFINED if ttl is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/

RVAPI RvInt16 RVCALLCONV RvSipAddrUrlGetTtlNum(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return UNDEFINED;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetTtlNum - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return UNDEFINED;
    }
    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;
    return ((MsgAddrUrl*)hUrl)->ttlNum;
}


/***************************************************************************
 * RvSipAddrUrlSetTtlNum
 * ------------------------------------------------------------------------
 * General:  Sets the ttl number in the address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    ttlNum   - The ttl number value to be set in the object. In order to remove the ttl number
 *             from the address object, you can set this parameter to UNDEFINED.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetTtlNum(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvInt16           ttlNum)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*  pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetTtlNum - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    if(ttlNum > 255)
    {
       RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                 "RvSipAddrUrlSetTtlNum - TTL must be below 256"));
       return RV_ERROR_BADPARAM;
    }

    hUrl = pAddr->uAddrBody.hUrl;
    ((MsgAddrUrl*)hUrl)->ttlNum = ttlNum;
    return RV_OK;
 }

/***************************************************************************
 * RvSipAddrUrlGetLrParam
 * ------------------------------------------------------------------------
 * General: Returns RV_TRUE if the lr param exists in the address.
 * Return Value: Returns RV_TRUE if the lr param exists in the address.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetLrParam(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_FALSE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetLrParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_FALSE;
    }


    hUrl = pAddr->uAddrBody.hUrl;

    if(RVSIP_URL_LR_TYPE_UNDEFINED == ((MsgAddrUrl*)hUrl)->lrParamType)
    {
        return RV_FALSE;
    }
    else
    {
        return RV_TRUE;
    }
}

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
                                              IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_URL_LR_TYPE_UNDEFINED;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetLrParamType - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RVSIP_URL_LR_TYPE_UNDEFINED;
    }


    hUrl = pAddr->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->lrParamType;
}

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
                                          IN    RvBool            bLrParam)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetLrParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    hUrl = pAddr->uAddrBody.hUrl;
    if(bLrParam == RV_TRUE)
    {
        ((MsgAddrUrl*)hUrl)->lrParamType = RVSIP_URL_LR_TYPE_EMPTY;
    }
    else
    {
        ((MsgAddrUrl*)hUrl)->lrParamType = RVSIP_URL_LR_TYPE_UNDEFINED;
    }
    return RV_OK;
}

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
                                          IN    RvSipUrlLrType     eLrParamType)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetLrParamType - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    hUrl = pAddr->uAddrBody.hUrl;
    ((MsgAddrUrl*)hUrl)->lrParamType = eLrParamType;
    return RV_OK;
}

/***************************************************************************
 * RvSipAddrUrlIsSecure
 * ------------------------------------------------------------------------
 * General: Defines if the url address scheme is secure or not.
 *          (does the url has "sips:" or "sip:" prefix)
 * Return Value: is secure.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlIsSecure(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
    {
        return RV_FALSE;
    }

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlIsSecure - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_FALSE;
    }

    hUrl = pAddr->uAddrBody.hUrl;
    return ((MsgAddrUrl*)hUrl)->bIsSecure;
}


/***************************************************************************
 * RvSipAddrUrlSetIsSecure
 * ------------------------------------------------------------------------
 * General:  Sets the secure flag in the address URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle to the URL address object.
 *    bIsSecure   - indicates the secure (e.g sips)/ not secured
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetIsSecure(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvBool             bIsSecure)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetIsSecure - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    hUrl = pAddr->uAddrBody.hUrl;
    ((MsgAddrUrl*)hUrl)->bIsSecure = bIsSecure;
    return RV_OK;
}

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
                                          IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_METHOD_UNDEFINED;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetMethod - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RVSIP_METHOD_UNDEFINED;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->eMethod;
}

/***************************************************************************
 * SipAddrUrlGetStrMethod
 * ------------------------------------------------------------------------
 * General: Gets the method string.
 *          If this function RvSipAddrUrlGetMethod returns RVSIP_METHOD_OTHER,
 *          call this function to get the actual method in a string.
 *          format.
 * Return Value: Returns the offset of the actual method from the URL address object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr   - Handle to the URL address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetStrMethod(IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;

    if(hSipAddr == NULL)
        return UNDEFINED;

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;
    return ((MsgAddrUrl*)hUrl)->strOtherMethod;
}

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
                                          OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    if(hSipAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(actualLen == NULL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrUrlGetStrMethod: bad pointer."));
        return RV_ERROR_NULLPTR;
    }
    *actualLen = 0;


    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrMethod - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(pAddr->hPool,
                                  pAddr->hPage,
                                  SipAddrUrlGetStrMethod(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipAddrUrlSetMethod
 * ------------------------------------------------------------------------
 * General:This method sets the method type in the URL address object. If eMethodType
 *         is RVSIP_METHOD_OTHER, the pMethodTypeStr will be copied to the header,
 *         otherwise it will be ignored.
 *         the API will call it with NULL pool
 *         and page, to make a real allocation and copy. internal modules
 *         (such as parser) will call directly to this function, with valid
 *         pool and page to avoid unneeded coping.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr      - Handle of the URL address object.
 *  eMethodType   - The method type to be set in the object.
 *    strMethodType - text string giving the method type to be set in the object.
 *                  This argument is needed when eMethodType is RVSIP_METHOD_OTHER.
 *                  otherwise it may be NULL.
 *  strMethodOffset - Offset of a string on the page  (if relevant).
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetMethod(IN    RvSipAddressHandle hSipAddr,
                             IN    RvSipMethodType    eMethodType,
                             IN    RvChar*           strMethodType,
                             IN    HRPOOL             hPool,
                             IN    HPAGE              hPage,
                             IN    RvInt32           strMethodOffset)
{
    RvInt32       newStr;
    MsgAddress*    pAddr;
    RvStatus      retStatus;
    MsgAddrUrl*    pUrl;

    if(hSipAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pAddr = (MsgAddress*)hSipAddr;
    pUrl  = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);

    pUrl->eMethod = eMethodType;

    if(eMethodType == RVSIP_METHOD_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strMethodOffset,
                                      strMethodType, pAddr->hPool,
                                      pAddr->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        pUrl->strOtherMethod = newStr;
#ifdef SIP_DEBUG
        pUrl->pOtherMethod = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                             pUrl->strOtherMethod);
#endif
    }
    else
    {
        pUrl->strOtherMethod = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pOtherMethod = NULL;
#endif
    }

    return RV_OK;
}



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
                                             IN    RvChar*           pMethodTypeStr)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    if(hSipAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetMethod - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return SipAddrUrlSetMethod(hSipAddr, eMethodType, pMethodTypeStr, NULL, NULL_PAGE, UNDEFINED);
}

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_TRANSPORT_UNDEFINED;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetTransport - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RVSIP_TRANSPORT_UNDEFINED;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->eTransport;
}

/***************************************************************************
 * SipAddrUrlGetStrTransport
 * ------------------------------------------------------------------------
 * General: This method returns the transport protocol string value from the
 *          MsgAddrUrl object.
 * Return Value: String of transportType
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strTransport;
}

/***************************************************************************
 * RvSipAddrUrlGetStrTransport
 * ------------------------------------------------------------------------
 * General: Copies the transport protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrUrlGetTransport() returns
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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrTransport - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetStrTransport(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


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
                                            IN    RvChar*           strTransport)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetTransport - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetTransport(hSipAddr, eTransport, strTransport, NULL, NULL_PAGE, UNDEFINED,UNDEFINED);
    }
}

/***************************************************************************
 * SipAddrUrlGetMaddrParam
 * ------------------------------------------------------------------------
 * General: This method is used to get the Maddr parameter value.
 * Return Value: string provides the server address to be contacted for this user
 *                 or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetMaddrParam(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strMaddrParam;
}
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
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetMaddrParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetMaddrParam(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetMaddrParam(
                                         IN    RvSipAddressHandle hSipAddr,
                                         IN    RvChar*           strMaddrParam)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetMaddrParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetMaddrParam(hSipAddr, strMaddrParam, NULL, NULL_PAGE, UNDEFINED);
    }
}


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
                                            IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_USERPARAM_UNDEFINED;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetUserParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RVSIP_USERPARAM_UNDEFINED;
    }

    hUrl = pAddr->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->eUserParam;
}

/***************************************************************************
 * SipAddrUrlGetUserParam
 * ------------------------------------------------------------------------
 * General:This method returns the user param string value from the
 *           MsgAddrUrl object.
 * Return Value: string with the user param.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetStrUserParam(IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strUserParam;
}

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
                                           OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrUserParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetStrUserParam(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetUserParam(
                                            IN RvSipAddressHandle hSipAddr,
                                            IN    RvSipUserParam     eUserParam,
                                            IN    RvChar*           strUserParam)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetUserParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetUserParam(hSipAddr, eUserParam, strUserParam, NULL, NULL_PAGE, UNDEFINED);
    }

}

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
RvInt32 SipAddrUrlGetTokenizedByParam(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strTokenizedByParam;
}

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
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetTokenizedByParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetTokenizedByParam(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
                                         IN    RvChar*				strTokenizedBy)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetTokenizedByParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetTokenizedByParam(hSipAddr, strTokenizedBy, NULL, NULL_PAGE, UNDEFINED);
    }
}

/***************************************************************************
 * RvSipAddrUrlGetOrigParam
 * ------------------------------------------------------------------------
 * General:Gets the Orig param from the URL object.
 * Return Value: Boolean value indicating whether or not the orig parameter exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetOrigParam(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_FALSE;

    if(IsUrlAddrType((MsgAddress*)hSipAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetOrigParam - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_FALSE;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->bOrigParam;
}


/***************************************************************************
 * RvSipAddrUrlSetOrigParam
 * ------------------------------------------------------------------------
 * General:  Sets the OrigParam in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 *    bOrigParam - The OrigParam value to be set in the object. In order to remove the 
 *              OrigParam from the address object, you can set this parameter to RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetOrigParam(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvBool           bOrigParam)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*  pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetOrigParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        hUrl = pAddr->uAddrBody.hUrl;
        ((MsgAddrUrl*)hUrl)->bOrigParam = bOrigParam;
        return RV_OK;
    }
}

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
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetOldAddrSpec(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
    {
        return RV_FALSE;
    }

    if(IsUrlAddrType((MsgAddress*)hSipAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetOldAddrSpec - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_FALSE;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->bOldAddrSpec;
}

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
                                          IN    RvBool              bOldAddrSpec)
{
    MsgAddrUrlHandle  hUrl;
    MsgAddress*  pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetOldAddrSpec - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        hUrl = pAddr->uAddrBody.hUrl;
        ((MsgAddrUrl*)hUrl)->bOldAddrSpec = bOldAddrSpec;
        return RV_OK;
    }
}

/***************************************************************************
 * SipAddrUrlGetUrlParams
 * ------------------------------------------------------------------------
 * General: This method is used to get the UrlParams parameter value.
 * Return Value: string provides the headers to be contacted for this user
 *                 or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetUrlParams(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strUrlParams;
}

/***************************************************************************
 * RvSipAddrUrlGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the URL other params field of the Address object into a given buffer.
 *          Not all the address parameters have separated fields in the address object.
 *          Parameters with no specific fields are referred to as other params. They are kept
 *          in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetOtherParams(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetOtherParams - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetUrlParams(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipAddrUrlSetOtherParams
 * ------------------------------------------------------------------------
 * General: Sets the other params field in the address object.
 *          Not all the address parameters have separated fields in the address object.
 *          Parameters with no specific fields are referred to as other params. They are kept
 *          in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle to the URL address object.
 *    strUrlParams  - The extended parameters field to be set in the Address object. If NULL is
 *                  supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetOtherParams(
                                        IN    RvSipAddressHandle hSipAddr,
                                        IN    RvChar*           strUrlParams)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetOtherParams - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetUrlParams(hSipAddr, strUrlParams, NULL, NULL_PAGE, UNDEFINED);
    }

}


/***************************************************************************
 * SipUrlGetHeaders
 * ------------------------------------------------------------------------
 * General: This method is used to get the headers parameter value.
 * Return Value: string provides the headers to be contacted for this user
 *                 or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrUrlGetHeaders(RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strHeaders;
}

/***************************************************************************
 * RvSipAddrUrlGetHeaders
 * ------------------------------------------------------------------------
 * General: Copies the headers field of the Address object into a given buffer.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, it returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen param contains
 *          the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr   - Handle to the URL address object.
 *        strBuffer  - Buffer to include the requested parameter.
 *        bufferLen  - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include a NULL value at the end
 *                     of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHeaders(
                                               IN RvSipAddressHandle hSipAddr,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetHeaders - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetHeaders(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * RvSipAddrUrlSetHeaders
 * ------------------------------------------------------------------------
 * General: Used to set the headers parameter in the URL address object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl    - Handle to the URL address object.
 *    strHeaders - The headers parameter to be set in the object. If NULL is supplied, the existing
 *               headers parameter is removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHeaders(
                                              IN    RvSipAddressHandle hSipAddr,
                                              IN    RvChar*           strHeaders)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetHeaders - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetHeaders(hSipAddr, strHeaders, NULL, NULL_PAGE, UNDEFINED);
    }

}

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
    {
        return RVSIP_COMP_UNDEFINED;
    }

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetCompParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RVSIP_COMP_UNDEFINED;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->eComp;
}

/***************************************************************************
 * SipAddrUrlGetStrCompParam
 * ------------------------------------------------------------------------
 * General: This method returns the compression type string value from the
 *          MsgAddrUrl object.
 * Return Value: String of compression Type
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrUrlGetStrCompParam(
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strCompParam;
}

/***************************************************************************
 * RvSipAddrUrlGetStrCompParam
 * ------------------------------------------------------------------------
 * General: Copies the compression type string from the URL address object into a given
 *          buffer. Use this function if RvSipAddrUrlGetCompParam() returns
 *          RVSIP_COMP_OTHER.
 *          If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr     - Handle to the URL address object.
 *          strBuffer    - Buffer to include the requested parameter.
 *          bufferLen    - The length of the given buffer.
 *  output: actualLen   - The length of the requested parameter + 1, to include
 *                         NULL character at the end of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetStrCompParam(
                                           IN  RvSipAddressHandle hSipAddr,
                                           IN  RvChar*           strBuffer,
                                           IN  RvUint            bufferLen,
                                           OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    RV_UNUSED_ARG(hSipAddr);
    RV_UNUSED_ARG(strBuffer);
    RV_UNUSED_ARG(bufferLen);
    if(actualLen == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrCompParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetStrCompParam(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


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
                                            IN    RvSipCompType      eComp,
                                            IN    RvChar*            strCompParam)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetCompParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetCompParam(hSipAddr, eComp, strCompParam, NULL, NULL_PAGE, UNDEFINED);
    }
}


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

/***************************************************************************
 * RvSipAddrUrlIsCompParam
 * ------------------------------------------------------------------------
 * General:  Check if the compression type is Sigcomp.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * INPUT   hSipAddr     - Handle to the URL address object.
 * OUTPUT  1: Sigcomp type, 0: not sigcomp type
 ***************************************************************************/
RVAPI int RVCALLCONV RvSipAddrUrlIsCompParam(IN    RvSipAddressHandle hSipAddr)
{
    MsgAddress *pAddr = (MsgAddress*)hSipAddr;
    if(hSipAddr == NULL)
        return 0;    
    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetCompParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return 0;
    }
    else
        return SipAddrUrlIsCompParam(hSipAddr);
    
}

/***************************************************************************
 * RvSipAddrUrlGetSigCompIdParamOffset
 * ------------------------------------------------------------------------
 * General: This method returns the sigcomp-id string value from the
 *          MsgAddrUrl object.
 * Return Value: String of sigcomp-id
 * ------------------------------------------------------------------------
 * Arguments:
 * INPUT:  hSipAddr - Handle of the url address object.
 * OUTPUT: Offset on of the URN on the PAGE
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAddrUrlGetSigCompIdParamOffset(
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strSigCompIdParam;
}

RVAPI RvStatus RvSipAddrUrlSetSigCompIdParamOverWrite(IN  RvSipAddressHandle hSipAddr,
                                 IN  HRPOOL            sourcePool,
                                 IN  HPAGE             sourcePage,
                                 IN  RvInt32           strSigCompIdOffset)
{
   
   return SipAddrUrlSetSigCompIdParamOverWrite(hSipAddr,sourcePool, sourcePage, strSigCompIdOffset);
}
/***************************************************************************
 * SipAddrUrlRpSigCompIdParam
 * ------------------------------------------------------------------------
 * General: This method sets the sigcomp-id parameter value in the MsgAddrUrl
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hSipAddr      - Handle of the address object.
 *         strSigCompIdParam - String of URN Scheme
 *  
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlRpSigCompIdParam(IN  RvSipAddressHandle hSipAddr,
														   IN    RvChar*            strSigCompIdParam,
														   int size)
{
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    MsgAddrUrl*       pUrl;
    MsgAddrUrlHandle  hUrl; 
	RvChar* purn = NULL;
    if(hSipAddr == NULL)
	   return RV_ERROR_INVALID_HANDLE;	

    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;

	purn = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
		pUrl->strSigCompIdParam);
	if(purn)
       strcpy(purn, strSigCompIdParam);
    return RV_OK;

}

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strSigCompIdParam;
}

/***************************************************************************
 * RvSipAddrUrlGetSigCompIdParam
 * ------------------------------------------------------------------------
 * General: Copies the sigcompid parameter from the address object into a given buffer. If the
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
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetSigCompIdParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetSigCompIdParam(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
                                            IN    RvChar*            strSigCompIdParam)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetSigCompIdParam - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetSigCompIdParam(hSipAddr, strSigCompIdParam, NULL, NULL_PAGE, UNDEFINED);
    }
}

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
 *  hSipAddr        - Handle to the address object.
 *    hHeadersList    - A handle to the Address object.
 *    pHeadersBuffer  - Buffer containing a textual url headers parameter string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlGetHeadersList(
                                     IN    RvSipAddressHandle    hSipAddr,
                                     IN    RvSipCommonListHandle hHeadersList,
                                     IN    RvChar*              pHeadersBuffer)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if((pAddr == NULL) || (hHeadersList == NULL) || (pHeadersBuffer == NULL))
        return RV_ERROR_BADPARAM;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                   "RvSipAddrUrlGetHeadersList - eAddrType is %d. not URL!!!",
                   pAddr->eAddrType));
            return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "RvSipAddrUrlGetHeadersList: Parsing url headers. hSipAddr 0x%p, list 0x%p",
             hSipAddr, hHeadersList));

    return AddrUrlParseHeaders(pAddr, hHeadersList, pHeadersBuffer);
}

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
 *  hSipAddr        - Handle to the address object.
 *    hHeadersList    - A handle to the headers list object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrUrlSetHeadersList(
                                     IN    RvSipAddressHandle    hSipAddr,
                                     IN    RvSipCommonListHandle hHeadersList)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if((pAddr == NULL) || (hHeadersList == NULL))
        return RV_ERROR_BADPARAM;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                   "RvSipAddrUrlSetHeadersList - eAddrType is %d. not URL!!!",
                   pAddr->eAddrType));
            return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "RvSipAddrUrlSetHeadersList: encoding url headers list. hSipAddr 0x%p, list 0x%p",
             hSipAddr, hHeadersList));

    return AddrUrlEncodeHeadersList(pAddr, hHeadersList);
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrl* pUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_CPC_TYPE_UNDEFINED;

    /* Make sure the address type is of AddrUrl */
    if(IsUrlAddrType((MsgAddress*)hSipAddr)== RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetCPC - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RVSIP_CPC_TYPE_UNDEFINED;
    }

    pUrl = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    return pUrl->eCPCType;
}

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
                                           OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of AddrUrl */
    if(IsUrlAddrType((MsgAddress*)hSipAddr)== RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrCPC - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_ERROR_BADPARAM;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pAddr);
#endif

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetStrCPC(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
RvInt32 RVCALLCONV SipAddrUrlGetStrCPC(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strCPC;
}

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
                                            IN    RvChar*           strCPC)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    /* Make sure the address type is of AddrUrl */
    if(IsUrlAddrType((MsgAddress*)hSipAddr)== RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlSetCPC - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_ERROR_BADPARAM;
    }
    
    return SipAddrUrlSetCPC(hSipAddr, eCPC, strCPC, NULL, NULL_PAGE, UNDEFINED);
}

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
RvInt32 SipAddrUrlGetStrGr(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->strGrParam;
}

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
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetStrGr - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrUrlGetStrGr(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipAddrUrlGetGr
 * ------------------------------------------------------------------------
 * General:Gets the Gr param from the URL object.
 * Return Value: Returns the Gr param. RV_FALSE if missing.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipAddrUrlGetGr(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrUrlHandle hUrl;
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_FALSE;

    if(IsUrlAddrType((MsgAddress*)hSipAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrUrlGetGr - illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));

        return RV_FALSE;
    }

    hUrl = ((MsgAddress*)hSipAddr)->uAddrBody.hUrl;

    return ((MsgAddrUrl*)hUrl)->bGrParam;
}

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
                                         IN    RvChar*				strGr)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(IsUrlAddrType(pAddr) == RV_FALSE)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlSetGr - Illegal address type %d for this function",((MsgAddress*)hSipAddr)->eAddrType ));
        return RV_ERROR_BADPARAM;
    }
    else
    {
        return SipAddrUrlSetGr(hSipAddr, bGrActive, strGr, NULL, NULL_PAGE, UNDEFINED);
    }
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                  ABS URI - GET AND SET FUNCTIONS                      */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipAddrAbsUriGetScheme
 * ------------------------------------------------------------------------
 * General:  This method return the scheme field from the Address object.
 * Return Value: scheme or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrAbsUriGetScheme(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrAbsUri *pAbsUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pAbsUri = ((MsgAddress*)hSipAddr)->uAddrBody.pAbsUri;

    return ((MsgAddrAbsUri*)pAbsUri)->strScheme;
}

/***************************************************************************
 * RvSipAddrAbsUriGetScheme
 * ------------------------------------------------------------------------
 * General:  Copies the scheme field from the address object into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter into the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains
 *           the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the given buffer.
 * output:actualLen - The length of the requested parameter + 1, to include NULL character at the end
 *                    of the string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrAbsUriGetScheme(IN RvSipAddressHandle hSipAddr,
                                                    IN RvChar*           strBuffer,
                                                    IN RvUint            bufferLen,
                                                    OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_ABS_URI)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrAbsUriGetScheme - The given address is not an ABS URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrAbsUriGetScheme(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                                    IN    RvChar*           strScheme)
{
    MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
        return SipAddrAbsUriSetScheme(hSipAddr, strScheme, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrAbsUriSetScheme - The given address is not an ABS URI"));
        return RV_ERROR_BADPARAM;
    }
}


/***************************************************************************
 * SipAddrAbsUriGetIdentifier
 * ------------------------------------------------------------------------
 * General:  This method return the identifier field from the Address object.
 * Return Value: Identifier or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrAbsUriGetIdentifier(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrAbsUri *pAbsUri;

    if(hSipAddr == NULL)
        return UNDEFINED;
    pAbsUri = ((MsgAddress*)hSipAddr)->uAddrBody.pAbsUri;

    return ((MsgAddrAbsUri*)pAbsUri)->strIdentifier;
}

/***************************************************************************
 * RvSipAddrAbsUriGetIdentifier
 * ------------------------------------------------------------------------
 * General:  Copies the Identifier field from the absolute URI address object
 *           into a given buffer.
 *           If the bufferLen is adequate, the function copies the parameter into
 *           the strBuffer.
 *           Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains
 *           the required buffer length.
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
                                                        OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_ABS_URI)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrAbsUriGetIdentifier - The given address is not an ABS URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrAbsUriGetIdentifier(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}


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
                                                        IN    RvChar*           strIdentifier)
{
    MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_ABS_URI)
        return SipAddrAbsUriSetIdentifier(hSipAddr, strIdentifier, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrAbsUriSetIdentifier - The given address is not an ABS URI"));
        return RV_ERROR_BADPARAM;
    }
}

#ifdef RV_SIP_JSR32_SUPPORT
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
												IN  RvChar             *strAddr)
{
	MsgMgr*            pMsgMgr   = (MsgMgr*)hMsgMgr;
	SipParserMgrHandle hParser;	
	RvSipAddressType   eAddrType;
	RvStatus           rv;

    if (strAddr == NULL || pMsgMgr == NULL)
    {
        return RVSIP_ADDRTYPE_UNDEFINED;
    }

	hParser = pMsgMgr->hParserMgr;	
	
	/* Parse the address only to determine its type (there is no object yet) */
    rv = MsgParserParseStandAloneAddress(pMsgMgr, 
                                          SIP_PARSETYPE_ANYADDRESS, 
                                          strAddr, 
                                          (void*)&eAddrType);
	if (RV_OK != rv)
	{
		return RVSIP_ADDRTYPE_UNDEFINED;
	}

	return eAddrType;
}
#else /* #ifdef RV_SIP_JSR32_SUPPORT */ 
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
												IN  RvChar             *strAddr)
{
	MsgMgr*            pMsgMgr   = (MsgMgr*)hMsgMgr;

    if (strAddr == NULL || pMsgMgr == NULL)
    {
        return RVSIP_ADDRTYPE_UNDEFINED;
    }

	return SipAddrGetAddressTypeFromString(hMsgMgr,strAddr);
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */ 

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
												IN  RvChar             *strAddr)
{
#ifdef RV_SIP_JSR32_SUPPORT
	MsgMgr*            pMsgMgr   = (MsgMgr*)hMsgMgr;
	SipParserMgrHandle hParser;	
	RvSipAddressType   eAddrType;
	RvStatus           rv;
#else /* RV_SIP_JSR32_SUPPORT */
	RvUint32         i = 0;
    RvUint32         strLen = 0;
#endif /* RV_SIP_JSR32_SUPPORT */

    if (strAddr == NULL)
    {
        return RVSIP_ADDRTYPE_UNDEFINED;
    }

#ifdef RV_SIP_JSR32_SUPPORT

	if (pMsgMgr == NULL)
	{
		return RVSIP_ADDRTYPE_UNDEFINED;
	}

	hParser = pMsgMgr->hParserMgr;	
	/* Parse the address only to determine its type (there is no object yet) */
    rv = MsgParserParseStandAloneAddress(pMsgMgr, 
                                          SIP_PARSETYPE_ANYADDRESS, 
                                          strAddr, 
                                          (void*)&eAddrType);
	if (RV_OK != rv)
	{
		return RVSIP_ADDRTYPE_UNDEFINED;
	}
	return eAddrType;

#else /* #ifdef RV_SIP_JSR32_SUPPORT */

    strLen = (RvUint32)strlen(strAddr);
    /* passing over the spaces */
    while (i < strLen && strAddr[i] == ' ')
    {
        ++i;
    }

    /* passing over '<' and spaces */
    if (strAddr[i] == '<')
    {
        ++i;
        while (i < strLen && strAddr[i] == ' ')
        {
            ++i;
        }
    }

#ifdef RV_SIP_OTHER_URI_SUPPORT
    if(i+2 < strLen)
    {
        /* looking for "im:"  */
        if ((strAddr[i] == 'i' || strAddr[i] == 'I') &&
            (strAddr[i + 1] == 'm' || strAddr[i + 1] == 'M') &&
            (strAddr[i + 2] == ':'))
        {
            return RVSIP_ADDRTYPE_IM;
        }
    }
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
    if (i + 3 < strLen)
    {
        /* looking for "sip:" or "SIP:" */
		if ((strAddr[i] == 's' || strAddr[i] == 'S') &&
            (strAddr[i + 1] == 'i' || strAddr[i + 1] == 'I') &&
            (strAddr[i + 2] == 'p' || strAddr[i + 2] == 'P') &&
            (strAddr[i + 3] == ':'))
        {
            return RVSIP_ADDRTYPE_URL;
        }
#ifdef RV_SIP_TEL_URI_SUPPORT
		/* looking for "tel:" (case insensitive) */
		if ((strAddr[i] == 't' || strAddr[i] == 'T') &&
            (strAddr[i + 1] == 'e' || strAddr[i + 1] == 'E') &&
            (strAddr[i + 2] == 'l' || strAddr[i + 2] == 'L') &&
            (strAddr[i + 3] == ':'))
        {
            return RVSIP_ADDRTYPE_TEL;
        }
#endif /*#ifdef RV_SIP_TEL_URI_SUPPORT*/
    }
    /* looking for "sips:" or "SIPS:" */
    if (i + 4 < strLen)
    {
        if ((strAddr[i] == 's' || strAddr[i] == 'S') &&
            (strAddr[i + 1] == 'i' || strAddr[i + 1] == 'I') &&
            (strAddr[i + 2] == 'p' || strAddr[i + 2] == 'P') &&
            (strAddr[i + 3] == 's' || strAddr[i + 3] == 'S') &&
            (strAddr[i + 4] == ':'))
        {
            return RVSIP_ADDRTYPE_URL;
        }
#ifdef RV_SIP_OTHER_URI_SUPPORT
        /* looking for "PRES:" */
        if ((strAddr[i] == 'p' || strAddr[i] == 'P') &&
            (strAddr[i + 1] == 'r' || strAddr[i + 1] == 'R') &&
            (strAddr[i + 2] == 'e' || strAddr[i + 2] == 'E') &&
            (strAddr[i + 3] == 's' || strAddr[i + 3] == 'S') &&
            (strAddr[i + 4] == ':'))
        {
            return RVSIP_ADDRTYPE_PRES;
        }
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
    }


   /* looking for ':' to know if the type is Abs-Uri */
    while (i < strLen && strAddr[i] != ':')
    {
        ++i;
    }
    /* ':' was found and there is identifier after it */
    if (i + 1 < strLen)
    {
            return RVSIP_ADDRTYPE_ABS_URI;
    }

    RV_UNUSED_ARG(hMsgMgr)
    return RVSIP_ADDRTYPE_UNDEFINED;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */
}

/*-----------------------------------------------------------------------*/
/*                  TEL URI - GET AND SET FUNCTIONS                      */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_TEL_URI_SUPPORT

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
												IN RvSipAddressHandle hSipAddr)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_FALSE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetIsGlobalPhoneNumber - The given address is not TEL URL"));
        return RV_FALSE;
    }


    return pAddr->uAddrBody.pTelUri->bIsGlobalPhoneNumber;
}

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
                                  IN    RvBool             bIsGlobalPhoneNumber)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriSetIsGlobalPhoneNumber - The given address is not TEL URL"));
        return RV_ERROR_BADPARAM;
    }

    pAddr->uAddrBody.pTelUri->bIsGlobalPhoneNumber = bIsGlobalPhoneNumber;

    return RV_OK;
}
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
                                               IN RV_CHAR*           strBuffer)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

	if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

	if(pAddr->eAddrType != RVSIP_ADDRTYPE_URL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrUrlReplaceUser - The given address is not a SIP URL"));
        return RV_ERROR_BADPARAM;

    }

    return RPOOL_CopyFromExternal(pAddr->hPool, pAddr->hPage, SipAddrUrlGetUser(hSipAddr), strBuffer, strlen(strBuffer) + 1);
}

/***************************************************************************
 * RvSipAddrFree
 * ------------------------------------------------------------------------
 * General:  Free memory for the address.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrFree(IN RvSipAddressHandle hSipAddr)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    RPOOL_FreePage( pAddr->hPool, pAddr->hPage );
    //printf( "%s: hPool = %x, hPage = %x\n", __FUNCTION__, pAddr->hPool, pAddr->hPage );

    return RV_Success;
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

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
                                            IN    RvChar*            strPhoneNumber)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetPhoneNumber(hSipAddr, strPhoneNumber, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetPhoneNumber - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetPhoneNumber - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetPhoneNumber(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strExtension)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetExtension(hSipAddr, strExtension, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetExtension - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetExtension - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetExtension(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strIsdnSubAddr)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetIsdnSubAddr(hSipAddr, strIsdnSubAddr, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetIsdnSubAddr - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetIsdnSubAddr - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetIsdnSubAddr(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strPostDial)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetPostDial(hSipAddr, strPostDial, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetPostDial - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetPostDial - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetPostDial(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strContext)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetContext(hSipAddr, strContext, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetContext(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                          IN    RvSipTelUriEnumdiType   eEnumdiType)
{
    MsgAddrTelUri*  pTelUri;
	MsgAddress*     pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriSetEnumdiParamType - The given address is not a TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    pTelUri = pAddr->uAddrBody.pTelUri;

    pTelUri->eEnumdiType = eEnumdiType;

    return RV_OK;
}

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
                                              IN RvSipAddressHandle hSipAddr)
{
    MsgAddress*     pAddr = (MsgAddress*)hSipAddr;
	MsgAddrTelUri*  pTelUri;

    if(hSipAddr == NULL)
        return RVSIP_ENUMDI_TYPE_UNDEFINED;/*NANA*/

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetEnumdiParamType - The given address is not a TEL URI"));
        return RVSIP_ENUMDI_TYPE_UNDEFINED;/*NANA*/
    }


    pTelUri = pAddr->uAddrBody.pTelUri;

    return pTelUri->eEnumdiType;
}

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
                                            IN    RvChar*            strOtherParams)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetOtherParams(hSipAddr, strOtherParams, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetOtherParams - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetOtherParams - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetOtherParams(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}
#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * RvSipAddrTelUriGetCPC
 * ------------------------------------------------------------------------
 * General: Gets the CPC enumeration value from the URL object. If the
 *          returned CPC enumeration is RVSIP_CPC_TYPE_OTHER, you can get
 *          the CPC protocol string by calling RvSipTelUriGetStrCPC().
 *          If this function returns RVSIP_CPC_TYPE_OTHER, call
 *          RvSipTelUriGetStrCPC().
 * Return Value: Returns the CPCType enumeration value.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle to the URL address object.
 ***************************************************************************/
RVAPI RvSipUriCPCType RVCALLCONV RvSipAddrTelUriGetCPC(
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri* pUrl;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_CPC_TYPE_UNDEFINED;

    /* Make sure the address type is of TelUri */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetCPC - The given address is not TEL URI"));
        return RVSIP_CPC_TYPE_UNDEFINED;
    }

    pUrl = (MsgAddrTelUri*)pAddr->uAddrBody.hUrl;

    return pUrl->eCPCType;
}

/***************************************************************************
 * RvSipAddrTelUriGetStrCPC
 * ------------------------------------------------------------------------
 * General: Copies the CPC protocol string from the URL address object into a given
 *          buffer. Use this function if RvSipTelUriGetCPC() returns
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
                                           OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of TelUri */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetStrCPC - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrTelUriGetStrCPC(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
                                            IN    RvChar*           strCPC)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    /* Make sure the address type is of TelUri */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriSetCPC - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
    
    return SipAddrTelUriSetCPC(hSipAddr, eCPC, strCPC, NULL, NULL_PAGE, UNDEFINED);
}

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
                                            IN    RvChar*            strRn)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetRn(hSipAddr, strRn, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetRn - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetRn - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetRn(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strRnContext)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetRnContext(hSipAddr, strRnContext, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetRnContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetRnContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetRnContext(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strCic)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetCic(hSipAddr, strCic, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetCic - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetCic - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetCic(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
                                            IN    RvChar*            strCicContext)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_TEL)
        return SipAddrTelUriSetCicContext(hSipAddr, strCicContext, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrTelUriSetCicContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetCicContext - The given address is not TEL URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrTelUriGetCicContext(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

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
												IN RvSipAddressHandle hSipAddr)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_FALSE;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriGetNpdi - The given address is not TEL URL"));
        return RV_FALSE;
    }

    return pAddr->uAddrBody.pTelUri->bNpdi;
}

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
                                  IN    RvBool             bNpdi)
{
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_TEL)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrTelUriSetNpdi - The given address is not TEL URL"));
        return RV_ERROR_BADPARAM;
    }

    pAddr->uAddrBody.pTelUri->bNpdi = bNpdi;

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
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
RVAPI RvBool RVCALLCONV RvSipAddrDiameterUriIsSecure(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress*         pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
    {
        return RV_FALSE;
    }

    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriIsSecure - The given address is not a DIAMETER URI"));
        return RV_FALSE;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;
    return pDiameterUri->bIsSecure;
}


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
                                          IN    RvBool             bIsSecure)
{
    MsgAddrDiameterUri*  pDiameterUri;
    MsgAddress*   pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriSetIsSecure - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;
    pDiameterUri->bIsSecure = bIsSecure;
    return RV_OK;
}

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
                                          IN    RvSipDiameterProtocol   eProtocolType)
{
    MsgAddrDiameterUri* pDiameterUri;
	MsgAddress*         pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriSetProtocolType - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;
    pDiameterUri->eProtocol = eProtocolType;

    return RV_OK;
}

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
                                              IN RvSipAddressHandle hSipAddr)
{
    MsgAddress*         pAddr = (MsgAddress*)hSipAddr;
	MsgAddrDiameterUri* pDiameterUri;

    if(hSipAddr == NULL)
        return RVSIP_DIAMETER_PROTOCOL_UNDEFINED;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetProtocolType - The given address is not a DIAMETER URI"));
        return RVSIP_DIAMETER_PROTOCOL_UNDEFINED;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;
    return pDiameterUri->eProtocol;
}

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
                                               OUT RvUint*          actualLen)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetHost - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrDiameterUriGetHost(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

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
                                           IN    RvChar*           strHost)
{
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriSetHost - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    return SipAddrDiameterUriSetHost(hSipAddr, strHost, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipAddrDiameterUriGetPortNum
 * ------------------------------------------------------------------------
 * General:Gets the port number from the DiameterUri object.
 * Return Value: Returns the port number, or UNDEFINED if the port is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the DiameterUri address object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipAddrDiameterUriGetPortNum(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress* pAddr;

    pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return UNDEFINED;
    
    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetPortNum - The given address is not a DIAMETER URI"));
        return UNDEFINED;
    }

    pDiameterUri = ((MsgAddress*)hSipAddr)->uAddrBody.pDiameterUri;
    return pDiameterUri->portNum;
}


/***************************************************************************
 * RvSipAddrDiameterUriSetPortNum
 * ------------------------------------------------------------------------
 * General:  Sets the port number in the URL object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle to the URL address object.
 *    portNum - The port number value to be set in the object. In order to remove the port
 *            number from the address object, you can set this parameter to UNDEFINED.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetPortNum(
                                          IN    RvSipAddressHandle hSipAddr,
                                          IN    RvInt32           portNum)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress*         pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriSetPortNum - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;
    pDiameterUri->portNum = portNum;

    return RV_OK;
}

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(hSipAddr == NULL)
        return RVSIP_TRANSPORT_UNDEFINED;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetTransport - The given address is not a DIAMETER URI"));
        return RVSIP_TRANSPORT_UNDEFINED;
    }

    pDiameterUri = pAddr->uAddrBody.pDiameterUri;

    return pDiameterUri->eTransport;
}

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
                                           OUT RvUint*          actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetStrTransport - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                  SipAddrDiameterUriGetStrTransport(hSipAddr),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipAddrDiameterUriSetTransport
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
RVAPI RvStatus RVCALLCONV RvSipAddrDiameterUriSetTransport(
                                            IN    RvSipAddressHandle hSipAddr,
                                            IN    RvSipTransport     eTransport,
                                            IN    RvChar*           strTransport)
{
    MsgAddress *pAddr;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriSetTransport - The given address is not a DIAMETER URI"));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    return SipAddrDiameterUriSetTransport(hSipAddr, eTransport, strTransport, NULL, NULL_PAGE, UNDEFINED);
}

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
                                            IN    RvChar*            strOtherParams)
{
	MsgAddress*   pAddr;

    if((hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    if(pAddr->eAddrType == RVSIP_ADDRTYPE_DIAMETER)
        return SipAddrDiameterUriSetOtherParams(hSipAddr, strOtherParams, NULL, NULL_PAGE, UNDEFINED);

    else
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "RvSipAddrDiameterUriSetOtherParams - The given address is not DIAMETER URI"));
        return RV_ERROR_BADPARAM;
    }
}

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
                                                OUT RvUint*           actualLen)
{
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    /* Make sure the address type is of DiameterURI */
    if(pAddr->eAddrType != RVSIP_ADDRTYPE_DIAMETER)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "RvSipAddrDiameterUriGetOtherParams - The given address is not DIAMETER URI"));
        return RV_ERROR_BADPARAM;
    }

    return MsgUtilsFillUserBuffer(((MsgAddress*)hSipAddr)->hPool,
                                  ((MsgAddress*)hSipAddr)->hPage,
                                   SipAddrDiameterUriGetOtherParams(hSipAddr),
                                   strBuffer,
                                   bufferLen,
                                   actualLen);
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * IsUrlAddrType
 * ------------------------------------------------------------------------
 * General: Checks if the address type is url/im/pres
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 ***************************************************************************/
static RvBool IsUrlAddrType(MsgAddress* pAddr)
{
    switch(pAddr->eAddrType)
    {
        case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
        case RVSIP_ADDRTYPE_IM:
        case RVSIP_ADDRTYPE_PRES:
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
            return RV_TRUE;
        default:
            return RV_FALSE;
    }
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * EncodeDisplayName
 * ------------------------------------------------------------------------
 * General: Encodes the display name of an address object on a given pool and page
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pAddr    - Pointer to the address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * Inout: length   - The given length + the encoded address length.
 ***************************************************************************/
static RvStatus RVCALLCONV  EncodeDisplayName(
								IN    MsgAddress*        pAddr,
								IN    HRPOOL             hPool,
								IN    HPAGE              hPage,
								INOUT RvUint32*          length)
{
	RvStatus    stat;

	/* encode display name */
    if(pAddr->strDisplayName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pAddr->strDisplayName,
									length);
		if(stat != RV_OK)
		{
			RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
						"EncodeDisplayName - Failed to encode display name. address=%x",
						pAddr));
            return stat;
		}

    }

	return RV_OK;
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * EncodeBracket
 * ------------------------------------------------------------------------
 * General: Encodes '<' or '>' sign
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pAddr      - Pointer to the address object.
 *        bIsOpening - RV_TRUE to encode '<'. RV_FALSE to encode '>'
 *        hPool      - Handle of the pool of pages
 *        hPage      - Handle of the page that will contain the encoded message.
 * Inout: length     - The given length + the encoded address length.
 ***************************************************************************/
static RvStatus RVCALLCONV  EncodeBracket(
								IN  MsgAddress*        pAddr,
								IN  RvBool             bIsOpening,
								IN  HRPOOL             hPool,
								IN  HPAGE              hPage,
								OUT RvUint32*          length)
{
	RvChar   *strToEncode;
	RvStatus  stat = RV_OK;

    if (bIsOpening == RV_TRUE)
	{
		strToEncode = MsgUtilsGetLeftAngleChar(RV_FALSE);
	}
	else
	{
		strToEncode = MsgUtilsGetRightAngleChar(RV_FALSE);
	}

	stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr,
										hPool,
										hPage,
										strToEncode,
										length);
	if (stat != RV_OK)
	{
		RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
					"EncodeBracket - Failed to encode brackets. address=%x",
					pAddr));
	}

	return stat;
}


#ifdef __cplusplus
}
#endif
