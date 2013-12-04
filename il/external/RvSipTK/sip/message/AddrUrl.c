/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrUrl.c                                      *
 *                                                                            *
 * This file implements AddrUrl object methods.                               *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
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
#include "_SipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include "RvSipCommonTypes.h"
#include "RvSipCommonList.h"
#include "RvSipAddress.h"

#include <string.h>
#include <stdlib.h>



#define MAX_URL_HEADERS_PARSE 50

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AddrUrlClean( IN MsgAddrUrl* pUrl);

#ifndef RV_SIP_PRIMITIVES
static RvStatus PrepareBufferForParsing(IN    RvChar* pHeadersBuffer,
                                         IN    RvInt   bufferLen,
                                         INOUT RvInt32* pStartHeadersArray);

static RvStatus ConvertEscapedReservedChar(IN  RvChar* pEcapedStr,
                                            OUT RvChar* pRegularChar);
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddrUrlConstruct
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
 * output: hSipUrl - Handle of the url address object..
 ***************************************************************************/
RvStatus AddrUrlConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                           IN  HRPOOL            hPool,
                           IN  HPAGE             hPage,
                           OUT MsgAddrUrlHandle* hSipUrl)
{
    MsgAddrUrl*   pUrl;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr)||(hPage == NULL_PAGE)||(hSipUrl == NULL))
        return RV_ERROR_NULLPTR;

    *hSipUrl = NULL;

    pUrl = (MsgAddrUrl*)SipMsgUtilsAllocAlign(pMsgMgr,
                                              hPool,
                                              hPage,
                                              sizeof(MsgAddrUrl),
                                              RV_TRUE);
    if(pUrl == NULL)
    {
        *hSipUrl = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrUrlConstruct - Failed - No more resources"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    AddrUrlClean(pUrl);

    *hSipUrl = (MsgAddrUrlHandle)pUrl;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
        "AddrUrlConstruct - Url object was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
        hPool, hPage, pUrl));

    return RV_OK;
}


/***************************************************************************
 * AddrUrlCopy
 * ------------------------------------------------------------------------
 * General:Copy one url address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the url address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrUrlCopy(IN RvSipAddressHandle hDestination,
                      IN RvSipAddressHandle hSource)
{
    MsgAddress*        destAdd;
    MsgAddress*        sourceAdd;
    MsgAddrUrl*        dest;
    MsgAddrUrl*        source;

    if((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_NULLPTR;

    destAdd = (MsgAddress*)hDestination;
    sourceAdd = (MsgAddress*)hSource;

    dest = (MsgAddrUrl*)destAdd->uAddrBody.hUrl;
    source = (MsgAddrUrl*)sourceAdd->uAddrBody.hUrl;

    /* str user */
    if(source->strUser > UNDEFINED)
    {
        dest->strUser = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                    destAdd->hPool,
                                                    destAdd->hPage,
                                                    sourceAdd->hPool,
                                                    sourceAdd->hPage,
                                                    source->strUser);
        if(dest->strUser == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strUser"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pUser = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strUser);
#endif
    }
    else
    {
        dest->strUser = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pUser = NULL;
#endif
    }

    /* strHost */
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
                "AddrUrlCopy - Failed to copy strHost"));
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

    /* MaddrParam */
    if(source->strMaddrParam > UNDEFINED)
    {
        dest->strMaddrParam = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                            destAdd->hPool,
                                                            destAdd->hPage,
                                                            sourceAdd->hPool,
                                                            sourceAdd->hPage,
                                                            source->strMaddrParam);
        if(dest->strMaddrParam == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy MaddrParam"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMaddrParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                         dest->strMaddrParam);
#endif
    }
    else
    {
        dest->strMaddrParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMaddrParam = NULL;
#endif
    }

	/* TokenizedByParam */
    if(source->strTokenizedByParam > UNDEFINED)
    {
        dest->strTokenizedByParam = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                            destAdd->hPool,
                                                            destAdd->hPage,
                                                            sourceAdd->hPool,
                                                            sourceAdd->hPage,
                                                            source->strTokenizedByParam);
        if(dest->strTokenizedByParam == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy TokenizedByParam"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pTokenizedByParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                         dest->strTokenizedByParam);
#endif
    }
    else
    {
        dest->strTokenizedByParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pTokenizedByParam = NULL;
#endif
    }

	/* bOrigParam */
	dest->bOrigParam = source->bOrigParam;

    /* bOldAddrSpec */
    dest->bOldAddrSpec = source->bOldAddrSpec;
    
    /* strUrlParams */
    if(source->strUrlParams > UNDEFINED)
    {
        dest->strUrlParams = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                        destAdd->hPool,
                                                        destAdd->hPage,
                                                        sourceAdd->hPool,
                                                        sourceAdd->hPage,
                                                        source->strUrlParams);
        if(dest->strUrlParams == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strUrlParams"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pUrlParams = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                         dest->strUrlParams);
#endif
    }
    else
    {
        dest->strUrlParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pUrlParams = NULL;
#endif
    }

    /* strHeaders */
    if(source->strHeaders > UNDEFINED)
    {
        dest->strHeaders = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                        destAdd->hPool,
                                                        destAdd->hPage,
                                                        sourceAdd->hPool,
                                                        sourceAdd->hPage,
                                                        source->strHeaders);
        if(dest->strHeaders == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strHeaders"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pHeaders = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                      dest->strHeaders);
#endif
    }
    else
    {
        dest->strHeaders = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pHeaders = NULL;
#endif
    }

    /* eTransport */

    /*the transport original string*/
    if(source->strMessageTransport > UNDEFINED)
    {
        dest->strMessageTransport = MsgUtilsAllocCopyRpoolString(
                                                    sourceAdd->pMsgMgr,
                                                    destAdd->hPool,
                                                    destAdd->hPage,
                                                    sourceAdd->hPool,
                                                    sourceAdd->hPage,
                                                    source->strMessageTransport);
        if(dest->strMessageTransport == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strMessageTransport"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMessageTransport = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strMessageTransport);
#endif
    }
    else
    {
        dest->strMessageTransport = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMessageTransport = NULL;
#endif
    }


    dest->eTransport = source->eTransport;

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
                "AddrUrlCopy - Failed to copy strTransport"));
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

    /* eUserParam */
    dest->eUserParam = source->eUserParam;

    if((dest->eUserParam == RVSIP_USERPARAM_OTHER) && (source->strUserParam > UNDEFINED))
    {
        dest->strUserParam = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                        destAdd->hPool,
                                                        destAdd->hPage,
                                                        sourceAdd->hPool,
                                                        sourceAdd->hPage,
                                                        source->strUserParam);
        if(dest->strUserParam == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strUserParam"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pUserParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                        dest->strUserParam);
#endif
    }
    else
    {
        dest->strUserParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pUserParam = NULL;
#endif
    }

    /* ttl */
    dest->ttlNum    = source->ttlNum;
    /*lr*/
    dest->lrParamType = source->lrParamType;
    /* is secure */
    dest->bIsSecure = source->bIsSecure;
    /* method */
    dest->eMethod   = source->eMethod;
    if((source->eMethod == RVSIP_METHOD_OTHER) && (source->strOtherMethod > UNDEFINED))
    {
        dest->strOtherMethod = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                               destAdd->hPool,
                                                               destAdd->hPage,
                                                               sourceAdd->hPool,
                                                               sourceAdd->hPage,
                                                               source->strOtherMethod);
#ifdef SIP_DEBUG
        dest->pOtherMethod = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                            dest->strOtherMethod);
#endif
    }
    else
    {
        dest->strOtherMethod = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOtherMethod = NULL;
#endif
    }

    /* comp */
    dest->eComp = source->eComp;

    if((dest->eComp == RVSIP_COMP_OTHER) && (source->strCompParam > UNDEFINED))
    {
        dest->strCompParam = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                          destAdd->hPool,
                                                          destAdd->hPage,
                                                          sourceAdd->hPool,
                                                          sourceAdd->hPage,
                                                          source->strCompParam);

        if(dest->strCompParam == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strCompParam"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCompParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                        dest->strCompParam);
#endif
    }
    else
    {
        dest->strCompParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCompParam = NULL;
#endif
    }

	/* sigcomp-id */
	if(source->strSigCompIdParam > UNDEFINED)
	{
		dest->strSigCompIdParam = MsgUtilsAllocCopyRpoolString(
											sourceAdd->pMsgMgr,
											destAdd->hPool,
											destAdd->hPage,
											sourceAdd->hPool,
											sourceAdd->hPage,
											source->strSigCompIdParam);
		if (dest->strSigCompIdParam == UNDEFINED)
		{
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strSigCompIdParam"));
            return RV_ERROR_OUTOFRESOURCES;
		}
#ifdef SIP_DEBUG
		dest->pSigCompIdParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strSigCompIdParam);
