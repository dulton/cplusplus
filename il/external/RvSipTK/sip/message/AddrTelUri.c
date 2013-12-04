/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrTelUri.c                                   *
 *                                                                            *
 * The file implements internal functions of the Tel-Uri object.              *
 * The methods are construct,destruct and copy an object.It also implements   *
 * the encode and parse methods.                                              *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Tamar Barzuza     Feb.2005                                            *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_TEL_URI_SUPPORT

#include "AddrTelUri.h"
#include "_SipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipCommonUtils.h"
#include <string.h>
#include <stdlib.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void AddrTelUriClean( IN MsgAddrTelUri* pAddress);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddrTelUriConstruct
 * ------------------------------------------------------------------------
 * General: The function construct an Tel-Uri address object, which is 'stand alone'
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
 * output: pTelUri - Handle of the Tel-Uri address object..
 ***************************************************************************/
RvStatus AddrTelUriConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                              IN  HRPOOL            hPool,
                              IN  HPAGE             hPage,
                              OUT MsgAddrTelUri**   pTelUri)
{
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(pTelUri == NULL))
        return RV_ERROR_NULLPTR;

    *pTelUri = NULL;

    *pTelUri = (MsgAddrTelUri*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                    hPool,
                                                    hPage,
                                                    sizeof(MsgAddrTelUri),
                                                    RV_TRUE);
    if(*pTelUri == NULL)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrTelUriConstruct - Failed - No more resources"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    else
    {
        AddrTelUriClean(*pTelUri);


        RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "AddrTelUriConstruct - Tel Uri object was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, pTelUri));

        return RV_OK;
    }
}


