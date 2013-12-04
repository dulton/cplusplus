/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrDiameterUri.c                              *
 *                                                                            *
 * This file implements AddrDiameterUri object methods.                       *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ------------                                          *
 *      Mickey           Mar.2007                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "AddrDiameterUri.h"
#include "_SipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include "RvSipCommonTypes.h"
#include "RvSipCommonList.h"

#include <string.h>
#include <stdlib.h>



#define MAX_URL_HEADERS_PARSE 50

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AddrDiameterUriClean( IN MsgAddrDiameterUri* pDiameterUri);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddrDiameterUriConstruct
 * ------------------------------------------------------------------------
 * General: The function construct a url address object, which is 'stand alone'
 *          (means - relate to no message).
 *          The function 'allocate' the object (from the given page), initialized
 *          it's parameters, and return the handle of the object.
 *          Note - The function keeps the page in the url structure, therefore
 *          in every allocation that will come, it will be done from the same page..
 * Return Value: RV_OK, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPage   - Handle of the memory page that the url will be allocated from..
 * output: hDiameterUri - Handle of the url address object..
 ***************************************************************************/
 RvStatus AddrDiameterUriConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                           IN  HRPOOL            hPool,
                           IN  HPAGE             hPage,
                           OUT MsgAddrDiameterUri** hDiameterUri)
{
    MsgAddrDiameterUri*   pDiameterUri;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr)||(hPage == NULL_PAGE)||(hDiameterUri == NULL))
        return RV_ERROR_NULLPTR;

    *hDiameterUri = NULL;

    pDiameterUri = (MsgAddrDiameterUri*)SipMsgUtilsAllocAlign(pMsgMgr,
                                              hPool,
                                              hPage,
                                              sizeof(MsgAddrDiameterUri),
                                              RV_TRUE);
    if(pDiameterUri == NULL)
    {
        *hDiameterUri = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrDiameterUriConstruct - Failed - No more resources"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    AddrDiameterUriClean(pDiameterUri);

    *hDiameterUri = (MsgAddrDiameterUri*)pDiameterUri;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
        "AddrDiameterUriConstruct - Url object was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
        hPool, hPage, pDiameterUri));

    return RV_OK;
}


/***************************************************************************
 * AddrDiameterUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one url address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the url address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrDiameterUriCopy(IN RvSipAddressHandle hDestination,
                      IN RvSipAddressHandle hSource)
{
   MsgAddress*         destAdd;
   MsgAddress*         sourceAdd;
   MsgAddrDiameterUri* dest;
   MsgAddrDiameterUri* source;

    if((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_NULLPTR;

    destAdd = (MsgAddress*)hDestination;
    sourceAdd = (MsgAddress*)hSource;

    dest = (MsgAddrDiameterUri*)destAdd->uAddrBody.hUrl;
    source = (MsgAddrDiameterUri*)sourceAdd->uAddrBody.hUrl;

    /* StrHost */
    if(source->strHost > UNDEFINED)
    {
        dest->strHost = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                    destAdd->hPool,
                                                    destAdd->hPage,
                                                    sourceAdd->hPool,
                                                    sourceAdd->hPage,
                                                    source->strHost);
        if(dest->strHost == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrDiameterUriCopy - Failed to copy strHost"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pHost = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strHost);
#endif
    }
    else
    {
        dest->strHost = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pHost = NULL;
#endif
    }

    /* portNum */
    dest->portNum   = source->portNum;

    /* eTransport */
    dest->eTransport = source->eTransport;

    /* The transport */
    if((dest->eTransport == RVSIP_TRANSPORT_OTHER) && (source->strTransport > UNDEFINED))
    {
        dest->strTransport = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                        destAdd->hPool,
                                                        destAdd->hPage,
                                                        sourceAdd->hPool,
                                                        sourceAdd->hPage,
                                                        source->strTransport);

        if(dest->strTransport == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrDiameterUriCopy - Failed to copy strTransport"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pTransport = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                        dest->strTransport);
#endif
    }
    else
    {
        dest->strTransport = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTransport = NULL;
#endif
    }

    /* eProtocol */
    dest->eProtocol = source->eProtocol;

    /* Is secure */
    dest->bIsSecure = source->bIsSecure;

    /* Other Parameters */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
															destAdd->hPool,
															destAdd->hPage,
															sourceAdd->hPool,
															sourceAdd->hPage,
															source->strOtherParams);
        if(dest->strOtherParams == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrDiameterUriCopy - Failed to copy other parameters"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOtherParams = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strOtherParams);
#endif
    }
    else
    {
        dest->strOtherParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOtherParams = NULL;
#endif
    }

    return RV_OK;
}