#endif
	}
	else
	{
		dest->strSigCompIdParam = UNDEFINED;
#ifdef SIP_DEBUG
		dest->pSigCompIdParam = NULL;
#endif
	}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
	/* eCPCType */
    dest->eCPCType = source->eCPCType;

    if((dest->eCPCType == RVSIP_CPC_TYPE_OTHER) && (source->strCPC > UNDEFINED))
    {
        dest->strCPC = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                        destAdd->hPool,
                                                        destAdd->hPage,
                                                        sourceAdd->hPool,
                                                        sourceAdd->hPage,
                                                        source->strCPC);
        if(dest->strCPC == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy strCPC"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCPC = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                        dest->strCPC);
#endif
    }
    else
    {
        dest->strCPC = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCPC = NULL;
#endif
    }

    /* Gr Parameter */
    dest->bGrParam = source->bGrParam;
    if(source->strGrParam > UNDEFINED)
    {
        dest->strGrParam = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                            destAdd->hPool,
                                                            destAdd->hPage,
                                                            sourceAdd->hPool,
                                                            sourceAdd->hPage,
                                                            source->strGrParam);
        if(dest->strGrParam == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrUrlCopy - Failed to copy Gr Parameter"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pGrParam = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage,
                                         dest->strGrParam);
#endif
    }
    else
    {
        dest->strGrParam = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pGrParam = NULL;
#endif
    }

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
    return RV_OK;
}


/***************************************************************************
 * AddrUrlEncode
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
RvStatus AddrUrlEncode(IN  RvSipAddressHandle hAddr,
                        IN  HRPOOL             hPool,
                        IN  HPAGE              hPage,
                        IN  RvBool            bInUrlHeaders,
                        OUT RvUint32*         length)
{
    RvStatus     stat;
    MsgAddrUrl*   pUrl;
    MsgAddress*   pAddr = (MsgAddress*)hAddr;
    RvChar       strHelper[16];

    if(NULL == hAddr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "AddrUrlEncode - Encoding Url Address. hSipUrl 0x%p, hPool 0x%p, hPage 0x%p",
             hAddr, hPool, hPage));

    if((hPool == NULL)||(hPage == NULL_PAGE)||(length == NULL))
    {
        RvLogWarning(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrUrlEncode - Failed - InvalidParameter. hSipUrl: 0x%p, hPool: 0x%p, hPage: 0x%p",
            hAddr,hPool, hPage));
        return RV_ERROR_BADPARAM;
    }


    pUrl = (MsgAddrUrl*)(pAddr->uAddrBody.hUrl);

    /* First check if the address is old RFC 822 'addr-spec',
       in that case, only encode user@host                      
     */
    if(pUrl->bOldAddrSpec == RV_TRUE)
    {
        /* set strUser "@" */
        if(pUrl->strUser > UNDEFINED)
        {
            /* user */
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr, hPool, hPage, pAddr->hPool, pAddr->hPage, pUrl->strUser, length);

            if(stat != RV_OK)
            {
                return stat;
            }

            /* @ */
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                              MsgUtilsGetAtChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
        else
        {
            /* This parameter is not optional */
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "AddrUrlEncode - Failed. strUser is NULL. Cannot encode: not an optional parameter"));
            return RV_ERROR_BADPARAM;
        }

        /* set strHost */
        if(pUrl->strHost > UNDEFINED)
        {
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr, hPool, hPage, pAddr->hPool, pAddr->hPage, pUrl->strHost, length);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
        else
        {
            /* This parameter is not optional */
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "AddrUrlEncode - Failed. strHost is NULL. Cannot encode: not an optional parameter"));
            return RV_ERROR_BADPARAM;
        }

        return RV_OK;
    }

    /* encode the scheme */
    switch(pAddr->eAddrType)
    {
    case RVSIP_ADDRTYPE_URL:
        if (RV_TRUE == pUrl->bIsSecure)
        {
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "sips:", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "sip:", length);
        }
        break;
#ifdef RV_SIP_OTHER_URI_SUPPORT
    case RVSIP_ADDRTYPE_IM:
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "im:", length);
        break;
    case RVSIP_ADDRTYPE_PRES:
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "pres:", length);
        break;
#endif /*RV_SIP_OTHER_URI_SUPPORT*/
    default:
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrUrlEncode - Failed. unknown address type %d",pAddr->eAddrType));
        return RV_ERROR_BADPARAM;
    }

    if(stat != RV_OK)
    {
        return stat;
    }
    
    /* set strUser "@" */
    if(pUrl->strUser > UNDEFINED)
    {
        /* user */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strUser,
                                    length);

      if(stat != RV_OK)
      {
            return stat;
      }
      /* @ */
      stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                          MsgUtilsGetAtChar(bInUrlHeaders), length);
      if(stat != RV_OK)
      {
            return stat;
      }
    }

    /* set strHost */
    if(pUrl->strHost > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strHost,
                                    length);
      if(stat != RV_OK)
      {
            return stat;
      }
    }
    else
    {
        /* this is not optional */
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrUrlEncode - Failed. strHost is NULL. cannot encode. not optional parameter"));
        return RV_ERROR_BADPARAM;
    }

    /* set ":"portNum */
    if(pUrl->portNum > UNDEFINED)
    {
        /*set ":"*/
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, ":", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        /* portNum */
        MsgUtils_itoa(pUrl->portNum, strHelper);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, strHelper, length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* set urlParams */

    /* maddr */
    if(pUrl->strMaddrParam > UNDEFINED)
    {
        /* set " ;maddr=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "maddr", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* set maddrParam */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strMaddrParam,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* transport */
    if(pUrl->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        /* set " ;transport="eTransport */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "transport", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(pUrl->strMessageTransport != UNDEFINED)
        {
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                        hPage,
                                        pAddr->hPool,
                                        pAddr->hPage,
                                        pUrl->strMessageTransport,
                                        length);
        }
        else
        {
            stat = MsgUtilsEncodeTransportType( pAddr->pMsgMgr,
                                                hPool,
                                                hPage,
                                                pUrl->eTransport,
                                                pAddr->hPool,
                                                pAddr->hPage,
                                                pUrl->strTransport,
                                                length);
        }
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* user */
    if(pUrl->eUserParam != RVSIP_USERPARAM_UNDEFINED)
    {
        /* set " ;user=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "user", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(pUrl->eUserParam == RVSIP_USERPARAM_PHONE)
        {
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "phone", length);
        }
        else if(pUrl->eUserParam == RVSIP_USERPARAM_IP)
        {
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "ip", length);
        }
        else if(pUrl->eUserParam == RVSIP_USERPARAM_OTHER)
        {
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                        hPage,
                                        pAddr->hPool,
                                        pAddr->hPage,
                                        pUrl->strUserParam,
                                        length);
        }
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* ttl */
    if(pUrl->ttlNum > UNDEFINED)
    {
        /*set ";ttl="*/
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "ttl", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* ttlNum */
        MsgUtils_itoa(pUrl->ttlNum, strHelper);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, strHelper, length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    if(pUrl->lrParamType != RVSIP_URL_LR_TYPE_UNDEFINED)
    {
        /*;lr*/
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "lr", length);
        switch(pUrl->lrParamType)
        {
         case RVSIP_URL_LR_TYPE_EMPTY:
            break;
         case RVSIP_URL_LR_TYPE_1:
             /* =1 */
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "1", length);
            break;
         case RVSIP_URL_LR_TYPE_TRUE:
             /* =true */
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "true", length);
            break;
		 case RVSIP_URL_LR_TYPE_ON:
             /* =on */
			 stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
												 MsgUtilsGetEqualChar(bInUrlHeaders), length);
			 stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "on", length);
			 break;
         default:
             RvLogExcep(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "AddrUrlEncode - Failed to encoding URL. unknown lr type. stat %d",
                stat));
        }
        if(stat != RV_OK)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "AddrUrlEncode - Failed to encoding URL. problem in lr encoding. stat %d",
                stat));
        }
    }

    /* method */
    if(pUrl->eMethod != RVSIP_METHOD_UNDEFINED)
    {
        /*set ";method="*/
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                                    MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "method", length);
        if(stat != RV_OK)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "AddrUrlEncode - Failed to encoding URL. problem in method encoding. state %d",
                stat));
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeMethodType(pAddr->pMsgMgr, hPool, hPage, pUrl->eMethod, pAddr->hPool,
                                        pAddr->hPage, pUrl->strOtherMethod, length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* comp */
    if(pUrl->eComp != RVSIP_COMP_UNDEFINED)
    {
        /* set " ;comp="eComp */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                           MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "comp", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeCompType(pAddr->pMsgMgr,
                                      hPool,
                                      hPage,
                                      pUrl->eComp,
                                      pAddr->hPool,
                                      pAddr->hPage,
                                      pUrl->strCompParam,
                                      length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	/* sigcomp-id */
	if (pUrl->strSigCompIdParam > UNDEFINED)
	{
		/* set ";sigcomp-id=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "sigcomp-id", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* set sigcomp-id value */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strSigCompIdParam,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }

	}

	/* tokenized-by */
    if(pUrl->strTokenizedByParam > UNDEFINED)
    {
        /* set " ;tokenized-by=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "tokenized-by", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        /* set TokenizedByParam */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strTokenizedByParam,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	/* Orig */
	if(pUrl->bOrigParam == RV_TRUE)
	{
		/* set ";orig" */
		stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "orig", length);
        if(stat != RV_OK)
        {
            return stat;
        }
	}
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* CPC */
    if(pUrl->eCPCType != RVSIP_CPC_TYPE_UNDEFINED)
    {
        /* set " ;cpc=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "cpc", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeUriCPCType(pAddr->pMsgMgr, 
                                        hPool, 
                                        hPage, 
                                        pUrl->eCPCType, 
                                        pAddr->hPool,
                                        pAddr->hPage,
                                        pUrl->strCPC,
                                        length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* Gr Parameter */
	if(pUrl->bGrParam == RV_TRUE)
	{
		/* set ";gr" */
		stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "gr", length);
        if(stat != RV_OK)
        {
            return stat;
        }
    
        if(pUrl->strGrParam > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            /* set GrParam */
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                        hPool,
                                        hPage,
                                        pAddr->hPool,
                                        pAddr->hPage,
                                        pUrl->strGrParam,
                                        length);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

    /* params */
    if(pUrl->strUrlParams > UNDEFINED)
    {
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strUrlParams,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }


    /* headers */
    if(pUrl->strHeaders > UNDEFINED)
    {
        /* ? */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage, "?", length);
        if(stat != RV_OK)
        {
            return stat;
        }
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pUrl->strHeaders,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    if(stat != RV_OK)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrUrlEncode - Failed to encode URL. state %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * AddrUrlParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse a SIP url address text and construct
 *           a MsgAddrUrl object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the url address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrUrlParse( IN RvSipAddressHandle hSipAddr,
                        IN RvChar*           buffer)
{
    MsgAddress* pAddress = (MsgAddress*)hSipAddr;

    if (pAddress == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    AddrUrlClean((MsgAddrUrl*)pAddress->uAddrBody.hUrl);
    return MsgParserParseStandAloneAddress(pAddress->pMsgMgr, 
                                              SIP_PARSETYPE_URLADDRESS, 
                                              buffer, 
                                              (void*)hSipAddr);
}

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
 *    input: hSipUrl      - Handle of the address object.
 *         eTransport   - Enumeration value of transportType.
 *         strTransport - String of transportType, in case eTransport is
 *                        RVSIP_TRANSPORT_OTHER.
 *         hPool - The pool on which the string lays (if relevant).
 *         hPage - The page on which the string lays (if relevant).
 *         strTransportOffset - The offset of the string in the page.
 *         strTransportFromMsgOffset - in case the transport is enumeration we save
 *                                     the string exactly as it appeared in the message
 *                                     along with the enumeration.
 ***************************************************************************/
RvStatus SipAddrUrlSetTransport( IN  RvSipAddressHandle hSipAddr,
                                  IN  RvSipTransport     eTransport,
                                  IN  RvChar*           strTransport,
                                  IN  HRPOOL             hPool,
                                  IN  HPAGE              hPage,
                                  IN  RvInt32           strTransportOffset,
                                  IN  RvInt32           strTransportFromMsgOffset)
{
    MsgAddrUrl*       pUrl;
    RvInt32          newStrOffset;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    RvStatus         retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;

    pUrl->eTransport = eTransport;

    if(eTransport == RVSIP_TRANSPORT_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strTransportOffset,
                                      strTransport, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetTransport - Failed to set strTransport"));
            return retStatus;
        }
        pUrl->strTransport = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pTransport = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pUrl->strTransport);
#endif
    }
    else
    {
        pUrl->strTransport = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pTransport = NULL;
#endif
    }


    if(strTransportFromMsgOffset != UNDEFINED)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strTransportFromMsgOffset,
                                      NULL, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetTransport - Failed to set strMessageTransport"));
            return retStatus;
        }
        pUrl->strMessageTransport = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pMessageTransport = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                                          pUrl->strMessageTransport);
#endif
    }
    else
    {
        pUrl->strMessageTransport = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pMessageTransport = NULL;
#endif
    }



    return RV_OK;
}