/***************************************************************************
 * AddrTelUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one Tel-Uri address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the Tel-Uri address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrTelUriCopy(IN RvSipAddressHandle hDestination,
                         IN RvSipAddressHandle hSource)
{
   MsgAddress*        destAdd;
   MsgAddress*        sourceAdd;
   MsgAddrTelUri*     dest;
   MsgAddrTelUri*     source;

    if((hDestination == NULL)||(hSource == NULL))
        return RV_ERROR_NULLPTR;

    destAdd = (MsgAddress*)hDestination;
    sourceAdd = (MsgAddress*)hSource;

    dest = (MsgAddrTelUri*)destAdd->uAddrBody.pTelUri;
    source = (MsgAddrTelUri*)sourceAdd->uAddrBody.pTelUri;

	/* bool is global phone number */
	dest->bIsGlobalPhoneNumber = source->bIsGlobalPhoneNumber;

	/* enumeration - enumdi type */
	dest->eEnumdiType = source->eEnumdiType;

    /* str Phone Number */
    if(source->strPhoneNumber > UNDEFINED)
    {
        dest->strPhoneNumber = MsgUtilsAllocCopyRpoolString(
														  sourceAdd->pMsgMgr,
                                                          destAdd->hPool,
                                                          destAdd->hPage,
                                                          sourceAdd->hPool,
                                                          sourceAdd->hPage,
                                                          source->strPhoneNumber);
        if(dest->strPhoneNumber == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy phone number"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPhoneNumber = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strPhoneNumber);
#endif
    }
    else
    {
        dest->strPhoneNumber = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPhoneNumber = NULL;
#endif
    }

    /* str Extension */
    if(source->strExtension > UNDEFINED)
    {
        dest->strExtension = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
                                                          destAdd->hPool,
                                                          destAdd->hPage,
                                                          sourceAdd->hPool,
                                                          sourceAdd->hPage,
                                                          source->strExtension);
        if(dest->strExtension == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy Extension"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pExtension = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strExtension);
#endif
    }
    else
    {
        dest->strExtension = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pExtension = NULL;
#endif
    }

	/* str Isdn Sub Addr */
    if(source->strIsdnSubAddr > UNDEFINED)
    {
        dest->strIsdnSubAddr = MsgUtilsAllocCopyRpoolString(
														sourceAdd->pMsgMgr,
														destAdd->hPool,
														destAdd->hPage,
														sourceAdd->hPool,
														sourceAdd->hPage,
														source->strIsdnSubAddr);
        if(dest->strIsdnSubAddr == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy Isdn sub address"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pIsdnSubAddr = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strIsdnSubAddr);
#endif
    }
    else
    {
        dest->strIsdnSubAddr = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIsdnSubAddr = NULL;
#endif
    }

    /* str Post Dial */
    if(source->strPostDial > UNDEFINED)
    {
        dest->strPostDial = MsgUtilsAllocCopyRpoolString(sourceAdd->pMsgMgr,
														 destAdd->hPool,
														 destAdd->hPage,
														 sourceAdd->hPool,
														 sourceAdd->hPage,
														 source->strPostDial);
        if(dest->strPostDial == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy post dial"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPostDial = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strPostDial);
#endif
    }
    else
    {
        dest->strPostDial = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPostDial = NULL;
#endif
    }

	/* str Context */
    if(source->strContext > UNDEFINED)
    {
        dest->strContext = MsgUtilsAllocCopyRpoolString(
													sourceAdd->pMsgMgr,
													destAdd->hPool,
													destAdd->hPage,
													sourceAdd->hPool,
													sourceAdd->hPage,
													source->strContext);
        if(dest->strContext == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy context"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pContext = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strContext);
#endif
    }
    else
    {
        dest->strContext = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pContext = NULL;
#endif
    }

#ifdef RV_SIP_IMS_HEADER_SUPPORT
	/* str Rn */
    if(source->strRn > UNDEFINED)
    {
        dest->strRn = MsgUtilsAllocCopyRpoolString(
													sourceAdd->pMsgMgr,
													destAdd->hPool,
													destAdd->hPage,
													sourceAdd->hPool,
													sourceAdd->hPage,
													source->strRn);
        if(dest->strRn == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy Rn"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pRn = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strRn);
#endif
    }
    else
    {
        dest->strRn = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pRn = NULL;
#endif
    }

    /* str RnContext */
    if(source->strRnContext > UNDEFINED)
    {
        dest->strRnContext = MsgUtilsAllocCopyRpoolString(
													sourceAdd->pMsgMgr,
													destAdd->hPool,
													destAdd->hPage,
													sourceAdd->hPool,
													sourceAdd->hPage,
													source->strRnContext);
        if(dest->strRnContext == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy RnContext"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pRnContext = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strRnContext);
#endif
    }
    else
    {
        dest->strRnContext = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pRnContext = NULL;
#endif
    }

    /* str Cic */
    if(source->strCic > UNDEFINED)
    {
        dest->strCic = MsgUtilsAllocCopyRpoolString(
													sourceAdd->pMsgMgr,
													destAdd->hPool,
													destAdd->hPage,
													sourceAdd->hPool,
													sourceAdd->hPage,
													source->strCic);
        if(dest->strCic == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy Cic"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCic = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strCic);
#endif
    }
    else
    {
        dest->strCic = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCic = NULL;
#endif
    }

    /* str CicContext */
    if(source->strCicContext > UNDEFINED)
    {
        dest->strCicContext = MsgUtilsAllocCopyRpoolString(
													sourceAdd->pMsgMgr,
													destAdd->hPool,
													destAdd->hPage,
													sourceAdd->hPool,
													sourceAdd->hPage,
													source->strCicContext);
        if(dest->strCicContext == UNDEFINED)
        {
            RvLogError(sourceAdd->pMsgMgr->pLogSrc,(sourceAdd->pMsgMgr->pLogSrc,
                "AddrTelUriCopy - Failed to copy CicContext"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pCicContext = (RvChar *)RPOOL_GetPtr(destAdd->hPool, destAdd->hPage, dest->strCicContext);
#endif
    }
    else
    {
        dest->strCicContext = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pCicContext = NULL;
#endif
    }

    /* Npdi */
    dest->bNpdi = source->bNpdi;

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
                "AddrTelUriCopy - Failed to copy strCPC"));
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
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

    /* str Other Parameters */
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
                "AddrTelUriCopy - Failed to copy other parameters"));
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
 * AddrTelUriEncode
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
RvStatus AddrTelUriEncode(IN  RvSipAddressHandle hAddr,
                          IN  HRPOOL            hPool,
                          IN  HPAGE             hPage,
                          IN  RvBool            bInUrlHeaders,
                          OUT RvUint32*         length)
{
    RvStatus         stat;
    MsgAddress*      pAddr   = (MsgAddress*)hAddr;
    MsgAddrTelUri*   pTelUri;

    if(NULL == hAddr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
             "AddrTelUriEncode - Encoding TEL URI Address. hSipUrl 0x%p, hPool 0x%p, hPage 0x%p",
             hAddr, hPool, hPage));

    if((hPool == NULL)||(hPage == NULL_PAGE)||(length == NULL))
    {
        RvLogWarning(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
            "AddrTelUriEncode - Failed - InvalidParameter. hSipUrl: 0x%p, hPool: 0x%p, hPage: 0x%p",
            hAddr,hPool, hPage));
        return RV_ERROR_BADPARAM;
    }


    pTelUri = pAddr->uAddrBody.pTelUri;

    /* encode the scheme */
    stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "tel:", length);
    if(stat != RV_OK)
    {
        return stat;
    }

	/* if the phone number is global, preceed it by "+" */
	if (RV_TRUE == pTelUri->bIsGlobalPhoneNumber)
	{
		stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetPlusChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
		{
			return stat;
		}
	}

    /* set phone number */
    if(pTelUri->strPhoneNumber > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pTelUri->strPhoneNumber,
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
					"AddrTelUriEncode - Failed. strPhoneNumber is NULL. cannot encode. not optional parameter"));
        return RV_ERROR_BADPARAM;
    }

    /* set TEL URI Params */

	/* context parameter */
    if(pTelUri->strContext > UNDEFINED)
    {
        /* set " ;phone-context=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "phone-context", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set extension parameters */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pTelUri->strContext,
									length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }
	else if (RV_FALSE == pTelUri->bIsGlobalPhoneNumber)
    {
        /* this is not optional for local number */
        RvLogWarning(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
			       "AddrTelUriEncode - strContext is NULL. not optional parameter for local number"));
    }

    /* extension */
    if(pTelUri->strExtension > UNDEFINED)
    {
        /* set " ;ext=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "ext", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set extension parameters */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pAddr->hPool,
                                    pAddr->hPage,
                                    pTelUri->strExtension,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	/* post dial parameter */
    if(pTelUri->strPostDial > UNDEFINED)
    {
        /* set " ;postd=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "postd", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set post dial parameter */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pTelUri->strPostDial,
									length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	/* isdn sub address */
    if(pTelUri->strIsdnSubAddr > UNDEFINED)
    {
        /* set " ;isub=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "isub", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set isdn sub parameter */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pTelUri->strIsdnSubAddr,
									length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

	/* enumdi parameter */
	if (RVSIP_ENUMDI_TYPE_EXISTS_EMPTY == pTelUri->eEnumdiType)
	{
		/* set ";enumdi" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "enumdi", length);
        if(stat != RV_OK)
        {
            return stat;
        }
	}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* CPC */
    if(pTelUri->eCPCType != RVSIP_CPC_TYPE_UNDEFINED)
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
                                        pTelUri->eCPCType, 
                                        pAddr->hPool,
                                        pAddr->hPage,
                                        pTelUri->strCPC,
                                        length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }	
    
    /* Rn parameter */
    if(pTelUri->strRn > UNDEFINED)
    {
        /* set " ;rn=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "rn", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set Rn parameter */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pTelUri->strRn,
									length);
        if(stat != RV_OK)
        {
            return stat;
        }

        /* RnContext parameter */
        if(pTelUri->strRnContext > UNDEFINED)
        {
            /* set " ;rn-context=" */
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											    MsgUtilsGetSemiColonChar(bInUrlHeaders),
											    length);
		    if(stat != RV_OK)
            {
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "rn-context", length);
            if(stat != RV_OK)
            {
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											    MsgUtilsGetEqualChar(bInUrlHeaders),
											    length);
		    if(stat != RV_OK)
            {
                return stat;
            }

            /* set RnContext parameter */
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									    hPool,
									    hPage,
									    pAddr->hPool,
									    pAddr->hPage,
									    pTelUri->strRnContext,
									    length);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
    }

    /* Cic parameter */
    if(pTelUri->strCic > UNDEFINED)
    {
        /* set " ;cic=" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "cic", length);
        if(stat != RV_OK)
        {
            return stat;
        }

        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders),
											length);
		if(stat != RV_OK)
        {
            return stat;
        }

        /* set Cic parameter */
        stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									hPool,
									hPage,
									pAddr->hPool,
									pAddr->hPage,
									pTelUri->strCic,
									length);
        if(stat != RV_OK)
        {
            return stat;
        }

        /* CicContext parameter */
        if(pTelUri->strCicContext > UNDEFINED)
        {
            /* set " ;cic-context=" */
            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											    MsgUtilsGetSemiColonChar(bInUrlHeaders),
											    length);
		    if(stat != RV_OK)
            {
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "cic-context", length);
            if(stat != RV_OK)
            {
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
											    MsgUtilsGetEqualChar(bInUrlHeaders),
											    length);
		    if(stat != RV_OK)
            {
                return stat;
            }

            /* set CicContext parameter */
            stat = MsgUtilsEncodeString(pAddr->pMsgMgr,
									    hPool,
									    hPage,
									    pAddr->hPool,
									    pAddr->hPage,
									    pTelUri->strCicContext,
									    length);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
    }

    /* Npdi */
    if(pTelUri->bNpdi == RV_TRUE)
    {
        /* set " ;npdi" */
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pAddr->pMsgMgr, hPool, hPage, "npdi", length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
    
    /* other params */
    if(pTelUri->strOtherParams > UNDEFINED)
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
                                    pTelUri->strOtherParams,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * AddrTelUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse an Tel-Uri address text and construct
 *           a MsgAddrTelUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the Tel-Uri address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrTelUriParse(IN RvSipAddressHandle hSipAddr,
                         IN RvChar*           buffer)
{
    MsgAddress*        pAddress = (MsgAddress*)hSipAddr;

    if (pAddress == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    AddrTelUriClean(pAddress->uAddrBody.pTelUri);

    return MsgParserParseStandAloneAddress(pAddress->pMsgMgr, 
                                          SIP_PARSETYPE_TELURIADDRESS, 
                                          buffer, 
                                          (void*)hSipAddr);

}


/***************************************************************************
 * AddrTelUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two tel-uri objects:
 *           TEL URIs are compared according to the following rules:
 *           1. String-wise comparison of the parameters that the stack
 *              identifies and parses (phone number, Extension, isdn, post dial
 *              and context.
 *           2. No comparison of other parameters.
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrTelUriIsEqual(IN RvSipAddressHandle pHeader,
                         IN RvSipAddressHandle pOtherHeader)
{
    MsgAddrTelUri*    pTelUri;
    MsgAddrTelUri*    pOtherTelUri;
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

    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    pOtherTelUri = (MsgAddrTelUri*)pOtherAddr->uAddrBody.pTelUri;

    /* bIsGlobalPhoneNumber */
	if (pTelUri->bIsGlobalPhoneNumber != pOtherTelUri->bIsGlobalPhoneNumber)
	{
		return RV_FALSE;
	}

    /* eEnumdiType */
	if (pTelUri->eEnumdiType != pOtherTelUri->eEnumdiType)
	{
		return RV_FALSE;
	}

    /* strPhoneNumber */
    if(pTelUri->strPhoneNumber > UNDEFINED)
    {
        if (pOtherTelUri->strPhoneNumber > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strPhoneNumber,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strPhoneNumber)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strPhoneNumber > UNDEFINED)
    {
        return RV_FALSE;
    }

    /* strExtension */
    if(pTelUri->strExtension > UNDEFINED)
    {
        if(pOtherTelUri->strExtension > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strExtension,
                             pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strExtension)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strExtension > UNDEFINED)
    {
        return RV_FALSE;
    }

	/* strIsdnSubAddr */
    if(pTelUri->strIsdnSubAddr > UNDEFINED)
    {
        if(pOtherTelUri->strIsdnSubAddr > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strIsdnSubAddr,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strIsdnSubAddr)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strIsdnSubAddr > UNDEFINED)
    {
        return RV_FALSE;
    }

	/* strPostDial */
    if(pTelUri->strPostDial > UNDEFINED)
    {
        if(pOtherTelUri->strPostDial > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strPostDial,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strPostDial)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strPostDial > UNDEFINED)
    {
        return RV_FALSE;
    }

	/* strContext */
    if(pTelUri->strContext > UNDEFINED)
    {
        if(pOtherTelUri->strContext > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strContext,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strContext)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strContext > UNDEFINED)
    {
        return RV_FALSE;
    }

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* CPC */
    if((pTelUri->eCPCType      != RVSIP_CPC_TYPE_UNDEFINED) && 
       (pOtherTelUri->eCPCType != RVSIP_CPC_TYPE_UNDEFINED))
    {
        /* both parameters are present */
        if(pTelUri->eCPCType != pOtherTelUri->eCPCType)
        {
            /* CPC param is not equal */
            return RV_FALSE;
        }
        else if(pTelUri->eCPCType == RVSIP_CPC_TYPE_OTHER)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strCPC,
                            pOtherAddr->hPool, pOtherAddr->hPage,
                            pOtherTelUri->strCPC)!= RV_TRUE)
            {
                /* CPC param is not equal */
                return RV_FALSE;
            }
        }
    }

    /* strRn */
    if(pTelUri->strRn > UNDEFINED)
    {
        if(pOtherTelUri->strRn > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strRn,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strRn)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strRn > UNDEFINED)
    {
        return RV_FALSE;
    }
    
    /* strRnContext */
    if(pTelUri->strRnContext > UNDEFINED)
    {
        if(pOtherTelUri->strRnContext > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strRnContext,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strRnContext)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strRnContext > UNDEFINED)
    {
        return RV_FALSE;
    }

    /* strCic */
    if(pTelUri->strCic > UNDEFINED)
    {
        if(pOtherTelUri->strCic > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strCic,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strCic)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strCic > UNDEFINED)
    {
        return RV_FALSE;
    }
    
    /* strCicContext */
    if(pTelUri->strCicContext > UNDEFINED)
    {
        if(pOtherTelUri->strCicContext > UNDEFINED)
        {
            if(RPOOL_Stricmp(pAddr->hPool, pAddr->hPage, pTelUri->strCicContext,
				pOtherAddr->hPool, pOtherAddr->hPage, pOtherTelUri->strCicContext)!= RV_TRUE)
            {
                return RV_FALSE;
            }
        }
        else
        {
            return RV_FALSE;
        }
    }
    else if (pOtherTelUri->strCicContext > UNDEFINED)
    {
        return RV_FALSE;
    }

    /* bNpdi */
	if (pTelUri->bNpdi != pOtherTelUri->bNpdi)
	{
		return RV_FALSE;
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    return RV_TRUE;
}


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
                                IN    RvInt32            strPhoneNumberOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strPhoneNumberOffset, strPhoneNumber,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetPhoneNumber - Failed to set phone number"));
        return retStatus;
    }
    pTelUri->strPhoneNumber = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pPhoneNumber = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												   pTelUri->strPhoneNumber);
#endif

    return RV_OK;
}

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
                                IN    RvInt32            strExtensionOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strExtensionOffset, strExtension,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetExtension - Failed to set Extension"));
        return retStatus;
    }
    pTelUri->strExtension = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pExtension = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												 pTelUri->strExtension);
#endif

    return RV_OK;
}

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
                                IN    RvInt32            strIsdnSubAddrOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strIsdnSubAddrOffset, strIsdnSubAddr,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetIsdnSubAddr - Failed to set Isdn sub address"));
        return retStatus;
    }
    pTelUri->strIsdnSubAddr = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pIsdnSubAddr = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												   pTelUri->strIsdnSubAddr);
#endif

    return RV_OK;
}

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
                                IN    RvInt32            strPostDialOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strPostDialOffset, strPostDial,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetPostDial - Failed to set post dial"));
        return retStatus;
    }
    pTelUri->strPostDial = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pPostDial = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strPostDial);
#endif

    return RV_OK;
}

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
                                IN    RvInt32            strContextOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strContextOffset, strContext,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetContext - Failed to set context"));
        return retStatus;
    }
    pTelUri->strContext = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pContext = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strContext);
#endif

    return RV_OK;
}

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
                                IN    RvInt32            strOtherParamsOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strOtherParamsOffset, strOtherParams,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetOtherParams - Failed to set other parameters"));
        return retStatus;
    }
    pTelUri->strOtherParams = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pOtherParams = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												   pTelUri->strOtherParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrTelUriGetPhoneNumber
 * ------------------------------------------------------------------------
 * General:  This method returns the phone-number field from the Address object.
 * Return Value: phone-number offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetPhoneNumber(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strPhoneNumber;
}

/***************************************************************************
 * SipAddrTelUriGetExtension
 * ------------------------------------------------------------------------
 * General:  This method returns the Extension field from the Address object.
 * Return Value: Extension offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetExtension(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strExtension;
}

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
RvInt32 SipAddrTelUriGetIsdnSubAddr(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strIsdnSubAddr;
}

/***************************************************************************
 * SipAddrTelUriGetPostDial
 * ------------------------------------------------------------------------
 * General:  This method returns the post-dial field from the Address object.
 * Return Value: Post-dial offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetPostDial(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strPostDial;
}

/***************************************************************************
 * SipAddrTelUriGetContext
 * ------------------------------------------------------------------------
 * General:  This method returns the context field from the Address object.
 * Return Value: Context offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetContext(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strContext;
}

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
RvInt32 SipAddrTelUriGetOtherParams(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strOtherParams;
}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
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
                                  IN  RvInt32           strCPCOffset)
{
    MsgAddrTelUri* pTelUri;
    RvInt32     newStrOffset;
    MsgAddress* pAddr = (MsgAddress*)hSipAddr;
    RvStatus    retStatus;
    
    if(hSipAddr == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pTelUri = (MsgAddrTelUri*)pAddr->uAddrBody.hUrl;

    pTelUri->eCPCType = eCPC;

    if(eCPC == RVSIP_CPC_TYPE_OTHER)
    {
        retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strCPCOffset,
                                      strCPC, pAddr->hPool,
                                      pAddr->hPage, &newStrOffset);
        if (RV_OK != retStatus)
        {
            RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                      "SipAddrTelUriSetCPC - Failed to set strCPC"));
            return retStatus;
        }
        pTelUri->strCPC = newStrOffset;
#ifdef SIP_DEBUG
        pTelUri->pCPC = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
                                        pTelUri->strCPC);
#endif
    }
    else
    {
        pTelUri->strCPC = UNDEFINED;
#ifdef SIP_DEBUG
        pTelUri->pCPC = NULL;
#endif
    }

    return RV_OK;
}

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
                                           IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri* pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return pTelUri->strCPC;
}

/***************************************************************************
 * SipAddrTelUriGetRn
 * ------------------------------------------------------------------------
 * General:  This method returns the Rn field from the Address object.
 * Return Value: Rn offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetRn(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strRn;
}

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
                                IN    RvInt32            strRnOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strRnOffset, strRn,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetRn - Failed to set Rn"));
        return retStatus;
    }
    pTelUri->strRn = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pRn = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strRn);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrTelUriGetRnContext
 * ------------------------------------------------------------------------
 * General:  This method returns the RnContext field from the Address object.
 * Return Value: RnContext offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetRnContext(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strRnContext;
}

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
                                IN    RvInt32            strRnContextOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strRnContextOffset, strRnContext,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetRnContext - Failed to set RnContext"));
        return retStatus;
    }
    pTelUri->strRnContext = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pRnContext = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strRnContext);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrTelUriGetCic
 * ------------------------------------------------------------------------
 * General:  This method returns the Cic field from the Address object.
 * Return Value: Cic offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetCic(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strCic;
}

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
                                IN    RvInt32            strCicOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strCicOffset, strCic,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetCic - Failed to set Cic"));
        return retStatus;
    }
    pTelUri->strCic = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pCic = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strCic);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipAddrTelUriGetCicContext
 * ------------------------------------------------------------------------
 * General:  This method returns the CicContext field from the Address object.
 * Return Value: CicContext offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr - Handle of the address object.
 ***************************************************************************/
RvInt32 SipAddrTelUriGetCicContext(IN RvSipAddressHandle hSipAddr)
{
    MsgAddrTelUri *pTelUri;

    if(hSipAddr == NULL)
        return UNDEFINED;

    pTelUri = ((MsgAddress*)hSipAddr)->uAddrBody.pTelUri;

    return ((MsgAddrTelUri*)pTelUri)->strCicContext;
}

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
                                IN    RvInt32            strCicContextOffset)
{
    RvInt32         newStrOffset;
    MsgAddrTelUri*  pTelUri;
    MsgAddress*     pAddr;
    RvStatus        retStatus;

    if(hSipAddr == NULL)
        return RV_ERROR_NULLPTR;

    pAddr    = (MsgAddress*)hSipAddr;
    pTelUri  = (MsgAddrTelUri*)pAddr->uAddrBody.pTelUri;

    retStatus = MsgUtilsSetString(pAddr->pMsgMgr, hPool, hPage, strCicContextOffset, strCicContext,
                                  pAddr->hPool, pAddr->hPage, &newStrOffset);
    if (RV_OK != retStatus)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                  "SipAddrTelUriSetCicContext - Failed to set CicContext"));
        return retStatus;
    }
    pTelUri->strCicContext = newStrOffset;
#ifdef SIP_DEBUG
    pTelUri->pCicContext = (RvChar *)RPOOL_GetPtr(pAddr->hPool, pAddr->hPage,
												pTelUri->strCicContext);