/***************************************************************************
 * AddrDiameterUriEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *            Url as encoded buffer (string) onto it.
 *            The user should check the return code.  If the size of
 *            the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *            and the user should send bigger buffer for the encoded message.
 * Return Value:RV_OK, RV_ERROR_INSUFFICIENT_BUFFER, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hAddr    - Handle of the address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus AddrDiameterUriEncode(IN  RvSipAddressHandle hAddr,
                        IN  HRPOOL                    hPool,
                        IN  HPAGE                     hPage,
                        IN  RvBool                    bInUrlHeaders,
                        OUT RvUint32*                 length)
{
    RvStatus                stat;
    MsgAddrDiameterUri*     pDiameterUri;
    MsgAddress*             pAddr = (MsgAddress*)hAddr;
    RvChar                  strHelper[16];

    if(NULL == hAddr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "AddrDiameterUriEncode - Encoding Url Address. hDiameterUri 0x%p, hPool 0x%p, hPage 0x%p",
             hAddr, hPool, hPage));

    if((hPool == NULL)||(hPage == NULL_PAGE)||(length == NULL))
    {
        RvLogWarning(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrDiameterUriEncode - Failed - InvalidParameter. hDiameterUri: 0x%p, hPool: 0x%p, hPage: 0x%p",
            hAddr,hPool, hPage));
        return RV_ERROR_BADPARAM;
    }


    pDiameterUri = (MsgAddrDiameterUri*)(pAddr->uAddrBody.hUrl);

    /* Encode the scheme */
    if (RV_TRUE == pDiameterUri->bIsSecure)
    {
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "aaas://", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "aaa://", length);
    }

    if(stat != RV_OK)
    {
        return stat;
    }

    /* Set strHost */
    if(pDiameterUri->strHost > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pDiameterUri->strHost,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }
    else
    {
        /* This is not optional */
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrDiameterUriEncode - Failed. strHost is NULL. cannot encode. not optional parameter"));
        return RV_ERROR_BADPARAM;
    }

    /* Set ":"portNum */
    if(pDiameterUri->portNum > UNDEFINED)
    {
        /* Set ":"*/
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, ":", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        /* portNum */
        MsgUtils_itoa(pDiameterUri->portNum, strHelper);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, strHelper, length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* Transport */
    if(pDiameterUri->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        /* Set " ;transport=" eTransport */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "transport", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeTransportType( pAddr->pMsgMgr,
                                                hPool,
                                                hPage,
                                                pDiameterUri->eTransport,
                                                pAddr->hPool,
                                                pAddr->hPage,
                                                pDiameterUri->strTransport,
                                                length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* Protocol */
    if(pDiameterUri->eProtocol != RVSIP_DIAMETER_PROTOCOL_UNDEFINED)
    {
        /* Set " ;protocol=" eProtocol */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "protocol", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeDiameterProtocolType( pAddr->pMsgMgr,
                                                hPool,
                                                hPage,
                                                pDiameterUri->eProtocol,
                                                length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* Other params */
    if(pDiameterUri->strOtherParams > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);

        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pDiameterUri->strOtherParams,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    if(stat != RV_OK)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrDiameterUriEncode - Failed to encoding URL. state %d",
            stat));
    }
    
    return stat;
}

/***************************************************************************
 * AddrDiameterUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse a DIAMETER url address text and construct
 *           a MsgAddrDiameterUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the url address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrDiameterUriParse( IN RvSipAddressHandle hSipAddr,
                        IN RvChar*           buffer)
{
    MsgAddress* pAddress = (MsgAddress*)hSipAddr;

    if (pAddress == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    AddrDiameterUriClean((MsgAddrDiameterUri*)pAddress->uAddrBody.hUrl);
    return MsgParserParseStandAloneAddress(pAddress->pMsgMgr, 
                                              SIP_PARSETYPE_DIAMETER_URI_ADDRESS, 
                                              buffer, 
                                              (void*)hSipAddr);
}

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
 *    input: hDiameterUri      - Handle of the address object.
 *         eTransport   - Enumeration value of transportType.
 *         strTransport - String of transportType, in case eTransport is
 *                        RVSIP_TRANSPORT_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strTransportOffset - The offset of the string in the page.
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetTransport( IN  RvSipAddressHandle hSipAddr,
                                  IN  RvSipTransport     eTransport,
                                  IN  RvChar*           strTransport,
                                  IN  HRPOOL             hPool,
                                  IN  HPAGE              hPage,
                                  IN  RvInt32           strTransportOffset)
{
    MsgAddrDiameterUri* pDiameterUri;
    RvInt32             newStrOffset;
    MsgAddress*         pAddr = (MsgAddress*)hSipAddr;
    RvStatus            retStatus;
    
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pDiameterUri = (MsgAddrDiameterUri*)pAddr->uAddrBody.pDiameterUri;

    pDiameterUri->eTransport = eTransport;

    if(eTransport == RVSIP_TRANSPORT_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strTransportOffset,
                                      strTransport, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrDiameterUriSetTransport - Failed to set strTransport"));
            return retStatus;
        }
        pDiameterUri->strTransport = newStrOffset;
#ifdef SIP_DEBUG
        pDiameterUri->pTransport = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pDiameterUri->strTransport);
#endif
    }
    else
    {
        pDiameterUri->strTransport = UNDEFINED;
#ifdef SIP_DEBUG
        pDiameterUri->pTransport = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * SipAddrDiameterUriGetStrTransport
 * ------------------------------------------------------------------------
 * General: This method returns the transport protocol string value from the
 *          MsgDiameterUri object.
 * Return Value: String of transportType
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hSipAddr - Handle of the url address object.
 *
 ***************************************************************************/
RvInt32 RVCALLCONV SipAddrDiameterUriGetStrTransport(
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri* pDiameterUri = ((MsgAddress*)hSipAddr)->uAddrBody.pDiameterUri;

    return pDiameterUri->strTransport;
}

/***************************************************************************
 * SipAddrDiameterUriSetProtocol
 * ------------------------------------------------------------------------
 * General: This method sets the transport protocol value in the MsgAddrDiameterUri
 *          object.
 *          
 * Return Value: RV_OK, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 *    input: hSipAddr   - Handle of the address object.
 *           eProtocol  - Enumeration value of transportType.
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetProtocol( IN  RvSipAddressHandle  hSipAddr,
                                  IN  RvSipDiameterProtocol     eProtocol)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress*         pAddr = (MsgAddress*)hSipAddr;
    
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pDiameterUri = (MsgAddrDiameterUri*)pAddr->uAddrBody.pDiameterUri;
    pDiameterUri->eProtocol = eProtocol;

    return RV_OK;
}

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
                            IN    RvInt32           strHostOffset)
{
    RvInt32        newStrOffset;
    MsgAddrDiameterUri*     pDiameterUri;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    pDiameterUri  = (MsgAddrDiameterUri*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strHostOffset, strHost,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrDiameterUriSetHost - Failed to set strHost"));
        return retStatus;
    }

    pDiameterUri->strHost = newStrOffset;
#ifdef SIP_DEBUG
    pDiameterUri->pHost = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                               pDiameterUri->strHost);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrDiameterUriGetHost
 * ------------------------------------------------------------------------
 * General:  This method returns the name of the host, from the URL object.
 * Return Value: host name or NULL if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the url address object.
 ***************************************************************************/
RvInt32 SipAddrDiameterUriGetHost(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri* pDiameterUri = ((MsgAddress*)hSipAddr)->uAddrBody.pDiameterUri;
    return pDiameterUri->strHost;
}

/***************************************************************************
 * SipAddrDiameterUriSetOtherParams
 * ------------------------------------------------------------------------
 * General: This function is used to set the OtherParams value of the MsgAddrDiameterUri
 *          object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDiameterUri       - Handle of the url address object.
 *    strOtherParams  - The OtherParams parameter to be set in the object. - if NULL,
 *                  the exist strOtherParams be removed.
 *  strUrlOffset  - The offset of the string.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrDiameterUriSetOtherParams( IN    RvSipAddressHandle hSipAddr,
                                  IN    RvChar*            strOtherParams,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32            strUrlOffset)
{
    RvInt32             newStrOffset;
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddress*         pAddr;
    RvStatus            retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;
    pDiameterUri  = (MsgAddrDiameterUri*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strUrlOffset, strOtherParams,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrDiameterUriSetOtherParams - Failed to set strOtherParams"));
        return retStatus;
    }

    pDiameterUri->strOtherParams = newStrOffset;
#ifdef SIP_DEBUG
    pDiameterUri->pOtherParams = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                    pDiameterUri->strOtherParams);
#endif

    return RV_OK;
}

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
RvInt32 SipAddrDiameterUriGetOtherParams(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrDiameterUri *pDiameterUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pDiameterUri = ((MsgAddress*)hSipAddr)->uAddrBody.pDiameterUri;

    return pDiameterUri->strOtherParams;
}
/***************************************************************************
 * AddrDiameterUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two URL objects.
 *
 *         DIAMETER URL format is:
 *         aaa::/host:port.
 *
 *           DIAMETER URLs are compared for equality according to the following rules:
 *			1. A AAA and AAAS URI are never equivalent.
 *          2. Comparison of all components is case-insensitive.
 *          3. The ordering of parameters is not significant.
 *			4. (not implemented) characters are equivalent to their encoding.
 *			   (e.g. '%61lice' = 'alice')
 *			5. An IP address that is the result of a DNS lookup of a host name
 *             does not match that host name.
 *          6. The host and port components must match.
 *             if any of those parameters exists in one url, it must be present
 *             in the other url too.
 *          7. A Url omitting any component with a default value (port, transport,
 *             protocol) will not match a url explicitly containing
 *             that component with its default value.
 *          8. Any other-parameter appearing in both urls, must match.
 *
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrDiameterUriIsEqual(IN RvSipAddressHandle pHeader,
                              IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrDiameterUri* pDiameterUri;
    MsgAddrDiameterUri* pOtherUrl;
    MsgAddress*         pAddr;
    MsgAddress*         pOtherAddr;

    if((pHeader == NULL) || (pOtherHeader == NULL))
        return RV_FALSE;
	
	if (pHeader == pOtherHeader)
	{
		/* This is the same object */
		return RV_TRUE;
	}

    pAddr      = (MsgAddress*)pHeader;
    pOtherAddr = (MsgAddress*)pOtherHeader;

    pDiameterUri      = (MsgAddrDiameterUri*)pAddr->uAddrBody.hUrl;
    pOtherUrl = (MsgAddrDiameterUri*)pOtherAddr->uAddrBody.hUrl;


    /* ----------------------------- host ------------------------- */
    /* host - must be in both addresses*/
    if((pDiameterUri->strHost > UNDEFINED && pOtherUrl->strHost == UNDEFINED) ||
        (pDiameterUri->strHost == UNDEFINED && pOtherUrl->strHost > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pDiameterUri->strHost > UNDEFINED)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pDiameterUri->strHost,
                  pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHost)!= RV_TRUE)
        {
            /* host is not equal in both urls */
            return RV_FALSE;
        }
    }

    /* --------------------------- port ----------------------------- */
    if(pDiameterUri->portNum != pOtherUrl->portNum)
    {
        return RV_FALSE;
    }

    /* --------------------------- scheme ----------------------------- */
    if(pDiameterUri->bIsSecure != pOtherUrl->bIsSecure)
    {
        return RV_FALSE;
    }

    /* The following uri-parameters, if exist in one url, must be present
       and equal in second url too: transport, protocol.
       Other url parameters that exist in only one URL are ignored.*/

    /* -------------- Transport ---------------- */
    if((pDiameterUri->eTransport > UNDEFINED && pOtherUrl->eTransport == UNDEFINED) ||
        (pDiameterUri->eTransport == UNDEFINED && pOtherUrl->eTransport > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pDiameterUri->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        /* Both parameters are present */
        if(pDiameterUri->eTransport != pOtherUrl->eTransport)
        {
            /* Transport param is not equal */
            return RV_FALSE;
        }
        else if(pDiameterUri->eTransport == RVSIP_TRANSPORT_OTHER)
        {
            /*if(strcmp(pDiameterUri->strTransport, pOtherUrl->strTransport) != 0)*/
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pDiameterUri->strTransport,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strTransport)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
    }

    /* ------------- Protocol ---------------- */
    if (pDiameterUri->eProtocol != pOtherUrl->eProtocol)
    {
        return RV_FALSE;
    }

    return RV_TRUE;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AddrDiameterUriClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 ***************************************************************************/
static void AddrDiameterUriClean( IN MsgAddrDiameterUri* pDiameterUri)
{
    pDiameterUri->portNum				= UNDEFINED;
    pDiameterUri->eTransport			= RVSIP_TRANSPORT_UNDEFINED;
    pDiameterUri->strTransport			= UNDEFINED;
    pDiameterUri->strHost				= UNDEFINED;
    pDiameterUri->eProtocol	    		= RVSIP_DIAMETER_PROTOCOL_UNDEFINED;
    pDiameterUri->strOtherParams		= UNDEFINED;
    pDiameterUri->bIsSecure				= RV_FALSE;

#ifdef SIP_DEBUG
    pDiameterUri->pTransport			= NULL;
    pDiameterUri->pHost					= NULL;
    pDiameterUri->pOtherParams			= NULL;
#endif
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif
