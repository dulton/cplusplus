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
 * This file implements AddrAbsUri object methods.                            *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Oren Libis       Oct.2001                                             *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "AddrAbsUri.h"
#include "_SipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include <string.h>
#include <stdlib.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void AddrAbsUriClean( IN MsgAddrAbsUri* pAddress);
/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddrAbsUriConstruct
 * ------------------------------------------------------------------------
 * General: The function construct an Abs-Uri address object, which is 'stand alone'
 *          (means - relate to no message).
 *          The function 'allocate' the object (from the given page), initialized
 *          it's parameters, and return the handle of the object.
 *          Note - The function keeps the page in the MsgAddress structure, therefore
 *          in every allocation that will come, it will be done from the same page..
 * Return Value: RV_OK, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool   - Handle of the memory pool.
 *         hPage   - Handle of the memory page that the url will be allocated from..
 * output: pAbsUri - Handle of the Abs-Uri address object..
 ***************************************************************************/
RvStatus AddrAbsUriConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                              IN  HRPOOL            hPool,
                              IN  HPAGE             hPage,
                              OUT MsgAddrAbsUri**   pAbsUri)
{
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(pAbsUri == NULL))
        return RV_ERROR_NULLPTR;

    *pAbsUri = NULL;

    *pAbsUri = (MsgAddrAbsUri*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgAddrAbsUri),
                                                    RV_TRUE);
    if(*pAbsUri == NULL)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrAbsUriConstruct - Failed - No more resources"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    else
    {
        AddrAbsUriClean(*pAbsUri);


        RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrAbsUriConstruct - Abs Uri object was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, pAbsUri));

        return RV_OK;
    }
}


/***************************************************************************
 * AddrAbsUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one Abs-Uri address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the Abs-Uri address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrAbsUriCopy(IN RvSipAddressHandle hDestination,
                         IN RvSipAddressHandle hSource)
{
   MsgAddress*        destAdd;
   MsgAddress*        sourceAdd;
   MsgAddrAbsUri*     dest;
   MsgAddrAbsUri*     source;

    if((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_NULLPTR;

    destAdd = (MsgAddress*)hDestination;
    sourceAdd = (MsgAddress*)hSource;

    dest = (MsgAddrAbsUri*)destAdd->uAddrBody.pAbsUri;
    source = (MsgAddrAbsUri*)sourceAdd->uAddrBody.pAbsUri;

    /* str scheme */
    if(source->strScheme > UNDEFINED)
    {
        dest->strScheme = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                          destAdd->hPool,
                                                          destAdd->hPage,
                                                          sourceAdd->hPool,
                                                          sourceAdd->hPage,
                                                          source->strScheme);
        if(dest->strScheme == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrAbsUriCopy - Failed to copy strScheme"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pScheme = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strScheme);
#endif
    }
    else
    {
        dest->strScheme = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pScheme = NULL;
#endif
    }

    /* str Identifier */
    if(source->strIdentifier > UNDEFINED)
    {
        dest->strIdentifier = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                              destAdd->hPool,
                                                              destAdd->hPage,
                                                              sourceAdd->hPool,
                                                              sourceAdd->hPage,
                                                              source->strIdentifier);
        if(dest->strIdentifier == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrAbsUriCopy - Failed to copy strIdentifier"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pIdentifier = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strIdentifier);
#endif
    }
    else
    {
        dest->strIdentifier = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIdentifier = NULL;
#endif
    }

    return RV_OK;
}


/***************************************************************************
 * AddrAbsUriEncode
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
RvStatus AddrAbsUriEncode(IN  RvSipAddressHandle hAddr,
                           IN  HRPOOL            hPool,
                           IN  HPAGE             hPage,
                           IN  RvBool            bInUrlHeaders,
                           OUT RvUint32*         length)
{
    RvStatus      stat;
    MsgAddrAbsUri* pAbsUri;
    MsgAddress*    pAddr = (MsgAddress*)hAddr;

    if(NULL == hAddr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "AddrAbsUriEncode - Encoding Abs Uri Address. pAbsUri 0x%p, hPool 0x%p, hPage 0x%p",
             hAddr, hPool, hPage));

    if((hPool == NULL)||(hPage == NULL_PAGE)||(length == NULL))
    {
        RvLogWarning(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrAbsUriEncode - Failed - InvalidParameter. pAbsUri: 0x%p, hPool: 0x%p, hPage: 0x%p",
            hAddr,hPool, hPage));
        return RV_ERROR_BADPARAM;
    }


    pAbsUri = (MsgAddrAbsUri*)(pAddr->uAddrBody.pAbsUri);


    /* set strScheme */
    if(pAbsUri->strScheme > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pAbsUri->strScheme,
                                    length);

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetNameValueSeperatorCharForAddr(bInUrlHeaders),
                                            length);
    }

    /* set strIdentifier */
    if(pAbsUri->strIdentifier > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pAbsUri->strIdentifier,
                                    length);
    }
    else
    {
        /* this is not optional */
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrAbsUriEncode - Failed. strIdentifier is NULL. cannot encode. not optional parameter"));
        return RV_ERROR_BADPARAM;
    }

    return stat;
}