#endif

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddrTelUriClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 ***************************************************************************/
static void AddrTelUriClean( IN MsgAddrTelUri* pAddress)
{
	pAddress->strPhoneNumber		= UNDEFINED;
	pAddress->strExtension			= UNDEFINED;
	pAddress->strIsdnSubAddr		= UNDEFINED;
	pAddress->strPostDial			= UNDEFINED;
	pAddress->strOtherParams		= UNDEFINED;
	pAddress->strContext			= UNDEFINED;

	pAddress->bIsGlobalPhoneNumber  = RV_FALSE;

	pAddress->eEnumdiType           = RVSIP_ENUMDI_TYPE_UNDEFINED;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pAddress->eCPCType              = RVSIP_CPC_TYPE_UNDEFINED;
    pAddress->strCPC        		= UNDEFINED;
    pAddress->strRn        		    = UNDEFINED;
    pAddress->strRnContext  		= UNDEFINED;
    pAddress->strCic       		    = UNDEFINED;
    pAddress->strCicContext	    	= UNDEFINED;
	pAddress->bNpdi                 = RV_FALSE;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef SIP_DEBUG
    pAddress->pPhoneNumber		= NULL;
	pAddress->pExtension		= NULL;
	pAddress->pIsdnSubAddr		= NULL;
	pAddress->pPostDial			= NULL;
	pAddress->pOtherParams		= NULL;
	pAddress->pContext			= NULL;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pAddress->pCPC        		= NULL;
    pAddress->pRn               = NULL;
    pAddress->pRnContext  		= NULL;
    pAddress->pCic       		= NULL;
    pAddress->pCicContext	    = NULL;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif
}

#endif /* RV_SIP_TEL_URI_SUPPORT */

#ifdef __cplusplus
}
#endif