/***************************************************************************
 * SipAddrUrlSetUser
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the user in the
 *            sip url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle of the url address object.
 *    strUser - The user value to be set in the object - if NULL, the exist strUser
 *            will be removed.
 *  strUserOffset - The offset of the string in the page.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetUser(IN    RvSipAddressHandle hSipAddr,
                            IN    RvChar*           strUser,
                            IN    HRPOOL             hPool,
                            IN    HPAGE              hPage,
                            IN    RvInt32           strUserOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strUserOffset, strUser,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetUser - Failed to set strUser"));
        return retStatus;
    }
    pUrl->strUser = newStrOffset;
#ifdef SIP_DEBUG
    pUrl->pUser = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                               pUrl->strUser);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrUrlSetHost
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the host in the url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl - Handle of the url address object.
 *    strHost - the host value to be set in the object. - if NULL, the exist strHost
 *            will be removed.
 *  strHostOffset - The offset of the host string.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetHost(IN    RvSipAddressHandle hSipAddr,
                            IN    RvChar*           strHost,
                            IN    HRPOOL             hPool,
                            IN    HPAGE              hPage,
                            IN    RvInt32           strHostOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;

    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strHostOffset, strHost,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetHost - Failed to set strHost"));
        return retStatus;
    }
    pUrl->strHost = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pHost = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                   pUrl->strHost);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrUrlSetMaddrParam
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the maddr param in the
 *            sip url object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle of the url address object.
 *    strMaddrParam - The maddr parameter to be set in the object. - if NULL, the
 *                  exist strMaddrParam be removed.
 *  strMaddrOffset - The string offset.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetMaddrParam(IN     RvSipAddressHandle hSipAddr,
                                  IN    RvChar*           strMaddrParam,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32           strMaddrOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;
    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strMaddrOffset, strMaddrParam,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetMaddrParam - didn't manage to set strMaddrParam"));
        return retStatus;
    }
    pUrl->strMaddrParam = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pMaddrParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                         pUrl->strMaddrParam);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipUrlGetUserParam
 * ------------------------------------------------------------------------
 * General:This method returns the user param value from the MsgAddrUrl object.
 *           if eUserParam is RVSIP_USERPARAM_OTHER, then the strUserParam will
 *         point to string which contain the param, else, strUserParam should
 *         be ignored.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 *    input:  hUrl         - Handle of the url address object.
 *  output: eUserParam   - enum value of the requested userParam.
 *          strUserParam - string with the user param, in case that eUserParam
 *                         is RVSIP_USERPARAM_OTHER.
 ***************************************************************************/
/*RvStatus AddrUrlGetUserParam(IN  MsgAddrUrlHandle  hUrl,
                              OUT RvSipUserParam* eUserParam,
                              OUT RvChar**       strUserParam)
{
    MsgAddrUrl* pUrl = (MsgAddrUrl*)hUrl;

    *eUserParam  = pUrl->eUserParam;
    *strUserParam = pUrl->strUserParam;

    return RV_OK;
}*/


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
 *    input:  hUrl         - Handle of the url address object.
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
                                IN    RvInt32           offset)
{
    RvInt32         newStrOffset;
    MsgAddrUrl*      pUrl;
    MsgAddrUrlHandle hUrl;
    MsgAddress*      pAddr = (MsgAddress*)hSipAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    hUrl = pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;

    pUrl->eUserParam = eUserParam;

    if (eUserParam == RVSIP_USERPARAM_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, offset, strUserParam,
                                      pAddr->hPool, pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetUserParam - Failed to set strUserParam"));
            return retStatus;
        }
        pUrl->strUserParam = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pUserParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                         pUrl->strUserParam);
#endif
    }
    else
    {
        pUrl->strUserParam = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pUserParam = NULL;
#endif
    }
    return RV_OK;
}

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
                                  IN    RvInt32           strTokenizedByOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;
    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strTokenizedByOffset, strTokenizedByParam,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetTokenizedByParam - didn't manage to set strTokenizedByParam"));
        return retStatus;
    }
    pUrl->strTokenizedByParam = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pTokenizedByParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
											pUrl->strTokenizedByParam);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrUrlSetUrlParams
 * ------------------------------------------------------------------------
 * General: This function is used to set the UrlParams value of the MsgAddrUrl
 *          object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle of the url address object.
 *    strUrlParams  - The UrlParams parameter to be set in the object. - if NULL,
 *                  the exist strUrlParams be removed.
 *  strUrlOffset  - The offset of the string.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetUrlParams( IN    RvSipAddressHandle hSipAddr,
                                  IN    RvChar*           strUrlParams,
                                  IN    HRPOOL             hPool,
                                  IN    HPAGE              hPage,
                                  IN    RvInt32           strUrlOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;
    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strUrlOffset, strUrlParams,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetUrlParams - Failed to set strUrlParams"));
        return retStatus;
    }
    pUrl->strUrlParams = newStrOffset;
#ifdef SIP_DEBUG
    pUrl->pUrlParams = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                    pUrl->strUrlParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrUrlSetHeaders
 * ------------------------------------------------------------------------
 * General: This function is used to set the Headers value of the maddr param
 *          object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipUrl       - Handle of the url address object.
 *    strHeaders    - The headers parameter to be set in the object. - if NULL,
 *                  the exist strHeaders be removed.
 *  strHeadersOffset - offset of the string.
 *  hPool - The pool on which the string lays (if relevant).
 *  hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrUrlSetHeaders(IN    RvSipAddressHandle hSipAddr,
                               IN    RvChar*           strHeaders,
                               IN    HRPOOL             hPool,
                               IN    HPAGE              hPage,
                               IN    RvInt32           strHeadersOffset)
{
    RvInt32        newStrOffset;
    MsgAddrUrl*     pUrl;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pAddr = (MsgAddress*)hSipAddr;
    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strHeadersOffset, strHeaders,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrUrlSetHeaders - Failed to set Headers"));
        return retStatus;
    }
    pUrl->strHeaders = newStrOffset;
#ifdef SIP_DEBUG
    pUrl->pHeaders = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                  pUrl->strHeaders);
#endif

    return RV_OK;
}

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
 *    input: hSipUrl      - Handle of the address object.
 *         eComp        - Enumeration value of compression Type.
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
                                 IN  RvInt32           strCompOffset)
{
    MsgAddrUrl*       pUrl;
    RvInt32          newStrOffset;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    RvStatus         retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;

    pUrl->eComp = eComp;

    if(eComp == RVSIP_COMP_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strCompOffset,
                                      strCompParam, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetCompParam - Failed to set strCompParam"));
            return retStatus;
        }
        pUrl->strCompParam = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pCompParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pUrl->strCompParam);
#endif
    }
    else
    {
        pUrl->strCompParam = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pCompParam   = NULL;
#endif
    }

    return RV_OK;
}

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
                                 IN  RvInt32           strSigCompIdOffset)
{
    MsgAddrUrl*       pUrl;
    RvInt32          newStrOffset;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    RvStatus         retStatus;

    if(hSipAddr == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;

	retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strSigCompIdOffset,
									strSigCompIdParam, pAddr->hPool,
									pAddr->hPage, &newStrOffset);
	if (RV_OK != retStatus)
	{
		RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
			"SipAddrUrlSetSigCompIdParam - Failed to set strSigCompIdParam"));
		return retStatus;
	}
	pUrl->strSigCompIdParam = newStrOffset;
#ifdef SIP_DEBUG
	pUrl->pSigCompIdParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
		pUrl->strSigCompIdParam);
#endif

    return RV_OK;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RvInt32 SipAddrUrlIsCompParam(IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrl*       pUrl;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    if(hSipAddr == NULL)
        return 0;
    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;
    return (pUrl->eComp == RVSIP_COMP_SIGCOMP);
}
void SipAddrUrlResetCompIdParam(IN  RvSipAddressHandle hSipAddr)
{
    MsgAddrUrl*       pUrl;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    if(hSipAddr == NULL)
        return;
    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;
    pUrl->strSigCompIdParam = UNDEFINED;
}

RvStatus SipAddrUrlSetSigCompIdParamOverWrite(IN  RvSipAddressHandle hSipAddr,
                                 IN  HRPOOL            sourcePool,
                                 IN  HPAGE             sourcePage,
                                 IN  RvInt32           strSigCompIdOffset)
{

    MsgAddrUrl*       pUrl;
    MsgAddrUrlHandle  hUrl;
    MsgAddress*       pAddr = (MsgAddress*)hSipAddr;
    RvStatus rv;
	RvInt strLen = 0;

    if(hSipAddr == NULL || sourcePool == NULL || sourcePage == NULL_PAGE)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
    hUrl =  pAddr->uAddrBody.hUrl;
    pUrl = (MsgAddrUrl*)hUrl;     

	strLen = RPOOL_Strlen(sourcePool, sourcePage, strSigCompIdOffset);

	RPOOL_CopyFrom(pAddr->hPool, pAddr->hPage,pUrl->strSigCompIdParam,
                          sourcePool,sourcePage,strSigCompIdOffset, strLen + 1);
    
    return RV_OK;
}

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */


#ifdef RV_SIP_IMS_HEADER_SUPPORT
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
                                  IN  RvInt32           strCPCOffset)
{
    MsgAddrUrl* pUrl;
    RvInt32     newStrOffset;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    RvStatus    retStatus;
    
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pUrl = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    pUrl->eCPCType = eCPC;

    if(eCPC == RVSIP_CPC_TYPE_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strCPCOffset,
                                      strCPC, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetCPC - Failed to set strCPC"));
            return retStatus;
        }
        pUrl->strCPC = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pCPC = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pUrl->strCPC);
#endif
    }
    else
    {
        pUrl->strCPC = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pCPC = NULL;
#endif
    }

    return RV_OK;
}

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
                          IN  RvInt32            strGrOffset)
{
    MsgAddrUrl* pUrl;
    RvInt32     newStrOffset;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    RvStatus    retStatus;
    
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pUrl = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    pUrl->bGrParam = bGrActive;

    if(bGrActive == RV_TRUE)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strGrOffset,
                                      strGr, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrUrlSetGr - Failed to set strGr"));
            return retStatus;
        }
        pUrl->strGrParam = newStrOffset;
#ifdef SIP_DEBUG
        pUrl->pGrParam = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pUrl->strGrParam);
#endif
    }
    else
    {
        pUrl->strGrParam = UNDEFINED;
#ifdef SIP_DEBUG
        pUrl->pGrParam = NULL;
#endif
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * AddrUrlIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *         SIP URL format is: 
 *         sip:user:password@host:port;uri-parameters?headers.
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1. Comparison of the user-info (user:password) is case-sensitive.
 *		      compartion of all other components is case-insensitive.
 *		   2. The ordering of parameters and headers is not significant.
 *         3. (not implemented) characters are equivalent to their encoding.
 *            (e.g. '%61lice' = 'alice')
 *         4. An IP address that is the result of a DNS lookup of a host name
 *            does not match that host name.
 *         5. The user, password, host and port components must match.
 *            if any of those parameters exists in one url, it must be present
 *            in the other url too.
 *         6. A Url omitting any component with a default value (port, transport,
 *            method, ttl, user) will not match a url explicitly containing 
 *            that component with its default value.
 *         7. Any url-parameter appearing in both urls, must match.
 *         8. A url that includes a maddr parameter, will not match a url that
 *            includes no maddr parameter.
 *         9. Header components are never ignored. Any present header comonent
 *            MUST be present in both URIs, and match.
 *         
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrUrlIsEqual(IN RvSipAddressHandle pHeader,
		               IN RvSipAddressHandle pOtherHeader)
{
    return (AddrUrlCompare(pHeader, pOtherHeader) == 0 ? RV_TRUE : RV_FALSE);
}
/***************************************************************************
 * AddrUrlCompare
 * ------------------------------------------------------------------------
 * General:This function compares between two url objects:
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1.Comparison of the scheme name ("sip"), domain names, parameter names
 *		   and header name are case-insensitive, all other comparisons are
 *		   case-sensitive.
 *		   2.The ordering of parameters and headers is not significant.
 *         3. Url parameters with no default value that exist in only one
 *            URL are ignored.
 *         4. URL parameters that has a default value and are omitted from
 *            one of the URL's are compared according to their default value.
 *         5. The ttl,maddr and transport are compared only if both are present
 *            even though they have default values. This is because we strip them
 *            from to headers.
 * Return Value: Returns 0 if the URLs are equal. Otherwise, the function returns
 *               non-zero value: <0 if first URL is less, or >0 if first URL is more
 *               as the comparison result of a first non-equal URL string element.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvInt8 AddrUrlCompare(IN RvSipAddressHandle pHeader,
		               IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrUrl*       pUrl;
    MsgAddrUrl*       pOtherUrl;
    MsgAddress*       pAddr;
    MsgAddress*       pOtherAddr;
    RvInt8           cmpStatus = 0;
    RvSipMethodType   methodTmp1, methodTmp2;

    if(pHeader == NULL)
        return ((RV_INT8) -1);
    else if(pOtherHeader == NULL)
        return ((RV_INT8) 1);

    pAddr      = (MsgAddress*)pHeader;
    pOtherAddr = (MsgAddress*)pOtherHeader;
    
    pUrl  = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;

    pOtherUrl = (MsgAddrUrl*)pOtherAddr->uAddrBody.hUrl;


    /* strUser */
    if(pUrl->strUser > UNDEFINED)
    {
        if (pOtherUrl->strUser > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strUser,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUser);
            if(cmpStatus != 0)
                return cmpStatus;
        }
        else
            return ((RV_INT8) 1);
    }
    else if (pOtherUrl->strUser > UNDEFINED)
    {
        return ((RV_INT8) -1);
    }

    /* strHost - must be in both addresses*/
    if(pUrl->strHost > UNDEFINED)
    {
        if(pOtherUrl->strHost > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strHost,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHost);
            if(cmpStatus != 0)
                return cmpStatus;
        }
        else
            return ((RV_INT8) 1);
    }
    else
    {
        if(pOtherUrl->strHost > UNDEFINED)
        {
            return ((RV_INT8) -1);
        }
    }

    /* strMaddrParam (url parameter with no default)*/
    if(pUrl->strMaddrParam > UNDEFINED)
    {
        if(pOtherUrl->strMaddrParam > UNDEFINED)
        {
            /*if(stricmp(pUrl->strMaddrParam, pOtherUrl->strMaddrParam) != 0)*/
            /*if(MsgUtils_stricmp(pUrl->strMaddrParam, pOtherUrl->strMaddrParam)== RV_FALSE)*/
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strMaddrParam,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strMaddrParam);
            if(cmpStatus != 0)
                return cmpStatus;
        }
        /* if not both are present - we ignore it */
        /*else
            return RV_FALSE;*/
    }

    /* strUrlParams*/
    if(pUrl->strUrlParams > UNDEFINED)
    {
        if(pOtherUrl->strUrlParams > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strUrlParams,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUrlParams);
            if(cmpStatus != 0)
                return cmpStatus;
        }
    } /*if not both are present - we ignore it */

    /* ttl number (url parameter with default = 1)*/
    if(pUrl->ttlNum != UNDEFINED)
    {
        if(pOtherUrl->ttlNum != UNDEFINED)
        {
            if(pUrl->ttlNum != pOtherUrl->ttlNum)
                return ((RV_INT8)(pUrl->ttlNum - pOtherUrl->ttlNum));
        }
    } 
    /* if not both are present - we ignore it - this is not according
           to the standard*/

    /* method parameter */
    methodTmp1 = pUrl->eMethod;
    methodTmp2 = pOtherUrl->eMethod;

    if(methodTmp1 == RVSIP_METHOD_UNDEFINED)
    {
        methodTmp1 = RVSIP_METHOD_INVITE; /* default value */
    }
    if(methodTmp2 == RVSIP_METHOD_UNDEFINED)
    {
        methodTmp2 = RVSIP_METHOD_INVITE; /* default value */
    }
    if(methodTmp1 != methodTmp2)
    {
        return ((RV_INT8)(methodTmp1 - methodTmp2));
    }
    else if(methodTmp1 == RVSIP_METHOD_OTHER)
    {
        cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strOtherMethod,
                         pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strOtherMethod);
        if(cmpStatus != 0)
            return cmpStatus;
    }

    /* strHeaders */
    if(pUrl->strHeaders > UNDEFINED)
    {
        if(pOtherUrl->strHeaders > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strHeaders,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHeaders);
            if(cmpStatus != 0)
                return cmpStatus;
        }
        /* if not both are present - we ignore it - this is not according
           to the standard*/
    }

    /* if succeed till here, we check the numbers, considering it's defaults */

    /* portNum */
    if(pUrl->portNum != pOtherUrl->portNum)
    {

        /* checking defaults */
        if(pUrl->portNum == UNDEFINED)
        {
            cmpStatus = (RV_INT8)((pOtherUrl->portNum == 5060)? 0 :
                                  ((pOtherUrl->portNum > 5060)? -1 : 1) );
        }
        else if(pOtherUrl->portNum == UNDEFINED)
        {
            cmpStatus = (RV_INT8)((pUrl->portNum == 5060)? 0 : 
                                  ((pUrl->portNum < 5060)? -1 : 1) );
        }
        else
        {
            cmpStatus = (RV_INT8)( (pUrl->portNum < pOtherUrl->portNum)? -1 : 1);
        }
        if(cmpStatus != 0)
            return cmpStatus;
    }

    /* eTransport */
    if(pUrl->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        if(pOtherUrl->eTransport != RVSIP_TRANSPORT_UNDEFINED)
        {
            /* if both are present - we compare it */
            if (pUrl->eTransport != pOtherUrl->eTransport)
                return ((RV_INT8)pUrl->eTransport - (RV_INT8)pOtherUrl->eTransport);
            if ((pUrl->eTransport == RVSIP_TRANSPORT_OTHER)&&
                (pUrl->strTransport > UNDEFINED)&&
                (pOtherUrl->strTransport > UNDEFINED))
            {
                cmpStatus = (SPIRENT_RPOOL_Strcmp(pAddr->hPool, pAddr->hPage, pUrl->strTransport,
                                 pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strTransport) == 0);
                if(cmpStatus != 0)
                    return cmpStatus;
            }
        }
    }/* if not both are present - we ignore it  */
    
    return cmpStatus;
}

#else /* !defined(UPDATED_BY_SPIRENT) */

/***************************************************************************
 * AddrUrlIsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two URL objects.
 * IMPORTANT: This function is equivalent to the following function	
 *			  AddrUrl3261IsEqual(). Thus, if this function gets updated 
 *			  please update the compatible function.
 *
 *         SIP URL format is:
 *         sip:user:password@host:port;uri-parameters?headers.
 *
 *           SIP URLs are compared for equality according to the following rules:
 *			1. A SIP and SIPS URI are never equivalent.
 *          2. Comparison of the user-info (user:password) is case-sensitive.
 *             comparison of all other components is case-insensitive.
 *          3. The ordering of parameters and headers is not significant.
 *			4. (not implemented) characters are equivalent to their encoding.
 *			   (e.g. '%61lice' = 'alice')
 *			5. An IP address that is the result of a DNS lookup of a host name
 *             does not match that host name.
 *          6. The user, password, host and port components must match.
 *             if any of those parameters exists in one url, it must be present
 *             in the other url too.
 *          7. A Url omitting any component with a default value (port, transport,
 *             method, ttl, user) will not match a url explicitly containing
 *             that component with its default value.
 *          8. Any url-parameter appearing in both urls, must match.
 *          9. A url that includes a maddr parameter, will not match a url that
 *             includes no maddr parameter.
 *			10. Header components are never ignored. Any present header component
 *              MUST be present in both URIs, and match.
 *
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrUrlIsEqual(IN RvSipAddressHandle pHeader,
                       IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrUrl*       pUrl;
    MsgAddrUrl*       pOtherUrl;
    MsgAddress*       pAddr;
    MsgAddress*       pOtherAddr;

    if((pHeader == NULL) || (pOtherHeader == NULL))
        return RV_FALSE;
	
	if (pHeader == pOtherHeader)
	{
		/* this is the same object */
		return RV_TRUE;
	}

    pAddr      = (MsgAddress*)pHeader;
    pOtherAddr = (MsgAddress*)pOtherHeader;

    pUrl      = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;
    pOtherUrl = (MsgAddrUrl*)pOtherAddr->uAddrBody.hUrl;

    /* ------------- User info (includes the password) -----------
       Comparison of user info (user:password) is case sensitive.*/
    if((pUrl->strUser > UNDEFINED && pOtherUrl->strUser == UNDEFINED) ||
        (pUrl->strUser == UNDEFINED && pOtherUrl->strUser > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strUser > UNDEFINED) /* both are not UNDEFINED */
    {
        if(RPOOL_Strcmp(pAddr->hPool, pAddr->hPage, pUrl->strUser,
                       pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUser)!= RV_TRUE)
        {
            /* user-info is not equal in both urls */
            return RV_FALSE;
        }
    }

    /* ----------------------------- host ------------------------- */
    /* host - must be in both addresses*/
    if((pUrl->strHost > UNDEFINED && pOtherUrl->strHost == UNDEFINED) ||
        (pUrl->strHost == UNDEFINED && pOtherUrl->strHost > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strHost > UNDEFINED)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strHost,
                  pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHost)!= RV_TRUE)
        {
            /* host is not equal in both urls */
            return RV_FALSE;
        }
    }

    
    /* -------------------------- bOldAddrSpec ---------------------- */
    /* Check if the url is in old RFC 822 format, 
       if so, compare only user and host */
    if(pUrl->bOldAddrSpec != pOtherUrl->bOldAddrSpec)
    {
        return RV_FALSE;
    }
    else if(pUrl->bOldAddrSpec == RV_TRUE)
    {
        return RV_TRUE;
    }

    /* --------------------------- port ----------------------------- */
    if(pUrl->portNum != pOtherUrl->portNum)
    {
        return RV_FALSE;
    }

    /* --------------------------- scheme ----------------------------- */
    if(pUrl->bIsSecure != pOtherUrl->bIsSecure)
    {
        return RV_FALSE;
    }

    /* the following uri-parameters, if exist in one url, must be present
       and equal in second url too: ttl, user, method, maddr.
       Other url parameters that exist in only one URL are ignored.*/
    /* ------------- Maddr ---------------- */
    if((pUrl->strMaddrParam > UNDEFINED && pOtherUrl->strMaddrParam == UNDEFINED) ||
       (pUrl->strMaddrParam == UNDEFINED && pOtherUrl->strMaddrParam > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strMaddrParam > UNDEFINED)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strMaddrParam,
                         pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strMaddrParam)!= RV_TRUE)
        {
            /* maddr param is not the same */
            return RV_FALSE;
        }
    }

    /* -------------- Transport ---------------- */
    if((pUrl->eTransport > UNDEFINED && pOtherUrl->eTransport == UNDEFINED) ||
        (pUrl->eTransport == UNDEFINED && pOtherUrl->eTransport > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        /* both parameters are present */
        if(pUrl->eTransport != pOtherUrl->eTransport)
        {
            /* transport param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eTransport == RVSIP_TRANSPORT_OTHER)
        {
            /*if(strcmp(pUrl->strTransport, pOtherUrl->strTransport) != 0)*/
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strTransport,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strTransport)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
    }

    /* ------------- Ttl ---------------- */
    if(pUrl->ttlNum != pOtherUrl->ttlNum)
    {
        return RV_FALSE;
    }

    /* ------------- User ---------------- */
    if (pUrl->eUserParam != pOtherUrl->eUserParam)
    {
        return RV_FALSE;
    }
    else if (pUrl->eUserParam == RVSIP_USERPARAM_OTHER)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strUserParam,
                pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUserParam)!= RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* ------------- Method ---------------- */
    if(pUrl->eMethod != pOtherUrl->eMethod)
    {
        return RV_FALSE;
    }
    else if(pUrl->eMethod == RVSIP_METHOD_OTHER)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strOtherMethod,
            pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strOtherMethod) != RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* ------------- headers ---------- */
    /* header components are never ignored. Any present header component
       MUST be present in both URIs, and match.*/
    if((pUrl->strHeaders > UNDEFINED && pOtherUrl->strHeaders == UNDEFINED) ||
       (pUrl->strHeaders == UNDEFINED && pOtherUrl->strHeaders > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strHeaders > UNDEFINED)
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strHeaders,
                         pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHeaders)!= RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* -------------- comp ---------------- */
    if((pUrl->eComp != RVSIP_COMP_UNDEFINED && pOtherUrl->eComp == RVSIP_COMP_UNDEFINED) ||
       (pUrl->eComp == RVSIP_COMP_UNDEFINED && pOtherUrl->eComp != RVSIP_COMP_UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->eComp != RVSIP_COMP_UNDEFINED)
    {
        /* both parameters are present */
        if(pUrl->eComp != pOtherUrl->eComp)
        {
            /* comp param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eComp == RVSIP_COMP_OTHER)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strCompParam,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strCompParam)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
    }

	/* ------------------------- sigcomp-id ------------------- */
	if (((pUrl->strSigCompIdParam > UNDEFINED) && (pOtherUrl->strSigCompIdParam == UNDEFINED)) ||
		((pUrl->strSigCompIdParam == UNDEFINED) && (pOtherUrl->strSigCompIdParam > UNDEFINED)))
	{
		return RV_FALSE;
	}
	else if (pUrl->strSigCompIdParam > UNDEFINED)
	{
		RPOOL_Ptr firstAddr, secAddr;
		RvUint firstLen, secLen;

		firstAddr.hPool  = pAddr->hPool;
		firstAddr.hPage  = pAddr->hPage;
		firstAddr.offset = pUrl->strSigCompIdParam;

		secAddr.hPool  = pOtherAddr->hPool;
		secAddr.hPage  = pOtherAddr->hPage;
		secAddr.offset = pOtherUrl->strSigCompIdParam;

		firstLen = RvSipAddrGetStringLength((RvSipAddressHandle)pAddr,RVSIP_ADDRESS_SIGCOMPID_PARAM);
		secLen   = RvSipAddrGetStringLength((RvSipAddressHandle)pOtherAddr,RVSIP_ADDRESS_SIGCOMPID_PARAM);

		if (SipCommonStrcmpUrn(&firstAddr, firstLen, RV_FALSE, RV_FALSE,
			&secAddr, secLen, RV_FALSE, RV_FALSE) != RV_TRUE)
		{
			/* sigcomp-id is not the same */
			return RV_FALSE;
		}
	}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* -------------- CPC ---------------- */
    if((pUrl->eCPCType      != RVSIP_CPC_TYPE_UNDEFINED) && 
       (pOtherUrl->eCPCType != RVSIP_CPC_TYPE_UNDEFINED))
    {
        /* both parameters are present */
        if(pUrl->eCPCType != pOtherUrl->eCPCType)
        {
            /* CPC param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eCPCType == RVSIP_CPC_TYPE_OTHER)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strCPC,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strCPC)!= RV_TRUE)
            {
                /* CPC param is not equal */
                return RV_FALSE;
            }
        }
    }

    /* --------------------------- Gr ----------------------------- */
    if((pUrl->bGrParam == pOtherUrl->bGrParam) && 
       (pUrl->bGrParam == RV_TRUE))
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strGrParam,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strGrParam)!= RV_TRUE)
        {
            /* Gr param is not equal */
            return RV_FALSE;
        }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    return RV_TRUE;
}
#endif
/* SPIRENT_END */

/***************************************************************************
 * AddrUrl3261IsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two URL objects.
 * IMPORTANT: This function is equivalent to the previous function	
 *			  AddrUrlIsEqual(). Thus, if this function gets updated 
 *			  please update the compatible function.
 *         SIP URL format is:
 *         sip:user:password@host:port;uri-parameters?headers.
 *
 *           SIP URLs are compared for equality according to the following rules:
 *			1. A SIP and SIPS URI are never equivalent.
 *          2. Comparison of the user-info (user:password) is case-sensitive.
 *             compartion of all other components is case-insensitive.
 *          3. The ordering of parameters and headers is not significant.
 *			4. Characters are equivalent to their encoding - it's implemented 
 *			   (e.g. '%61lice' = 'alice')
 *			5. An IP address that is the result of a DNS lookup of a host name
 *             does not match that host name.
 *          6. The user, password, host and port components must match.
 *             if any of those parameters exists in one url, it must be present
 *             in the other url too.
 *          7. A Url omitting any component with a default value (port, transport,
 *             method, ttl, user) will not match a url explicitly containing
 *             that component with its default value.
 *          8. Any url-parameter appearing in both urls, must match.
 *          9. A url that includes a maddr parameter, will not match a url that
 *             includes no maddr parameter.
 *			10. (not implemented) Header components are never ignored. Any present 
 *              header component MUST be present in both URIs, and match. 
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Handle of the url object.
 *    pOtherHeader    - Handle of the url object.
 ***************************************************************************/
RvBool AddrUrl3261IsEqual(IN RvSipAddressHandle pHeader,
                          IN RvSipAddressHandle pOtherHeader)
{
	MsgAddrUrl*       pUrl;
    MsgAddrUrl*       pOtherUrl;
    MsgAddress*       pAddr;
    MsgAddress*       pOtherAddr;
	
    if((pHeader == NULL) || (pOtherHeader == NULL))
        return RV_FALSE;
	
	if (pHeader == pOtherHeader)
	{
		/* this is the same object */
		return RV_TRUE;
	}
	
    pAddr      = (MsgAddress*)pHeader;
    pOtherAddr = (MsgAddress*)pOtherHeader;
	
    pUrl      = (MsgAddrUrl*)pAddr->uAddrBody.hUrl;
    pOtherUrl = (MsgAddrUrl*)pOtherAddr->uAddrBody.hUrl;

	/* ------------- User info (includes the password) -----------
       Comparison of user info (user:password) is case sensitive.*/
    if((pUrl->strUser > UNDEFINED && pOtherUrl->strUser == UNDEFINED) ||
        (pUrl->strUser == UNDEFINED && pOtherUrl->strUser > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strUser > UNDEFINED) /* both are not UNDEFINED */
    {
		if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strUser,
			pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUser,RV_TRUE)!= RV_TRUE)
        {
            /* user-info is not equal in both urls */
            return RV_FALSE;
        }
    }

    /* ----------------------------- host ------------------------- */
    /* host - must be in both addresses*/
    if((pUrl->strHost > UNDEFINED && pOtherUrl->strHost == UNDEFINED) ||
        (pUrl->strHost == UNDEFINED && pOtherUrl->strHost > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strHost > UNDEFINED)
    {
		if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strHost,
                  pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHost,RV_FALSE)!= RV_TRUE)
        {
            /* host is not equal in both urls */
            return RV_FALSE;
        }
    }

    /* --------------------------- port ----------------------------- */
    if(pUrl->portNum != pOtherUrl->portNum)
    {
        return RV_FALSE;
    }

    /* --------------------------- scheme ----------------------------- */
    if(pUrl->bIsSecure != pOtherUrl->bIsSecure)
    {
        return RV_FALSE;
    }

    /* the following uri-parameters, if exist in one url, must be present
       and equal in second url too: ttl, user, method, maddr.
       Other url parameters that exist in only one URL are ignored.*/
    /* ------------- Maddr ---------------- */
    if((pUrl->strMaddrParam > UNDEFINED && pOtherUrl->strMaddrParam == UNDEFINED) ||
       (pUrl->strMaddrParam == UNDEFINED && pOtherUrl->strMaddrParam > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strMaddrParam > UNDEFINED)
    {
        if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strMaddrParam,
            pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strMaddrParam,RV_FALSE)!= RV_TRUE)
        {
            /* maddr param is not the same */
            return RV_FALSE;
        }
    }

    /* -------------- Transport ---------------- */
    if((pUrl->eTransport > UNDEFINED && pOtherUrl->eTransport == UNDEFINED) ||
        (pUrl->eTransport == UNDEFINED && pOtherUrl->eTransport > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->eTransport != RVSIP_TRANSPORT_UNDEFINED)
    {
        /* both parameters are present */
        if(pUrl->eTransport != pOtherUrl->eTransport)
        {
            /* transport param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eTransport == RVSIP_TRANSPORT_OTHER)
        {
            /*if(strcmp(pUrl->strTransport, pOtherUrl->strTransport) != 0)*/
            if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strTransport,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strTransport,RV_FALSE)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
    }

    /* ------------- Ttl ---------------- */
    if(pUrl->ttlNum != pOtherUrl->ttlNum)
    {
        return RV_FALSE;
    }

    /* ------------- User ---------------- */
    if (pUrl->eUserParam != pOtherUrl->eUserParam)
    {
        return RV_FALSE;
    }
    else if (pUrl->eUserParam == RVSIP_USERPARAM_OTHER)
    {
        if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strUserParam,
            pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strUserParam,RV_FALSE)!= RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* ------------- Method ---------------- */
    if(pUrl->eMethod != pOtherUrl->eMethod)
    {
        return RV_FALSE;
    }
    else if(pUrl->eMethod == RVSIP_METHOD_OTHER)
    {
        if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strOtherMethod,
            pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strOtherMethod,RV_FALSE) != RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* ------------- headers ---------- */
    /* URI header components are never ignored.  Any present header    */
	/* component MUST be present in both URIs and match for the URIs   */ 
	/* The matching rules are defined for each header field			   */ 
	/* in RFC3261 Section 20. Moreover, the ordering of parameters and */
	/* header fields is not significant in comparing SIP and SIPS URIs.*/
    if((pUrl->strHeaders > UNDEFINED && pOtherUrl->strHeaders == UNDEFINED) ||
       (pUrl->strHeaders == UNDEFINED && pOtherUrl->strHeaders > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->strHeaders > UNDEFINED)
    {
		if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strHeaders,
			pOtherAddr->hPool, pOtherAddr->hPage, pOtherUrl->strHeaders,RV_FALSE)!= RV_TRUE)
        {
            return RV_FALSE;
        }
    }

    /* -------------- comp ---------------- */
    if((pUrl->eComp > UNDEFINED && pOtherUrl->eComp == UNDEFINED) ||
        (pUrl->eComp == UNDEFINED && pOtherUrl->eComp > UNDEFINED))
    {
        return RV_FALSE;
    }
    else if(pUrl->eComp != RVSIP_COMP_UNDEFINED)
    {
        /* both parameters are present */
        if(pUrl->eComp != pOtherUrl->eComp)
        {
            /* comp param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eComp == RVSIP_COMP_OTHER)
        {
            if(RPOOL_StrcmpHexa(pAddr->hPool, pAddr->hPage, pUrl->strCompParam,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strCompParam,RV_FALSE)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
    }
    
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* -------------- CPC ---------------- */
    if((pUrl->eCPCType      != RVSIP_CPC_TYPE_UNDEFINED) && 
       (pOtherUrl->eCPCType != RVSIP_CPC_TYPE_UNDEFINED))
    {
        /* both parameters are present */
        if(pUrl->eCPCType != pOtherUrl->eCPCType)
        {
            /* CPC param is not equal */
            return RV_FALSE;
        }
        else if(pUrl->eCPCType == RVSIP_CPC_TYPE_OTHER)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strCPC,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strCPC)!= RV_TRUE)
            {
                /* CPC param is not equal */
                return RV_FALSE;
            }
        }
    }

    /* --------------------------- Gr ----------------------------- */
    if((pUrl->bGrParam == pOtherUrl->bGrParam) && 
       (pUrl->bGrParam == RV_TRUE))
    {
        if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pUrl->strGrParam,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherUrl->strGrParam)!= RV_TRUE)
        {
            /* Gr param is not equal */
            return RV_FALSE;
        }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    return RV_TRUE;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddrUrlParseHeaders
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
RvStatus RVCALLCONV AddrUrlParseHeaders(
                                     IN    MsgAddress*           pAddress,
                                     IN    RvSipCommonListHandle hHeadersList,
                                     IN    RvChar*              pHeadersBuffer)
{
    RvInt    bufferLen;
    RvInt32  headersStartIndexes[MAX_URL_HEADERS_PARSE];
    RvInt32  i;
    RvStatus rv, retStatus = RV_OK;


    /* the headersStartIndexes arrays will contain the indexes in the buffer,
       where a new header begins */
    for(i=0; i<MAX_URL_HEADERS_PARSE; i++)
    {
        headersStartIndexes[i] = UNDEFINED;
    }

    bufferLen = (RvInt)strlen(pHeadersBuffer);

    RvLogDebug(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlParseHeaders: address 0x%p - prepare buffer for parsing...",
            pAddress));
    /*lint -e545*/
    PrepareBufferForParsing(pHeadersBuffer, bufferLen, (RvInt32*)&headersStartIndexes);

    RvLogDebug(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlParseHeaders: address 0x%p - starting to parse headers...",
            pAddress));
    i=0;

    /* for each header found in the given buffer, call to SipParserParse() */
    while(headersStartIndexes[i] != UNDEFINED)
    {
        /* parse the headers string */
        rv = MsgParserParseStandAloneHeader(pAddress->pMsgMgr, 
                                            SIP_PARSETYPE_URL_HEADERS_LIST, 
                                            pHeadersBuffer + headersStartIndexes[i], 
                                            RV_FALSE /* bOnlyValue */, 
                                            (void*)hHeadersList);
        if(rv != RV_OK)
        {
            RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlParseHeaders: address 0x%p - Failed to parse header %d",
            pAddress, i+1));
            retStatus = rv;
        }
        else
        {
            RvLogDebug(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlParseHeaders: address 0x%p - header %d was parsed successfully",
            pAddress, i+1));
        }
        ++i;
    }
    return retStatus;

}

/***************************************************************************
 * AddrUrlEncodeHeaders
 * ------------------------------------------------------------------------
 * General: This function is used to set the Headers param from headers list.
 *          The function encode all headers. during encoding each header
 *          coverts reserved characters to it's escaped form. each header
 *          also set '=' instead of ':' after header name.
 *          This function also sets '&' between headers.
 *          At last the function sets pAddress->strHeader to point to the new
 *          encoded string of headers.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress      - The url address object.
 *    hHeadersList  - The headers list to be set in the object.
 ***************************************************************************/
RvStatus AddrUrlEncodeHeadersList(IN    MsgAddress*           pAddress,
                                   IN    RvSipCommonListHandle hHeadersList)
{
    RvSipHeaderType eHeaderType = RVSIP_HEADERTYPE_UNDEFINED;
    void*           hHeader = NULL;
    RvSipCommonListElemHandle hPos;
    RvStatus       rv;
    RvUint32       length = 0;
    RvInt32        headersOffset;

    rv = RvSipCommonListGetElem(hHeadersList, RVSIP_FIRST_ELEMENT, NULL,
                                (RvInt*)&eHeaderType, &hHeader, &hPos);
    if(RV_OK != rv || hHeader == NULL)
    {
        RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlEncodeHeadersList: address 0x%p - Failed to get header from list 0x%p",
            pAddress, hHeadersList));
        return rv;
    }

    /* append one dummy character on addr page, to know where the new
    headers encoded list will begin */
    rv = RPOOL_Append(pAddress->hPool, pAddress->hPage, 0, RV_FALSE, &headersOffset);
    if(RV_OK != rv )
    {
        RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlEncodeHeadersList: address 0x%p - Failed to append dummy character(rv=%d:%s)",
            pAddress, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    /* 2. first header should be encoded without '&' */
    rv = MsgHeaderEncode(hHeader, eHeaderType, pAddress->hPool, pAddress->hPage, RV_TRUE, RV_FALSE,&length);
    if(RV_OK != rv )
    {
        RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlEncodeHeadersList: address 0x%p - Failed to encode header 0x%p type %d from list 0x%p rv=%d",
            pAddress, hHeader, eHeaderType, hHeadersList,rv));
        return rv;
    }

    /* 3. get next header from list */
    rv = RvSipCommonListGetElem(hHeadersList, RVSIP_NEXT_ELEMENT, hPos,
                                (RvInt*)&eHeaderType, &hHeader, &hPos);
    if(RV_OK != rv )
    {
        RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
            "AddrUrlEncodeHeadersList: address 0x%p - Failed to get next header from list 0x%p",
            pAddress, hHeadersList));
        return rv;
    }

    while (hHeader != NULL)
    {
        /* 1. add '&' */
        rv = MsgUtilsEncodeExternalString(pAddress->pMsgMgr, pAddress->hPool, pAddress->hPage, "&", &length);
        if(RV_OK != rv )
        {
            RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
                "AddrUrlEncodeHeadersList: address 0x%p - Failed to encode '&' after header, rv=%d",
                pAddress, rv));
            return rv;
        }
        /* 2. encode the header to address page */
        rv = MsgHeaderEncode(hHeader, eHeaderType, pAddress->hPool, pAddress->hPage, RV_TRUE,RV_FALSE, &length);
        if(RV_OK != rv )
        {
            RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
                "AddrUrlEncodeHeadersList: address 0x%p - Failed to encode header 0x%p type %d from list 0x%p rv=%d",
                pAddress, hHeader, eHeaderType, hHeadersList,rv));
            return rv;
        }

        /* 3. get next header from list */
        rv = RvSipCommonListGetElem(hHeadersList, RVSIP_NEXT_ELEMENT, hPos,
                                    (RvInt*)&eHeaderType, &hHeader, &hPos);
        if(RV_OK != rv )
        {
            RvLogError(pAddress->pMsgMgr->pLogSrc,(pAddress->pMsgMgr->pLogSrc,
                "AddrUrlEncodeHeadersList: address 0x%p - Failed to get next header from list 0x%p",
                pAddress, hHeadersList));
            return rv;
        }
    }
    /* encode '\0' at the end of the headers string */
    MsgUtilsAllocSetString(pAddress->pMsgMgr, pAddress->hPool, pAddress->hPage, "");

    /* set pAddr->strHeaders to point to the new headers string */
    ((MsgAddrUrl*)pAddress->uAddrBody.hUrl)->strHeaders = headersOffset;
#ifdef SIP_DEBUG
    ((MsgAddrUrl*)pAddress->uAddrBody.hUrl)->pHeaders = RPOOL_GetPtr(pAddress->hPool,
                                                pAddress->hPage, headersOffset);
#endif


    return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AddrUrlClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 ***************************************************************************/
static void AddrUrlClean( IN MsgAddrUrl* pUrl)
{
    pUrl->portNum				= UNDEFINED;
    pUrl->eTransport			= RVSIP_TRANSPORT_UNDEFINED;
    pUrl->strTransport			= UNDEFINED;
    pUrl->eUserParam			= RVSIP_USERPARAM_UNDEFINED;
    pUrl->strUserParam			= UNDEFINED;
    pUrl->strHeaders			= UNDEFINED;
    pUrl->strHost				= UNDEFINED;
    pUrl->strMaddrParam			= UNDEFINED;
    pUrl->strTokenizedByParam	= UNDEFINED;
	pUrl->bOrigParam			= RV_FALSE;
    pUrl->bOldAddrSpec			= RV_FALSE;
    pUrl->strUrlParams			= UNDEFINED;
    pUrl->strUser				= UNDEFINED;
    pUrl->ttlNum				= UNDEFINED;
    pUrl->lrParamType			= RVSIP_URL_LR_TYPE_UNDEFINED;
    pUrl->eMethod				= RVSIP_METHOD_UNDEFINED;
    pUrl->strOtherMethod		= UNDEFINED;
    pUrl->bIsSecure				= RV_FALSE;
    pUrl->strMessageTransport	= UNDEFINED;
    pUrl->eComp					= RVSIP_COMP_UNDEFINED;
    pUrl->strCompParam			= UNDEFINED;
	pUrl->strSigCompIdParam     = UNDEFINED;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pUrl->eCPCType              = RVSIP_CPC_TYPE_UNDEFINED;
    pUrl->strCPC                = UNDEFINED;
    pUrl->bGrParam              = RV_FALSE;
    pUrl->strGrParam            = UNDEFINED;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef SIP_DEBUG
    pUrl->pTransport			= NULL;
    pUrl->pUserParam			= NULL;
    pUrl->pHeaders				= NULL;
    pUrl->pHost					= NULL;
    pUrl->pMaddrParam			= NULL;
	pUrl->pTokenizedByParam		= NULL;
    pUrl->pUrlParams			= NULL;
    pUrl->pUser					= NULL;
    pUrl->pOtherMethod			= NULL;
    pUrl->pMessageTransport		= NULL;
    pUrl->pCompParam			= NULL;
	pUrl->pSigCompIdParam       = NULL;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pUrl->pCPC        		    = NULL;
    pUrl->pGrParam              = NULL;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif
}


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * PrepareBufferForParsing
 * ------------------------------------------------------------------------
 * General: The function goes over the buffer, and prepare it as follows:
 *          1. Every '=' between header-name and header-value is changed to ':'
 *          2. Every '&' between headers, is changed to '\0', and a list of
 *             indexes of header begins is built.
 *          3. Change all reserverd characters from escaped form to a regular
 *             form. escaped value is spread on 3 chracters in the buffer (%3D),
 *             and the regular value is spread only on 1 character (=), so
 *             the function close the spaces, by copying the next charater to
 *             the first free space.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *
 ***************************************************************************/
static RvStatus PrepareBufferForParsing(IN    RvChar* pHeadersBuffer,
                                         IN    RvInt   bufferLen,
                                         INOUT RvInt32* pStartHeadersArray)
{
    RvStatus rv = RV_OK;
    RvBool   bHeaderName = RV_TRUE; /* true if we are now in header-name.
                                        false if in header value */
    RvInt32  indexToCheck = 0;      /* the index in the array that we are checking now */
    RvInt32  indexToSet = 0;        /* the index in the array to set the next character */
    RvInt32  indexInStartHeadersArray = 0;
    RvChar   convertedChar;

    pStartHeadersArray[indexInStartHeadersArray] = 0;
    ++indexInStartHeadersArray;
    while(indexToCheck<bufferLen)
    {
        /* 1. try to change '=' to ':' */
        if(pHeadersBuffer[indexToCheck] == '=' )
        {
            if(bHeaderName == RV_TRUE)
            {
                pHeadersBuffer[indexToSet] = ':';
                bHeaderName = RV_FALSE;
            }
            else
            {
                /* copy next character as is */
                pHeadersBuffer[indexToSet] = pHeadersBuffer[indexToCheck];
            }
        }
        /* 2. try to change '&' to '\0' */
        else if(pHeadersBuffer[indexToCheck] == '&')
        {
            if(bHeaderName == RV_FALSE)
            {
                /* reached the end of the header */
                pHeadersBuffer[indexToSet] = '\0';
                bHeaderName = RV_TRUE;
                pStartHeadersArray[indexInStartHeadersArray] = indexToSet + 1;
                ++indexInStartHeadersArray;
            }
            else
            {
                /* copy next character as is */
                pHeadersBuffer[indexToSet] = pHeadersBuffer[indexToCheck];
            }

        }
        /* 3. try to change escaped character to a regular character */
        else if(pHeadersBuffer[indexToCheck] == '%')
        {
            if(indexToCheck + 2 > bufferLen)
            {
                /* no place to the next to characters needed to be checked to convert
                   the escaped character */
                /* copy character as is */
                pHeadersBuffer[indexToSet] = pHeadersBuffer[indexToCheck];
            }
            else
            {
                rv = ConvertEscapedReservedChar(&(pHeadersBuffer[indexToCheck]),
                                                &convertedChar);
                if(rv == RV_OK)
                {
                    /* managed to convert the escaped character - set the new character
                       in buffer, and skip indexToCheck over the next charaters, of the
                       escaped value */
                    pHeadersBuffer[indexToSet] = convertedChar;
                    indexToCheck += 2;
                }
                else
                {
                    /* failed to convert the escaped character - copy it as is */
                    pHeadersBuffer[indexToSet] = pHeadersBuffer[indexToCheck];
                }
            }
        }
        /* 4. regular character. (not '=', '&', '%') */
        else
        {
            /* copy next character as is */
            pHeadersBuffer[indexToSet] = pHeadersBuffer[indexToCheck];
        }
        ++indexToCheck;
        ++indexToSet;
    }

    /* set null in the end of this header, in case indexToSet is not equal to
       indexToCheck */
    if(indexToSet < indexToCheck)
    {
        pHeadersBuffer[indexToSet] = '\0';
    }
    return RV_OK;
}

/***************************************************************************
 * ConvertEscapedReservedChar
 * ------------------------------------------------------------------------
 * General: The function converts escaped characters to the regular form of
 *          the character.
 *      reserved characters that should be converted are according to the
 *      SIP-URI rule of characters that should not be present in the headers
 *      string (reserved=";" / "/" / "?" / ":" / "@" / "&" / "=" / "+" / "$" / ","):
 *      %24 -->'$'
 *      %26 -->'&'
 *      %2B -->'+'
 *      %2C -->','
 *      %2F -->'/'
 *      %3A -->':'
 *      %3B -->';'
 *      %3D -->'='
 *      %3F -->'?'
 *      %40 -->'@'
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *
 ***************************************************************************/
static RvStatus ConvertEscapedReservedChar(IN  RvChar* pEcapedStr,
                                            OUT RvChar* pRegularChar)
{
    *pRegularChar = '\0';

    switch(pEcapedStr[1])
    {
    case '2':
        switch(pEcapedStr[2])
        {
        case '0': /* space */
             *pRegularChar = ' ';
            break;
        case '2': /* " */
            *pRegularChar = '"';
            break;
        case '6': /* & */
            *pRegularChar = '&';
            break;
        case 'B':
        case 'b': /* + */
            *pRegularChar = '+';
            break;
        case 'C':
        case 'c': /* , */
            *pRegularChar = ',';
            break;
        case 'D':
        case 'd': /* - */
            *pRegularChar = '-';
            break;
        case 'E':
        case 'e': /* . */
            *pRegularChar = '.';
            break;
        case 'F':
        case 'f': /* / */
            *pRegularChar = '/';
            break;
        default:
            return RV_ERROR_NOT_FOUND;
        }
        break;
    case '3':
        switch(pEcapedStr[2])
        {
        case 'B':
        case 'b': /* ; */
            *pRegularChar = ';';
            break;
        case 'C':
        case 'c': /* < */
            *pRegularChar = '<';
            break;
        case 'D':
        case 'd': /* = */
            *pRegularChar = '=';
            break;
        case 'E':
        case 'e': /* ; */
            *pRegularChar = '>';
            break;
        default:
            return RV_ERROR_NOT_FOUND;
        }
        break;
    case '4':
        switch(pEcapedStr[2])
        {
        case  '0': /* @ */
            *pRegularChar = '@';
            break;
        default:
            return RV_ERROR_NOT_FOUND;
        }
        break;
    default:
        return RV_ERROR_NOT_FOUND;
    }
    return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/



#ifdef __cplusplus
}
#endif