/***************************************************************************
 * AddrAbsUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse an Abs-Uri address text and construct
 *           a MsgAddrAbsUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the Abs-Uri address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrAbsUriParse( IN RvSipAddressHandle hSipAddr,
                           IN RvChar*           buffer)
{
    MsgAddress*        pAddress = (MsgAddress*)hSipAddr;

    if (pAddress == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    AddrAbsUriClean(pAddress->uAddrBody.pAbsUri);

    return MsgParserParseStandAloneAddress(pAddress->pMsgMgr, 
                                          SIP_PARSETYPE_URIADDRESS, 
                                          buffer, 
                                          (void*)hSipAddr);
}



/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * AddrAbsUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1.Comparison of the scheme name. 
 *		   2.Comparison of the identifier value.
 *         3.The ordering of parameters and headers is not significant.
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrAbsUriIsEqual(IN RvSipAddressHandle pHeader,
		                  IN RvSipAddressHandle pOtherHeader)
{
    return ((AddrAbsUriCompare(pHeader, pOtherHeader) == 0)? RV_TRUE : RV_FALSE);
}

/***************************************************************************
 * AddrAbsUriCompare
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1.Comparison of the scheme name. 
 *		   2.Comparison of the identifier value.
 *         3.The ordering of parameters and headers is not significant.
 * Return Value: Returns 0 if the URLs are equal. Otherwise, the function returns
 *               non-zero value: <0 if first URL is less, or >0 if first URL is more
 *               as the comparison result of a first non-equal URL string element.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvInt8 AddrAbsUriCompare(IN RvSipAddressHandle pHeader,
		                  IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrAbsUri*    pAbsUri;
    MsgAddrAbsUri*    pOtherAbsUri;
    MsgAddress*       pAddr;
    MsgAddress*       pOtherAddr;
    RvInt8            cmpStatus = 0;

    if (pHeader == NULL)
        return ((RV_INT8) -1);
    else if (pOtherHeader == NULL)
        return ((RV_INT8) 1);

    pAddr      = (MsgAddress*)pHeader;
    pOtherAddr = (MsgAddress*)pOtherHeader;
    
    pAbsUri  = (MsgAddrAbsUri*)pAddr->uAddrBody.pAbsUri;

    pOtherAbsUri = (MsgAddrAbsUri*)pOtherAddr->uAddrBody.pAbsUri;


    /* strScheme */
    if(pAbsUri->strScheme > UNDEFINED)
    {
        if (pOtherAbsUri->strScheme > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pAbsUri->strScheme,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherAbsUri->strScheme);
            if(cmpStatus != 0)
                return cmpStatus;
        }
        else
            return ((RV_INT8) 1);
    }
    else if (pOtherAbsUri->strScheme > UNDEFINED)
    {
        return ((RV_INT8) -1);
    }

    /* strIdentifier - must be in both addresses*/
    if(pAbsUri->strIdentifier > UNDEFINED)
    {
        if(pOtherAbsUri->strIdentifier > UNDEFINED)
        {
            cmpStatus = SPIRENT_RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pAbsUri->strIdentifier,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherAbsUri->strIdentifier);
            return cmpStatus;
        }
        else
            return ((RV_INT8) 1);
    }
    else if (pOtherAbsUri->strIdentifier > UNDEFINED)
    {
        return ((RV_INT8) -1);
    }

    return ((RV_INT8) 0);
}

#else /* !defined(UPDATED_BY_SPIRENT) */
/***************************************************************************
 * AddrAbsUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *           SIP URLs are compared for equality according to the following rules:
 *           1.Comparison of the scheme name.
 *           2.Comparison of the identifier value.
 *         3.The ordering of parameters and headers is not significant.
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrAbsUriIsEqual(IN RvSipAddressHandle pHeader,
                          IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrAbsUri*    pAbsUri;
    MsgAddrAbsUri*    pOtherAbsUri;
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

    pAbsUri  = (MsgAddrAbsUri*)pAddr->uAddrBody.pAbsUri;

    pOtherAbsUri = (MsgAddrAbsUri*)pOtherAddr->uAddrBody.pAbsUri;

    /* strScheme */
    if(pAbsUri->strScheme > UNDEFINED)
    {
        if (pOtherAbsUri->strScheme > UNDEFINED)
        {
            if(RPOOL_Strcmp(pAddr->hPool, pAddr->hPage, pAbsUri->strScheme,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherAbsUri->strScheme)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherAbsUri->strScheme > UNDEFINED)
    {
        return RV_FALSE;
    }

    /* strIdentifier - must be in both addresses*/
    if(pAbsUri->strIdentifier > UNDEFINED)
    {
        if(pOtherAbsUri->strIdentifier > UNDEFINED)
        {
            if(RPOOL_Strcmp(pAddr->hPool, pAddr->hPage, pAbsUri->strIdentifier,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherAbsUri->strIdentifier)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherAbsUri->strIdentifier > UNDEFINED)
    {
        return RV_FALSE;
    }


    return RV_TRUE;
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


/***************************************************************************
 * SipAddrAbsUriSetScheme
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the scheme in the
 *            sip Abs-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strScheme       - The scheme value to be set in the object - if NULL, the exist strScheme
 *                    will be removed.
 *  strSchemeOffset - The offset of the string in the page.
 *  hPool           - The pool on which the string lays (if relevant).
 *  hPage           - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrAbsUriSetScheme(IN    RvSipAddressHandle hSipAddr,
                                 IN    RvChar*           strScheme,
                                 IN    HRPOOL             hPool,
                                 IN    HPAGE              hPage,
                                 IN    RvInt32           strSchemeOffset)
{
    RvInt32        newStrOffset;
    MsgAddrAbsUri*  pAbsUri;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr = (MsgAddress*)hSipAddr;
    pAbsUri  = (MsgAddrAbsUri*)pAddr->uAddrBody.pAbsUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strSchemeOffset, strScheme,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrAbsUriSetScheme - Failed to set strScheme"));
        return retStatus;
    }
    pAbsUri->strScheme = newStrOffset;
#ifdef SIP_DEBUG
        pAbsUri->pScheme = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                           pAbsUri->strScheme);
#endif

    return RV_OK;
}



/***************************************************************************
 * SipAddrAbsUriSetIdentifier
 * ------------------------------------------------------------------------
 * General: This function is used to set the value of the Identifier in the
 *            sip Abs-Uri object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr        - Handle of the address object.
 *    strIdentifier       - The Identifier value to be set in the object - if NULL, the exist strIdentifier
 *                    will be removed.
 *  strIdentifierOffset - The offset of the string in the page.
 *  hPool           - The pool on which the string lays (if relevant).
 *  hPage           - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipAddrAbsUriSetIdentifier(IN    RvSipAddressHandle hSipAddr,
                                     IN    RvChar*           strIdentifier,
                                     IN    HRPOOL             hPool,
                                     IN    HPAGE              hPage,
                                     IN    RvInt32           strIdentifierOffset)
{
    RvInt32        newStrOffset;
    MsgAddrAbsUri*  pAbsUri;
    MsgAddress*     pAddr;
    RvStatus       retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr = (MsgAddress*)hSipAddr;

    pAbsUri  = (MsgAddrAbsUri*)pAddr->uAddrBody.pAbsUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strIdentifierOffset, strIdentifier,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrAbsUriSetIdentifier - Failed to set strIdentifier"));
        return retStatus;
    }
    pAbsUri->strIdentifier = newStrOffset;
#ifdef SIP_DEBUG
        pAbsUri->pIdentifier = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                               pAbsUri->strIdentifier);
#endif

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * AddrAbsUriClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 ***************************************************************************/
static void AddrAbsUriClean( IN MsgAddrAbsUri* pAddress)
{
    (pAddress)->strScheme     = UNDEFINED;
    (pAddress)->strIdentifier = UNDEFINED;

#ifdef SIP_DEBUG
    (pAddress)->pScheme     = NULL;
    (pAddress)->pIdentifier = NULL;
#endif
}

#ifdef __cplusplus
}
#endif


