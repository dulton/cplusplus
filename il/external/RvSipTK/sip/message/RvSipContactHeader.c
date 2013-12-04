/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             RvSipContactHeader.c                           *
 *                                                                            *
 * The file defines the methods of the Contact header object:                 *
 * construct, destruct, copy, encode, parse and the ability to access and     *
 * change it's parameters.                                                    *
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
#ifndef RV_SIP_LIGHT

#include "RvSipContactHeader.h"
#include "RvSipExpiresHeader.h"
#include "_SipContactHeader.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipAddress.h"
#include "_SipCommonUtils.h"
#include <string.h>


static RvBool isLegalQVal(RvChar *pQVal);

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static void ContactHeaderClean(IN MsgContactHeader* pHeader,
							   IN RvBool            bCleanBS);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipContactHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a Contact header object inside a given message object. The header is
 *          kept in the header list of the message. You can choose to insert the header either
 *          at the head or tail of the list.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg          - Handle to the message object.
 *         pushHeaderAtHead - Boolean value indicating whether the header should be pushed to the head of the
 *                            list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hHeader          - Handle to the newly constructed Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderConstructInMsg(
                                       IN  RvSipMsgHandle            hSipMsg,
                                       IN  RvBool                   pushHeaderAtHead,
                                       OUT RvSipContactHeaderHandle* hHeader)
{
    MsgMessage*                 msg;
    RvSipHeaderListElemHandle   hListElem = NULL;
    RvSipHeadersLocation        location;
    RvStatus                   stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    msg = (MsgMessage*)hSipMsg;

    stat = RvSipContactHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hHeader);
    if(stat != RV_OK)
        return stat;

    if(pushHeaderAtHead == RV_TRUE)
        location = RVSIP_FIRST_HEADER;
    else
        location = RVSIP_LAST_HEADER;

    /* attach the header in the msg */
    return RvSipMsgPushHeader(hSipMsg,
                              location,
                              (void*)*hHeader,
                              RVSIP_HEADERTYPE_CONTACT,
                              &hListElem,
                              (void**)hHeader);
}

/***************************************************************************
 * RvSipContactHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone Contact Header object. The header is
 *          constructed on a given page taken from a specified pool. The handle to the new
 *          header object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the Message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hHeader - Handle to the newly constructed contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderConstruct(
                                           IN  RvSipMsgMgrHandle         hMsgMgr,
                                           IN  HRPOOL                    hPool,
                                           IN  HPAGE                     hPage,
                                           OUT RvSipContactHeaderHandle* hHeader)
{
    MsgContactHeader*   pHeader;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hMsgMgr == NULL)||(hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hHeader == NULL)
        return RV_ERROR_NULLPTR;

    *hHeader = NULL;

    pHeader = (MsgContactHeader*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                       hPool,
                                                       hPage,
                                                       sizeof(MsgContactHeader),
                                                       RV_TRUE);
    if(pHeader == NULL)
    {
        *hHeader = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipContactHeaderConstruct - Failed to construct contact header. outOfResources. hPool 0x%p, hPage 0x%p",
            hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pHeader->pMsgMgr          = pMsgMgr;
    pHeader->hPage            = hPage;
    pHeader->hPool            = hPool;
    ContactHeaderClean(pHeader, RV_TRUE);

    *hHeader = (RvSipContactHeaderHandle)pHeader;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipContactHeaderConstruct - Contact header was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
            hPool, hPage, *hHeader));

    return RV_OK;
}


/***************************************************************************
 * RvSipContactHeaderCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source Contact header object to a destination Contact
 *          header object.
 *          You must construct the destination object before using this function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination Contact header object.
 *    hSource      - Handle to the destination Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderCopy(
                                         INOUT  RvSipContactHeaderHandle hDestination,
                                         IN     RvSipContactHeaderHandle hSource)
{
    RvStatus           stat;
    MsgContactHeader*   source;
    MsgContactHeader*   dest;
    
    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (MsgContactHeader*)hSource;
    dest   = (MsgContactHeader*)hDestination;

    dest->eHeaderType = source->eHeaderType;

    /* The Contact header source is a star Contact header */
    if (RV_TRUE == source->bIsStar)
    {
        dest->bIsStar = RV_TRUE;
        return RV_OK;
    }
    else
    {
        dest->bIsStar = RV_FALSE;
    }

    dest->bIsCompact = source->bIsCompact;
    dest->eAction    = source->eAction;

    /* qVal */
    if(source->strQVal > UNDEFINED)
    {
        dest->strQVal = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                      dest->hPool,
                                                      dest->hPage,
                                                      source->hPool,
                                                      source->hPage,
                                                      source->strQVal);
        if(dest->strQVal == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy 'q' parameter"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pQVal = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strQVal);
#endif
    }
    else
    {
        dest->strQVal = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pQVal = NULL;
#endif
    }

#ifndef RV_SIP_JSR32_SUPPORT
    /* strDisplayName */
    /*stat = RvSipContactHeaderSetDisplayName(hDestination, source->strDisplayName);*/
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
                "RvSipContactHeaderCopy - Failed to copy displayName"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pDisplayName = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDisplayName);
#endif
    }
    else
    {
        dest->strDisplayName = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDisplayName = NULL;
#endif
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    /* pAddrSpec */
    stat = RvSipContactHeaderSetAddrSpec(hDestination, source->hAddrSpec);
	if (RV_OK != stat)
	{
		RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
			   "RvSipContactHeaderCopy - Failed in RvSipContactHeaderSetAddrSpec. stat=%d", stat));
        return stat;
    }

    /* hExpiresParam */
    if(source->hExpiresParam != NULL)
    {
        if(dest->hExpiresParam == NULL)
        {
            RvSipExpiresHeaderHandle hExpires;

            stat = RvSipExpiresConstructInContactHeader(
                                                    hDestination, &hExpires);
            if(stat != RV_OK)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipContactHeaderCopy - Failed to construct expires parameter"));
                return stat;
            }
        }
        stat = RvSipExpiresHeaderCopy(dest->hExpiresParam, source->hExpiresParam);
        if(stat != RV_OK)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContactHeaderCopy - Failed to copy expires parameter"));
            return stat;
        }
    }
    else
        dest->hExpiresParam = NULL;

#ifdef RV_SIP_IMS_HEADER_SUPPORT 
    /* strPubGruu */
    if(source->strPubGruu > UNDEFINED)
    {
        dest->strPubGruu = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strPubGruu);
        if(dest->strPubGruu == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy PubGruu"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPubGruu = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strPubGruu);
#endif
    }
    else
    {
        dest->strPubGruu = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPubGruu = NULL;
#endif
    }

    /* strPubGruu */
    if(source->strPubGruu > UNDEFINED)
    {
        dest->strPubGruu = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strPubGruu);
        if(dest->strPubGruu == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy PubGruu"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPubGruu = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strPubGruu);
#endif
    }
    else
    {
        dest->strPubGruu = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPubGruu = NULL;
#endif
    }

    /* reg-id */
    dest->regIDNum = source->regIDNum;

    /* strAudio */
    dest->bIsAudio = source->bIsAudio;
    if(source->strAudio > UNDEFINED)
    {
        dest->strAudio = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAudio);
        if(dest->strAudio == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Audio"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAudio = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strAudio);
#endif
    }
    else
    {
        dest->strAudio = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAudio = NULL;
#endif
    }

    /* strAutomata */
    dest->bIsAutomata = source->bIsAutomata;
    if(source->strAutomata > UNDEFINED)
    {
        dest->strAutomata = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strAutomata);
        if(dest->strAutomata == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Automata"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pAutomata = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strAutomata);
#endif
    }
    else
    {
        dest->strAutomata = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pAutomata = NULL;
#endif
    }

    /* strClass */
    dest->bIsClass = source->bIsClass;
    if(source->strClass > UNDEFINED)
    {
        dest->strClass = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strClass);
        if(dest->strClass == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Class"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pClass = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strClass);
#endif
    }
    else
    {
        dest->strClass = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pClass = NULL;
#endif
    }

    /* strDuplex */
    dest->bIsDuplex = source->bIsDuplex;
    if(source->strDuplex > UNDEFINED)
    {
        dest->strDuplex = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strDuplex);
        if(dest->strDuplex == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Duplex"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pDuplex = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDuplex);
#endif
    }
    else
    {
        dest->strDuplex = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDuplex = NULL;
#endif
    }

    /* strData */
    dest->bIsData = source->bIsData;
    if(source->strData > UNDEFINED)
    {
        dest->strData = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strData);
        if(dest->strData == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Data"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pData = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strData);
#endif
    }
    else
    {
        dest->strData = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pData = NULL;
#endif
    }

    /* strControl */
    dest->bIsControl = source->bIsControl;
    if(source->strControl > UNDEFINED)
    {
        dest->strControl = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strControl);
        if(dest->strControl == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Control"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pControl = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strControl);
#endif
    }
    else
    {
        dest->strControl = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pControl = NULL;
#endif
    }

    /* strMobility */
    dest->bIsMobility = source->bIsMobility;
    if(source->strMobility > UNDEFINED)
    {
        dest->strMobility = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strMobility);
        if(dest->strMobility == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Mobility"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMobility = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strMobility);
#endif
    }
    else
    {
        dest->strMobility = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMobility = NULL;
#endif
    }

    /* strDescription */
    dest->bIsDescription = source->bIsDescription;
    if(source->strDescription > UNDEFINED)
    {
        dest->strDescription = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strDescription);
        if(dest->strDescription == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Description"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pDescription = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strDescription);
#endif
    }
    else
    {
        dest->strDescription = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pDescription = NULL;
#endif
    }

    /* strEvents */
    dest->bIsEvents = source->bIsEvents;
    if(source->strEvents > UNDEFINED)
    {
        dest->strEvents = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strEvents);
        if(dest->strEvents == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Events"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pEvents = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strEvents);
#endif
    }
    else
    {
        dest->strEvents = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pEvents = NULL;
#endif
    }

    /* strPriority */
    dest->bIsPriority = source->bIsPriority;
    if(source->strPriority > UNDEFINED)
    {
        dest->strPriority = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strPriority);
        if(dest->strPriority == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Priority"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pPriority = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strPriority);
#endif
    }
    else
    {
        dest->strPriority = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pPriority = NULL;
#endif
    }

    /* strMethods */
    dest->bIsMethods = source->bIsMethods;
    if(source->strMethods > UNDEFINED)
    {
        dest->strMethods = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strMethods);
        if(dest->strMethods == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Methods"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pMethods = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strMethods);
#endif
    }
    else
    {
        dest->strMethods = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pMethods = NULL;
#endif
    }

    /* strSchemes */
    dest->bIsSchemes = source->bIsSchemes;
    if(source->strSchemes > UNDEFINED)
    {
        dest->strSchemes = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strSchemes);
        if(dest->strSchemes == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Schemes"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pSchemes = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strSchemes);
#endif
    }
    else
    {
        dest->strSchemes = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pSchemes = NULL;
#endif
    }

    /* strApplication */
    dest->bIsApplication = source->bIsApplication;
    if(source->strApplication > UNDEFINED)
    {
        dest->strApplication = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strApplication);
        if(dest->strApplication == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Application"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pApplication = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strApplication);
#endif
    }
    else
    {
        dest->strApplication = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pApplication = NULL;
#endif
    }

    /* strVideo */
    dest->bIsVideo = source->bIsVideo;
    if(source->strVideo > UNDEFINED)
    {
        dest->strVideo = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strVideo);
        if(dest->strVideo == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Video"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pVideo = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strVideo);
#endif
    }
    else
    {
        dest->strVideo = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pVideo = NULL;
#endif
    }

    /* strLanguage */
    dest->bIsLanguage = source->bIsLanguage;
    if(source->strLanguage > UNDEFINED)
    {
        dest->strLanguage = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strLanguage);
        if(dest->strLanguage == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Language"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pLanguage = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strLanguage);
#endif
    }
    else
    {
        dest->strLanguage = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pLanguage = NULL;
#endif
    }

    /* strType */
    dest->bIsType = source->bIsType;
    if(source->strType > UNDEFINED)
    {
        dest->strType = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strType);
        if(dest->strType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Type"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strType);
#endif
    }
    else
    {
        dest->strType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pType = NULL;
#endif
    }

    /* strIsFocus */
    dest->bIsIsFocus = source->bIsIsFocus;
    if(source->strIsFocus > UNDEFINED)
    {
        dest->strIsFocus = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strIsFocus);
        if(dest->strIsFocus == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy IsFocus"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pIsFocus = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strIsFocus);
#endif
    }
    else
    {
        dest->strIsFocus = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pIsFocus = NULL;
#endif
    }

    /* strActor */
    dest->bIsActor = source->bIsActor;
    if(source->strActor > UNDEFINED)
    {
        dest->strActor = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strActor);
        if(dest->strActor == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Actor"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pActor = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strActor);
#endif
    }
    else
    {
        dest->strActor = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pActor = NULL;
#endif
    }

    /* strText */
    dest->bIsText = source->bIsText;
    if(source->strText > UNDEFINED)
    {
        dest->strText = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strText);
        if(dest->strText == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Text"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pText = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strText);
#endif
    }
    else
    {
        dest->strText = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pText = NULL;
#endif
    }

    /* strExtensions */
    dest->bIsExtensions = source->bIsExtensions;
    if(source->strExtensions > UNDEFINED)
    {
        dest->strExtensions = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strExtensions);
        if(dest->strExtensions == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy Extensions"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pExtensions = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strExtensions);
#endif
    }
    else
    {
        dest->strExtensions = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pExtensions = NULL;
#endif
    }

    /* strSipInstance */
    dest->bIsSipInstance = source->bIsSipInstance;
    if(source->strSipInstance > UNDEFINED)
    {
        dest->strSipInstance = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strSipInstance);
        if(dest->strSipInstance == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipContactHeaderCopy - Failed to copy SipInstance"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pSipInstance = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strSipInstance);
#endif
    }
    else
    {
        dest->strSipInstance = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pSipInstance = NULL;
#endif
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    /* strContactParams */
    /*stat =  RvSipContactHeaderSetOtherParam(hDestination,
                                              source->strContactParams);*/
    if(source->strContactParams > UNDEFINED)
    {
        dest->strContactParams = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                              dest->hPool,
                                                              dest->hPage,
                                                              source->hPool,
                                                              source->hPage,
                                                              source->strContactParams);
        if(dest->strContactParams == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipContactHeaderCopy - didn't manage to copy contactParams"));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pContactParams = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                         dest->strContactParams);
#endif
    }
    else
    {
        dest->strContactParams = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pContactParams = NULL;
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
                "RvSipContactHeaderCopy - failed in coping strBadSyntax."));
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
 * RvSipContactHeaderEncode
 * ------------------------------------------------------------------------
 * General: Encodes a contact header object to a textual Contact header. The textual header
 *          is placed on a page taken from a specified pool. In order to copy the textual
 *          header from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the contact header object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded header.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderEncode(
                                          IN    RvSipContactHeaderHandle hHeader,
                                          IN    HRPOOL                   hPool,
                                          OUT   HPAGE*                   hPage,
                                          OUT   RvUint32*               length)
{
    RvStatus stat;
    MsgContactHeader* pHeader;

    pHeader = (MsgContactHeader*)hHeader;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((hHeader == NULL) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hHeader is 0x%p",
                hPool, hHeader));
        return stat;
    }
    else
    {
        RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderEncode - Got a new page 0x%p on pool 0x%p. hHeader is 0x%p",
                *hPage, hPool, hHeader));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
    stat = ContactHeaderEncode(hHeader, hPool, *hPage, RV_FALSE,RV_FALSE, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderEncode - Failed. Free page 0x%p on pool 0x%p. ContactHeaderEncode fail",
                *hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * ContactHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Contract header (as string) on it.
 *          format: "Contact: "
 *                  ("*"|(1#((name-addr|addr-spec)
 *                    *(";"contact-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the contact header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  ContactHeaderEncode(
                                  IN    RvSipContactHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  IN    RvBool                bForceCompactForm,
                                  INOUT RvUint32*               length)
{
    MsgContactHeader*  pHeader;
    RvStatus          stat;

    if((hHeader == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pHeader = (MsgContactHeader*) hHeader;

    RvLogInfo(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "ContactHeaderEncode - Encoding Contact header. hHeader 0x%p, hPool 0x%p, hPage 0x%p",
             hHeader, hPool, hPage));

    /* put "Contact: " */
    if(pHeader->bIsCompact == RV_TRUE || bForceCompactForm == RV_TRUE)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "m", length);
    }
    else
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "Contact", length);
    }
    if (RV_OK != stat)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContactHeaderEncode - Failed to encoding contact header. stat is %d",
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
                "ContactHeaderEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }
        return stat;
    }

    /* If the Contact header is "Contact: *" encode "*" */
    if (RV_TRUE == pHeader->bIsStar)
    {
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "*", length);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ContactHeaderEncode - Failed to encoding contact header. stat is %d",
                stat));
        }
        return stat;
    }


#ifndef RV_SIP_JSR32_SUPPORT
    /* display name */

    /* "contact: *"is not supported at this stage */
    /* [displayNeme]"<"addr-spec">" */

    if(pHeader->strDisplayName > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strDisplayName,
                                    length);
    }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    if (pHeader->hAddrSpec != NULL)
    {
        /* set the adder-spec */
        stat = AddrEncode(pHeader->hAddrSpec, hPool, hPage, bInUrlHeaders, RV_TRUE, length);
        if (stat != RV_OK)
            return stat;
    }
    else
    {
        /* this is not an optional parameter */
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContactHeaderEncode: Failed. No hAddrSpec was given. not an optional parameter. hHeader 0x%p",
            hHeader));
        return RV_ERROR_BADPARAM;
    }
    /* no need of spaces because of the ';' */

    /* encode expires parameter */
    if (pHeader->hExpiresParam != NULL)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode expires parameter parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }

        stat = ExpiresHeaderEncode(pHeader->hExpiresParam, hPool, hPage, RV_TRUE,
                                   bInUrlHeaders, length);
        if (RV_OK != stat)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode expires parameter parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }
    }

    if(pHeader->strQVal > UNDEFINED)
    {
        /* set ";q=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "q", length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders), length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode 'q' parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strQVal,
                                    length);

        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode 'q' parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }
    }

    if(pHeader->eAction != RVSIP_CONTACT_ACTION_UNDEFINED)
    {
        /* set ";action=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "action", length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode action parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                          MsgUtilsGetEqualChar(bInUrlHeaders), length);

        if(pHeader->eAction == RVSIP_CONTACT_ACTION_PROXY)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "proxy", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "redirect", length);
        }
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                      "ContactHeaderEncode: Failed to encode action parameter. hHeader 0x%p",
                      hHeader));
            return stat;
        }
    }

#ifdef RV_SIP_IMS_HEADER_SUPPORT    
    if(pHeader->strTempGruu > UNDEFINED)
    {
        /* Set ";temp-gruu=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "temp-gruu", length);
        if(pHeader->strTempGruu > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'temp-gruu' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strTempGruu,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'temp-gruu' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }

    if(pHeader->strPubGruu > UNDEFINED)
    {
        /* Set ";pub-gruu=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "pub-gruu", length);
        if(pHeader->strPubGruu > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'pub-gruu' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strPubGruu,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'pub-gruu' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->regIDNum != UNDEFINED)
    {
        RvChar  strHelper[16];

        /* set " ;reg-id=" */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders),length);
        if(stat != RV_OK)
             return stat;
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, "reg-id", length);
        if(stat != RV_OK)
             return stat;

        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage,
                                            MsgUtilsGetEqualChar(bInUrlHeaders),length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ContactHeaderEncode: Failed to Encode 'reg-id' parameter. hHeader 0x%p",
                hHeader));
            return stat;
        }

        MsgUtils_itoa(pHeader->regIDNum, strHelper);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool, hPage, strHelper, length);
        if(stat != RV_OK)
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "ContactHeaderEncode: Failed to Encode 'reg-id' parameter. hHeader 0x%p",
                hHeader));
            return stat;
        }
    }

    if(pHeader->bIsAudio == RV_TRUE)
    {
        /* Set ";audio=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "audio", length);
        if(pHeader->strAudio > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'audio' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strAudio,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'audio' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsAutomata == RV_TRUE)
    {
        /* Set ";automata=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "automata", length);
        if(pHeader->strAutomata > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'automata' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strAutomata,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'automata' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsClass == RV_TRUE)
    {
        /* Set ";class=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "class", length);
        if(pHeader->strClass > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'class' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strClass,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'class' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsDuplex == RV_TRUE)
    {
        /* Set ";duplex=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "duplex", length);
        if(pHeader->strDuplex > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'duplex' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strDuplex,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'duplex' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsData == RV_TRUE)
    {
        /* Set ";data=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "data", length);
        if(pHeader->strData > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'data' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strData,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'data' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsControl == RV_TRUE)
    {
        /* Set ";control=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "control", length);
        if(pHeader->strControl > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'control' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strControl,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'control' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsMobility == RV_TRUE)
    {
        /* Set ";mobility=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "mobility", length);
        if(pHeader->strMobility > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'mobility' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strMobility,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'mobility' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsDescription == RV_TRUE)
    {
        /* Set ";description=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "description", length);
        if(pHeader->strDescription > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'description' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strDescription,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'description' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsEvents == RV_TRUE)
    {
        /* Set ";events=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "events", length);
        if(pHeader->strEvents > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'events' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strEvents,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'events' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsPriority == RV_TRUE)
    {
        /* Set ";priority=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "priority", length);
        if(pHeader->strPriority > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'priority' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strPriority,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'priority' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsMethods == RV_TRUE)
    {
        /* Set ";methods=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "methods", length);
        if(pHeader->strMethods > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'methods' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strMethods,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'methods' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsSchemes == RV_TRUE)
    {
        /* Set ";schemes=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "schemes", length);
        if(pHeader->strSchemes > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'schemes' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strSchemes,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'schemes' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsApplication == RV_TRUE)
    {
        /* Set ";application=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "application", length);
        if(pHeader->strApplication > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'application' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strApplication,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'application' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsVideo == RV_TRUE)
    {
        /* Set ";video=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "video", length);
        if(pHeader->strVideo > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'video' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strVideo,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'video' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsLanguage == RV_TRUE)
    {
        /* Set ";language=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "language", length);
        if(pHeader->strLanguage > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'language' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strLanguage,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'language' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsType == RV_TRUE)
    {
        /* Set ";type=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "type", length);
        if(pHeader->strType > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'type' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strType,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'type' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsIsFocus == RV_TRUE)
    {
        /* Set ";isfocus=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "isfocus", length);
        if(pHeader->strIsFocus > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'isfocus' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strIsFocus,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'isfocus' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsActor == RV_TRUE)
    {
        /* Set ";actor=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "actor", length);
        if(pHeader->strActor > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'actor' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strActor,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'actor' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsText == RV_TRUE)
    {
        /* Set ";text=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "text", length);
        if(pHeader->strText > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'text' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strText,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'text' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsExtensions == RV_TRUE)
    {
        /* Set ";extensions=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "extensions", length);
        if(pHeader->strExtensions > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'extensions' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strExtensions,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode 'extensions' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
    
    if(pHeader->bIsSipInstance == RV_TRUE)
    {
        /* Set ";+sip.instance=" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage, "+sip.instance", length);
        if(pHeader->strSipInstance > UNDEFINED)
        {
            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                MsgUtilsGetEqualChar(bInUrlHeaders), length);

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode '+sip.instance' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }
            stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                hPool,
                hPage,
                pHeader->hPool,
                pHeader->hPage,
                pHeader->strSipInstance,
                length);
            
            if(stat != RV_OK)
            {
                RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                    "ContactHeaderEncode: Failed to Encode '+sip.instance' parameter. hHeader 0x%p",
                    hHeader));
                return stat;
            }

            stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), length);
        }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

    /* set the contact-params. (insert ";" in the beginning) */
    if(pHeader->strContactParams > UNDEFINED)
    {
        /* set ";" in the beginning */
        stat = MsgUtilsEncodeExternalString(pHeader->pMsgMgr, hPool,hPage,
                                            MsgUtilsGetSemiColonChar(bInUrlHeaders), length);

        /* set contactParmas */
        stat = MsgUtilsEncodeString(pHeader->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pHeader->hPool,
                                    pHeader->hPage,
                                    pHeader->strContactParams,
                                    length);
    }

    if (stat != RV_OK)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "ContactHeaderEncode - Failed to encoding contact header. stat is %d",
            stat));
    }
    return stat;
}

/***************************************************************************
 * RvSipContactHeaderParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Contact header-for example,
 *          "Contact:sip:172.20.5.3:5060"-into a contact header object. All the textual
 *          fields are placed inside the object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - A handle to the Contact header object.
 *    buffer    - Buffer containing a textual Contact header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderParse(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgContactHeader*    pHeader = (MsgContactHeader*)hHeader;
	RvStatus             rv;
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    ContactHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTACT,
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
 * RvSipContactHeaderParseValue
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual Contact header value into an Contact header object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. This function takes the header-value part as
 *          a parameter and parses it into the supplied object.
 *          All the textual fields are placed inside the object.
 *          Note: Use the RvSipContactHeaderParse() function to parse strings that also
 *          include the header-name.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - The handle to the Contact header object.
 *    buffer    - The buffer containing a textual Contact header value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderParseValue(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 buffer)
{
    MsgContactHeader*    pHeader = (MsgContactHeader*)hHeader;
    RvBool               bIsCompact;
	RvStatus             rv;
	
    if(hHeader == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;

    /* The is-compact indicaation is stored because this method only parses the header value */
	bIsCompact = pHeader->bIsCompact;
	
    ContactHeaderClean(pHeader, RV_FALSE);

    rv = MsgParserParseStandAloneHeader(pHeader->pMsgMgr,
                                        SIP_PARSETYPE_CONTACT,
                                        buffer,
                                        RV_TRUE,
                                         (void*)hHeader);
    
	/* restore is-compact indication */
	pHeader->bIsCompact = bIsCompact;
	
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
 * RvSipContactHeaderFix
 * ------------------------------------------------------------------------
 * General: Fixes an Contact header with bad-syntax information.
 *          A SIP header has the following grammar:
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderFix(
                                     IN RvSipContactHeaderHandle hHeader,
                                     IN RvChar*                 pFixedBuffer)
{
    MsgContactHeader* pHeader = (MsgContactHeader*)hHeader;
    RvStatus             rv;

    if(hHeader == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    rv = RvSipContactHeaderParseValue(hHeader, pFixedBuffer);
    if(rv == RV_OK)
    {

        pHeader->strBadSyntax = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax = NULL;
#endif
        RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContactHeaderFix: header 0x%p was fixed", hHeader));
    }
    else
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
            "RvSipContactHeaderFix: header 0x%p - Failure in parsing header value", hHeader));
    }

    return rv;
}

/***************************************************************************
 * SipContactHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Contact object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HRPOOL SipContactHeaderGetPool(RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->hPool;
}

/***************************************************************************
 * SipContactHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Contact object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hAddr - The address to take the page from.
 ***************************************************************************/
HPAGE SipContactHeaderGetPage(RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->hPage;
}

/***************************************************************************
 * RvSipContactHeaderGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Contact header fields are kept in a string format-for example, the
 *          contact header display name. In order to get such a field from the Contact header
 *          object, your application should supply an adequate buffer to where the string
 *          will be copied.
 *          This function provides you with the length of the string to enable you to allocate
 *          an appropriate buffer size before calling the Get function.
 * Return Value: Returns the length of the specified string.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader    - Handle to the Contact header object.
 *  stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipContactHeaderGetStringLength(
                                      IN  RvSipContactHeaderHandle     hHeader,
                                      IN  RvSipContactHeaderStringName stringName)
{
    RvInt32    stringOffset;
    MsgContactHeader* pHeader = (MsgContactHeader*)hHeader;

    if(hHeader == NULL)
        return 0;

    switch (stringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
        case RVSIP_CONTACT_DISPLAYNAME:
        {
            stringOffset = SipContactHeaderGetDisplayName(hHeader);
            break;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        case RVSIP_CONTACT_OTHER_PARAMS:
        {
            stringOffset = SipContactHeaderGetContactParam(hHeader);
            break;
        }
        case RVSIP_CONTACT_QVAL:
        {
            stringOffset = SipContactHeaderGetQVal(hHeader);
            break;
        }
#ifdef RV_SIP_IMS_HEADER_SUPPORT
        case RVSIP_CONTACT_TEMP_GRUU:
        {
            stringOffset = SipContactHeaderGetTempGruu(hHeader);
            break;
        }
        case RVSIP_CONTACT_PUB_GRUU:
        {
            stringOffset = SipContactHeaderGetPubGruu(hHeader);
            break;
        }
        case RVSIP_CONTACT_ACTOR:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR);
            break;
        }
        case RVSIP_CONTACT_APPLICATION:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION);
            break;
        }
        case RVSIP_CONTACT_AUDIO:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO);
            break;
        }
        case RVSIP_CONTACT_AUTOMATA:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA);
            break;
        }
        case RVSIP_CONTACT_CLASS:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS);
            break;
        }
        case RVSIP_CONTACT_CONTROL:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL);
            break;
        }
        case RVSIP_CONTACT_DATA:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA);
            break;
        }
        case RVSIP_CONTACT_DESCRIPTION:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION);
            break;
        }
        case RVSIP_CONTACT_DUPLEX:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX);
            break;
        }
        case RVSIP_CONTACT_EVENTS:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS);
            break;
        }
        case RVSIP_CONTACT_EXTENSIONS:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS);
            break;
        }
        case RVSIP_CONTACT_ISFOCUS:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS);
            break;
        }
        case RVSIP_CONTACT_LANGUAGE:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE);
            break;
        }
        case RVSIP_CONTACT_METHODS:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS);
            break;
        }
        case RVSIP_CONTACT_MOBILITY:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY);
            break;
        }
        case RVSIP_CONTACT_PRIORITY:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY);
            break;
        }
        case RVSIP_CONTACT_SCHEMES:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES);
            break;
        }
        case RVSIP_CONTACT_TEXT:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT);
            break;
        }
        case RVSIP_CONTACT_TYPE:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE);
            break;
        }
        case RVSIP_CONTACT_VIDEO:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO);
            break;
        }
        case RVSIP_CONTACT_SIP_INSTANCE:
        {
            stringOffset = SipContactHeaderGetStrFeatureTag(hHeader, RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE);
            break;
        }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */         
        case RVSIP_CONTACT_BAD_SYNTAX:
        {
            stringOffset = SipContactHeaderGetStrBadSyntax(hHeader);
            break;
        }
        default:
        {
            RvLogExcep(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderGetStringLength - Unknown stringName %d", stringName));
            return 0;
        }
    }
    if(stringOffset > UNDEFINED)
        return (RPOOL_Strlen(pHeader->hPool, pHeader->hPage, stringOffset)+1);
    else
        return 0;
}

#ifndef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipContactHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General:This method gets the display name embedded in the Contact header.
 * Return Value: display name offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetDisplayName(
                                    IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strDisplayName;
}

/***************************************************************************
 * RvSipContactHeaderGetDisplayName
 * ------------------------------------------------------------------------
 * General: Copies the display name from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetDisplayName(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetDisplayName(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContactHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pDisplayName in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pDisplayName - The display name to be set in the Contact header - If
 *                NULL, the exist displayName string in the header will be removed.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetDisplayName(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pDisplayName,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgContactHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContactHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pDisplayName,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strDisplayName = newStr;
#ifdef SIP_DEBUG
    pHeader->pDisplayName = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strDisplayName);
#endif

    pHeader->bIsStar = RV_FALSE;

    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderSetDisplayName
 * ------------------------------------------------------------------------
 * General:Sets the display name in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle to the header object.
 *    strDisplayName - The display name to be set in the Contact header. If NULL is supplied, the
 *                 existing display name is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetDisplayName(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar*                 strDisplayName)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

     return SipContactHeaderSetDisplayName(hHeader, strDisplayName, NULL, NULL_PAGE, UNDEFINED);
}
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

/***************************************************************************
 * RvSipContactHeaderGetAddrSpec
 * ------------------------------------------------------------------------
 * General: The Address Spec field is held in the contact header object as an Address object.
 *          This function returns the handle to the address object.
 * Return Value: Returns a handle to the Address Spec object, or NULL if the Address Spec
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipContactHeaderGetAddrSpec(
                                    IN RvSipContactHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgContactHeader*)hHeader)->hAddrSpec;
}

/***************************************************************************
 * RvSipContactHeaderSetAddrSpec
 * ------------------------------------------------------------------------
 * General: Sets the Address Spec parameter in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Contact header object.
 *    hAddrSpec - Handle to the Address Spec Address object. If NULL is supplied, the existing
 *              Address Spec is removed from the Contact header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetAddrSpec(
                                        IN    RvSipContactHeaderHandle hHeader,
                                        IN    RvSipAddressHandle       hAddrSpec)
{
    RvStatus           stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hAddrSpec;
    MsgContactHeader   *pHeader         = (MsgContactHeader*)hHeader;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hAddrSpec == NULL)
    {
        pHeader->hAddrSpec = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
               return RV_ERROR_BADPARAM;
        }
        if (NULL != pHeader->hAddrSpec)
        {
            currentAddrType = RvSipAddrGetAddrType(pHeader->hAddrSpec);
        }

        /* if no address object was allocated, we will construct it */
        if((pHeader->hAddrSpec == NULL) ||
           (currentAddrType != pAddr->eAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInContactHeader(hHeader,
                                                     pAddr->eAddrType,
                                                     &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }
        stat = RvSipAddrCopy(pHeader->hAddrSpec,
                            hAddrSpec);
        if (RV_OK == stat)
        {
            pHeader->bIsStar = RV_FALSE;
        }
        return stat;
    }
}


/***************************************************************************
 * RvSipContactHeaderGetExpires
 * ------------------------------------------------------------------------
 * General: The Contact header contains an Expires header object. This function returns the
 *          handle to the Expires header object.
 * Return Value: Returns the handle to the Expires header object, or NULL if the Expires header
 *               object does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipExpiresHeaderHandle RVCALLCONV RvSipContactHeaderGetExpires(
                                         IN RvSipContactHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return NULL;

    return ((MsgContactHeader*)hHeader)->hExpiresParam;
}

/***************************************************************************
 * RvSipContactHeaderSetExpires
 * ------------------------------------------------------------------------
 * General: Sets the Expires header object in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    hExpires - Handle to the Expires header object to be set in the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetExpires(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvSipExpiresHeaderHandle hExpires)
{
    RvStatus   stat;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(hExpires == NULL)
    {
        ((MsgContactHeader*)hHeader)->hExpiresParam = NULL;
        return RV_OK;
    }
    else
    {
        /* if no address object was allocated, we will construct it */
        if(((MsgContactHeader*)hHeader)->hExpiresParam == NULL)
        {
            RvSipExpiresHeaderHandle hNewExpires;

            stat = RvSipExpiresConstructInContactHeader(hHeader, &hNewExpires);
            if(stat != RV_OK)
            {
                return stat;
            }
        }
        stat = RvSipExpiresHeaderCopy(
                                  ((MsgContactHeader*)hHeader)->hExpiresParam,
                                  hExpires);
        if (RV_OK != stat)
        {
            ((MsgContactHeader*)hHeader)->hExpiresParam = NULL;
        }
        return stat;
    }
}

