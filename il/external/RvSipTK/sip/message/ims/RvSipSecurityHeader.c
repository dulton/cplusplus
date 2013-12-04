/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                  RvSipSecurityHeader.c									  *
 *                                                                            *
 * The file defines the methods of the Security header object:				  *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ----------                                            *
 *      Mickey           Dec.2005                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_HEADER_SUPPORT

#include "RvSipSecurityHeader.h"
#include "_SipSecurityHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "_SipCommonUtils.h"
#include "_SipMsgUtils.h"
#include "rvansi.h"
#include <string.h>


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void SecurityHeaderClean(IN MsgSecurityHeader* pHeader,
								IN RvBool             bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipSecurityHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Security header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderConstructInMsg(
                                       IN  RvSipMsgHandle				hSipMsg,
                                       IN  RvBool						pushHeaderAtHead,
                                       OUT RvSipSecurityHeaderHandle*	hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus					stat;

    if(hSipMsg == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipSecurityHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
	{
        return stat;
	}

    if(pushHeaderAtHead == RV_TRUE)
	{
        location = RVSIP_FIRST_HEADER;
	}
    else
	{
        location = RVSIP_LAST_HEADER;
	}

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_SECURITY,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipSecurityHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Security Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipSecurityHeaderHandle* hHeader)
{
    MsgSecurityHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(hHeader == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hHeader = NULL;

    pHeader = (MsgSecurityHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgSecurityHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipSecurityHeaderConstruct - Failed to construct Security header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    SecurityHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipSecurityHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipSecurityHeaderConstruct - Security header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Security header object to a destination Security
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Security header object.
 *    hSource      - Handle to the destination Security header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderCopy(
                                         INOUT	RvSipSecurityHeaderHandle hDestination,
                                         IN		RvSipSecurityHeaderHandle hSource)
{
    MsgSecurityHeader*   source;
    MsgSecurityHeader*   dest;

    if((hDestination == NULL) || (hSource == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    source = (MsgSecurityHeader*)hSource;
    dest   = (MsgSecurityHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

	/* Security Header Type */
	dest->eSecurityHeaderType = source->eSecurityHeaderType;
    
	/* Mechanism Type */
	dest->eMechanismType = source->eMechanismType;
    if(source->eMechanismType == RVSIP_SECURITY_MECHANISM_TYPE_OTHER)
    {
        dest->strMechanismType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strMechanismType);
        if(dest->strMechanismType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - Failed to copy strMechanismType"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pMechanismType */
#ifdef SIP_DEBUG
        dest->pMechanismType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strMechanismType);
#endif
    }
    else
    {
        dest->strMechanismType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMechanismType = NULL;
#endif
    }

	/* strPreference */
	if(source->strPreference > UNDEFINED)
    {
        dest->strPreference = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strPreference);
        if(dest->strPreference == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - Failed to copy strPreference"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pPreference */
#ifdef SIP_DEBUG
        dest->pPreference = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strPreference);
#endif
    }
    else
    {
        dest->strPreference = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPreference = NULL;
#endif
    }
	
	/* eDigestAlgorithm */
	dest->eDigestAlgorithm = source->eDigestAlgorithm;
	
	/* strDigestAlgorithm */
	if(source->strDigestAlgorithm > UNDEFINED)
    {
        dest->strDigestAlgorithm = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strDigestAlgorithm);
        if(dest->strDigestAlgorithm == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - Failed to copy strDigestAlgorithm"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pDigestAlgorithm */
#ifdef SIP_DEBUG
        dest->pDigestAlgorithm = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDigestAlgorithm);
#endif
    }
    else
    {
        dest->strDigestAlgorithm = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDigestAlgorithm = NULL;
#endif
    }
	
	/* AKA Version */
    dest->nAKAVersion = source->nAKAVersion;

	/* strDigestQop */
	dest->eDigestQop = source->eDigestQop;

	if(source->strDigestQop > UNDEFINED)
    {
        dest->strDigestQop = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strDigestQop);
        if(dest->strDigestQop == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - Failed to copy strDigestQop"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pDigestQop */
#ifdef SIP_DEBUG
        dest->pDigestQop = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDigestQop);
#endif
    }
    else
    {
        dest->strDigestQop = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDigestQop = NULL;
#endif
    }

	/* strDigestVerify */
	if(source->strDigestVerify > UNDEFINED)
    {
        dest->strDigestVerify = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strDigestVerify);
        if(dest->strDigestVerify == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - Failed to copy strDigestVerify"));
            return RV_ERROR_OUTOFRESOURCES;
        }

		/* Copy the pDigestVerify */
#ifdef SIP_DEBUG
        dest->pDigestVerify = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDigestVerify);
#endif
    }
    else
    {
        dest->strDigestVerify = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDigestVerify = NULL;
#endif
    }
	
	/* Algorithm */
	dest->eAlgorithm = source->eAlgorithm;

	/* Protocol */
	dest->eProtocol = source->eProtocol;

	/* Mode */
	dest->eMode = source->eMode;

	/* EncryptAlgorithm */
	dest->eEncryptAlgorithm = source->eEncryptAlgorithm;

	/* Spic */
	dest->spiC.bInitialized = source->spiC.bInitialized;
	dest->spiC.value = source->spiC.value;

	/* SpiS */
	dest->spiS.bInitialized = source->spiS.bInitialized;
	dest->spiS.value = source->spiS.value;

	/* portC */
	dest->portC = source->portC;

	/* portS */
	dest->portS = source->portS;

    /* strOtherParams */
    if(source->strOtherParams > UNDEFINED)
    {
        dest->strOtherParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strOtherParams);
        if(dest->strOtherParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipSecurityHeaderCopy - didn't manage to copy OtherParams"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pOtherParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strOtherParams);
#endif
    }
    else
    {
        dest->strOtherParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pOtherParams = NULL;
#endif
    }

    /* bad syntax */
    if(source->strBadSyntax > UNDEFINED)
    {
        dest->strBadSyntax = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                            dest->hPool,
                                                            dest->hPage,
                                                            source->hPool,
                                                            source->hPage,
                                                            source->strBadSyntax);
        if(dest->strBadSyntax == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderCopy - failed in copying strBadSyntax."));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pBadSyntax = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strBadSyntax);
#endif
    }
    else
    {
        dest->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBadSyntax = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a Security header object to a textual Security header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Security header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderEncode(
                                          IN    RvSipSecurityHeaderHandle hHeader,
                                          IN    HRPOOL                    hPool,
                                          OUT   HPAGE*                    hPage,
                                          OUT   RvUint32*				  length)
{
    RvStatus stat;
    MsgSecurityHeader* pHeader;

    pHeader = (MsgSecurityHeader*)hHeader;

    if(hPage == NULL || length == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL) || (hPool == NULL))
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = SecurityHeaderEncode(hHeader, hPool, *hPage, RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderEncode - Failed. Free page 0x%p on pool 0x%p. SecurityHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * SecurityHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Security header (as string) on it.
 *          example format: "Security-Server: mechanism-name; mech-parameters, ..."
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Security header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV SecurityHeaderEncode(
                                  IN    RvSipSecurityHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length)
{
    MsgSecurityHeader*  pHeader;
    RvStatus			stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
	{
        return RV_ERROR_BADPARAM;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "SecurityHeaderEncode - Encoding Security header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* set "Security-Client", "Security-Server" or "Security-Verify" */
    if(pHeader->eSecurityHeaderType == RVSIP_SECURITY_CLIENT_HEADER)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Security-Client", length);
    }
	else if(pHeader->eSecurityHeaderType == RVSIP_SECURITY_SERVER_HEADER)
	{
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Security-Server", length);
	}
	else if(pHeader->eSecurityHeaderType == RVSIP_SECURITY_VERIFY_HEADER)
	{
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Security-Verify", length);
	}
	else
	{
		RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SecurityHeaderEncode - Failed to encode Security header. stat is %d",
            RV_ERROR_BADPARAM));
        return RV_ERROR_BADPARAM;
	}
	
    if(RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SecurityHeaderEncode - Failed to encode Security header. stat is %d",
            stat));
        return stat;
    }
	
    /* encode ": " or "=" */
    stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                        MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders),
                                        length);
    /* bad - syntax */
    if(pHeader->strBadSyntax > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strBadSyntax,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SecurityHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
			return stat;
        }
        
    }

    /* Mechanism Type */
	stat = MsgUtilsEncodeSecurityMechanismType(pHeader->pMsgMgr,
												hPool,
												hPage,
												pHeader->eMechanismType,
												pHeader->hPool,
												pHeader->hPage,
												pHeader->strMechanismType,
												length);
	if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SecurityHeaderEncode - Failed to encode Mechanism Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
            stat, hPool, hPage));
		return stat;
    }
    
	/* preference (insert ";" in the beginning) */
    if(pHeader->strPreference > UNDEFINED)
    {
        /* set ";q=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "q", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the preference string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strPreference,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode preference String. stat is %d",
				stat));
		}
    }

	/* d-alg (insert ";" in the beginning) */
    if(pHeader->eDigestAlgorithm != RVSIP_AUTH_ALGORITHM_UNDEFINED)
    {
        /* set ";d-alg=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "d-alg", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

		/* Check the AKA Version and add it to the header if defined */
		if(pHeader->nAKAVersion > UNDEFINED)
		{
			RvChar strHelper[32];
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "AKAv", length);
			
			MsgUtils_itoa(pHeader->nAKAVersion, strHelper);
			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);

			stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "-", length);

            if(stat != RV_OK)
			{
				RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SecurityHeaderEncode - Failed to encode Security header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
                return stat;
			}
		}
		
        /* set the d-alg string */
        stat = MsgUtilsEncodeAuthAlgorithm(pHeader->pMsgMgr,
                                           hPool,
                                           hPage,
                                           pHeader->eDigestAlgorithm,
                                           pHeader->hPool,
                                           pHeader->hPage,
                                           pHeader->strDigestAlgorithm,
                                           length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode d-alg String. stat is %d",
				stat));
		}
    }
    
	/* d-qop (insert ";" in the beginning) */
    if (pHeader->eDigestQop != RVSIP_AUTH_QOP_UNDEFINED)
    {
		/* set ";d-qop=" in the beginning */
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											"d-qop", length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
											MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

		if (pHeader->eDigestQop == RVSIP_AUTH_QOP_OTHER)
		{
			if(pHeader->strDigestQop > UNDEFINED)
			{
				/* set the d-qop string */
				stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
											hPool,
											hPage,
											pHeader->hPool,
											pHeader->hPage,
											pHeader->strDigestQop,
											length);
				if(stat != RV_OK)
				{
					RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
						"SecurityHeaderEncode - Failed to encode d-qop String. stat is %d",
						stat));
				}
			}
		}
		else
		{
			stat = MsgUtilsEncodeQopOptions(pHeader->pMsgMgr, hPool, hPage, pHeader->eDigestQop, length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "SecurityHeaderEncode - Failed to encode Security header. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                    stat, hPool, hPage));
                return stat;
            }
		}
    }
	
	/* d-ver (insert ";" in the beginning) */
    if(pHeader->strDigestVerify > UNDEFINED)
    {
        /* set ";d-ver=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "d-ver", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the d-ver string */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDigestVerify,
                                    length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode d-ver String. stat is %d",
				stat));
		}
    }

	/* Algorithm */
	if(pHeader->eAlgorithm > RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED)
    {
        /* set ";alg=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "alg", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeSecurityAlgorithmType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eAlgorithm,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Algorithm Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}

	/* Protocol */
	if(pHeader->eProtocol > RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED)
    {
        /* set ";prot=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "prot", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeSecurityProtocolType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eProtocol,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Protocol Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}

	/* Mode */
	if(pHeader->eMode > RVSIP_SECURITY_MODE_TYPE_UNDEFINED)
    {
        /* set ";mod=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "mod", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeSecurityModeType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eMode,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Mode Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}

	/* EncryptAlgorithm */
	if(pHeader->eEncryptAlgorithm > RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED)
    {
        /* set ";ealg=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "ealg", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeSecurityEncryptAlgorithmType(pHeader->pMsgMgr,
													hPool,
													hPage,
													pHeader->eEncryptAlgorithm,
													length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Encrypt Algorithm Type string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
				stat, hPool, hPage));
			return stat;
		}
	}
	
	/* Spic (insert ";" in the beginning) */
    if(pHeader->spiC.bInitialized == RV_TRUE)
    {
		RvChar strHelper[16];

        /* set ";spi-c=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "spi-c", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the Spic string */
		RvSprintf(strHelper,"%u",pHeader->spiC.value);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											strHelper, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Spic String. stat is %d",
				stat));
		}
    }
	
	/* SpiS (insert ";" in the beginning) */
    if(pHeader->spiS.bInitialized == RV_TRUE)
    {
		RvChar strHelper[16];

        /* set ";spi-s=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "spi-s", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the SpiS string */
		RvSprintf(strHelper,"%u",pHeader->spiS.value);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											strHelper, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode SpiS String. stat is %d",
				stat));
		}
    }

	/* portC (insert ";" in the beginning) */
    if(pHeader->portC > UNDEFINED)
    {
		RvChar strHelper[16];

        /* set ";port-c=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "port-c", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the portC string */
		MsgUtils_itoa(pHeader->portC, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											strHelper, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode portC String. stat is %d",
				stat));
		}
    }

	/* portS (insert ";" in the beginning) */
    if(pHeader->portS > UNDEFINED)
    {
		RvChar strHelper[16];

        /* set ";port-s=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
		stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            "port-s", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);

		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode Equal character. stat is %d",
				stat));
		}

        /* set the portS string */
		MsgUtils_itoa(pHeader->portS, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
											strHelper, length);
		if(stat != RV_OK)
		{
			RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
				"SecurityHeaderEncode - Failed to encode portS String. stat is %d",
				stat));
		}
    }
	
	/* set the OtherParams. (insert ";" in the beginning) */
    if(pHeader->strOtherParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set OtherParmas */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strOtherParams,
                                    length);
    }

    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "SecurityHeaderEncode - Failed to encoding Security header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipSecurityHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Security header-for example,
 *          "Security:sip:172.20.5.3:5060"-into a Security header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Security header object.
 *    buffer    - Buffer containing a textual Security header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderParse(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgSecurityHeader*	pHeader = (MsgSecurityHeader*)hHeader;
	RvStatus			rv;

    if(hHeader == NULL || (buffer == NULL))
	{
		return RV_ERROR_BADPARAM;
	}

    SecurityHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_SECURITY_CLIENT,
                                        buffer,
                                        RV_FALSE,
                                         (void*)hHeader);
    if(rv == RV_OK)
    {
        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
    }
    return rv;
}

/***************************************************************************
 * RvSipSecurityHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Security header value into an Security header object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipSecurityHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Security header object.
 *    buffer    - The buffer containing a textual Security header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderParseValue(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgSecurityHeader*	pHeader = (MsgSecurityHeader*)hHeader;
	SipParseType		eParseType;
	RvStatus			rv;
	
    if(hHeader == NULL || (buffer == NULL))
	{
		return RV_ERROR_BADPARAM;
	}

	switch(pHeader->eSecurityHeaderType) {
	case RVSIP_SECURITY_CLIENT_HEADER:
		{
			eParseType = SIP_PARSETYPE_SECURITY_CLIENT;
			break;
		}
	case RVSIP_SECURITY_SERVER_HEADER:
		{
			eParseType = SIP_PARSETYPE_SECURITY_SERVER;
			break;
		}
	default:
		{
			eParseType = SIP_PARSETYPE_SECURITY_VERIFY;
			break;
		}
	}
	
    SecurityHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        eParseType,
                                        buffer,
                                        RV_TRUE,
                                         (void*)hHeader);
    
	
    if(rv == RV_OK)
    {
        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
    }
    return rv;
}

/***************************************************************************
 * RvSipSecurityHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Security header with bad-syntax information.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          Use this function to fix the header. This function parses a given
 *          correct header-value string to the supplied header object.
 *          If parsing succeeds, this function places all fields inside the
 *          object and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string in the header remains as it was.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader      - The handle to the header object.
 *        pFixedBuffer - The buffer containing a legal header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderFix(
                                     IN RvSipSecurityHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgSecurityHeader* pHeader = (MsgSecurityHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    rv = RvSipSecurityHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSecurityHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipSecurityHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * RvSipSecurityHeaderIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two event header objects.
 *         Security headers considered equal if all parameters, excluding extension parameters,
 *         are equal (case insensitive).
 *         The string parameters are compared byte-by-byte,
 *         A header containing a parameter never matches a header without that parameter.
 * Return Value: Returns RV_TRUE if the Security header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - A handle to the Security Header object.
 *    hOtherHeader - Handle to the Security header object with which a comparison is being made.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipSecurityHeaderIsEqual(
                                 IN  const RvSipSecurityHeaderHandle  hHeader,
                                 IN  const RvSipSecurityHeaderHandle  hOtherHeader)
{
	return SipSecurityHeaderIsEqual(hHeader, hOtherHeader, RV_TRUE, RV_TRUE);
}

/***************************************************************************
 * SipSecurityHeaderIsEqual
 * ------------------------------------------------------------------------
 * General:Compares two event header objects.
 *         Security headers considered equal if all parameters, excluding extension parameters,
 *         are equal (case insensitive).
 *         The string parameters are compared byte-by-byte,
 *         A header containing a parameter never matches a header without that parameter.
 * Return Value: Returns RV_TRUE if the Security header objects being compared are equal.
 *               Otherwise, the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - A handle to the Security Header object.
 *    hOtherHeader - Handle to the Security header object with which a comparison is being made.
 *    bCompareType - If RV_FALSE, the header types (client, server or verify) will not 
 *                   be considered in comparison.
 *    bCompareDigestVerify - If RV_FALSE, the digest-verify parameter will not
 *                           be considered in comparison.
 ***************************************************************************/
RvBool RVCALLCONV SipSecurityHeaderIsEqual(
                                 IN  const RvSipSecurityHeaderHandle  hHeader,
                                 IN  const RvSipSecurityHeaderHandle  hOtherHeader,
								 IN  RvBool                           bCompareType,
								 IN  RvBool                           bCompareDigestVerify)
{
    MsgSecurityHeader*   first = (MsgSecurityHeader*)hHeader;
    MsgSecurityHeader*   other = (MsgSecurityHeader*)hOtherHeader;

    if((hHeader == NULL) || (hOtherHeader == NULL))
	{
        return RV_FALSE;
	}
	
	if (hHeader == hOtherHeader)
	{
		/* this is the same object */
		return RV_TRUE;
	}

	if(first->strBadSyntax != UNDEFINED || other->strBadSyntax != UNDEFINED)
	{
		/* bad syntax string is uncomparable */
		return RV_FALSE;
	}

	if(bCompareType == RV_TRUE && first->eSecurityHeaderType != other->eSecurityHeaderType)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
				"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eSecurityHeaderType is not equal",
				hHeader, hOtherHeader));
		return RV_FALSE;
	}

	if(first->eMechanismType != other->eMechanismType)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eMechanismType is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare strMechanismType */
    if(first->strMechanismType > UNDEFINED)
    {
        if(other->strMechanismType > UNDEFINED)
        {
            if(RPOOL_Stricmp(first->hPool, first->hPage, first->strMechanismType,
                             other->hPool, other->hPage, other->strMechanismType) == RV_FALSE)
            {
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                        "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strMechanismType is not equal!!!",
                        hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strMechanismType is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if(other->strMechanismType > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strMechanismType is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
    }

	/* Compare strPreference */
	if(first->strPreference > UNDEFINED)
	{
		if(other->strPreference > UNDEFINED)
        {
			RvInt32 firstIntPart, firstDecPart, otherIntPart, otherDecPart;
			SipMsgUtilsParseQValue(first->hPool, first->hPage, first->strPreference, NULL, &firstIntPart, &firstDecPart);
			SipMsgUtilsParseQValue(other->hPool, other->hPage, other->strPreference, NULL, &otherIntPart, &otherDecPart);
            
			if(	firstIntPart != otherIntPart ||
				firstDecPart != otherDecPart)
			{
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                        "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strPreference is not equal!!!",
                        hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strPreference is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if(other->strPreference > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestAlgorithm is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
    }

	/* Compare DigestAlgorithm */
	if(first->eDigestAlgorithm != other->eDigestAlgorithm)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eDigestAlgorithm is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}
	
	/* Compare strDigestAlgorithm */
    if(RVSIP_AUTH_ALGORITHM_OTHER == first->eDigestAlgorithm)
	{
		if(first->strDigestAlgorithm > UNDEFINED)
		{
			if(other->strDigestAlgorithm > UNDEFINED)
			{
				if(RPOOL_Stricmp(first->hPool, first->hPage, first->strDigestAlgorithm,
								 other->hPool, other->hPage, other->strDigestAlgorithm) == RV_FALSE)
				{
					RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
							"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestAlgorithm is not equal!!!",
							hHeader, hOtherHeader));
					return RV_FALSE;
				}
			}
			else
			{
				RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestAlgorithm is not equal",
					hHeader, hOtherHeader));
				return RV_FALSE;
			}
		}
		else if(other->strDigestAlgorithm > UNDEFINED)
		{
			RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestAlgorithm is not equal",
					hHeader, hOtherHeader));
			return RV_FALSE;
		}
	}

	/* Compare nAKAVersion */
	if(first->nAKAVersion != other->nAKAVersion)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
            "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. nAKAVersion is not equal",
            hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare DigestQop */
	if(first->eDigestQop != other->eDigestQop)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eDigestQop is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare strDigestQop */
    if(RVSIP_AUTH_QOP_OTHER == first->eDigestQop)
	{
		if(first->strDigestQop > UNDEFINED)
		{	
			if(other->strDigestQop > UNDEFINED)
			{
				if(RPOOL_Stricmp(first->hPool, first->hPage, first->strDigestQop,
								 other->hPool, other->hPage, other->strDigestQop) == RV_FALSE)
				{
					RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
							"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestQop is not equal!!!",
							hHeader, hOtherHeader));
					return RV_FALSE;
				}
			}
			else
			{
				RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestQop is not equal",
					hHeader, hOtherHeader));
				return RV_FALSE;
			}
		}
		else if(other->strDigestQop > UNDEFINED)
		{
			RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestQop is not equal",
					hHeader, hOtherHeader));
			return RV_FALSE;
		}
	}
	
	/* Compare strDigestVerify */
    if(bCompareDigestVerify == RV_TRUE && first->strDigestVerify > UNDEFINED)
    {
        if(other->strDigestVerify > UNDEFINED)
        {
            if(RPOOL_Stricmp(first->hPool, first->hPage, first->strDigestVerify,
                             other->hPool, other->hPage, other->strDigestVerify) == RV_FALSE)
            {
                RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                        "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestVerify is not equal!!!",
                        hHeader, hOtherHeader));
                return RV_FALSE;
            }
        }
        else
        {
            RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestVerify is not equal",
                hHeader, hOtherHeader));
            return RV_FALSE;
        }
    }
    else if (bCompareDigestVerify == RV_TRUE && other->strDigestVerify > UNDEFINED)
    {
        RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. strDigestVerify is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
    }

	/* Compare eAlgorithm */
	if(first->eAlgorithm != other->eAlgorithm)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eAlgorithm is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare eProtocol */
	if(first->eProtocol != other->eProtocol)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eProtocol is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare eMode */
	if(first->eMode != other->eMode)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eMode is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}
	
	/* Compare eEncryptAlgorithm */
	if(first->eEncryptAlgorithm != other->eEncryptAlgorithm)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. eEncryptAlgorithm is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare spiC */
	if(first->spiC.bInitialized == other->spiC.bInitialized)
	{
		if(RV_TRUE == first->spiC.bInitialized && first->spiC.value != other->spiC.value)
		{
			RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. spiC is not equal",
					hHeader, hOtherHeader));
			return RV_FALSE;
		}
	}
	else
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
				"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. spiC is not equal",
				hHeader, hOtherHeader));
		return RV_FALSE;
	}

	/* Compare spiS */
	if(first->spiS.bInitialized == other->spiS.bInitialized)
	{
		if(RV_TRUE == first->spiS.bInitialized && first->spiS.value != other->spiS.value)
		{
			RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
					"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. spiS is not equal",
					hHeader, hOtherHeader));
			return RV_FALSE;
		}
	}
	else
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
				"SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. spiS is not equal",
				hHeader, hOtherHeader));
		return RV_FALSE;
	}

	/* Compare portC */
	if(first->portC != other->portC)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. portC is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* Compare portS */
	if(first->portS != other->portS)
	{
		RvLogInfo(other->pMsgMgr->pLogSrc,(other->pMsgMgr->pLogSrc,
                "SipSecurityHeaderIsEqual - Header 0x%p and header 0x%p are not equal. portS is not equal",
                hHeader, hOtherHeader));
        return RV_FALSE;
	}

	/* The two header are equal */
    return RV_TRUE;
}

/***************************************************************************
 * SipSecurityHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Security object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipSecurityHeaderGetPool(RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipSecurityHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Security object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipSecurityHeaderGetPage(RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Security header fields are kept in a string format-for example, the
 *          Security header VNetworkSpec name. In order to get such a field from the Security header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Security header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipSecurityHeaderGetStringLength(
                                      IN  RvSipSecurityHeaderHandle     hHeader,
                                      IN  RvSipSecurityHeaderStringName stringName)
{
    RvInt32				stringOffset;
    MsgSecurityHeader*	pHeader = (MsgSecurityHeader*)hHeader;

    if(hHeader == NULL)
	{
        return 0;	
	}

    switch (stringName)
    {
        case RVSIP_SECURITY_MECHANISM_TYPE:
        {
            stringOffset = SipSecurityHeaderGetStrMechanismType(hHeader);
            break;
        }
		case RVSIP_SECURITY_PREFERENCE:
        {
            stringOffset = SipSecurityHeaderGetStrPreference(hHeader);
            break;
        }
		case RVSIP_SECURITY_DIGEST_ALGORITHM:
        {
            stringOffset = SipSecurityHeaderGetStrDigestAlgorithm(hHeader);
            break;
        }
		case RVSIP_SECURITY_DIGEST_QOP:
        {
            stringOffset = SipSecurityHeaderGetStrDigestQop(hHeader);
            break;
        }
		case RVSIP_SECURITY_DIGEST_VERIFY:
        {
            stringOffset = SipSecurityHeaderGetStrDigestVerify(hHeader);
            break;
        }
        case RVSIP_SECURITY_OTHER_PARAMS:
        {
            stringOffset = SipSecurityHeaderGetOtherParams(hHeader);
            break;
        }
        case RVSIP_SECURITY_BAD_SYNTAX:
        {
            stringOffset = SipSecurityHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipSecurityHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }

    if(stringOffset > UNDEFINED)
	{
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
	}
    else
	{
        return 0;
	}
}

/***************************************************************************
 * RvSipSecurityHeaderGetSecurityHeaderType
 * ------------------------------------------------------------------------
 * General: Gets the header type enumeration from the Security Header object.
 * Return Value: Returns the Security header type enumeration from the Security
 *               header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipSecurityHeaderType RVCALLCONV RvSipSecurityHeaderGetSecurityHeaderType(
                                   IN RvSipSecurityHeaderHandle hHeader)
{
    MsgSecurityHeader* pHeader;

    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_UNDEFINED_HEADER;
	}

    pHeader = (MsgSecurityHeader*)hHeader;
    return pHeader->eSecurityHeaderType;
}

/***************************************************************************
 * RvSipSecurityHeaderSetSecurityHeaderType
 * ------------------------------------------------------------------------
 * General: Sets the header type in the Security Header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader			  - Handle to the Security header object.
 *    eSecurityHeaderType - The header type to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSecurityHeaderType(
                                 IN    RvSipSecurityHeaderHandle hHeader,
                                 IN    RvSipSecurityHeaderType   eSecurityHeaderType)
{
    MsgSecurityHeader* pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*)hHeader;
    pHeader->eSecurityHeaderType = eSecurityHeaderType;
    return RV_OK;
}

/***************************************************************************
 * SipSecurityHeaderGetStrMechanismType
 * ------------------------------------------------------------------------
 * General:This method gets the StrMechanismType embedded in the Security header.
 * Return Value: StrMechanismType offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrMechanismType(
                                    IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strMechanismType;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrMechanismType
 * ------------------------------------------------------------------------
 * General: Copies the StrMechanismType from the Security header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrMechanismType(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    if(strBuffer == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrMechanismType(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetMechanismType
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pMechanismType in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Security header object.
 *         pMechanismType - The Mechanism Type to be set in the Security header - If
 *                NULL, the existing Mechanism Type string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetMechanismType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityMechanismType	eMechanismType,
                                     IN    RvChar *						pMechanismType,
                                     IN    HRPOOL						hPool,
                                     IN    HPAGE						hPage,
                                     IN    RvInt32						strOffset)
{
    RvInt32				newStr;
    MsgSecurityHeader*	pHeader;
    RvStatus			retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->eMechanismType = eMechanismType;
	
	if(eMechanismType == RVSIP_SECURITY_MECHANISM_TYPE_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pMechanismType,
									  pHeader->hPool, pHeader->hPage, &newStr);
		if(RV_OK != retStatus)
		{
			return retStatus;
		}
		pHeader->strMechanismType = newStr;
#ifdef SIP_DEBUG
	    pHeader->pMechanismType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strMechanismType);
#endif
    }
	else
	{
		pHeader->strMechanismType = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pMechanismType = NULL;
#endif
	}
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetMechanismType
 * ------------------------------------------------------------------------
 * General:Sets the Mechanism Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strMechanismType - The Mechanism Type to be set in the Security header. If NULL is supplied, the
 *                 existing Mechanism Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetMechanismType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityMechanismType	eMechanismType,
                                     IN    RvChar*						strMechanismType)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

     return SipSecurityHeaderSetMechanismType(hHeader, eMechanismType, strMechanismType, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipSecurityHeaderGetMechanismType
 * ------------------------------------------------------------------------
 * General: Copies the StrMechanismType from the Security header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvSipSecurityMechanismType RVCALLCONV RvSipSecurityHeaderGetMechanismType(
                                               IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eMechanismType;
}

/***************************************************************************
 * SipSecurityHeaderGetStrPreference
 * ------------------------------------------------------------------------
 * General:This method gets the Preference in the Security header object.
 * Return Value: Preference offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrPreference(
										  IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strPreference;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrPreference
 * ------------------------------------------------------------------------
 * General: Copies the Security header Preference field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrPreference(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrPreference(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetStrPreference
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the Preference in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pPreference - The Preference to be set in the Security header.
 *                          If NULL, the existing Preference string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrPreference(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pPreference,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgSecurityHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pPreference,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strPreference = newStr;
#ifdef SIP_DEBUG
    pHeader->pPreference = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strPreference);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetStrPreference
 * ------------------------------------------------------------------------
 * General:Sets the Preference field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *    strPreference	- The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrPreference(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pPreference)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipSecurityHeaderSetStrPreference(hHeader, pPreference, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipSecurityHeaderGetStrDigestAlgorithm
 * ------------------------------------------------------------------------
 * General:This method gets the DigestAlgorithm in the Security header object.
 * Return Value: DigestAlgorithm offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestAlgorithm(
                                            IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strDigestAlgorithm;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestAlgorithm field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestAlgorithm(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrDigestAlgorithm(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * RvSipSecurityHeaderGetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Gets the Digest algorithm enumeration from the Security header
 *          object.
 *          if this function returns RVSIP_AUTH_ALGORITHM_OTHER, call
 *          RvSipSecurityHeaderGetStrDigestAlgorithm() to get the algorithm in a
 *          string format.
 * Return Value: Returns the digest algorithm enumeration from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipAuthAlgorithm RVCALLCONV RvSipSecurityHeaderGetDigestAlgorithm(
                                      IN  RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return RVSIP_AUTH_ALGORITHM_UNDEFINED;

    return ((MsgSecurityHeader*)hHeader)->eDigestAlgorithm;
}

/***************************************************************************
 * SipSecurityHeaderSetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the strDigestAlgorithm in the
 *          Security Header object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hHeader          - Handle of the Security header object.
 *        eDigestAlgorithm   - The Security algorithm to be set in the object.
 *        strDigestAlgorithm - text string giving the algorithm type to be set in the object.
 *        strOffset     - Offset of a string on the page  (if relevant).
 *        hPool - The pool on which the string lays (if relevant).
 *        hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetDigestAlgorithm(
                                  IN    RvSipSecurityHeaderHandle	hHeader,
                                  IN    RvSipAuthAlgorithm			eDigestAlgorithm,
                                  IN    RvChar*						strDigestAlgorithm,
                                  IN    HRPOOL						hPool,
                                  IN    HPAGE						hPage,
                                  IN    RvInt32						strOffset)
{
    MsgSecurityHeader	*header;
    RvInt32				newStr;
    RvStatus			retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgSecurityHeader*)hHeader;

    header->eDigestAlgorithm = eDigestAlgorithm;

    if(eDigestAlgorithm == RVSIP_AUTH_ALGORITHM_OTHER)
    {
        retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strOffset, strDigestAlgorithm,
                                      header->hPool, header->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        header->strDigestAlgorithm = newStr;
#ifdef SIP_DEBUG
        header->pDigestAlgorithm = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                         header->strDigestAlgorithm);
#endif
    }
    else
    {
        header->strDigestAlgorithm = UNDEFINED;
#ifdef SIP_DEBUG
        header->pDigestAlgorithm = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetDigestAlgorithm
 * ------------------------------------------------------------------------
 * General: Sets the Digest algorithm in Security header object.
 *          If eDigestAlgorithm is RVSIP_AUTH_SCHEME_OTHER, strDigestAlgorithm is
 *          copied to the header. Otherwise, strDigestAlgorithm is ignored.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hHeader				- Handle to the Security header object.
 *  eDigestAlgorithm	- The Digest algorithm to be set in the object.
 *	strDigestAlgorithm	- Text string giving the Digest algorithm to be set in the object. You can
 *							use this parameter if eDigestAlgorithm is set to
 *							RVSIP_AUTH_ALGORITHM_OTHER. Otherwise, the parameter is set to
 *							NULL.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetDigestAlgorithm(
                                  IN  RvSipSecurityHeaderHandle	hHeader,
                                  IN  RvSipAuthAlgorithm		eDigestAlgorithm,
                                  IN  RvChar					*strDigestAlgorithm)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipSecurityHeaderSetDigestAlgorithm(hHeader,
                                                  eDigestAlgorithm,
                                                  strDigestAlgorithm,
                                                  NULL,
                                                  NULL_PAGE,
                                                  UNDEFINED);

}

/***************************************************************************
 * RvSipSecurityHeaderGetDigestAlgorithmAKAVersion
 * ------------------------------------------------------------------------
 * General: Gets the AKAVersion from the Security header object.
 * Return Value: Returns the AKAVersion, or UNDEFINED if the AKAVersion
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetDigestAlgorithmAKAVersion(
                                         IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgSecurityHeader*)hHeader)->nAKAVersion;
}

/***************************************************************************
 * RvSipSecurityHeaderSetDigestAlgorithmAKAVersion
 * ------------------------------------------------------------------------
 * General:  Sets the AKAVersion number of the Security header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader		- Handle to the Security header object.
 *    AKAVersion	- The AKAVersion number value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetDigestAlgorithmAKAVersion(
                                         IN    RvSipSecurityHeaderHandle	hHeader,
                                         IN    RvInt32						AKAVersion)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgSecurityHeader*)hHeader)->nAKAVersion = AKAVersion;
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetDigestQopOption
 * ------------------------------------------------------------------------
 * General: Gets the Qop option from the Security header object.
 * Return Value: The Qop option from the header object.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvSipAuthQopOption  RVCALLCONV RvSipSecurityHeaderGetDigestQopOption(
                                      IN  RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_AUTH_QOP_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eDigestQop;
}

/***************************************************************************
 * SipSecurityHeaderGetStrDigestQop
 * ------------------------------------------------------------------------
 * General:This method gets the DigestQop in the Security header object.
 * Return Value: DigestQop offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestQop(IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strDigestQop;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestQop
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestQop field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestQop(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrDigestQop(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetStrDigestQop
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DigestQop in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pDigestQop - The DigestQop to be set in the Security header.
 *                          If NULL, the existing DigestQop string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrDigestQop(
                                     IN    RvSipSecurityHeaderHandle hHeader,
									 IN    RvSipAuthQopOption		 eDigestQop,
                                     IN    RvChar *					 pDigestQop,
                                     IN    HRPOOL					 hPool,
                                     IN    HPAGE					 hPage,
                                     IN    RvInt32					 strOffset)
{
    RvInt32          newStr;
    MsgSecurityHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*)hHeader;

	if (eDigestQop == RVSIP_AUTH_QOP_OTHER)
	{
		retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pDigestQop,
									  pHeader->hPool, pHeader->hPage, &newStr);
		if(RV_OK != retStatus)
		{
			return retStatus;
		}
		pHeader->strDigestQop = newStr;
#ifdef SIP_DEBUG
		pHeader->pDigestQop = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
													 pHeader->strDigestQop);
#endif
	}

	pHeader->eDigestQop = eDigestQop;
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetStrDigestQop
 * ------------------------------------------------------------------------
 * General:Sets the DigestQop field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strDigestQop - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrDigestQop(
                                     IN	RvSipSecurityHeaderHandle	hHeader,
									 IN	RvSipAuthQopOption			eDigestQop,
                                     IN	RvChar *					pDigestQop)
{
    	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	if (eDigestQop == RVSIP_AUTH_QOP_AUTH_AND_AUTHINT)
	{
		return RV_ERROR_BADPARAM;
	}

	return SipSecurityHeaderSetStrDigestQop(hHeader, eDigestQop, pDigestQop, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipSecurityHeaderGetStrDigestVerify
 * ------------------------------------------------------------------------
 * General:This method gets the DigestVerify in the Security header object.
 * Return Value: DigestVerify offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrDigestVerify(
                                            IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strDigestVerify;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrDigestVerify
 * ------------------------------------------------------------------------
 * General: Copies the Security header DigestVerify field of the Security header object into a
 *          given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrDigestVerify(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrDigestVerify(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetStrDigestVerify
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the DigestVerify in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pDigestVerify - The DigestVerify to be set in the Security header.
 *                          If NULL, the existing DigestVerify string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetStrDigestVerify(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pDigestVerify,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgSecurityHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pDigestVerify,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strDigestVerify = newStr;
#ifdef SIP_DEBUG
    pHeader->pDigestVerify = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strDigestVerify);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetStrDigestVerify
 * ------------------------------------------------------------------------
 * General:Sets the DigestVerify field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strDigestVerify - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrDigestVerify(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pDigestVerify)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipSecurityHeaderSetStrDigestVerify(hHeader, pDigestVerify, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipSecurityHeaderSetAlgorithmType
 * ------------------------------------------------------------------------
 * General:Sets the Algorithm Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eAlgorithmType - The Algorithm Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Algorithm Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetAlgorithmType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityAlgorithmType	eAlgorithmType)
{
    MsgSecurityHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->eAlgorithm = eAlgorithmType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetAlgorithmType
 * ------------------------------------------------------------------------
 * General: Returns value Algorithm Type in the header.
 * Return Value: Returns RvSipSecurityAlgorithmType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityAlgorithmType RVCALLCONV RvSipSecurityHeaderGetAlgorithmType(
                                               IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eAlgorithm;
}

/***************************************************************************
 * RvSipSecurityHeaderSetProtocolType
 * ------------------------------------------------------------------------
 * General:Sets the Protocol Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eProtocolType - The Protocol Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Protocol Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetProtocolType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityProtocolType	eProtocolType)
{
    MsgSecurityHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->eProtocol = eProtocolType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetProtocolType
 * ------------------------------------------------------------------------
 * General: Returns value Protocol Type in the header.
 * Return Value: Returns RvSipSecurityProtocolType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityProtocolType RVCALLCONV RvSipSecurityHeaderGetProtocolType(
                                               IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eProtocol;
}

/***************************************************************************
 * RvSipSecurityHeaderSetModeType
 * ------------------------------------------------------------------------
 * General:Sets the Mode Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eModeType - The Mode Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing Mode Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetModeType(
                                     IN    RvSipSecurityHeaderHandle	hHeader,
									 IN	   RvSipSecurityModeType		eModeType)
{
    MsgSecurityHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->eMode = eModeType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetModeType
 * ------------------------------------------------------------------------
 * General: Returns value Mode Type in the header.
 * Return Value: Returns RvSipSecurityModeType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityModeType RVCALLCONV RvSipSecurityHeaderGetModeType(
									IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_MODE_TYPE_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eMode;
}

/***************************************************************************
 * RvSipSecurityHeaderSetEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General:Sets the EncryptAlgorithm Type in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    eEncryptAlgorithmType - The EncryptAlgorithm Type to be set in the Security header. If UNDEFINED is supplied, the
 *                 existing EncryptAlgorithm Type is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetEncryptAlgorithmType(
                                     IN    RvSipSecurityHeaderHandle			hHeader,
									 IN	   RvSipSecurityEncryptAlgorithmType	eEncryptAlgorithmType)
{
	MsgSecurityHeader*	pHeader;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->eEncryptAlgorithm = eEncryptAlgorithmType;
	
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General: Returns value EncryptAlgorithm Type in the header.
 * Return Value: Returns RvSipSecurityEncryptAlgorithmType.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader - Handle to the header object.
 ***************************************************************************/
RVAPI RvSipSecurityEncryptAlgorithmType RVCALLCONV RvSipSecurityHeaderGetEncryptAlgorithmType(
                                               IN RvSipSecurityHeaderHandle hHeader)
{
    if(hHeader == NULL)
	{
        return RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
	}

    return ((MsgSecurityHeader*)hHeader)->eEncryptAlgorithm;
}

/***************************************************************************
 * RvSipSecurityHeaderGetSpiC
 * ------------------------------------------------------------------------
 * General: return spiC value and boolean stating if it is initialized.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader		- Handle to the Security header object.
 * output:isInitialized - is the spiC value initialized.
 *		  spiC			- The spiC value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetSpiC(
                                               IN RvSipSecurityHeaderHandle hHeader,
											  OUT RvBool*					isInitialized,
                                              OUT RvUint32*	                spiC)
{
	MsgSecurityHeader*	pHeader;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgSecurityHeader*) hHeader;

	*isInitialized = pHeader->spiC.bInitialized;
	if(pHeader->spiC.bInitialized == RV_TRUE)
	{
		*spiC = pHeader->spiC.value;
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetSpiC
 * ------------------------------------------------------------------------
 * General:Sets the SpiC Value field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *	  isInitialized - is the spiC value initialized.
 *	  spiC		- The spiC value. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSpiC(
                                     IN RvSipSecurityHeaderHandle	hHeader,
                                     IN RvBool						isInitialized,
                                     IN RvUint32	                spiC)
{
    MsgSecurityHeader*	pHeader;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->spiC.bInitialized = isInitialized;
	if(pHeader->spiC.bInitialized == RV_TRUE)
	{
		pHeader->spiC.value = spiC;
	}
	
	return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetSpiS
 * ------------------------------------------------------------------------
 * General: return spiS value and boolean stating if it is initialized.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader		- Handle to the Security header object.
 * output:isInitialized - is the spiS value initialized.
 *		  spiS			- The spiS value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetSpiS(
                                               IN RvSipSecurityHeaderHandle hHeader,
											  OUT RvBool*					isInitialized,
                                              OUT RvUint32*	                spiS)
{
	MsgSecurityHeader*	pHeader;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgSecurityHeader*) hHeader;

	*isInitialized = pHeader->spiS.bInitialized;
	if(pHeader->spiS.bInitialized == RV_TRUE)
	{
		*spiS = pHeader->spiS.value;
	}

	return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetSpiS
 * ------------------------------------------------------------------------
 * General:Sets the SpiS Value field in the Security header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader       - Handle to the Security header object.
 *	  isInitialized - is the spiS value initialized.
 *	  spiS		- The spiS value. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetSpiS(
                                     IN RvSipSecurityHeaderHandle	hHeader,
                                     IN RvBool						isInitialized,
                                     IN RvUint32	                spiS)
{
    MsgSecurityHeader*	pHeader;
    
	if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

	pHeader = (MsgSecurityHeader*) hHeader;

	pHeader->spiS.bInitialized = isInitialized;
	if(pHeader->spiS.bInitialized == RV_TRUE)
	{
		pHeader->spiS.value = spiS;
	}
	
	return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetPortC
 * ------------------------------------------------------------------------
 * General: Gets portC parameter from the Security header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetPortC(
                                         IN RvSipSecurityHeaderHandle hSecurityHeader)
{
    if(hSecurityHeader == NULL)
	{
        return UNDEFINED;
	}

    return ((MsgSecurityHeader*)hSecurityHeader)->portC;
}

/***************************************************************************
 * RvSipSecurityHeaderSetPortC
 * ------------------------------------------------------------------------
 * General:  Sets portC parameter of the Security header object.
 *           Notice: Supplying port larger than 65535 will lead to unexpected results.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 *    portNum         - The port number value to be set in the object (must be
 *                      smaller than 65535).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetPortC(
                                         IN    RvSipSecurityHeaderHandle hSecurityHeader,
                                         IN    RvInt32					 portNum)
{
    if(hSecurityHeader == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgSecurityHeader*)hSecurityHeader)->portC = portNum;
    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderGetPortS
 * ------------------------------------------------------------------------
 * General: Gets portS parameter from the Security header object.
 * Return Value: Returns the port number, or UNDEFINED if the port number
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipSecurityHeaderGetPortS(
                                         IN RvSipSecurityHeaderHandle hSecurityHeader)
{
    if(hSecurityHeader == NULL)
	{
		return UNDEFINED;
	}

    return ((MsgSecurityHeader*)hSecurityHeader)->portS;
}

/***************************************************************************
 * RvSipSecurityHeaderSetPortS
 * ------------------------------------------------------------------------
 * General:  Sets portS parameter of the Security header object.
 *           Notice: Supplying port larger than 65535 will lead to unexpected results.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSecurityHeader - Handle to the Security header object.
 *    portNum         - The port number value to be set in the object (must be
 *                      smaller than 65535).
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetPortS(
                                         IN    RvSipSecurityHeaderHandle hSecurityHeader,
                                         IN    RvInt32				 	 portNum)
{
    if(hSecurityHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    ((MsgSecurityHeader*)hSecurityHeader)->portS = portNum;
    return RV_OK;
}

/***************************************************************************
 * SipSecurityHeaderGetOtherParam
 * ------------------------------------------------------------------------
 * General:This method gets the Other Params in the Security header object.
 * Return Value: Other params offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Security header object..
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetOtherParams(
                                            IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strOtherParams;
}

/***************************************************************************
 * RvSipSecurityHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Security header other params field of the Security header object into a
 *          given buffer.
 *          Not all the Security header parameters have separated fields in the Security
 *          header object. Parameters with no specific fields are referred to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Security header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetOtherParams(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetOtherParams(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the OtherParams in the
 *          SecurityHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Security header object.
 *            pOtherParams - The Other Params to be set in the Security header.
 *                          If NULL, the existing OtherParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipSecurityHeaderSetOtherParams(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32				newStr;
    MsgSecurityHeader*	pHeader;
    RvStatus			retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    pHeader = (MsgSecurityHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pOtherParams,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strOtherParams = newStr;
#ifdef SIP_DEBUG
    pHeader->pOtherParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strOtherParams);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Security header object.
 *         Not all the Security header parameters have separated fields in the Security
 *         header object. Parameters with no specific fields are referred to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Security header object.
 *    strOtherParams - The extended parameters field to be set in the Security header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetOtherParams(
                                     IN    RvSipSecurityHeaderHandle hHeader,
                                     IN    RvChar *                pOtherParams)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    return SipSecurityHeaderSetOtherParams(hHeader, pOtherParams, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipSecurityHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipSecurityHeaderGetStrBadSyntax(
                                    IN RvSipSecurityHeaderHandle hHeader)
{
    return ((MsgSecurityHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipSecurityHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad Security header,
 *          and you wish to see the header-value.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - The handle to the header object.
 *        strBuffer  - The buffer with which to fill the bad syntax string.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the bad syntax + 1, to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderGetStrBadSyntax(
                                               IN RvSipSecurityHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
	if(actualLen == NULL)
	{
		return RV_ERROR_NULLPTR;
	}

    *actualLen = 0;

    if(hHeader == NULL)
    {
		return RV_ERROR_INVALID_HANDLE;
	}

    if(strBuffer == NULL)
	{
        return RV_ERROR_NULLPTR;
	}

    return MsgUtilsFillUserBuffer(((MsgSecurityHeader*)hHeader)->hPool,
                                  ((MsgSecurityHeader*)hHeader)->hPage,
                                  SipSecurityHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipSecurityHeaderSetStrBadSyntax
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
RvStatus SipSecurityHeaderSetStrBadSyntax(
                                  IN RvSipSecurityHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgSecurityHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}

    header = (MsgSecurityHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
                                  strBadSyntax, header->hPool, header->hPage, &newStrOffset);
    if(RV_OK != retStatus)
    {
        return retStatus;
    }
    header->strBadSyntax = newStrOffset;
#ifdef SIP_DEBUG
    header->pBadSyntax = (RvChar *)RPOOL_GetPtr(header->hPool, header->hPage,
                                      header->strBadSyntax);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipSecurityHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammer:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Security header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipSecurityHeaderSetStrBadSyntax(
                                  IN RvSipSecurityHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
	{
        return RV_ERROR_INVALID_HANDLE;
	}
	
    return SipSecurityHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecurityHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void SecurityHeaderClean( IN MsgSecurityHeader* pHeader,
							    IN RvBool            bCleanBS)
{
	pHeader->eHeaderType			= RVSIP_HEADERTYPE_SECURITY;
	pHeader->eSecurityHeaderType	= RVSIP_SECURITY_UNDEFINED_HEADER;
	pHeader->eMechanismType			= RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
	pHeader->eAlgorithm				= RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
	pHeader->eProtocol				= RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED;
	pHeader->eMode					= RVSIP_SECURITY_MODE_TYPE_UNDEFINED;
	pHeader->eEncryptAlgorithm		= RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
    pHeader->strOtherParams			= UNDEFINED;
	pHeader->strMechanismType		= UNDEFINED;
	pHeader->strPreference			= UNDEFINED;
	pHeader->eDigestAlgorithm		= RVSIP_AUTH_ALGORITHM_UNDEFINED;
	pHeader->strDigestAlgorithm		= UNDEFINED;
	pHeader->nAKAVersion			= UNDEFINED;
	pHeader->strDigestQop			= UNDEFINED;
	pHeader->eDigestQop				= RVSIP_AUTH_QOP_UNDEFINED;
	pHeader->strDigestVerify		= UNDEFINED;
	pHeader->spiC.bInitialized		= RV_FALSE;
	pHeader->spiS.bInitialized		= RV_FALSE;
	pHeader->portC					= UNDEFINED;
	pHeader->portS					= UNDEFINED;
									
#ifdef SIP_DEBUG					
    pHeader->pOtherParams			= NULL;
	pHeader->pMechanismType			= NULL;
	pHeader->pPreference			= NULL;
	pHeader->pDigestAlgorithm		= NULL;
	pHeader->pDigestQop				= NULL;
	pHeader->pDigestVerify			= NULL;
#endif

	if(bCleanBS == RV_TRUE)
    {
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef __cplusplus
}
#endif