/***************************************************************************
 * SipContactHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'q'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'q' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetQVal(
                                    IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strQVal;
}

/***************************************************************************
 * RvSipContactHeaderGetQVal
 * ------------------------------------------------------------------------
 * General: Copies the 'q' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetQVal(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetQVal(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipContactHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: Sets the 'q' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strQVal  - The value of the offset of the 'q' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetQVal(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strQVal)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipContactHeaderSetQVal(hHeader, strQVal, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContactHeaderSetQVal
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the QVal in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pQVal - The 'q' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetQVal(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pQVal,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgContactHeader* pHeader = (MsgContactHeader*) hHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(isLegalQVal(pQVal) == RV_FALSE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderSetQVal: Invalid 'q' parameter."));
        return RV_ERROR_BADPARAM;
    }

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pQVal,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderSetQVal: MsgUtilsSetString failed."));
        return retStatus;
    }
    pHeader->strQVal = newStr;
#ifdef SIP_DEBUG
    pHeader->pQVal = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strQVal);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderGetAction
 * ------------------------------------------------------------------------
 * General: The Contact header contains the Action type. This function
 *          returns this type.
 * Return Value: Returns the type of the action in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvSipContactAction RVCALLCONV RvSipContactHeaderGetAction(
                                         IN RvSipContactHeaderHandle hHeader)
{
    if(hHeader == NULL)
    {
        return RVSIP_CONTACT_ACTION_UNDEFINED;
    }

    return ((MsgContactHeader*)hHeader)->eAction;
}

/***************************************************************************
 * RvSipContactHeaderSetAction
 * ------------------------------------------------------------------------
 * General: Sets the action type in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    action   - The action type to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetAction(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvSipContactAction       action)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    ((MsgContactHeader*)hHeader)->eAction = action;
    return RV_OK;
}

/***************************************************************************
 * SipContactHeaderGetContactParam
 * ------------------------------------------------------------------------
 * General:This method gets the contact Params in the Contact header object.
 * Return Value: Contact param offset or UNDEFINED if not exist
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetContactParam(
                                            IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strContactParams;
}

/***************************************************************************
 * RvSipContactHeaderGetOtherParams
 * ------------------------------------------------------------------------
 * General: Copies the Contact header other params field of the Contact header object into a
 *          given buffer.
 *          Not all the Contact header parameters have separated fields in the Contact
 *          header object. Parameters with no specific fields are refered to as other params.
 *          They are kept in the object in one concatenated string in the form-
 *          "name=value;name=value..."
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the Contact header object.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include a NULL value at the end of
 *                     the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetOtherParams(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetContactParam(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * SipContactHeaderSetContactParam
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the ContactParam in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hHeader - Handle of the Contact header object.
 *            pContactParam - The contact Params to be set in the Contact header.
 *                          If NULL, the exist contactParam string in the header will
 *                          be removed.
 *          strOffset     - Offset of a string on the page  (if relevant).
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetContactParam(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pContactParam,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgContactHeader* pHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContactHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pContactParam,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pHeader->strContactParams = newStr;
#ifdef SIP_DEBUG
    pHeader->pContactParams = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                           pHeader->strContactParams);
#endif

    pHeader->bIsStar = RV_FALSE;

    return RV_OK;
}
/***************************************************************************
 * RvSipContactHeaderSetOtherParams
 * ------------------------------------------------------------------------
 * General:Sets the other params field in the Contact header object.
 *         Not all the Contact header parameters have separated fields in the Contact
 *         header object. Parameters with no specific fields are refered to as other params.
 *         They are kept in the object in one concatenated string in the form-
 *         "name=value;name=value..."
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader         - Handle to the Contact header object.
 *    strContactParam - The extended parameters field to be set in the Contact header. If NULL is
 *                    supplied, the existing extended parameters field is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetOtherParams(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pContactParam)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipContactHeaderSetContactParam(hHeader, pContactParam, NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipContactHeaderSetStar
 * ------------------------------------------------------------------------
 * General: A Contact header can be in the form of "Contact: * ". This function defines the
 *          Contact header as an Asterisk Contact header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle of the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStar(
                                     IN    RvSipContactHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    ((MsgContactHeader *)hHeader)->bIsStar = RV_TRUE;
    return RV_OK;
}


/***************************************************************************
 * RvSipContactHeaderIsStar
 * ------------------------------------------------------------------------
 * General:Determines whether or not the contact header object contains an Asterisk (*).
 *         This means that the header is of the form "Contact: * ".
 * Return Value: Returns RV_TRUE if the Contact header is of the form "Contact: * ". Otherwise,
 *               the function returns RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - Handle of the Contact header object.
 ***************************************************************************/
RVAPI RvBool RVCALLCONV RvSipContactHeaderIsStar(
                                     IN RvSipContactHeaderHandle hHeader)
{
    if (NULL == hHeader)
    {
        return RV_FALSE;
    }

    return ((MsgContactHeader *)hHeader)->bIsStar;
}

/***************************************************************************
 * SipContactHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetStrBadSyntax(
                                    IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strBadSyntax;
}

/***************************************************************************
 * RvSipContactHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax string from the header object into a
 *          given buffer.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          You use this function to retrieve the bad-syntax string.
 *          If the value of bufferLen is adequate, this function copies
 *          the requested parameter into strBuffer. Otherwise, the function
 *          returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          Use this function in the RvSipTransportBadSyntaxMsgEv() callback
 *          implementation if the message contains a bad Contact header,
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetStrBadSyntax(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetStrBadSyntax(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContactHeaderSetStrBadSyntax
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
RvStatus SipContactHeaderSetStrBadSyntax(
                                  IN RvSipContactHeaderHandle hHeader,
                                  IN RvChar*               strBadSyntax,
                                  IN HRPOOL                 hPool,
                                  IN HPAGE                  hPage,
                                  IN RvInt32               strBadSyntaxOffset)
{
    MsgContactHeader*   header;
    RvInt32          newStrOffset;
    RvStatus         retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    header = (MsgContactHeader*)hHeader;

    retStatus = MsgUtilsSetString(header->pMsgMgr, hPool, hPage, strBadSyntaxOffset,
                                  strBadSyntax, header->hPool, header->hPage, &newStrOffset);
    if (RV_OK != retStatus)
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
 * RvSipContactHeaderSetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: Sets a bad-syntax string in the object.
 *          A SIP header has the following grammar:
 *          header-name:header-value. When a header contains a syntax error,
 *          the header-value is kept as a separate "bad-syntax" string.
 *          By using this function you can create a header with "bad-syntax".
 *          Setting a bad syntax string to the header will mark the header as
 *          an invalid syntax header.
 *          You can use his function when you want to send an illegal Contact header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader      - The handle to the header object.
 *  strBadSyntax - The bad-syntax string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStrBadSyntax(
                                  IN RvSipContactHeaderHandle hHeader,
                                  IN RvChar*                     strBadSyntax)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipContactHeaderSetStrBadSyntax(hHeader, strBadSyntax, NULL, NULL_PAGE, UNDEFINED);

}

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * SipContactHeaderGetPubGruu
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'pub-gruu'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'pub-gruu' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetPubGruu(
                                    IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strPubGruu;
}

/***************************************************************************
 * RvSipContactHeaderGetPubGruu
 * ------------------------------------------------------------------------
 * General: Copies the 'pub-gruu' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetPubGruu(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetPubGruu(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipContactHeaderSetPubGruu
 * ------------------------------------------------------------------------
 * General: Sets the 'pub-gruu' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strPubGruu  - The value of the offset of the 'pub-gruu' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetPubGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strPubGruu)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipContactHeaderSetPubGruu(hHeader, strPubGruu, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContactHeaderSetPubGruu
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the PubGruu in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pPubGruu - The 'pub-gruu' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetPubGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pPubGruu,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgContactHeader* pHeader = (MsgContactHeader*) hHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pPubGruu,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderSetPubGruu: MsgUtilsSetString failed."));
        return retStatus;
    }
    pHeader->strPubGruu = newStr;
#ifdef SIP_DEBUG
    pHeader->pPubGruu = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strPubGruu);
#endif

    return RV_OK;
}
/***************************************************************************
 * SipContactHeaderGetTempGruu
 * ------------------------------------------------------------------------
 * General: The Contact header contains the value of the parameter 'temp-gruu'. This function
 *          returns this value.
 * Return Value: Returns the value of the offset 'temp-gruu' parameter in the Contact header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the Contact header object.
 ***************************************************************************/
RvInt32 SipContactHeaderGetTempGruu(
                                    IN RvSipContactHeaderHandle hHeader)
{
    return ((MsgContactHeader*)hHeader)->strTempGruu;
}

/***************************************************************************
 * RvSipContactHeaderGetTempGruu
 * ------------------------------------------------------------------------
 * General: Copies the 'temp-gruu' parameter from the Contact header into a given buffer.
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
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetTempGruu(
                                               IN RvSipContactHeaderHandle hHeader,
                                               IN RvChar*                 strBuffer,
                                               IN RvUint                  bufferLen,
                                               OUT RvUint*                actualLen)
{
     if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetTempGruu(hHeader),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipContactHeaderSetTempGruu
 * ------------------------------------------------------------------------
 * General: Sets the 'temp-gruu' parameter value in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader  - Handle to the Contact header object.
 *    strTempGruu  - The value of the offset of the 'temp-gruu' parameter to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetTempGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar                 *strTempGruu)
{
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    return SipContactHeaderSetTempGruu(hHeader, strTempGruu, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * SipContactHeaderSetTempGruu
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the TempGruu in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader - Handle of the Contact header object.
 *         pTempGruu - The 'temp-gruu' parameter to be set in the Contact Header.
 *       strOffset     - Offset of a string on the page  (if relevant).
 *       hPool - The pool on which the string lays (if relevant).
 *       hPage - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetTempGruu(
                                     IN    RvSipContactHeaderHandle hHeader,
                                     IN    RvChar *                pTempGruu,
                                     IN    HRPOOL                   hPool,
                                     IN    HPAGE                    hPage,
                                     IN    RvInt32                 strOffset)
{
    RvInt32          newStr;
    MsgContactHeader* pHeader = (MsgContactHeader*) hHeader;
    RvStatus         retStatus;

    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pTempGruu,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderSetTempGruu: MsgUtilsSetString failed."));
        return retStatus;
    }
    pHeader->strTempGruu = newStr;
#ifdef SIP_DEBUG
    pHeader->pTempGruu = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                         pHeader->strTempGruu);
#endif

    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderGetRegIDNum
 * ------------------------------------------------------------------------
 * General: Gets the reg-id from the Contact header object.
 * Return Value: Returns the reg-id number, or UNDEFINED if the reg-id
 *               does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the Contact header object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipContactHeaderGetRegIDNum(
                                         IN RvSipContactHeaderHandle hHeader)
{
    if(hHeader == NULL)
        return UNDEFINED;

    return ((MsgContactHeader*)hHeader)->regIDNum;
}


/***************************************************************************
 * RvSipContactHeaderSetRegIDNum
 * ------------------------------------------------------------------------
 * General: Sets the reg-id field in the Contact header object.
 * Return Value: RV_OK,  RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader   - Handle to the Contact header object.
 *    regIDNum  - The reg-id value to be set in the object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetRegIDNum(
                                         IN    RvSipContactHeaderHandle hHeader,
                                         IN    RvInt32                  regIDNum)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    ((MsgContactHeader*)hHeader)->regIDNum = regIDNum;
    return RV_OK;
}

/***************************************************************************
 * SipContactHeaderGetStrFeatureTag
 * ------------------------------------------------------------------------
 * General:This method gets one of the Feature Tag embedded in the Contact header.
 * Return Value: Feature Tag offset or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Contact header object..
 ***************************************************************************/
RvInt32 SipContactHeaderGetStrFeatureTag(
                                    IN RvSipContactHeaderHandle hHeader,
                                    IN RvSipContactFeatureTagType eType)
{
    MsgContactHeader*  pHeader;
    
    pHeader = (MsgContactHeader*) hHeader;

    switch(eType)
    {
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO:
        {
            return pHeader->strAudio;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA:
        {
            return pHeader->strAutomata;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS:
        {
            return pHeader->strClass;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX:
        {
            return pHeader->strDuplex;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA:
        {
            return pHeader->strData;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL:
        {
            return pHeader->strControl;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY:
        {
            return pHeader->strMobility;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION:
        {
            return pHeader->strDescription;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS:
        {
            return pHeader->strEvents;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY:
        {
            return pHeader->strPriority;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS:
        {
            return pHeader->strMethods;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES:
        {
            return pHeader->strSchemes;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION:
        {
            return pHeader->strApplication;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO:
        {
            return pHeader->strVideo;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE:
        {
            return pHeader->strLanguage;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE:
        {
            return pHeader->strType;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS:
        {
            return pHeader->strIsFocus;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR:
        {
            return pHeader->strActor;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT:
        {
            return pHeader->strText;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS:
        {
            return pHeader->strExtensions;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE:
        {
            return pHeader->strSipInstance;
            break;
        }
        default:
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderGetStrFeatureTag: Unknown Feature Tag type."));
            return UNDEFINED;
        }
    }
}

/***************************************************************************
 * RvSipContactHeaderIsFeatureTagActive
 * ------------------------------------------------------------------------
 * General: Check if the requested feature tag is active.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader   - Handle to the header object.
 *        eType     - Enumeration denoting type of feature tag.
 *        
 * output:bIsFeatureActive - Boolean denoting whether the requested tag is active.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderIsFeatureTagActive(
                                               IN  RvSipContactHeaderHandle   hHeader,
                                               IN  RvSipContactFeatureTagType eType,
                                               OUT RvBool*                    bIsFeatureActive)
{
    MsgContactHeader*  pHeader;
    
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    
    pHeader = (MsgContactHeader*) hHeader;

    switch(eType)
    {
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO:
        {
            *bIsFeatureActive = pHeader->bIsAudio;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA:
        {
            *bIsFeatureActive = pHeader->bIsAutomata;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS:
        {
            *bIsFeatureActive = pHeader->bIsClass;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX:
        {
            *bIsFeatureActive = pHeader->bIsDuplex;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA:
        {
            *bIsFeatureActive = pHeader->bIsData;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL:
        {
            *bIsFeatureActive = pHeader->bIsControl;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY:
        {
            *bIsFeatureActive = pHeader->bIsMobility;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION:
        {
            *bIsFeatureActive = pHeader->bIsDescription;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS:
        {
            *bIsFeatureActive = pHeader->bIsEvents;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY:
        {
            *bIsFeatureActive = pHeader->bIsPriority;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS:
        {
            *bIsFeatureActive = pHeader->bIsMethods;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES:
        {
            *bIsFeatureActive = pHeader->bIsSchemes;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION:
        {
            *bIsFeatureActive = pHeader->bIsApplication;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO:
        {
            *bIsFeatureActive = pHeader->bIsVideo;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE:
        {
            *bIsFeatureActive = pHeader->bIsLanguage;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE:
        {
            *bIsFeatureActive = pHeader->bIsType;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS:
        {
            *bIsFeatureActive = pHeader->bIsIsFocus;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR:
        {
            *bIsFeatureActive = pHeader->bIsActor;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT:
        {
            *bIsFeatureActive = pHeader->bIsText;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS:
        {
            *bIsFeatureActive = pHeader->bIsExtensions;
            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE:
        {
            *bIsFeatureActive = pHeader->bIsSipInstance;
            break;
        }
        default:
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderIsFeatureTagActive: Unknown Feature Tag type."));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderGetStrFeatureTag
 * ------------------------------------------------------------------------
 * General: Copies the Feature Tag from the Contact header into a given buffer.
 *          If the bufferLen is adequate, the function copies the requested parameter into
 *          strBuffer. Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and actualLen
 *          contains the required buffer length.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader    - Handle to the header object.
 *        eType      - Enumeration denoting type of feature tag.
 *        strBuffer  - Buffer to fill with the requested parameter.
 *        bufferLen  - The length of the buffer.
 * output:actualLen  - The length of the requested parameter, + 1 to include a NULL value at the end
 *                     of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetStrFeatureTag(
                                               IN RvSipContactHeaderHandle   hHeader,
                                               IN RvSipContactFeatureTagType eType,
                                               IN RvChar*                    strBuffer,
                                               IN RvUint                     bufferLen,
                                               OUT RvUint*                   actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgContactHeader*)hHeader)->hPool,
                                  ((MsgContactHeader*)hHeader)->hPage,
                                  SipContactHeaderGetStrFeatureTag(hHeader, eType),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * SipContactHeaderSetStrFeatureTag
 * ------------------------------------------------------------------------
 * General: This is an internal function that sets the pFeatureTag in the
 *          ContactHeader object. the API will call it with NULL pool
 *          and page, to make a real allocation and copy. internal modules
 *          (such as parser) will call directly to this function, with valid
 *          pool and page to avoid unneeded copying.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:hHeader     - Handle of the Contact header object.
 *       eType       - Enumeration denoting type of feature tag.
 *       pFeatureTag - The Feature Tag to be set in the Contact header - If NULL, 
 *                     the exist displayName string in the header will be removed.
 *       strOffset   - Offset of a string on the page  (if relevant).
 *       hPool       - The pool on which the string lays (if relevant).
 *       hPage       - The page on which the string lays (if relevant).
 ***************************************************************************/
RvStatus SipContactHeaderSetStrFeatureTag(
                                     IN    RvSipContactHeaderHandle   hHeader,
                                     IN    RvSipContactFeatureTagType eType,
                                     IN    RvChar*                    pFeatureTag,
                                     IN    HRPOOL                     hPool,
                                     IN    HPAGE                      hPage,
                                     IN    RvInt32                    strOffset)
{
    RvInt32           newStr;
    MsgContactHeader* pHeader;
    RvStatus          retStatus;

    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

    pHeader = (MsgContactHeader*) hHeader;

    retStatus = MsgUtilsSetString(pHeader->pMsgMgr, hPool, hPage, strOffset, pFeatureTag,
                                  pHeader->hPool, pHeader->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    
    switch(eType)
    {
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO:
        {
            pHeader->strAudio = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsAudio = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pAudio = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strAudio);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA:
        {
            pHeader->strAutomata = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsAutomata = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pAutomata = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strAutomata);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS:
        {
            pHeader->strClass = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsClass = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pClass = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strClass);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX:
        {
            pHeader->strDuplex = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsDuplex = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pDuplex = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strDuplex);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA:
        {
            pHeader->strData = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsData = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pData = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strData);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL:
        {
            pHeader->strControl = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsControl = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pControl = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strControl);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY:
        {
            pHeader->strMobility = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsMobility = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pMobility = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strMobility);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION:
        {
            pHeader->strDescription = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsDescription = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pDescription = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strDescription);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS:
        {
            pHeader->strEvents = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsEvents = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pEvents = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strEvents);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY:
        {
            pHeader->strPriority = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsPriority = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pPriority = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strPriority);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS:
        {
            pHeader->strMethods = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsMethods = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pMethods = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strMethods);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES:
        {
            pHeader->strSchemes = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsSchemes = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pSchemes = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strSchemes);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION:
        {
            pHeader->strApplication = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsApplication = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pApplication = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strApplication);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO:
        {
            pHeader->strVideo = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsVideo = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pVideo = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strVideo);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE:
        {
            pHeader->strLanguage = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsLanguage = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pLanguage = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strLanguage);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE:
        {
            pHeader->strType = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsType = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pType = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strType);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS:
        {
            pHeader->strIsFocus = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsIsFocus = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pIsFocus = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strIsFocus);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR:
        {
            pHeader->strActor = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsActor = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pActor = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strActor);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT:
        {
            pHeader->strText = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsText = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pText = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strText);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS:
        {
            pHeader->strExtensions = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsExtensions = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pExtensions = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strExtensions);
#endif

            break;
        }
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE:
        {
            pHeader->strSipInstance = newStr;
            if(newStr > UNDEFINED)
                pHeader->bIsSipInstance = RV_TRUE;
#ifdef SIP_DEBUG
            pHeader->pSipInstance = (RvChar *)RPOOL_GetPtr(pHeader->hPool, pHeader->hPage,
                                                 pHeader->strSipInstance);
#endif

            break;
        }
        default:
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "SipContactHeaderSetStrFeatureTag: Unknown Feature Tag type."));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderSetStrFeatureTag
 * ------------------------------------------------------------------------
 * General:Sets a Feature Tag in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader     - Handle to the header object.
 *    eType       - Enumeration denoting type of feature tag.
 *    pFeatureTag - The Feature Tag to be set in the Contact header. If NULL is supplied, the
 *                  existing Feature Tag is removed from the header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetStrFeatureTag(
                                     IN RvSipContactHeaderHandle   hHeader,
                                     IN RvSipContactFeatureTagType eType,
                                     IN RvChar*                    pFeatureTag)
{
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;

     return SipContactHeaderSetStrFeatureTag(hHeader, eType, pFeatureTag, NULL, NULL_PAGE, UNDEFINED);
}

/***************************************************************************
 * RvSipContactHeaderSetFeatureTagActivation
 * ------------------------------------------------------------------------
 * General: Activates/deactivates a specific Feature Tag in the Contact header object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle to the header object.
 *    eType   - The Feature Tag to be activated in the Contact header. 
 *    bIsActivated - boolean denoting should the feature be activated or deactivated.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetFeatureTagActivation(
                                     IN RvSipContactHeaderHandle   hHeader,
                                     IN RvSipContactFeatureTagType eType,
                                     IN RvBool                     bIsActivated)
{
    MsgContactHeader*  pHeader;
    
    if(hHeader == NULL)
        return RV_ERROR_INVALID_HANDLE;
    
    pHeader = (MsgContactHeader*) hHeader;

     switch (eType)
     {
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO:
            pHeader->bIsAudio = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA:
            pHeader->bIsAutomata = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS:
            pHeader->bIsClass = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX:
            pHeader->bIsDuplex = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA:
            pHeader->bIsData = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL:
            pHeader->bIsControl = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY:
            pHeader->bIsMobility = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION:
            pHeader->bIsDescription = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS:
            pHeader->bIsEvents = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY:
            pHeader->bIsPriority = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS:
            pHeader->bIsMethods = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES:
            pHeader->bIsSchemes = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION:
            pHeader->bIsApplication = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO:
            pHeader->bIsVideo = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE:
            pHeader->bIsLanguage = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE:
            pHeader->bIsType = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS:
            pHeader->bIsIsFocus = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR:
            pHeader->bIsActor = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT:
            pHeader->bIsText = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS:
            pHeader->bIsExtensions = bIsActivated;
            break;
        case RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE:
            pHeader->bIsSipInstance = bIsActivated;
            break;
        default:
        {
            RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderSetFeatureTagActivation: Unknown Feature Tag type."));
            return RV_ERROR_ILLEGAL_ACTION;
        }
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 

/***************************************************************************
 * RvSipContactHeaderGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the Contact header into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipContactHeader - Handle to the Contact header object.
 *           eStringName   - The string the user wish to get
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Contact header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetRpoolString(
                             IN    RvSipContactHeaderHandle      hSipContactHeader,
                             IN    RvSipContactHeaderStringName  eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgContactHeader* pHeader = (MsgContactHeader*)hSipContactHeader;
    RvInt32      requestedParamOffset;

    if(hSipContactHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContactHeaderGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_CONTACT_DISPLAYNAME:
        requestedParamOffset = pHeader->strDisplayName;
        break;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    case RVSIP_CONTACT_QVAL:
        requestedParamOffset = pHeader->strQVal;
        break;
#ifdef RV_SIP_IMS_HEADER_SUPPORT 
    case RVSIP_CONTACT_TEMP_GRUU:
        requestedParamOffset = pHeader->strTempGruu;
        break;
    case RVSIP_CONTACT_PUB_GRUU:
        requestedParamOffset = pHeader->strPubGruu;
        break;
    case RVSIP_CONTACT_AUDIO:
        requestedParamOffset = pHeader->strAudio;
        break;
    case RVSIP_CONTACT_AUTOMATA:
        requestedParamOffset = pHeader->strAutomata;
        break;
    case RVSIP_CONTACT_CLASS:
        requestedParamOffset = pHeader->strClass;
        break;
    case RVSIP_CONTACT_DUPLEX:
        requestedParamOffset = pHeader->strDuplex;
        break;
    case RVSIP_CONTACT_DATA:
        requestedParamOffset = pHeader->strData;
        break;
    case RVSIP_CONTACT_CONTROL:
        requestedParamOffset = pHeader->strControl;
        break;
    case RVSIP_CONTACT_MOBILITY:
        requestedParamOffset = pHeader->strMobility;
        break;
    case RVSIP_CONTACT_DESCRIPTION:
        requestedParamOffset = pHeader->strDescription;
        break;
    case RVSIP_CONTACT_EVENTS:
        requestedParamOffset = pHeader->strEvents;
        break;
    case RVSIP_CONTACT_PRIORITY:
        requestedParamOffset = pHeader->strPriority;
        break;
    case RVSIP_CONTACT_METHODS:
        requestedParamOffset = pHeader->strMethods;
        break;
    case RVSIP_CONTACT_SCHEMES:
        requestedParamOffset = pHeader->strSchemes;
        break;
    case RVSIP_CONTACT_APPLICATION:
        requestedParamOffset = pHeader->strApplication;
        break;
    case RVSIP_CONTACT_VIDEO:
        requestedParamOffset = pHeader->strVideo;
        break;
    case RVSIP_CONTACT_LANGUAGE:
        requestedParamOffset = pHeader->strLanguage;
        break;
    case RVSIP_CONTACT_TYPE:
        requestedParamOffset = pHeader->strType;
        break;
    case RVSIP_CONTACT_ISFOCUS:
        requestedParamOffset = pHeader->strIsFocus;
        break;
    case RVSIP_CONTACT_ACTOR:
        requestedParamOffset = pHeader->strActor;
        break;
    case RVSIP_CONTACT_TEXT:
        requestedParamOffset = pHeader->strText;
        break;
    case RVSIP_CONTACT_EXTENSIONS:
        requestedParamOffset = pHeader->strExtensions;
        break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
        
    case RVSIP_CONTACT_BAD_SYNTAX:
        requestedParamOffset = pHeader->strBadSyntax;
        break;
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContactHeaderGetRpoolString - Failed, the requested parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
    /* the parameter does not exit in the header - return UNDEFINED*/
    if(requestedParamOffset == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(
                                     pHeader->pMsgMgr,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pHeader->hPool,
                                     pHeader->hPage,
                                     requestedParamOffset);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                  "RvSipContactHeaderGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipContactHeaderSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the Contact header
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipContactHeader - Handle to the Contact header object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetRpoolString(
                             IN    RvSipContactHeaderHandle      hSipContactHeader,
                             IN    RvSipContactHeaderStringName  eStringName,
                             IN    RPOOL_Ptr                 *pRpoolPtr)
{
    MsgContactHeader   *pHeader;

    pHeader = (MsgContactHeader*)hSipContactHeader;
    if(hSipContactHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                 "RvSipContactHeaderSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
#ifndef RV_SIP_JSR32_SUPPORT
    case RVSIP_CONTACT_DISPLAYNAME:
        return SipContactHeaderSetDisplayName(hSipContactHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

    case RVSIP_CONTACT_QVAL:
        return SipContactHeaderSetQVal(hSipContactHeader,NULL,
                                              pRpoolPtr->hPool,
                                              pRpoolPtr->hPage,
                                              pRpoolPtr->offset);
    default:
        RvLogError(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
                "RvSipContactHeaderSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

}

/***************************************************************************
 * RvSipContactHeaderSetCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the header to use the compact header name when the
 *          header is encoded.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Contact header object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderSetCompactForm(
                                   IN    RvSipContactHeaderHandle hHeader,
                                   IN    RvBool                bIsCompact)
{
    MsgContactHeader* pHeader = (MsgContactHeader*)hHeader;
    if(hHeader == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pHeader->pMsgMgr->pLogSrc,(pHeader->pMsgMgr->pLogSrc,
             "RvSipContactHeaderSetCompactForm - Setting compact form of hHeader 0x%p to %d",
             hHeader, bIsCompact));

    pHeader->bIsCompact = bIsCompact;
    return RV_OK;
}

/***************************************************************************
 * RvSipContactHeaderGetCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form flag of the header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Contact header object.
 *        pbIsCompact - specifies whether or not compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipContactHeaderGetCompactForm(
                                   IN    RvSipContactHeaderHandle hHeader,
                                   IN    RvBool                *pbIsCompact)
{
    MsgContactHeader* pHeader = (MsgContactHeader*)hHeader;
    if(hHeader == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	*pbIsCompact = pHeader->bIsCompact;
    return RV_OK;

}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * isLegalQVal
 * ------------------------------------------------------------------------
 * General: The function returns RV_TRUE if the 'q' parameter in the Contact
 *          Header is legal, RV_FALSE - otherwise.
 * Return Value: The function returns RV_TRUE if the 'q' parameter in the Contact
 *               Header is legal, RV_FALSE - otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pQVal - a string contains the 'q' parameter.
 ***************************************************************************/
static RvBool isLegalQVal(RvChar *pQVal)
{
    RvInt32 len;

    if((pQVal == NULL) || (pQVal[0] == '\0'))
    {
        return RV_TRUE;
    }
    len = (RvInt32)strlen(pQVal);

    if( (len > 5) ||
        ((pQVal[0] != '0') && (pQVal[0] != '1')) ||
        ((pQVal[1] != '.') && (pQVal[1] != '\0')) )
    {
        return RV_FALSE;
    }
    else
    {
        if(len == 2 && (pQVal[1] == '.'))
        {
            return RV_FALSE;
        }
        if(len >= 3)
        {
            if(pQVal[2] < '0' || pQVal[2] > '9')
            {
                return RV_FALSE;
            }
            else
            {
                if(len == 3)
                {
                    return RV_TRUE;
                }
            }
        }
        if(len >= 4)
        {
            if(pQVal[3] < '0' || pQVal[3] > '9')
            {
                return RV_FALSE;
            }
            else
            {
                if(len == 4)
                {
                    return RV_TRUE;
                }
            }
        }
        if(len == 5)
        {
            if(pQVal[4] < '0' || pQVal[4] > '9')
            {
                return RV_FALSE;
            }
        }
        return RV_TRUE;
    }
}
/***************************************************************************
 * ContactHeaderClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress        - Pointer to the object to clean.
 *    bCleanBS        - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void ContactHeaderClean( IN MsgContactHeader* pHeader,
							    IN RvBool            bCleanBS)
{
    pHeader->hAddrSpec        = NULL;
    pHeader->strContactParams = UNDEFINED;
    pHeader->hExpiresParam    = NULL;
    pHeader->strQVal          = UNDEFINED;
    pHeader->eAction          = RVSIP_CONTACT_ACTION_UNDEFINED;
    pHeader->bIsStar          = RV_FALSE;
    pHeader->eHeaderType      = RVSIP_HEADERTYPE_CONTACT;
#ifndef RV_SIP_JSR32_SUPPORT
	pHeader->strDisplayName   = UNDEFINED;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pHeader->strTempGruu        = UNDEFINED;
    pHeader->strPubGruu         = UNDEFINED;
    pHeader->regIDNum           = UNDEFINED;

    pHeader->strAudio           = UNDEFINED;
    pHeader->bIsAudio           = RV_FALSE;
    pHeader->strAutomata        = UNDEFINED;
    pHeader->bIsAutomata        = RV_FALSE;
    pHeader->strClass           = UNDEFINED;
    pHeader->bIsClass           = RV_FALSE;
    pHeader->strDuplex          = UNDEFINED;
    pHeader->bIsDuplex          = RV_FALSE;
    pHeader->strData            = UNDEFINED;
    pHeader->bIsData            = RV_FALSE;
    pHeader->strControl         = UNDEFINED;
    pHeader->bIsControl         = RV_FALSE;
    pHeader->strMobility        = UNDEFINED;
    pHeader->bIsMobility        = RV_FALSE;
    pHeader->strDescription     = UNDEFINED;
    pHeader->bIsDescription     = RV_FALSE;
    pHeader->strEvents          = UNDEFINED;
    pHeader->bIsEvents          = RV_FALSE;
    pHeader->strPriority        = UNDEFINED;
    pHeader->bIsPriority        = RV_FALSE;
    pHeader->strMethods         = UNDEFINED;
    pHeader->bIsMethods         = RV_FALSE;
    pHeader->strSchemes         = UNDEFINED;
    pHeader->bIsSchemes         = RV_FALSE;
    pHeader->strApplication     = UNDEFINED;
    pHeader->bIsApplication     = RV_FALSE;
    pHeader->strVideo           = UNDEFINED;
    pHeader->bIsVideo           = RV_FALSE;
    pHeader->strLanguage        = UNDEFINED;
    pHeader->bIsLanguage        = RV_FALSE;
    pHeader->strType            = UNDEFINED;
    pHeader->bIsType            = RV_FALSE;
    pHeader->strIsFocus         = UNDEFINED;
    pHeader->bIsIsFocus         = RV_FALSE;
    pHeader->strActor           = UNDEFINED;
    pHeader->bIsActor           = RV_FALSE;
    pHeader->strText            = UNDEFINED;
    pHeader->bIsText            = RV_FALSE;
    pHeader->strExtensions      = UNDEFINED;
    pHeader->bIsExtensions      = RV_FALSE;
    
    pHeader->strSipInstance     = UNDEFINED;
    pHeader->bIsSipInstance     = RV_FALSE;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    
#ifdef SIP_DEBUG
    pHeader->pContactParams     = NULL;
    pHeader->pQVal              = NULL;
#ifndef RV_SIP_JSR32_SUPPORT    
	pHeader->pDisplayName       = NULL;
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    pHeader->pTempGruu          = NULL;
    pHeader->pPubGruu           = NULL;
    
    pHeader->pAudio			    = NULL;
    pHeader->pAutomata		    = NULL;
    pHeader->pClass			    = NULL;
    pHeader->pDuplex		    = NULL;
    pHeader->pData			    = NULL;
    pHeader->pControl		    = NULL;
    pHeader->pMobility		    = NULL;
    pHeader->pDescription	    = NULL;
    pHeader->pEvents		    = NULL;
    pHeader->pPriority		    = NULL;
    pHeader->pMethods		    = NULL;
    pHeader->pSchemes		    = NULL;
    pHeader->pApplication	    = NULL;
    pHeader->pVideo			    = NULL;
    pHeader->pLanguage		    = NULL;
    pHeader->pType			    = NULL;
    pHeader->pIsFocus		    = NULL;
    pHeader->pActor			    = NULL;
    pHeader->pText			    = NULL;
    pHeader->pExtensions	    = NULL;

    pHeader->pSipInstance       = NULL;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    
#endif

	if(bCleanBS == RV_TRUE)
    {
		pHeader->bIsCompact       = RV_FALSE;
        pHeader->strBadSyntax     = UNDEFINED;
#ifdef SIP_DEBUG
        pHeader->pBadSyntax       = NULL;
#endif
    }
}

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif



