/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              <TransmitterObject.c>
 *
 * This file contains the functionality of the Transmitter object
 * The transmitter object is used to send and to retransmit sip messages.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  January 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "RvSipTransport.h"

#include "RvSipAddress.h"
#include "_SipCommonCore.h"
#include "_SipCommonTypes.h"
#include "_SipCompartment.h"
#include "_SipTransmitter.h"
#include "_SipTransport.h"
#include "_SipViaHeader.h"
#include "TransmitterObject.h"
#include "TransmitterCallbacks.h"
#include "RvSipViaHeader.h"
#include "AdsRlist.h"
#include "TransmitterControl.h"
#include "TransmitterDest.h"
#include "rvansi.h"
#include "_SipResolverMgr.h"
#include "_SipResolver.h"

#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctp.h"
#endif
#ifdef RV_SIP_IMS_ON
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "RvSipSecurity.h"
#include "RvSipSecAgree.h"
#include "SecurityObject.h"
#endif
/* SPIRENT_END */
#include "_SipSecuritySecObj.h"
#endif

#ifdef RV_SIP_OTHER_URI_SUPPORT
#include "RvSipRouteHopHeader.h"
#include "RvSipMsg.h"
#include "_SipAddress.h"
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define TRANSMITTER_STATE_TERMINATED_STR "Terminated"
#define TRANSMITTER_OBJECT_NAME_STR      "Transmitter"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV MsgToSendListConstruct(
                                     IN Transmitter    *pTrx);

static void RVCALLCONV MsgToSendListFreeAllPages(
                              IN Transmitter    *pTrx);


static void RVCALLCONV MsgToSendListDestruct(
                                     IN Transmitter    *pTrx);

static RvStatus SetViaSentByParamByLocalAddress(
						IN Transmitter                   *pTrx,
						IN RvSipTransportLocalAddrHandle hLocalAddr,
                        IN RvSipViaHeaderHandle          hVia,
                        IN RvUint16*                     pSecureServerPort);

static RvStatus RVCALLCONV TerminateTransmitterEventHandler(
                                         IN void*      obj,
                                         IN RvInt32    reason);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"
                  
#endif
/* SPIRENT_END */



static RvStatus RVCALLCONV ChooseLocalAddressForViaFromLocalAddresses(
                                     IN    Transmitter*                   pTrx,
                                     IN    RvSipTransport                 eTransportType,
                                     OUT   RvSipTransportLocalAddrHandle* phLocalAddr);


static void AddScopeToDestIpv6AddrIfNeed(IN Transmitter* pTrx);


static RvSipTransmitterReason ConvertRvToTrxReason(
                                         IN  RvStatus        rv);
#ifdef RV_SIP_OTHER_URI_SUPPORT
static void FixMsgSendingAddrIfNeed(IN Transmitter* pTrx);
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

#ifdef RV_SIGCOMP_ON
static RvStatus LookForSigCompUrnInSentMsg(IN   Transmitter      *pTrx,
                                           IN   RvSipMsgHandle    hMsgToSend,
                                           OUT  RvBool           *pbUrnFound,
                                           OUT  RPOOL_Ptr        *pUrnRpoolPtr);

static void	convertTransportTypeToSigcompTransport(IN Transmitter *pTrx, OUT RvInt *pTransportType);

static void populateTransportAddessFromLocalAddr (IN  Transmitter         *pTrx, 
												  OUT SipTransportAddress *address);
#endif /* #ifdef RV_SIGCOMP_ON */

/*-----------------LOCKING FUNCTIONS------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
static RvStatus RVCALLCONV TransmitterLock(
                                     IN Transmitter      *pTrx,
                                     IN TransmitterMgr   *pTrxMgr,
                                     IN SipTripleLock    *pTripleLock,
                                     IN RvInt32           identifier);

static void RVCALLCONV TransmitterUnLock(
                                IN RvMutex *pLock,
                                IN RvLogMgr* logMgr);

static void RVCALLCONV TransmitterTerminateUnlock(
                              IN RvLogMgr              *pLogMgr,
                               IN RvLogSource*          pLogSrc,
                               IN RvMutex              *hObjLock,
                               IN RvMutex              *hProcessingLock);


#else
#define TransmitterLock(a,b,c,d) (RV_OK)
#define TransmitterUnLock(a)
#define TransmitterTerminateUnlock(a,b,c,d)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */

/*-----------------------------------------------------------------------*/
/*                         TRANSMITTER FUNCTIONS                         */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransmitterInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new transmitter in the Idle state.
 *          If a page is supplied the transmitter will use it. Otherwise
 *          a new page will be allocated.
 *          Note: When calling this function the transmitter manager is locked.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the new Transmitter
 *            bIsAppTrx - indicates whether the transmitter was created
 *                        by the application or by the stack.
 *            hAppTrx   - Application handle to the transmitter.
 *            hPool     - A pool for this transmitter
 *            hPage     - A memory page to be used by this transmitter. If NULL
 *                        is supplied the transmitter will allocate a new page.
 *            pTripleLock - A triple lock to use by the transmitter. If NULL
 *                        is supplied the transmitter will use its own lock.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterInitialize(
                             IN Transmitter*              pTrx,
                             IN RvBool                    bIsAppTrx,
                             IN RvSipAppTransmitterHandle hAppTrx,
                             IN HRPOOL                    hPool,
                             IN HPAGE                     hPage,
							 IN SipTripleLock*            pTripleLock)
{
    RvStatus rv = RV_OK;
    /*initialize the transmitters fields*/
    memset(&(pTrx->terminationInfo),0,sizeof(RvSipTransportObjEventInfo));
    memset(&(pTrx->localAddr),0,sizeof(SipTransportObjLocalAddresses));
    memset(&(pTrx->evHandlers),0,sizeof(RvSipTransmitterEvHandlers));
    SipTransportInitObjectOutboundAddr(pTrx->pTrxMgr->hTransportMgr,&pTrx->outboundAddr);
    pTrx->hAppTrx                    = hAppTrx;
    pTrx->hDnsList                   = NULL;
    pTrx->eState                     = RVSIP_TRANSMITTER_STATE_IDLE;
    pTrx->eResolutionState           = TRANSMITTER_RESOLUTION_STATE_UNDEFINED;
    pTrx->hMsgToSend                 = NULL;
    pTrx->hPage                      = NULL;
	pTrx->pTripleLock				 = (NULL == pTripleLock) ? &pTrx->trxTripleLock : pTripleLock;
	pTrx->bSendingBuffer             = RV_FALSE;


    /* resetting the dest address */
    memset(&pTrx->destAddr,0,sizeof(SipTransportAddress));
    pTrx->destAddr.eTransport        = RVSIP_TRANSPORT_UNDEFINED;


    /* resetting msg params */
    pTrx->msgParams.strHostName     = NULL;
    pTrx->msgParams.hostNameOffset  = UNDEFINED;
    pTrx->msgParams.transportType   = RVSIP_TRANSPORT_UNDEFINED;
    pTrx->msgParams.bIsSecure       = RV_FALSE;
    pTrx->msgParams.port            = UNDEFINED;
	pTrx->msgParams.msgType         = RVSIP_MSG_UNDEFINED;

    pTrx->strQuery                  = NULL;

    RvRandomGeneratorGetInRange(pTrx->pTrxMgr->seed,RV_INT32_MAX,(RvRandom*)&pTrx->trxUniqueIdentifier);
    pTrx->bTrxCreatedVia             = RV_FALSE;
    pTrx->bFixVia                    = RV_FALSE;
    pTrx->bSkipViaProcessing         = RV_FALSE;
    pTrx->bIgnoreOutboundProxy       = RV_FALSE;
    pTrx->bIsAppTrx                  = bIsAppTrx;
    pTrx->bIsPersistent              = pTrx->pTrxMgr->bIsPersistent;
    pTrx->bKeepMsg                   = RV_FALSE;
    pTrx->bSendToFirstRoute          = RV_FALSE;
    pTrx->bMoveToMsgSendFailure      = RV_FALSE;
#ifdef RV_SIGCOMP_ON
    pTrx->eOutboundMsgCompType       = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
    pTrx->eNextMsgCompType           = RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED;
    pTrx->bSigCompEnabled            = RV_TRUE;
    pTrx->hSigCompCompartment        = NULL;
    pTrx->hSigCompOutboundMsg        = NULL_PAGE;
	pTrx->sigcompIdOffset            = UNDEFINED;
#endif /*#ifdef RV_SIGCOMP_ON*/
    pTrx->bAddrManuallySet           = RV_FALSE;
    pTrx->hConn                      = NULL;
    pTrx->outboundProxyHostNameOffset= UNDEFINED;
    pTrx->viaBranchOffset            = UNDEFINED;
    pTrx->hSentOutboundMsg           = NULL_PAGE;
    pTrx->hEncodedOutboundMsg        = NULL_PAGE;
    pTrx->encodedMsgLen              = 0;

    pTrx->bCopyAppMsg                = RV_TRUE;
    pTrx->hMsgToSendList             = NULL;
    pTrx->eMsgType                   = SIP_TRANSPORT_MSG_UNDEFINED;
    pTrx->hNextMessageHop            = NULL;
    pTrx->bForceOutboundAddr         = RV_FALSE;
    pTrx->bFreePageOnTermination     = RV_FALSE;
    pTrx->bConnWasSetByApp           = RV_FALSE;
    pTrx->bSendByAlias               = RV_FALSE;
#ifdef RV_SIP_OTHER_URI_SUPPORT
    pTrx->eMsgAddrUsedForSending     = TRANSMITTER_MSG_UNDEFINED;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

#ifdef RV_SIP_IMS_ON
	pTrx->hSecObj                    = NULL;
    pTrx->hSecureConn                = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef SIP_DEBUG
    pTrx->strViaBranch               = NULL;
#endif /*#ifndef SIP_DEBUG*/
    /*initialize default local address*/
    rv = SipTransportLocalAddressInitDefaults(pTrx->pTrxMgr->hTransportMgr,
                                              &pTrx->localAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterInitialize - Failed for transmitter 0x%p, (rv=%d) in function SipTransportLocalAddressInitDefaults"
                  ,pTrx,rv));
        return rv;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    RvSipTransportSctpInitializeMsgSendingParams(&pTrx->sctpParams);
#endif

    if(hPage != NULL)
    {
        pTrx->hPool = hPool;
        pTrx->hPage = hPage;
        /* update the number of users of this page */
        RPOOL_AddPageUser(hPool, hPage);
    }
    else
    {
        pTrx->hPool = pTrx->pTrxMgr->hGeneralPool;
        RPOOL_GetPage(pTrx->hPool,0,&pTrx->hPage);
    }
    if(pTrx->hPage == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterInitialize - Failed to allocate page for the transmitter (rv=%d)",rv));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /*construct the message to send list. Note -  if this function fails there
      is no need to destruct this list since it is on the transmitter page*/
    rv = MsgToSendListConstruct(pTrx);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterInitialize - Failed for transmitter 0x%p, (rv=%d) in function MsgToSendListConstruct"
                  ,pTrx,rv));
        /* Free the new page was allocated for this transmitter  OR */
        /* reduce the PageUser counter of the input (non-NULL) page */
        RPOOL_FreePage(pTrx->hPool,pTrx->hPage);
        return rv;
    }

    /*construct an empty DNS list object for the transmitter*/
    rv = SipTransportDNSListConstruct(pTrx->hPool,pTrx->hPage,
                                      SipTransportGetMaxDnsElements(pTrx->pTrxMgr->hTransportMgr),
                                      &pTrx->hDnsList);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterInitialize - Failed to create a new DNS list (rv=%d)",rv));
        /* Free the new page was allocated for this transmitter  OR */
        /* reduce the PageUser counter of the input (non-NULL) page */
        RPOOL_FreePage(pTrx->hPool,pTrx->hPage);
        return rv;
    }

    /* creating the resolver with transmitter pages and locks */
    rv = SipResolverMgrCreateResolver(pTrx->pTrxMgr->hRslvMgr,
        (RvSipAppResolverHandle)pTrx,
        pTrx->pTripleLock,
        &pTrx->hRslv);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterInitialize - Failed to create resolver (rv=%d)",rv));
		/* Free the new page was allocated for this transmitter  OR */
        /* reduce the PageUser counter of the input (non-NULL) page */
        RPOOL_FreePage(pTrx->hPool,pTrx->hPage);
        return rv;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "TransmitterInitialize - Transmitter 0x%p was initialized,owner=0x%p,page=0x%p,pool=0x%p",
               pTrx,pTrx->hAppTrx,pTrx->hPage,pTrx->hPool));

    return RV_OK;
}


/***************************************************************************
 * TransmitterSetTripleLock
 * ------------------------------------------------------------------------
 * General: Sets the transmitter triple lock.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx - The transmitter handle
 *           pTripleLock - The new triple lock
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV TransmitterSetTripleLock(
                         IN Transmitter*       pTrx,
                         IN SipTripleLock*     pTripleLock)
{
    if(pTripleLock == NULL || pTrx == NULL)
    {
        return;
    }
    /* Set the new triple lock */
    RvMutexLock(&pTrx->trxTripleLock.hLock,pTrx->pTrxMgr->pLogMgr);
    pTrx->pTripleLock = pTripleLock;
    RvMutexUnlock(&pTrx->trxTripleLock.hLock,pTrx->pTrxMgr->pLogMgr);

	/* Set resolver triple lock */
	if (pTrx->hRslv != NULL)
	{
		SipResolverSetTripleLock(pTrx->hRslv,pTripleLock);
	}
}

/************************************************************************************
 * TransmitterCopy
 * ----------------------------------------------------------------------------------
 * General: Copies relevant information from the source transmitter
 *          to the destination transmitter
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pDestTrx      - Destination transmitter
 *          pSrcTrx       - Source transmitter
 *          bCopyDnsList - Indicates whether to copy the DNS list or not.
 *
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterCopy(
                     IN  Transmitter*    pDestTrx,
                     IN  Transmitter*    pSrcTrx,
                     IN  RvBool          bCopyDnsList)

{
#ifdef RV_SIGCOMP_ON
	RvInt32 sigcompIdLen;
#endif
    RvStatus   rv =  RV_OK;
    RvLogDebug(pSrcTrx->pTrxMgr->pLogSrc ,(pSrcTrx->pTrxMgr->pLogSrc ,
               "TransmitterCopy - Coping  pSrcTrx 0x%p to pDestTrx 0x%p (bCopyDnsList=%d)",
               pSrcTrx,pDestTrx,bCopyDnsList));

    /*copy the DNS list if needed*/
    if (RV_TRUE == bCopyDnsList)
    {
        rv = SipTransportDNSListClone(pSrcTrx->pTrxMgr->hTransportMgr,
                                      pSrcTrx->hDnsList,
                                      pDestTrx->hDnsList);
        if (rv != RV_OK)
        {
            return rv;
        }
    }
    memcpy(&pDestTrx->destAddr,&pSrcTrx->destAddr,sizeof(pSrcTrx->destAddr));

    pDestTrx->bTrxCreatedVia        = pSrcTrx->bTrxCreatedVia;
    pDestTrx->bIgnoreOutboundProxy  = pSrcTrx->bIgnoreOutboundProxy;
    pDestTrx->bIsAppTrx             = pSrcTrx->bIsAppTrx;
    pDestTrx->bIsPersistent         = pSrcTrx->bIsPersistent;
    pDestTrx->bSendToFirstRoute     = pSrcTrx->bSendToFirstRoute;


	/* The msgParams have to be copied into the new Trx since some */
	/* failure resolutions which lead to MSG_SEND_FAILURE may be   */
	/* continued by RvSipXXXDNSContinue. In this continuation      */
	/* function the msgParams are being retrieved (especially the  */
	/* parameter strHostName).  */
	memcpy(&pDestTrx->msgParams,&pSrcTrx->msgParams,sizeof(pDestTrx->msgParams));

	if (pSrcTrx->msgParams.strHostName != NULL)
	{
		rv = TransmitterDestAllocateHostBuffer(pDestTrx,
											   (RvUint)strlen(pSrcTrx->msgParams.strHostName)+1,
											   &pDestTrx->msgParams.strHostName,
											   &pDestTrx->msgParams.hostNameOffset);
		if (rv != RV_OK)
		{
			RvLogError(pSrcTrx->pTrxMgr->pLogSrc ,(pSrcTrx->pTrxMgr->pLogSrc ,
				"TransmitterCopy - Transmitter 0x%p - Failed to allocate host part",
				pSrcTrx));
			return rv;
		}
		strcpy(pDestTrx->msgParams.strHostName,pSrcTrx->msgParams.strHostName);
	}

    if (RV_FALSE == bCopyDnsList)
    {
        /* we are not going to do resolution. if address is resolved, leave it that way. */
        pDestTrx->eResolutionState = pSrcTrx->eResolutionState;
    }
    else
    {
        /* we will try to resolve, move to undefined*/
        pDestTrx->eResolutionState = TRANSMITTER_RESOLUTION_STATE_UNDEFINED;
    }

#ifdef RV_SIGCOMP_ON
    /* NOTE: The field pSrcTrx->hSigCompCompartment MUST NOT be copied */
    /* since it is related to the Trx's destination */
    pDestTrx->bSigCompEnabled       = pSrcTrx->bSigCompEnabled;
    pDestTrx->eOutboundMsgCompType  = pSrcTrx->eOutboundMsgCompType;
	/* copy the sigcomp-id */
	if (pSrcTrx->sigcompIdOffset > UNDEFINED)
	{
		sigcompIdLen = RPOOL_Strlen(pSrcTrx->hPool, pSrcTrx->hPage, pSrcTrx->sigcompIdOffset);
		
		rv = RPOOL_Append(pDestTrx->hPool, pDestTrx->hPage, sigcompIdLen+1, RV_FALSE,
			&pDestTrx->sigcompIdOffset);
		if (rv != RV_OK)
		{
			RvLogError(pSrcTrx->pTrxMgr->pLogSrc,(pSrcTrx->pTrxMgr->pLogSrc,
				"TransmitterCopy - Failed to append to page (rv=%d)", rv));
			return rv;
		}
		rv = RPOOL_CopyFrom(pDestTrx->hPool, pDestTrx->hPage, pDestTrx->sigcompIdOffset, 
			pSrcTrx->hPool, pSrcTrx->hPage, pSrcTrx->sigcompIdOffset, sigcompIdLen+1);
		if (rv != RV_OK)
		{
			RvLogError(pSrcTrx->pTrxMgr->pLogSrc,(pSrcTrx->pTrxMgr->pLogSrc,
				"TransmitterCopy - Failed to copy sigcomp-id (rv=%d)", rv));
			return rv;
		}
	}
#endif /* RV_SIGCOMP_ON */

    memcpy(&(pDestTrx->localAddr),&(pSrcTrx->localAddr),sizeof(SipTransportObjLocalAddresses));
    /*update the address in use*/
    if(pSrcTrx->localAddr.hAddrInUse != NULL &&
       *(pSrcTrx->localAddr.hAddrInUse) != NULL )
    {
        TransmitterSetInUseLocalAddrByLocalAddr(pDestTrx,*(pSrcTrx->localAddr.hAddrInUse));
    }

    memcpy(&(pDestTrx->outboundAddr),&(pSrcTrx->outboundAddr),sizeof(SipTransportOutboundAddress));
    /*if the outbound host was on the origin transaction page, the cloned transaction
      should copy the host to its page*/
    if(pSrcTrx->outboundAddr.rpoolHostName.hPage != NULL_PAGE)
    {
        if(pSrcTrx->outboundAddr.rpoolHostName.hPage == pSrcTrx->hPage)
        {
            pDestTrx->outboundAddr.rpoolHostName.hPage  = pDestTrx->hPage;
            pDestTrx->outboundAddr.rpoolHostName.hPool  = pDestTrx->hPool;
            pDestTrx->outboundAddr.rpoolHostName.offset = UNDEFINED;
            rv = SipCommonCopyStrFromPageToPage(&pSrcTrx->outboundAddr.rpoolHostName,
                                       &pDestTrx->outboundAddr.rpoolHostName);
            if(rv != RV_OK)
            {
                RvLogError(pSrcTrx->pTrxMgr->pLogSrc ,(pSrcTrx->pTrxMgr->pLogSrc ,
                    "TransmitterCopy - Transmitter 0x%p - Failed to copy outbound host",pSrcTrx));
                return rv;
            }
#ifdef SIP_DEBUG
            pDestTrx->outboundAddr.strHostName = RPOOL_GetPtr(pDestTrx->outboundAddr.rpoolHostName.hPool,
                                                              pDestTrx->outboundAddr.rpoolHostName.hPage,
                                                              pDestTrx->outboundAddr.rpoolHostName.offset);
#endif /*SIP_DEBUG*/
        }
    }

#ifdef RV_SIGCOMP_ON
    /*if the outbound sigcomp-id was on the origin transaction page, the cloned transaction
      should copy the sigcomp-id to its page*/
    if(pSrcTrx->outboundAddr.rpoolSigcompId.hPage != NULL_PAGE)
    {
        if(pSrcTrx->outboundAddr.rpoolSigcompId.hPage == pSrcTrx->hPage)
        {
            pDestTrx->outboundAddr.rpoolSigcompId.hPage  = pDestTrx->hPage;
            pDestTrx->outboundAddr.rpoolSigcompId.hPool  = pDestTrx->hPool;
            pDestTrx->outboundAddr.rpoolSigcompId.offset = UNDEFINED;
            rv = SipCommonCopyStrFromPageToPage(&pSrcTrx->outboundAddr.rpoolSigcompId,
                                       &pDestTrx->outboundAddr.rpoolSigcompId);
            if(rv != RV_OK)
            {
                RvLogError(pSrcTrx->pTrxMgr->pLogSrc ,(pSrcTrx->pTrxMgr->pLogSrc ,
                    "TransmitterCopy - Transmitter 0x%p - Failed to copy outbound sigcomp-id",pSrcTrx));
                return rv;
            }
#ifdef SIP_DEBUG
            pDestTrx->outboundAddr.strSigcompId = RPOOL_GetPtr(pDestTrx->outboundAddr.rpoolSigcompId.hPool,
                                                              pDestTrx->outboundAddr.rpoolSigcompId.hPage,
                                                              pDestTrx->outboundAddr.rpoolSigcompId.offset);
#endif /*SIP_DEBUG*/
        }
    }
#endif /* RV_SIGCOMP_ON */
	
    return RV_OK;
}


/************************************************************************************
 * TransmitterSetInUseLocalAddrByLocalAddr
 * ----------------------------------------------------------------------------------
 * General: Set the inUse local address to point to the place that holds the
 *          supplied local address.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - the transmitter pointer
 *          hLocalAddr - the local address to set in the address in use.
 ***********************************************************************************/
void RVCALLCONV TransmitterSetInUseLocalAddrByLocalAddr (
                            IN  Transmitter*                   pTrx,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr)
{

    RvSipTransportLocalAddrHandle* phLocalAddr    = NULL;
    RvSipTransport                 eTransportType;
    RvSipTransportAddressType      eAddrType;
    RvStatus rv;

    rv = SipTransportLocalAddressGetTransportType(hLocalAddr,&eTransportType);
    if (RV_OK != rv  ||  RVSIP_TRANSPORT_UNDEFINED == eTransportType)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "TransmitterSetInUseLocalAddrByLocalAddr - Transmitter 0x%p Failed to get transport type for local address 0x%p",
            pTrx,hLocalAddr));
        return;
    }
    rv = SipTransportLocalAddressGetAddrType(hLocalAddr, &eAddrType);
    if(RV_OK != rv  ||  RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED == eAddrType)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterSetInUseLocalAddrByLocalAddr -Transmitter 0x%p Failed to get address type for local address 0x%p",
                  pTrx,hLocalAddr));
        return;
    }
    rv = TransmitterGetLocalAddressHandleByTypes(pTrx,eTransportType,eAddrType,&phLocalAddr);
    if(RV_OK != rv  ||  phLocalAddr == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterSetInUseLocalAddrByLocalAddr: Transmitter 0x%p Failed in TransmitterGetLocalAddressHandleByTypes",
            pTrx));
        return;
    }
    *phLocalAddr = hLocalAddr;
    pTrx->localAddr.hAddrInUse = phLocalAddr;
}



/***************************************************************************
 * TransmitterTerminate
 * ------------------------------------------------------------------------
 * General: Moves the transmitter object to the terminated state and
 *          send the transmitter to the termination queue.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter to terminate.
 ***************************************************************************/
void RVCALLCONV TransmitterTerminate(
                                 IN  Transmitter    *pTrx)
{
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
        return;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "TransmitterTerminate - Transmitter 0x%p internal state change to Termiante. Sending to queue",
               pTrx));
    pTrx->eState = RVSIP_TRANSMITTER_STATE_TERMINATED;

    if (NULL != pTrx->hRslv)
    {
        SipResolverDestruct(pTrx->hRslv);
        pTrx->hRslv = NULL;
    }

    if(pTrx->hConn != NULL)
    {
        TransmitterDetachFromConnection(pTrx,
                                       pTrx->hConn);
    }
    MsgToSendListDestruct(pTrx);

#ifdef RV_SIP_IMS_ON
    /* Detach from Security Object */
    if (NULL != pTrx->hSecObj)
    {
        RvStatus rv;
        rv = TransmitterSetSecObj(pTrx, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "TransmitterTerminate - Transmitter 0x%p: Failed to unset Security Object %p",
                pTrx, pTrx->hSecObj));
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    /*change the state and insert into the event queue*/
    SipTransportSendTerminationObjectEvent(pTrx->pTrxMgr->hTransportMgr,
                                (void *)pTrx,
                                &(pTrx->terminationInfo),
                                0,
                                TerminateTransmitterEventHandler,
                                RV_TRUE,
                                TRANSMITTER_STATE_TERMINATED_STR,
                                TRANSMITTER_OBJECT_NAME_STR);

    return;

}


/***************************************************************************
 * TransmitterDestruct
 * ------------------------------------------------------------------------
 * General: Free the Transmitter resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter to free.
 ***************************************************************************/
void RVCALLCONV TransmitterDestruct(IN  Transmitter    *pTrx)
{
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterDestruct - transmitter 0x%p destructing..." ,pTrx));

    pTrx->trxUniqueIdentifier = 0;

    /*detach from the connection  - we do not want to get connection events*/
    if(pTrx->hConn != NULL)
    {
        TransmitterDetachFromConnection(pTrx, pTrx->hConn);
    }

    if(pTrx->hMsgToSend != NULL)
    {
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend = NULL;
    }

#ifdef RV_SIGCOMP_ON
    /* Detach from the transmitter SigComp compartment*/
    if(pTrx->hSigCompCompartment != NULL)
    {
        SipCompartmentDetach(pTrx->hSigCompCompartment, (void*)pTrx);
        pTrx->hSigCompCompartment = NULL;
    }
#endif /* RV_SIGCOMP_ON */

    MsgToSendListDestruct(pTrx);

#ifdef RV_SIGCOMP_ON
    /* Free the outbound messages that are still not deallocated.
	   The order is important: first destruct the msg2send list (above),
	   and only then free the redundant page
	*/
    if (pTrx->hEncodedOutboundMsg != NULL_PAGE)
    {
        RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool,pTrx->hEncodedOutboundMsg);
        pTrx->hEncodedOutboundMsg = NULL_PAGE;
    }
#endif

#ifdef RV_SIP_IMS_ON
    /* Detach from Security Object */
    if (NULL != pTrx->hSecObj)
    {
        RvStatus rv;
        rv = TransmitterSetSecObj(pTrx, NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "TransmitterDestruct - Transmitter 0x%p: Failed to unset Security Object %p",
                pTrx, pTrx->hSecObj));
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* The page will be destructed only if the transmitter is the only user */
    RPOOL_FreePage(pTrx->hPool, pTrx->hPage);
    pTrx->hPage = NULL_PAGE;

    if (NULL != pTrx->hRslv)
    {
        SipResolverDestruct(pTrx->hRslv);
        pTrx->hRslv = NULL;
    }

    /*the transmitter should be removed from the manager list.
      this must be the last step, since from this moment, the trx can be allocated again.*/
    TransmitterMgrRemoveTransmitter(pTrx->pTrxMgr,(RvSipTransmitterHandle)pTrx);
}

/***************************************************************************
 * TransmitterChangeState
 * ------------------------------------------------------------------------
 * General: Change the Transmitter state and notify the application about it.
 *
 * Caution !!!
 *  TransmitterChangeState(...,RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED,...)
 *  should be called indirectly by call to
 *  TransmitterMoveToStateFinalDestResolved() only.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter.
 *            eState   - The new state.
 *            eReason  - The state change reason.
 *            hMsg      - A message related to this state
 *            pExtraInfo- Extra info related to this state.
 * Output:  (-)
 ***************************************************************************/
void RVCALLCONV TransmitterChangeState(
                                 IN  Transmitter                      *pTrx,
                                 IN  RvSipTransmitterState             eNewState,
                                 IN  RvSipTransmitterReason            eReason,
                                 IN  RvSipMsgHandle                    hMsg,
                                 IN  void*                             pExtraInfo)
{
    RvBool    bNoStateChange     = RV_FALSE;
    if(eNewState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterChangeState - transmitter 0x%p state changed to Terminated"
                  ,pTrx));
    }
    else
    {
        /*if we started sending ACK and while we were on hold or resolving we got a
        notification that a request retransmission was sent we don't change the trx state
        we just notify the transaction that a message was sent*/
        if((pTrx->eState == RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR ||
            pTrx->eState == RVSIP_TRANSMITTER_STATE_ON_HOLD ||
            pTrx->eState == RVSIP_TRANSMITTER_STATE_MSG_SENT) &&
            eNewState == RVSIP_TRANSMITTER_STATE_MSG_SENT)
        {
             RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                      "TransmitterChangeState - transmitter 0x%p - message sent notification on state %s (reason = %s).Don't change state",
                       pTrx, TransmitterGetStateName(pTrx->eState),
                       TransmitterGetStateChangeReasonName(eReason)));
            bNoStateChange = RV_TRUE;
        }
        else
        {
             RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                          "TransmitterChangeState - transmitter 0x%p state changed: %s->%s, (reason = %s)",
                           pTrx, TransmitterGetStateName(pTrx->eState),
                           TransmitterGetStateName(eNewState),
                           TransmitterGetStateChangeReasonName(eReason)));

        }
    }

    /*do relevant activities for the new state*/
    switch(eNewState)
    {
    case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
        if (RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE == pTrx->eState)
        {
            return;
        }
        break;
    default:
        break;
    } /* switch*/

    if(bNoStateChange == RV_FALSE)
    {
        pTrx->eState = eNewState;
    }

    /* Call the state changed event*/
    TransmitterCallbackStateChangeEv(pTrx,eNewState, eReason,hMsg,pExtraInfo);

    /*In state msg send failure - free the message it will never be sent*/
    if(eNewState == RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE &&
       pTrx->hMsgToSend != NULL && pTrx->bKeepMsg == RV_FALSE)
    {
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend = NULL;
    }

    /* if message sending was finished, and the transmitter has no owner,
       we terminate it. */
    if((eNewState == RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE ||
        eNewState == RVSIP_TRANSMITTER_STATE_MSG_SENT) &&
       pTrx->evHandlers.pfnStateChangedEvHandler == NULL)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterChangeState - transmitter 0x%p terminate with no owner",
            pTrx));
        TransmitterTerminate(pTrx);
    }
}




/***************************************************************************
 * TransmitterMoveToMsgSendFaiureIfNeeded
 * ------------------------------------------------------------------------
 * General: Change the transmitter state to msg send failure in
 *          one of two cases. The rv indicates a network error
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - Pointer to the Transmitter.
 *            rv   - The status that caused us to call this function.
 *            eTrxReason - If the reason is undefined then the return value should be used.
 *            bReachedFromApp  - The state change reason.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMoveToMsgSendFaiureIfNeeded(
                               IN   Transmitter*           pTrx,
                               IN   RvStatus               rv,
                               IN   RvSipTransmitterReason eTrxReason,
                               IN   RvBool                 bReachedFromApp)
{


    RvSipTransmitterReason eReason    = RVSIP_TRANSMITTER_REASON_UNDEFINED;
    void*                  pExtraInfo = NULL;

    /*we reached this function because the transmitter was terminated
      from a callback and caused one of the functions in the flow to
      return error*/
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
		RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"TransmitterMoveToMsgSendFaiureIfNeeded - transmitter 0x%p state terminated %p",pTrx));
        return RV_OK;
    }

    /*decide whether to use the rv or the reason*/
    if(eTrxReason == RVSIP_TRANSMITTER_REASON_UNDEFINED)
    {
        eReason = ConvertRvToTrxReason(rv);
    }
    else
    {
        eReason = eTrxReason;
    }

    if (RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE == pTrx->eState)
    {
		RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"TransmitterMoveToMsgSendFaiureIfNeeded - transmitter 0x%p state msgSendFailure %p",pTrx));
        return RV_OK;
    }


    if(bReachedFromApp == RV_FALSE ||
       eReason == RVSIP_TRANSMITTER_REASON_NETWORK_ERROR)
    {
        if (pTrx->bIsAppTrx == RV_FALSE)
        {
            pExtraInfo = (void*)(RvIntPtr)pTrx->eMsgType;
        }
        else
        {
            /* Remove Transmitter messages from the Connection list of messages
               to be sent */
            if (NULL != pTrx->hConn)
            {
                rv = SipTransportConnectionRemoveOwnerMsgs(
                        pTrx->hConn,(RvSipTransportConnectionOwnerHandle)pTrx);
                if (RV_OK != rv)
                {
                    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterMoveToMsgSendFaiureIfNeeded - transmitter 0x%p failed to remove messages from Connection %p",
                        pTrx, pTrx->hConn));
                }
            }
#ifdef RV_SIP_IMS_ON
            if (NULL != pTrx->hSecureConn)
            {
                rv = SipTransportConnectionRemoveOwnerMsgs(pTrx->hSecureConn,
                        (RvSipTransportConnectionOwnerHandle)pTrx);
                if (RV_OK != rv)
                {
                    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterMoveToMsgSendFaiureIfNeeded - transmitter 0x%p failed to remove messages from secure Connection %p",
                        pTrx, pTrx->hSecureConn));
                }
            }
#endif /*#ifdef RV_SIP_IMS_ON*/

            /*destruct the msg_to_send list to free the encoded message*/
            MsgToSendListFreeAllPages(pTrx);
        }

        TransmitterChangeState(pTrx,
                               RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE,
                               eReason,
                               pTrx->hMsgToSend,
                               pExtraInfo);
        return RV_OK;
    }
    return rv;
}

/******************************************************************************
 * TransmitterMoveToStateFinalDestResolved
 * ----------------------------------------------------------------------------
 * General: Performs some action before state change,
 *          after that move the Transmitter to the FINAL_DEST_RESOLVED state
 *          by call to TransmitterChangeState().
 * Return Value: RV_OK on success,
 *               error code, declared in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx      - Pointer to the Transmitter.
 *            eReason   - The state change reason.
 *            hMsg      - A message related to the state
 *            pExtraInfo- Extra info related to the state.
 * Output:  (-)
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterMoveToStateFinalDestResolved(
                            IN  Transmitter                      *pTrx,
                            IN  RvSipTransmitterReason            eReason,
                            IN  RvSipMsgHandle                    hMsg,
                            IN  void*                             pExtraInfo)
{
    RvStatus rv;

    /* We have final destination address. Update the message local address */
    rv = TransmitterSetInUseLocalAddrByDestAddr(pTrx);
    if(RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterMoveToStateFinalDestResolved - TransmitterSetInUseLocalAddrByDestAddr(Transmitter 0x%p) failed",
            pTrx));
        return rv;
    }
    rv = TransmitterUpdateViaByFinalLocalAddress(pTrx,(pTrx->bFixVia||pTrx->bTrxCreatedVia));
    if(RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterMoveToStateFinalDestResolved - TransmitterUpdateViaByFinalLocalAddress(Transmitter 0x%p) failed",
            pTrx));
        return rv;
    }

#ifdef RV_SIP_OTHER_URI_SUPPORT
    /* O.W. if the address we resolved is 'im:' or 'pres:' we should change it to 'sip:' */
    FixMsgSendingAddrIfNeed(pTrx);
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/



    AddScopeToDestIpv6AddrIfNeed(pTrx);

    TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED,
                           RVSIP_TRANSMITTER_REASON_UNDEFINED,
                           hMsg,NULL);
    RV_UNUSED_ARG(eReason);
    RV_UNUSED_ARG(pExtraInfo);

    if(pTrx->bMoveToMsgSendFailure == RV_TRUE)
    {
        TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,RV_ERROR_UNKNOWN,
                                               RVSIP_TRANSMITTER_REASON_USER_COMMAND,
                                               RV_FALSE);
    }
    return RV_OK;
}

/***************************************************************************
 * TransmitterMsgToSendListAddElement
 * ------------------------------------------------------------------------
 * General: Adds an element to the message to send list.
 * Return Value:void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx   - The transmitter pointer
 *            msgPage - The message to send page
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterMsgToSendListAddElement(
                                       IN   Transmitter*          pTrx,
                                       IN   HPAGE                 hMsgPage)
{
    RvStatus    rv = RV_OK;
    HPAGE*      phNewPage;
    RLIST_ITEM_HANDLE  hListElement;
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterMsgToSendListAddElement - Transmitter 0x%p, adding page 0x%p",
                  pTrx,hMsgPage));

    rv = RLIST_InsertTail(NULL,
                          pTrx->hMsgToSendList,
                          (RLIST_ITEM_HANDLE *)&hListElement);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "TransmitterMsgToSendListAddElement - Failed, Transmitter 0x%p",
                  pTrx));

        return rv;
    }
    phNewPage  = (HPAGE*)hListElement;
    *phNewPage = hMsgPage;
    return RV_OK;
}

/***************************************************************************
 * TransmitterMsgToSendListRemoveElement
 * ------------------------------------------------------------------------
 * General: Go over the transmitter sent list and free all the pages of messages
 *          that where sent before the current message
 *
 * Return Value:void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx         - The transmitter pointer
 *            hMsgSentPage - The message page
 ***************************************************************************/
void RVCALLCONV TransmitterMsgToSendListRemoveElement(
                                       IN   Transmitter*          pTrx,
                                       IN   HPAGE                 hMsgSentPage)
{
    RLIST_ITEM_HANDLE hListElement = NULL;
    HPAGE             hMsgToSendPage;
    RvInt32           safeCounter  = 0;

    RLIST_GetHead(NULL, pTrx->hMsgToSendList,&hListElement);
    /* Free all the pages besides the page of the sent message */
    while( hListElement != NULL &&
           safeCounter  < pTrx->pTrxMgr->maxMessages)
    {
        hMsgToSendPage = *((HPAGE*)hListElement);
        if (hMsgToSendPage != hMsgSentPage)
        {
            /*free the current page */
            RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool, hMsgToSendPage);
            RLIST_Remove(NULL, pTrx->hMsgToSendList, hListElement);
            hListElement = NULL;
			if (pTrx->hEncodedOutboundMsg == hMsgToSendPage)
			{
				pTrx->hEncodedOutboundMsg = NULL_PAGE;
			}
#ifdef RV_SIGCOMP_ON
			if (pTrx->hSigCompOutboundMsg == hMsgToSendPage)
			{
				pTrx->hSigCompOutboundMsg = NULL_PAGE;
			}
#endif
        }
        else
        {
            return;
        }
        RLIST_GetHead(NULL, pTrx->hMsgToSendList, &hListElement);
        safeCounter++;
    }
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransmitterSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transmitter.
 *          As a result of this operation, all messages, sent by this
 *          Transmitter, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTrx    - Pointer to the Transmitter object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TransmitterSetSecObj(IN  Transmitter*      pTrx,
                              IN  RvSipSecObjHandle hSecObj)
{
    RvStatus        rv;
    TransmitterMgr* pTrxMgr = pTrx->pTrxMgr;

    /* If Security Object was already set, detach from it */
    if (NULL != pTrx->hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTrxMgr->hSecMgr, pTrx->hSecObj,
                -1 /*delta*/, RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSMITTER,
                (void*)pTrx);
        if (RV_OK != rv)
        {
            RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
                "TransmitterSetSecObj - Transmitter 0x%p: Failed to unset existing Security Object %p",
                pTrx, pTrx->hSecObj));
            return rv;
        }
        pTrx->hSecObj = NULL;
    }

    /* Register the Call_Leg to the new Security Object*/
    if (NULL != hSecObj)
    {
        rv = SipSecObjUpdateOwnerCounter(pTrxMgr->hSecMgr,hSecObj,1 /*delta*/,
                RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSMITTER, (void*)pTrx);
        if (RV_OK != rv)
        {
            RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
                "TransmitterSetSecObj - Transmitter 0x%p: Failed to set new Security Object %p",
                pTrx, hSecObj));
            return rv;
        }
    }

    pTrx->hSecObj = hSecObj;

    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/***************************************************************************
 * ConstructAndInitNewConn
 * ------------------------------------------------------------------------
 * General: constructs a new connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx         - The transmitter pointer
 ***************************************************************************/
static RvStatus RVCALLCONV ConstructAndInitNewConn(IN Transmitter* pTrx,
                                          IN RvBool       bForceCreation,
                                          OUT RvBool*     pbNewConnCreated)
{
    RvStatus rv;
    rv = SipTransportConnectionConstruct(
                                    pTrx->pTrxMgr->hTransportMgr,
                                    bForceCreation,
                                    RVSIP_TRANSPORT_CONN_TYPE_CLIENT,
                                    pTrx->destAddr.eTransport,
                                    *(pTrx->localAddr.hAddrInUse),
                                    &(pTrx->destAddr),
                                    (RvSipTransportConnectionOwnerHandle)pTrx,
                                    TransmitterControlConnStateChangedEv,
                                    TransmitterControlConnStatusEv,
                                    pbNewConnCreated,
                                    &(pTrx->hConn));
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc,
                   "ConstructAndInitNewConn - Failed, Transmitter 0x%p, Failed to create new connection",
                   pTrx));
        return rv;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if (RVSIP_TRANSPORT_TLS == pTrx->destAddr.eTransport)
    {
        rv = SipTransportConnectionSetHostStringName(pTrx->hConn,(RvChar*)pTrx->msgParams.strHostName);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
                "ConstructAndInitNewConn - pTrx=0x%p: Failed to set host name to connection (rv=%d)",pTrx,rv));

            if (RV_OK != RvSipTransportConnectionDetachOwner(pTrx->hConn,(RvSipTransportConnectionOwnerHandle)pTrx))
            {
                RvLogWarning(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
                    "ConstructAndInitNewConn - pTrx=0x%p: couldn't detach from connection 0x%p.",
                    pTrx,pTrx->hConn));
            }
            pTrx->hConn=NULL;
            return rv;
        }
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    if (RVSIP_TRANSPORT_SCTP == pTrx->destAddr.eTransport)
    {
        SipTransportConnectionSetSctpMsgSendingParams(pTrx->hConn,
                                                      &pTrx->sctpParams);
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    return RV_OK;

}

/******************************************************************************
 * TransmitterCreateConnectionIfNeeded
 * ----------------------------------------------------------------------------
 * General: Checks if the transmitter's current connection can be used and
 *          if not constructs a new connection. In this case NEW_CONN_IN_USE
 *          event will be reported.
 *          If the Secure Connection is provided, it will be used.
 *          If the Secure Connection is used the first time by the Transmitter,
 *          NEW_CONN_IN_USE will be reported.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx        - The transmitter pointer
 *          hSecureConn - Handle to the Secure Connection to be used
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterCreateConnectionIfNeeded(
                            IN Transmitter*                   pTrx,
                            IN RvSipTransportConnectionHandle hSecureConn)
{
    RvStatus               rv              = RV_OK;
    RvBool                 bIsUsable       = RV_TRUE;
    RvBool                 bForceCreation  = RV_FALSE;
    RvBool                 bNewConnCreated = RV_FALSE;
    RvSipTransmitterReason eReason = RVSIP_TRANSMITTER_REASON_CONN_FOUND_IN_HASH;

    if(pTrx->bIsPersistent == RV_FALSE)
    {
        bForceCreation = RV_TRUE;
    }
#ifndef RV_SIP_IMS_ON
	RV_UNUSED_ARG(hSecureConn)
#else
    /* If Trx already has a Secure Connection, there is nothing to be done. */
    if (NULL!=pTrx->hConn  &&  pTrx->hConn==hSecureConn)
    {
        return RV_OK;
    }
    /* If we have a connection (because application had set it) and Secure
    Connection is requested to be used, replace the current connection with
    the secure one */
    if (NULL != pTrx->hConn  &&  NULL != hSecureConn)
    {
        RvLogWarning(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterCreateConnectionIfNeeded - pTrx=0x%p: Application Connection %p is overriden by Security Connection %p",
            pTrx, pTrx->hConn, hSecureConn));
        TransmitterDetachFromConnection(pTrx, pTrx->hConn);
        pTrx->bConnWasSetByApp = RV_FALSE;
        /* The reset is made by TransmitterDetachFromConnection */
        /*pTrx->hConn = NULL*/
    }
    /* If there is no connection yet and Secure Connection is requested to be
    used, use it */
    if (NULL == pTrx->hConn  &&  NULL != hSecureConn)
    {
        rv = TransmitterAttachToConnection(pTrx, hSecureConn);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterCreateConnectionIfNeeded - pTrx=0x%p: failed to attach to Security Connection %p",
                pTrx, hSecureConn));
            return rv;
        }
        /* The set is made by TransmitterAttachToConnection */
        /*pTrx->hConn = hSecureConn*/
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* If we have a connection (because application had set it)
       we need to check that it is usable. There is no need to check usability
       of the Secure Connection. */
    if (pTrx->hConn != NULL
#ifdef RV_SIP_IMS_ON
        &&  pTrx->hConn != hSecureConn
#endif
       )
    {
        RvSipTransportConnectionType eConnectionType;
        RvSipMsgType msgType = pTrx->msgParams.msgType;
        RvBool bCheckStateOnly;

        rv = RvSipTransportConnectionGetType(pTrx->hConn,&eConnectionType);
        if(rv != RV_OK)
        {
            bIsUsable = RV_FALSE;
        }

        if(bIsUsable == RV_TRUE)
        {
            bCheckStateOnly = (RVSIP_MSG_REQUEST == msgType && RV_FALSE == pTrx->bSendByAlias)?RV_FALSE:RV_TRUE;
            bIsUsable = SipTransportConnectionIsUsable(pTrx->hConn,
                                            pTrx->destAddr.eTransport,
                                            *(pTrx->localAddr.hAddrInUse),
                                            &(pTrx->destAddr),
                                            bCheckStateOnly);
        }
        if(bIsUsable == RV_FALSE)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterCreateConnectionIfNeeded - pTrx=0x%p:connection 0x%p is not usable. a different connection will be used",
                pTrx,pTrx->hConn));

            TransmitterDetachFromConnection(pTrx,pTrx->hConn);
        }
    }

    /* The construct function also calls the 'new-conn-in-use' event. so we are calling
    this function even if we already found a connection... */
    if(pTrx->hConn == NULL ||
       (pTrx->hConn != NULL && pTrx->bConnWasSetByApp == RV_FALSE))
    {
        RvSipTransmitterNewConnInUseStateInfo stateInfo;

        if(pTrx->hConn == NULL)
        {
			rv = ConstructAndInitNewConn(pTrx, bForceCreation, &bNewConnCreated);
			if(RV_OK != rv)
			{
				RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc,
				   "TransmitterCreateConnectionIfNeeded - Transmitter 0x%p, Failed to create new connection",
				   pTrx));
				return rv;
			}
        }

        /*notify the owner that a new connection was created*/
        stateInfo.hConn = pTrx->hConn;
        if(bNewConnCreated == RV_TRUE)
        {
            eReason = RVSIP_TRANSMITTER_REASON_NEW_CONN_CREATED;
        }
        TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE,
                               eReason, NULL, (void*)&stateInfo);

        /* If Transmitter was terminated from the callback, don't touch it.*/
        if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "TransmitterCreateConnectionIfNeeded - pTrx=0x%p: was terminated from callback. returning from function",
                pTrx));

            return RV_ERROR_DESTRUCTED;
        }

    }
    return RV_OK;

}

/***************************************************************************
 * TransmitterAttachToConnection
 * ------------------------------------------------------------------------
 * General: Attach the Transmitter as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx  - pointer to Transmitter
 *            hConn         - The connection handle
 ***************************************************************************/
RvStatus TransmitterAttachToConnection(
                                IN Transmitter                    *pTrx,
                                IN RvSipTransportConnectionHandle  hConn)
{
    RvStatus rv;
    RvSipTransportConnectionEvHandlers evHandler;
    memset(&evHandler,0,sizeof(RvSipTransportConnectionEvHandlers));
    evHandler.pfnConnStateChangedEvHandler =
                          TransmitterControlConnStateChangedEv;
    evHandler.pfnConnStausEvHandler        =
                          TransmitterControlConnStatusEv;
    /*attach to new conn*/
    rv = SipTransportConnectionAttachOwner(hConn,
                                    (RvSipTransportConnectionOwnerHandle)pTrx,
                                    &evHandler,sizeof(evHandler));

    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "TransmitterAttachToConnection - Failed to attach Transmitter 0x%p to connection 0x%p",pTrx,hConn));
        return rv;
    }
    pTrx->hConn = hConn;
    return rv;
}


/***************************************************************************
 * TransmitterDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx    - pointer to Transmitter
 *            hConn   - the connection to detach from.
 ***************************************************************************/
void TransmitterDetachFromConnection(IN Transmitter                    *pTrx,
                                     IN RvSipTransportConnectionHandle  hConn)
{

    RvStatus rv;
    if(hConn == NULL)
    {
        return;
    }
    rv = RvSipTransportConnectionDetachOwner(hConn,
                                            (RvSipTransportConnectionOwnerHandle)pTrx);
    if(rv != RV_OK)
    {
        /* The connection object might be terminated or  */
        /* there might be other problem                  */
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                     "TransmitterDetachFromConnection - Failed to detach transmitter 0x%p from connection 0x%p",pTrx,hConn));

    }
    /*set null in the transmitter if we detached from the current connection*/
    if(hConn == pTrx->hConn)
    {
        pTrx->hConn = NULL;
    }
}

/***************************************************************************
 * TransmitterDetachFromAres
 * ------------------------------------------------------------------------
 * General: detach the Transmitter from the connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx    - pointer to Transmitter
 *            hConn   - the connection to detach from.
 ***************************************************************************/
void TransmitterDetachFromAres(IN Transmitter   *pTrx)
{

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "TransmitterDetachFromAres - transmitter 0x%p - detaching resolver 0x%p from ARES",pTrx, pTrx->hRslv));

    SipResolverDetachFromAres(pTrx->hRslv);
}



#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)

/************************************************************************************
 * TransmitterLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transmitter according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterLockAPI(
                                     IN  Transmitter *pTrx)
{
    RvStatus            rv;
    SipTripleLock      *pTripleLock;
    TransmitterMgr     *pMgr;
    RvInt32             identifier;

    if (pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    for(;;)
    {

        RvMutexLock(&pTrx->trxTripleLock.hLock,NULL);
        pMgr        = pTrx->pTrxMgr;
        pTripleLock = pTrx->pTripleLock;
        identifier  = pTrx->trxUniqueIdentifier;
        RvMutexUnlock(&pTrx->trxTripleLock.hLock,NULL);

        if ((pMgr == NULL) || (pTripleLock == NULL))
        {
            return RV_ERROR_NULLPTR;
        }

        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransmitterLockAPI - Locking Transmitter 0x%p: Triple Lock 0x%p", pTrx,
            pTripleLock));

        rv = TransmitterLock(pTrx,pMgr,pTripleLock,identifier);
        if (rv != RV_OK)
        {
            return rv;
        }

        /*make sure the triple lock was not changed after just before the locking*/
        if (pTrx->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransmitterLockAPI - Locking Transmitter 0x%p: Tripple lock 0x%p was changed, reentering lockAPI",
            pTrx, pTripleLock));
        TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);

    }
    rv = CRV2RV(RvSemaphoreTryWait(&pTripleLock->hApiLock,pMgr->pLogMgr));
    if ((rv != RV_ERROR_TRY_AGAIN) && (rv != RV_OK))
    {
        TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
        return rv;
    }
    pTripleLock->apiCnt++;
    /* This thread actually locks the API lock */
    if(pTripleLock->objLockThreadId == 0)
    {
        pTripleLock->objLockThreadId = RvThreadCurrentId();
        RvMutexGetLockCounter(&pTripleLock->hLock,pMgr->pLogMgr,
                              &pTripleLock->threadIdLockCount);
    }


    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransmitterLockAPI - Locking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
             pTrx, pTripleLock, pTripleLock->apiCnt));

    return RV_OK;
}


/************************************************************************************
 * TransmitterUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Relese Transmitter according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
void RVCALLCONV TransmitterUnLockAPI(
    IN  Transmitter *pTrx)
{
    SipTripleLock        *pTripleLock;
    TransmitterMgr       *pMgr;
    RvInt32               lockCnt = 0;

    if (pTrx == NULL)
    {
        return;
    }

    pMgr        = pTrx->pTrxMgr;
    pTripleLock = pTrx->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }
    RvMutexGetLockCounter(&pTripleLock->hLock,NULL,&lockCnt);

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransmitterUnLockAPI - UnLocking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d lockCnt=%d",
             pTrx, pTripleLock, pTripleLock->apiCnt,lockCnt));

    if (pTripleLock->apiCnt == 0)
    {
        RvLogExcep(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransmitterUnLockAPI - UnLocking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d",
             pTrx, pTripleLock, pTripleLock->apiCnt));
        return;
    }

    if (pTripleLock->apiCnt == 1)
    {
        RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
    }

    pTripleLock->apiCnt--;
    if(lockCnt == pTripleLock->threadIdLockCount)
    {
        pTripleLock->objLockThreadId = 0;
        pTripleLock->threadIdLockCount = UNDEFINED;
    }
    TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
}


/************************************************************************************
 * TransmitterLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Transmitter according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterLockEvent(
    IN  Transmitter *pTrx)
{
    RvStatus          rv           = RV_OK;
    RvBool            didILockAPI  = RV_FALSE;
    RvThreadId        currThreadId = RvThreadCurrentId();
    SipTripleLock    *pTripleLock;
    TransmitterMgr   *pMgr;
    RvInt32           identifier;

    if (pTrx == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    for(;;)
    {
        RvMutexLock(&pTrx->trxTripleLock.hLock,pTrx->pTrxMgr->pLogMgr);
        pMgr        = pTrx->pTrxMgr;
        pTripleLock = pTrx->pTripleLock;
        identifier  = pTrx->trxUniqueIdentifier;
        RvMutexUnlock(&pTrx->trxTripleLock.hLock,pTrx->pTrxMgr->pLogMgr);
        /*note: at this point the object locks can be replaced so after locking we need
          to check if we used the correct locks*/

        if (pTripleLock == NULL)
        {
            return RV_ERROR_BADPARAM;
        }

        /* In case that the current thread has already gained the API lock of */
        /* the object, locking again will be useless and will block the stack */
        if (pTripleLock->objLockThreadId == currThreadId)
        {
            RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransmitterLockEvent - Exiting without locking Transmitter 0x%p: Triple Lock 0x%p already locked",
                   pTrx, pTripleLock));

            return RV_OK;
        }
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransmitterLockEvent - Locking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d",
                  pTrx, pTripleLock, pTripleLock->apiCnt));

        RvMutexLock(&pTripleLock->hProcessLock,pMgr->pLogMgr);

        for(;;)
        {
            rv = TransmitterLock(pTrx,pMgr,pTripleLock,identifier);
            if (rv != RV_OK)
            {
                RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
                if (didILockAPI)
                {
                    RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
                }
                return rv;
            }
            if (pTripleLock->apiCnt == 0)
            {
                if (didILockAPI)
                {
                    RvSemaphorePost(&pTripleLock->hApiLock,pMgr->pLogMgr);
                }
                break;
            }

            TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);

            rv = CRV2RV(RvSemaphoreWait(&pTripleLock->hApiLock,pMgr->pLogMgr));
            if (rv != RV_OK)
            {
                RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
                return rv;
            }

            didILockAPI = RV_TRUE;
        }

        /*check if the transaction lock pointer was changed. If so release the locks and lock
          the transaction again with the new locks*/
        if (pTrx->pTripleLock == pTripleLock)
        {
            break;
        }
        RvLogWarning(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransmitterLockEvent - Locking Transmitter 0x%p: Tripple lock 0x%p was changed, reentering lockEvent",
            pTrx, pTripleLock));
        TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
        RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
              "TransmitterLockEvent - Locking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d exiting function",
              pTrx, pTripleLock, pTripleLock->apiCnt));

    return rv;
}


/************************************************************************************
 * TransmitterUnLockEvent
 * ----------------------------------------------------------------------------------
 * General: UnLocks Transmitter according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
void RVCALLCONV TransmitterUnLockEvent(
    IN  Transmitter *pTrx)
{
    SipTripleLock        *pTripleLock;
    TransmitterMgr       *pMgr;
    RvThreadId            currThreadId = RvThreadCurrentId();

    if (pTrx == NULL)
    {
        return;
    }

    pMgr        = pTrx->pTrxMgr;
    pTripleLock = pTrx->pTripleLock;

    if ((pMgr == NULL) || (pTripleLock == NULL))
    {
        return;
    }

    /* In case that the current thread has already gained the API lock of */
    /* the object wasn't lock internally, thus it shouldn't be unlocked   */
    if (pTripleLock->objLockThreadId == currThreadId)
    {
        RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
                  "TransmitterUnLockEvent - Exiting without unlocking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d wasn't locked",
                  pTrx, pTripleLock, pTripleLock->apiCnt));

        return;
    }

    RvLogSync(pMgr->pLogSrc ,(pMgr->pLogSrc ,
             "TransmitterUnLockEvent - UnLocking Transmitter 0x%p: Triple Lock 0x%p apiCnt=%d",
             pTrx, pTripleLock, pTripleLock->apiCnt));

    TransmitterUnLock(&pTripleLock->hLock,pMgr->pLogMgr);
    RvMutexUnlock(&pTripleLock->hProcessLock,pMgr->pLogMgr);
}

/***************************************************************************
 * TransmitterTerminateUnlock
 * ------------------------------------------------------------------------
 * General: Unlocks processing and transmitter locks. This function is called to unlock
 *          the locks that were locked by the LockEvent() function. The UnlockEvent() function
 *          is not called because in this stage the transmitter object was already destructed.
 *          and could have been reallocated
 * Return Value: - RV_Succees
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pLogMgr            - The log mgr pointer
 *            pLogSrc        - log handler
 *            hObjLock        - The transaction lock.
 *            hProcessingLock - The transaction processing lock
 ***************************************************************************/
static void RVCALLCONV TransmitterTerminateUnlock(IN RvLogMgr             *pLogMgr,
                                                  IN RvLogSource          *pLogSrc,
                                                  IN RvMutex              *hObjLock,
                                                  IN RvMutex              *hProcessingLock)
{
    RvLogSync(pLogSrc ,(pLogSrc ,
        "TransmitterTerminateUnlock - Unlocking Object lock 0x%p: Processing Lock 0x%p",
        hObjLock, hProcessingLock));

    if (hObjLock != NULL)
    {
        RvMutexUnlock(hObjLock,pLogMgr);
    }

    if (hProcessingLock != NULL)
    {
        RvMutexUnlock(hProcessingLock,pLogMgr);
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

    return;
}

#else /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) - single thread*/
/************************************************************************************
 * TransmitterLockEvent
 * ----------------------------------------------------------------------------------
 * General: Locks Transmitter according to MSG processing schema
 *          at the end of this function processingLock is locked. and hLock is locked.
 *          NOTE: If the stack is compiled in a non multithreaded way, the transmitter
 *          can still be destructed while it is wating for a DNS response.
 *          this is why in non multithreaded mode we still check that the transmitter
 *          location is not vacant
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - pointer to the Transmitter.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterLockEvent(
    IN  Transmitter *pTrx)
{
    if (RV_FALSE == RLIST_IsElementVacant(pTrx->pTrxMgr->hTrxListPool,
                                          pTrx->pTrxMgr->hTrxList,
                                          (RLIST_ITEM_HANDLE)(pTrx)))
    {
        /*The transmitter is valid*/
        return RV_OK;
    }
    RvLogWarning(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
        "TransmitterLockEvent - Transmitter 0x%p: RLIST_IsElementVacant returned TRUE!!!",
        pTrx));
    return RV_ERROR_INVALID_HANDLE;
}

#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/*-----------------------------------------------------------------------
       TRANSMITTER:  G E T  A N D  S E T  F U N C T I O N S
 ------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
          N A M E    F U N C T I O N S
 -------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * TransmitterGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransmitterGetStateName (
                                      IN  RvSipTransmitterState  eState)
{
    switch(eState)
    {
    case RVSIP_TRANSMITTER_STATE_UNDEFINED:
        return "Undefined";
    case RVSIP_TRANSMITTER_STATE_IDLE:
        return "Idle";
    case RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR:
        return "Resolving Addr";
    case RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED:
        return "Final Dest Resolved";
    case RVSIP_TRANSMITTER_STATE_ON_HOLD:
        return "On Hold";
    case RVSIP_TRANSMITTER_STATE_NEW_CONN_IN_USE:
        return "New Conn In Use";
    case RVSIP_TRANSMITTER_STATE_READY_FOR_SENDING:
        return "Ready For Sending";
    case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
        return "Msg Send Failure";
    case RVSIP_TRANSMITTER_STATE_MSG_SENT:
        return "Msg Sent";
    case RVSIP_TRANSMITTER_STATE_TERMINATED:
        return TRANSMITTER_STATE_TERMINATED_STR;
    default:
        return "No Such State";
    }
}

/***************************************************************************
 * TransmitterGetStateChangeReasonName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state change reason.
 * Return Value: The string with the state change reason name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eReason - The state change reason as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransmitterGetStateChangeReasonName (
                                IN  RvSipTransmitterReason  eReason)
{
    switch(eReason)
    {
    case RVSIP_TRANSMITTER_REASON_UNDEFINED:
        return "Undefined";
    case RVSIP_TRANSMITTER_REASON_NETWORK_ERROR:
        return "Network Error";
    case RVSIP_TRANSMITTER_REASON_CONNECTION_ERROR:
        return "Connection Error";
    case RVSIP_TRANSMITTER_REASON_OUT_OF_RESOURCES:
        return "Out Of Resources";
    case RVSIP_TRANSMITTER_REASON_NEW_CONN_CREATED:
        return "New Conn Created";
    case RVSIP_TRANSMITTER_REASON_CONN_FOUND_IN_HASH:
        return "Conn Found In Hash";
    case RVSIP_TRANSMITTER_REASON_USER_COMMAND:
        return "User Command";
    default:
        return "No such reason";
    }
}

/***************************************************************************
 * TransmitterGetResolutionStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransmitterGetResolutionStateName (
                                      IN  TransmitterResolutionState  eState)
{
    switch(eState)
    {
    case TRANSMITTER_RESOLUTION_STATE_UNRESOLVED:
        return "Unresolved";
    case TRANSMITTER_RESOLUTION_STATE_RESOLUTION_STARTED:
        return "Resolution Started";
    case TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI:
        return "Waiting for URI";
    case TRANSMITTER_RESOLUTION_STATE_URI_FOUND:
        return "Uri Found";
    case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR:
        return "Resolving URI Transport NAPTR";
    case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_3WAY_SRV:
        return "Resolving URI Transport 3Way SRV";
    case TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND:
        return "Transport Found";
    case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT:
        return "Resolving URI Host Port";
    case TRANSMITTER_RESOLUTION_STATE_RESOLVING_IP:
        return "Resolving IP";
    case TRANSMITTER_RESOLUTION_STATE_RESOLVED:
        return "Resolved";
    case TRANSMITTER_RESOLUTION_STATE_UNDEFINED:
        return "Undefined";
    default:
        return "No Such State";
    }
}

#endif  /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */



#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransmitterHandleSigCompCompartment
 * ------------------------------------------------------------------------
 * General: The function should be called when a SigComp-SIP message is
 *          sent. According to the SigComp model each SigComp-SIP
 *          message MUST be related to any SigComp compartment. Thus, the
 *          function verifies that the Transaction is currently associated
 *          with a SigComp Compartment. In case that the Trx instance is not
 *          related to any compartment, then new one is created for it.
 *
 * NOTE:    This function assumes that the transmitter has already resolved
 *          its destination address.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx       - A pointer to the Transmitter.
 *        hMsgToSend - Handle to the transmitted message.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterHandleSigCompCompartment(
                                    IN Transmitter     *pTrx,
                                    IN RvSipMsgHandle   hMsgToSend)
{
    RvStatus             rv           = RV_OK;
    RvBool               bUrnFound    = RV_FALSE;
    RPOOL_Ptr            urnRpoolPtr;
	SipTransportAddress  srcAddr;
    RPOOL_Ptr           *pUrnRpoolPtr = NULL;

    /* Check if the Trx is already related to a Compartment. Since the Trx  */
    /* transmits message to one destination only, the Compartment mapping   */
    /* should stand through a whole Trx lifetime */
    if (pTrx->hSigCompCompartment != NULL)
    {
        return RV_OK;
    }

    rv = LookForSigCompUrnInSentMsg(pTrx,hMsgToSend,&bUrnFound,&urnRpoolPtr);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterHandleSigCompCompartment - Trx 0x%p failed to look for URN in sent msg 0x%x (rv=%d)",
            pTrx,hMsgToSend,rv));

        return rv;
    }

    pUrnRpoolPtr = (bUrnFound == RV_TRUE) ? &urnRpoolPtr : NULL;

    /* The Trx can send his messages to one destination only, thus */
    /* each message is NOT related independently to a Compartment */

	populateTransportAddessFromLocalAddr(pTrx, &srcAddr);
    /* Find matching Compartment to the current sent message */
    rv = SipCompartmentRelateCompHandleToOutMsg(
				   pTrx->pTrxMgr->hCompartmentMgr,
				   pUrnRpoolPtr,
				   &srcAddr,
				   &pTrx->destAddr,
				   hMsgToSend,
                   pTrx->hConn,
				   &pTrx->hSigCompCompartment);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
           "TransmitterHandleSigCompCompartment - Transmitter 0x%p cannot find related compartment",
            pTrx));
        return rv;
    }

	RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
		"TransmitterHandleSigCompCompartment - Attaching Transmitter 0x%p to compartment=0x%p",
		pTrx,pTrx->hSigCompCompartment));

    rv = SipCompartmentAttach(pTrx->hSigCompCompartment,(void*)pTrx);
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterHandleSigCompCompartment - Transmitter 0x%p, failed to attach to compartment=0x%p",
            pTrx,pTrx->hSigCompCompartment));
        return rv;
    }

    RV_UNUSED_ARG(pTrx)

    return rv;
}

/***************************************************************************
* TransmitterFreeRedundantOutboundMsgIfNeeded
* ------------------------------------------------------------------------
* General: Frees the redundant encoded outbound message page in case that
*          only the SigComp outbound message page was sent. (The inverted
*          case is impossible because if the transaction message is
*          decompressed successfully the SigComp outbound message is created
*          and sent first). The redundant outbound message page can exists
*          only when the SigComp feature is enabled.
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* input:   pTrx         - A pointer to the Transmitter.
***************************************************************************/
void RVCALLCONV TransmitterFreeRedundantOutboundMsgIfNeeded(
                                       IN Transmitter   *pTrx)
{
	RvBool				bShouldFree  = RV_TRUE;
	RLIST_ITEM_HANDLE	hListElement = NULL;
    HPAGE				hPage;
    RvInt32				safeCounter  = 0;
	

	if (pTrx->hEncodedOutboundMsg == NULL_PAGE)
    {
		return;
	}

	/* In case the message was sent compressed, the handle to the compressed message was
	   pushed into the msg2send list, but the encoded message handle was not. This means no
	   one will free it. So we're freeing it here.
	   If the message was not compressed, then the encoded message handle was pushed into
	   the msg2send list, and once sent, the handle was dereferenced, so the following
	   condition will not apply (the handle will already be NULL). So there is no
	   risk the handle will be freed twice
    */
	
	/* The encoded message page is normally freed from within the message to send list.
	   Therefore we check if the encoded message was inserted to the message list. If
	   we find it there, there is no need to free the encoded message */
	
    if(pTrx->hMsgToSendList != NULL)
    {
		RLIST_GetHead(NULL, pTrx->hMsgToSendList, &hListElement);
		
		while (hListElement != NULL && safeCounter < pTrx->pTrxMgr->maxMessages)
		{
			hPage = *((HPAGE*)hListElement);
			if (hPage == pTrx->hEncodedOutboundMsg)
			{
				bShouldFree = RV_FALSE;
				break;	
			}
			RLIST_GetNext(NULL, pTrx->hMsgToSendList, hListElement, &hListElement);
			safeCounter++;
		}
    
    }
    
    if (bShouldFree == RV_TRUE)
    {
        RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool,
                       pTrx->hEncodedOutboundMsg);
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "TransmitterFreeRedundantOutboundMsgIfNeeded - Free redundant outbound msg 0x%p in Transmitter 0x%p",
                pTrx->hEncodedOutboundMsg,pTrx));
		pTrx->hEncodedOutboundMsg = NULL_PAGE;
    }
}


/***************************************************************************
 * TransmitterPrepareCompressedSentMsgPage
 * ------------------------------------------------------------------------
 * General: Prepare the outbound message that is going to be sent over
 *          the transaction, by compressing it.
 *          NOTE: The function assumes that the hEncodedOutboundMsg contains
 *                the valid data to be sent.
 * Return Value: RV_success/RV_ERROR_UNKNOWN
 * ------------------------------------------------------------------------
 * Arguments:
 * Input/Output:
 *      pTrx            - A pointer to the Transmitter.
 *
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterPrepareCompressedSentMsgPage(
                                     IN  Transmitter  *pTrx)
{
    RvStatus             rv;
    RvChar              *compBuf  = NULL;
    RvUint32             compLen  = 0;
    RvChar              *tempBuf  = NULL;
    RvChar              *finalBuf = NULL;
    TransmitterMgr      *pTrxMgr  = pTrx->pTrxMgr;
    RvSigCompMessageInfo sigCompMsgInfo;
    RvUint32             maxTempLen;
    RvUint32             tempLen;
    RvUint32             finalLen;
    RvInt32              compPageOffset;

    /* Initialize the output parameter */
    pTrx->hSigCompOutboundMsg = NULL_PAGE;

    /* Allocate a consecutive buffer for the input plain buffer */
    rv = SipTransportMgrAllocateRcvBuffer(pTrxMgr->hTransportMgr,&tempBuf,&maxTempLen);
    if (RV_OK != rv)
    {
        RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterPrepareCompressedSentMsgPage - Failed to allocate consecutive plain buffer for hEncodedOutboundMsg=%0x",
            pTrx->hEncodedOutboundMsg));

        return rv;
    }
    if (pTrx->encodedMsgLen > maxTempLen)
    {
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
        RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterPrepareCompressedSentMsgPage - Cannot handle too long hEncodedMsg=%0x",pTrx->hEncodedOutboundMsg));

        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    /* Allocate a consecutive buffer for the output compression buffer */
    rv = SipTransportMgrAllocateRcvBuffer(pTrxMgr->hTransportMgr,&compBuf,&compLen);
    if (RV_OK != rv)
    {
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
        RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterPrepareCompressedSentMsgPage - Failed to allocate consecutive comp buffer for hEncodedMsg=%0x",
            pTrx->hEncodedOutboundMsg));

        return rv;
    }

     /* Copy the plain message page to consecutive buffer */
    rv = RPOOL_CopyToExternal(pTrxMgr->hMessagePool,pTrx->hEncodedOutboundMsg,0,tempBuf,pTrx->encodedMsgLen);
    if (RV_OK != rv)
    {
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,compBuf);
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
        RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterPrepareCompressedSentMsgPage - Failed to copy hEncodedMsg=%0x to external buffer",pTrx->hEncodedOutboundMsg));

        return rv;
    }
    /* Compress the sent buffer by an external SigComp dll */
    sigCompMsgInfo.compressedMessageBuffSize = compLen;
    sigCompMsgInfo.pCompressedMessageBuffer  = (RvUint8*)compBuf;
    sigCompMsgInfo.plainMessageBuffSize      = pTrx->encodedMsgLen;
    sigCompMsgInfo.pPlainMessageBuffer       = (RvUint8*)tempBuf;
	convertTransportTypeToSigcompTransport(pTrx, &sigCompMsgInfo.transportType);

    rv = SipCompartmentCompressMessage(pTrx->hSigCompCompartment,&sigCompMsgInfo);

    if (RV_OK != rv)
    {
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
        SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,compBuf);
        RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterPrepareCompressedSentMsgPage - Transmitter=0x%p failed to compress encoded message 0x%p",
            pTrx,pTrx->hEncodedOutboundMsg));

        return RV_ERROR_COMPRESSION_FAILURE;
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvLogInfo(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
       "TransmitterPrepareCompressedSentMsgPage - The following message was compressed by Trx=0x%p (rv=%d):",pTrx,rv));
    sigCompMsgInfo.pPlainMessageBuffer[sigCompMsgInfo.plainMessageBuffSize] = '\0';
    SipTransportMsgBuilderPrintMsg(pTrx->pTrxMgr->hTransportMgr,
                                   (RvChar*)sigCompMsgInfo.pPlainMessageBuffer,
                                   SIP_TRANSPORT_MSG_BUILDER_OUTGOING);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


    /* In case of stream transport the SigComp message has to become streamed */
    if (pTrx->destAddr.eTransport == RVSIP_TRANSPORT_TCP ||
        pTrx->destAddr.eTransport == RVSIP_TRANSPORT_TLS)
    {
        tempLen = maxTempLen;
        rv = RvSigCompMessageToStream(sigCompMsgInfo.compressedMessageBuffSize,
                                      sigCompMsgInfo.pCompressedMessageBuffer,
                                      &tempLen,
                                      (RvUint8*)tempBuf);
        if (rv != RV_OK)
        {
            SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
            SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,compBuf);
            RvLogError(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
                "TransmitterPrepareCompressedSentMsgPage - Transmitter=0x%p failed to convert SigComp message to stream",
                pTrxMgr));
            return rv;
        }
        finalBuf = tempBuf;
        finalLen = tempLen;
    }
    else
    {
        finalBuf = (RvChar*)sigCompMsgInfo.pCompressedMessageBuffer;
        finalLen = sigCompMsgInfo.compressedMessageBuffSize;
    }

    /* Copy the compression output consecutive buffer to a transaction local page */
    rv = RPOOL_GetPage(pTrxMgr->hMessagePool, 0, &(pTrx->hSigCompOutboundMsg));
    if(RV_OK != rv)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "TransmitterPrepareCompressedSentMsgPage - RPOOL_GetPage failed (TransmitterMgr=%0x,pool=%0x)",
            pTrxMgr,pTrxMgr->hMessagePool));
        pTrx->hSigCompOutboundMsg = NULL_PAGE;
    }
    else
    {
        RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "TransmitterPrepareCompressedSentMsgPage - Got new msg page %0x on pool %0x for TransmitterMgr %0x",
            pTrx->hSigCompOutboundMsg, pTrxMgr->hMessagePool, pTrxMgr));
        rv = RPOOL_AppendFromExternalToPage(pTrxMgr->hMessagePool,
                                            pTrx->hSigCompOutboundMsg,
                                            finalBuf,
                                            finalLen,
                                            &compPageOffset);
        if (RV_OK != rv)
        {
            RPOOL_FreePage(pTrxMgr->hMessagePool,pTrx->hSigCompOutboundMsg);
            pTrx->hSigCompOutboundMsg = NULL_PAGE;
        }
    }
    /* Re-assign the updated SigCompOutboundMsg to the SentOutboundMsg page */
    pTrx->hSentOutboundMsg = pTrx->hSigCompOutboundMsg;

    SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,tempBuf);
    SipTransportMgrFreeRcvBuffer(pTrxMgr->hTransportMgr,compBuf);

    return rv;
}

/***************************************************************************
* TransmitterUseCompartment
* ------------------------------------------------------------------------
* General:Assign a transmitter with a compartment
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* input: pTrx- The Transmitter handle.
***************************************************************************/
RvStatus RVCALLCONV TransmitterUseCompartment(IN Transmitter *pTrx)
{
	RvStatus         rv;
    TransmitterMgr  *pTrxMgr;


	if (NULL == pTrx)
	{
		return (RV_ERROR_BADPARAM);
	}

	pTrxMgr = pTrx->pTrxMgr;

	rv = SipCompartmentAddToMgr(pTrx->hSigCompCompartment,pTrx->hConn);
	if (RV_OK != rv)
	{
		RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "TransmitterUseCompartment - Failed to use compartment (Transmitter=%0x)",
            pTrx));
		return (rv);
	}
	return RV_OK;
}

#endif /* RV_SIGCOMP_ON */

/***************************************************************************
* TransmitterFreeEncodedOutboundMsg
* ------------------------------------------------------------------------
* General:Free the outbound messages that are related to the Transmitter.
*         The Transmitter can own both hEncodedOutboundMsg and
*         hSigCompOutboundMsg
* Return Value: -.
* ------------------------------------------------------------------------
* Arguments:
* input: pTrx- The Transmitter handle.
***************************************************************************/
void RVCALLCONV TransmitterFreeEncodedOutboundMsg(IN Transmitter* pTrx)
{
    if(pTrx->hEncodedOutboundMsg != NULL_PAGE)
    {
        RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool,
                       pTrx->hEncodedOutboundMsg);
        pTrx->hEncodedOutboundMsg = NULL_PAGE;
    }
#ifdef RV_SIGCOMP_ON
    if(pTrx->hSigCompOutboundMsg != NULL_PAGE)
    {
        RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool,
                       pTrx->hSigCompOutboundMsg);
        pTrx->hSigCompOutboundMsg = NULL_PAGE;
    }
#endif /* RV_SIGCOMP_ON */
    pTrx->hSentOutboundMsg = NULL_PAGE;
}

/***************************************************************************
 * TransmitterGetLocalAddressHandleByTypes
 * ------------------------------------------------------------------------
 * General: Get the local address handle according to a given transport
 *          and address type.
 * Return Value: RV_ERROR_NOT_FOUND - This type of address does not exist.
 *               RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction
 *          eTransportType - The transport type (udp or tcp).
 *          eAddressType - The address type (ip4 or ip6).
 * Output:  pphLocalAddr   - the address of the local address handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterGetLocalAddressHandleByTypes(
                            IN  Transmitter                 *pTrx,
                            IN  RvSipTransport              eTransportType,
                            IN  RvSipTransportAddressType   eAddressType,
                            OUT RvSipTransportLocalAddrHandle **pphLocalAddr)
{
    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        {
            if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalUdpIpv4);
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalUdpIpv6);
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
    case RVSIP_TRANSPORT_TCP:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalTcpIpv4);
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalTcpIpv6);
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalTlsIpv4);
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalTlsIpv6);
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalSctpIpv4);
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                *pphLocalAddr = &(pTrx->localAddr.hLocalSctpIpv6);
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        {
            *pphLocalAddr = NULL;
            return RV_ERROR_NOT_FOUND;
        }
    } /* switch (eTransportType) */

    return RV_OK;
}

/***************************************************************************
 * TransmitterGetLocalAddressHandleByTypes
 * ------------------------------------------------------------------------
 * General: Set the local address handle into the Transmitter's Local Addresses
 *          structure according to a given transport and address type.
 * Return Value: RV_OK on success, error code otherwise
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx           - Pointer to the transmitter
 *          eTransportType - The transport type (udp or tcp).
 *          eAddressType   - The address type (ip4 or ip6).
 *          hLocalAddr     - The local address handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterSetLocalAddressHandleByTypes(
                            IN  Transmitter*                  pTrx,
                            IN  RvSipTransport                eTransportType,
                            IN  RvSipTransportAddressType     eAddressType,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr)
{
    switch (eTransportType)
    {
    case RVSIP_TRANSPORT_UDP:
        {
            if(eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                pTrx->localAddr.hLocalUdpIpv4 = hLocalAddr;
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                pTrx->localAddr.hLocalUdpIpv6 = hLocalAddr;
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
    case RVSIP_TRANSPORT_TCP:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                pTrx->localAddr.hLocalTcpIpv4 = hLocalAddr;
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                pTrx->localAddr.hLocalTcpIpv6 = hLocalAddr;
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    case RVSIP_TRANSPORT_TLS:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                pTrx->localAddr.hLocalTlsIpv4 = hLocalAddr;
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                pTrx->localAddr.hLocalTlsIpv6 = hLocalAddr;
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    case RVSIP_TRANSPORT_SCTP:
        {
            if (eAddressType == RVSIP_TRANSPORT_ADDRESS_TYPE_IP)
            {
                pTrx->localAddr.hLocalSctpIpv4 = hLocalAddr;
            }
#if(RV_NET_TYPE & RV_NET_IPV6)
            else
            {
                pTrx->localAddr.hLocalSctpIpv6 = hLocalAddr;
            }
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
        }
        break;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    default:
        return RV_ERROR_NOTSUPPORTED;
    } /* switch (eTransportType) */

    return RV_OK;
}

/***************************************************************************
 * TransmitterSetLocalAddressHandle
 * ------------------------------------------------------------------------
 * General: Set a local address handle in the transmitter local address
 *          structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTrx - Pointer to the transmitter
 *          hLocalAddr - the local address handle
 *           bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV TransmitterSetLocalAddressHandle(
                            IN  Transmitter*                  pTrx,
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            IN  RvBool                        bSetAddrInUse)
{

     SipTransportLocalAddrSetHandleInStructure(pTrx->pTrxMgr->hTransportMgr,
                                               &pTrx->localAddr,
                                               hLocalAddr,bSetAddrInUse);

}

/***************************************************************************
 * TransmitterSetDestAddr
 * ------------------------------------------------------------------------
 * General: Sets the destination address the transmitter will use.
 *          Use this function when you want the transmitter to use
 *          a specific address regardless of the message content.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx              - The Transmitter handle
 *            pDestAddr         - destination addr
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterSetDestAddr(
                          IN Transmitter*                  pTrx,
                          IN SipTransportAddress*          pDestAddr,
                          IN RvSipTransmitterExtOptions*   pOptions)
{
    memcpy(&pTrx->destAddr,pDestAddr,sizeof(pTrx->destAddr));

    pTrx->eResolutionState  = TRANSMITTER_RESOLUTION_STATE_RESOLVED;

    if (NULL != pOptions)
    {
        /* do options things */
#ifdef RV_SIGCOMP_ON
        if (pTrx->eOutboundMsgCompType != pOptions->eMsgCompType)
        {
            RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterSetDestAddr - Change the stack decision from comp type %d to comp %d",
                pTrx->eOutboundMsgCompType,pOptions->eMsgCompType));
        }
        pTrx->eOutboundMsgCompType = pOptions->eMsgCompType;
#endif /* RV_SIGCOMP_ON */
    }

    return RV_OK;
}


/************************************************************************************
 * TransmitterAddNewTopViaToMsg
 * ----------------------------------------------------------------------------------
 * General: Adds top via header to the message before address resolution and
 *          according to the transport of the request URI combined with the
 *          available local addresses. This is a best shot via header. The via
 *          header will be fixed after the final destination will be resolved.
 *          with the function TransmitterUpdateViaByFinalLocalAddress.
 *          Note: this function will use the branch found in the transmitter or
 *          will generate a new branch.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - Handle to the transmitter
 *          hMsg      - A message to add a top via to.
 *          hSourceVia- if NULL, use values extracted from transport,
 *                      o/w - copy this value to the new via header
 *          bIgnorTrxBranch - If the transmitter needs to add a branch it should create
 *                            a new branch and ignore its own existing branch.
 *                            this is good for ack on 2xx.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterAddNewTopViaToMsg (
                     IN  Transmitter*           pTrx,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipViaHeaderHandle   hSourceVia,
                     IN  RvBool                 bIgnorTrxBranch)
{
    RvSipTransportLocalAddrHandle hLocalAddr   = NULL;
    RvStatus                 rv                = RV_OK;
    RvSipAddressHandle       hRequestUriHeader = NULL;
    RvSipViaHeaderHandle     hViaHeader        = NULL;
    RvSipTransport           eTransport        = RVSIP_TRANSPORT_UNDEFINED;
    RvChar                   strBranch[SIP_COMMON_ID_SIZE];


    if(pTrx->bSkipViaProcessing == RV_TRUE)
    {
        return RV_OK;
    }
    /*if the transmitter is the one that created the top via, it needs to update
      it as well*/
    pTrx->bTrxCreatedVia = RV_TRUE;
    /* Create a Via header containing the local IP address and port, and insert
       it into the message */
    rv = RvSipViaHeaderConstructInMsg(hMsg, RV_TRUE, &hViaHeader);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: Failed to construct via header",
            pTrx));
        return rv;
    }

    if (NULL == hSourceVia) /* extract via params from transport */
    {

        /*get the transport from the request uri*/
        hRequestUriHeader = RvSipMsgGetRequestUri(hMsg);
        if(hRequestUriHeader == NULL)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                           "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: URI is secure but TLS is not supported (rv=%d)",
                           pTrx,RV_ERROR_ILLEGAL_ACTION));

            return RV_ERROR_UNKNOWN;
        }

        /*we can get the transport from the message only for sip url.*/
        if(RvSipAddrGetAddrType(hRequestUriHeader) == RVSIP_ADDRTYPE_URL)
        {
            if (RV_TRUE == RvSipAddrUrlIsSecure(hRequestUriHeader))
            {
#if (RV_TLS_TYPE != RV_TLS_NONE)
                eTransport = RVSIP_TRANSPORT_TLS;
#else
                RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                           "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: URI is secure but TLS is not supported (rv=%d)",
                           pTrx,RV_ERROR_ILLEGAL_ACTION));
                return RV_ERROR_ILLEGAL_ACTION;
#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */
            }
            else
            {
                eTransport = RvSipAddrUrlGetTransport(hRequestUriHeader);
            }
        }

        rv = ChooseLocalAddressForViaFromLocalAddresses(pTrx,eTransport,&hLocalAddr);
        if (rv != RV_OK)
        {
            RvLogWarning(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: Failed to get local address of type %s to set in the via header. defaulting to UDP",
                pTrx, SipCommonConvertEnumToStrTransportType(eTransport)));
            /* we could not get an address type as the URI indicated. this situation is possible when we will send through loose route,
               no the loose route can be UDP/TCP (as we support) but the URI can be TLS. so we change via to UDP, and
               we will update the via to the actual transport of the loose route after transport rsolution is done)*/
            rv = ChooseLocalAddressForViaFromLocalAddresses(pTrx,RVSIP_TRANSPORT_UNDEFINED,&hLocalAddr);
            if (rv != RV_OK)
            {
                RvLogWarning(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                    "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: no UDP address to use as default in via header.",
                    pTrx));
                return rv;
            }
        }
        rv = SipTransportLocalAddressGetTransportType(hLocalAddr,&eTransport);
        if (RV_OK != rv  ||  RVSIP_TRANSPORT_UNDEFINED == eTransport)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                      "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: Failed to get transport type for local address 0x%p",
                       pTrx,hLocalAddr));
            return ((RV_OK != rv) ? rv : RV_ERROR_UNKNOWN);
        }


        /* set the transport param in the via according to the Request URI */
        rv = RvSipViaHeaderSetTransport(hViaHeader,eTransport,NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: Failed to set transport to via header (rv=%d)",
                pTrx, rv));
            return rv;
        }

		/*set the sent by parameter*/
		rv = SetViaSentByParamByLocalAddress(pTrx, hLocalAddr, hViaHeader,
                                             NULL /*pSecureServerPort*/);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "TransmitterAddNewTopViaToMsg - Transmitter 0x%p, localAddr 0x%p: Failed in SetViaSentByParamByLocalAddress (rv=%d)",
                pTrx, hLocalAddr, rv));
            return rv;
        }
    }
    else
    {
        rv = RvSipViaHeaderCopy(hViaHeader,hSourceVia);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: could not copy source via (rv=%d)",
                pTrx, rv));
            return rv;
        }
    }
    if (NULL == hSourceVia) /* extract via params from transport */
    {
        /*Add via branch if specified already set on the transaction*/
        if(UNDEFINED != pTrx->viaBranchOffset && bIgnorTrxBranch == RV_FALSE)
        {

            rv = SipViaHeaderSetBranchParam(hViaHeader,NULL,
                                            pTrx->hPool,
                                            pTrx->hPage,
                                            pTrx->viaBranchOffset);

        }
        else /* Add a branch to the Via header if it doesn't contain a branch or if
            we need to use a different branch then the one the transmitter has*/
        {
            TransmitterGenerateBranchStr(pTrx,strBranch);
            /*don't update the transmitter branch if its already exists*/
            if(UNDEFINED == pTrx->viaBranchOffset)
            {

                rv = RPOOL_AppendFromExternal(pTrx->hPool,
											  pTrx->hPage,
											  strBranch,
											  (RvInt)strlen(strBranch) + 1,
											  RV_FALSE,
	                                          &(pTrx->viaBranchOffset),
											  NULL);

                if(rv != RV_OK)
                {
                    RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                        "TransmitterAddNewTopViaToMsg - Transmitter 0x%p: Error in RPOOL_AppendFromExternal(), (rv=%d)",
                         pTrx, rv));
                    return rv;
                }
#ifdef SIP_DEBUG
                pTrx->strViaBranch = RPOOL_GetPtr(pTrx->hPool,
                                                    pTrx->hPage,
                                                    pTrx->viaBranchOffset);
#endif
            }
            /*set the branch in the message*/
            rv = RvSipViaHeaderSetBranchParam(hViaHeader, strBranch);
            if(rv != RV_OK)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                    "TransmitterAddNewTopViaToMsg - Transaction 0x%p: Error setting branch parameter, err= %d.",
                    pTrx, rv));
                return rv;
            }
        }
    }
    return rv;

}

/************************************************************************************
 * TransmitterSetBranch
 * ----------------------------------------------------------------------------------
 * General: Sets the branch parameter to the transmitter object.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - Pointer to the transmitter
 *          strViaBranch  - The Via branch to add to the top via header.
 *                          This parameter is ignored for response messages or when
 *                          the handleTopVia parameter is RV_FALSE.
 *          pRpoolViaBranch - The branch supplied on a page. You should set this
 *                          parameter to NULL if the branch was supplied as a string.
 ***********************************************************************************/
RvStatus RVCALLCONV TransmitterSetBranch(
                                  IN Transmitter*           pTrx,
                                  IN RvChar*                strViaBranch,
                                  IN RPOOL_Ptr*             pRpoolViaBranch)
{
    RvStatus rv = RV_OK;
    if(strViaBranch != NULL)
    {

		rv = RPOOL_AppendFromExternal(pTrx->hPool,pTrx->hPage,strViaBranch,
			                         (RvInt)strlen(strViaBranch) + 1,RV_FALSE,
									 &(pTrx->viaBranchOffset),NULL);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                      "TransmitterSetBranch - Transmitter 0x%p: Error in RPOOL_AppendFromExternal(), (rv=%d)",
                       pTrx, rv));
            return rv;
        }
    }
    else
    {
        RPOOL_Ptr           destBranchPtr;

        destBranchPtr.hPool = pTrx->hPool;
        destBranchPtr.hPage = pTrx->hPage;
        destBranchPtr.offset = UNDEFINED;

        rv = SipCommonCopyStrFromPageToPage(pRpoolViaBranch,&destBranchPtr);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "TransmitterSetBranch - Transmitter 0x%p: Failed to copy via branch to transmitter page",
                pTrx));
            return rv;
        }

        pTrx->viaBranchOffset = destBranchPtr.offset;
    }
#ifdef SIP_DEBUG
    pTrx->strViaBranch = RPOOL_GetPtr(pTrx->hPool,
                                      pTrx->hPage,
                                      pTrx->viaBranchOffset);
#endif

    return RV_OK;

}





/***************************************************************************
 * TransmitterGenerateBranchStr
 * ------------------------------------------------------------------------
 * General: Generate branch string to be used for branch.
 * Return Value:none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx       - pointer to the Transmitter.
 * Output:    strBranch  - a generated string that contains the random ID.
 *                       the string must be of size SIP_COMMON_ID_SIZE
 ***************************************************************************/
void RVCALLCONV TransmitterGenerateBranchStr(IN   Transmitter* pTrx,
                                             OUT  RvChar*      strBranch)
{
    RvUint32   curTime = SipCommonCoreRvTimestampGet(IN_SEC);
    RvUint32   timer = SipCommonCoreRvTimestampGet(IN_MSEC);
    RvInt32    r;

    RvRandomGeneratorGetInRange(pTrx->pTrxMgr->seed,RV_INT32_MAX,(RvRandom*)&r);
    RvSprintf(strBranch, "z9hG4bK-%x-%x-%x", curTime, timer, r);
}

/***************************************************************************
 * TransmitterSetInUseLocalAddrByDestAddr
 * ------------------------------------------------------------------------
 * General: Sets the inuse local address according to the destination address
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc   - the transaction,
 *          pDestAddr - the destination address.
 ***************************************************************************/
RvStatus TransmitterSetInUseLocalAddrByDestAddr(
                                  IN Transmitter* pTrx)
{
    RvSipTransportLocalAddrHandle* phLocalAddr  = NULL;
    RvSipTransportAddressType      eSipAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
    RvStatus                       rv           = RV_OK;
	RvSipTransmitterMsgCompType   *peCompType;
#ifdef RV_SIP_IMS_ON
	RPOOL_Ptr                      strSigcompId;
#endif

#ifdef RV_SIGCOMP_ON
	peCompType = &pTrx->eOutboundMsgCompType;
#else
	peCompType = NULL;
#endif /* #ifdef RV_SIGCOMP_ON */

    /* If Security is supported, transport, destination address and local
    address should be taken from Security Object */
#ifdef RV_SIP_IMS_ON
    if (NULL != pTrx->hSecObj)
    {
        RvSipTransportLocalAddrHandle  hSecureLocalAddr;

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        int isResp=(RvSipMsgGetMsgType((const RvSipMsgHandle)pTrx->hMsgToSend)==RVSIP_MSG_RESPONSE);

        if(!RvSipSecObjIsReady(pTrx->hSecObj,isResp)) {
          SecObj* so=(SecObj*)pTrx->hSecObj;
          if(so->pMgr && so->pMgr->getAlternativeSecObjFunc) {
            SecObj* sonew=(SecObj*)(so->pMgr->getAlternativeSecObjFunc(so,isResp));
            if(sonew && sonew!=so) {
              RvSipTransmitterSetSecObj((RvSipTransmitterHandle)pTrx,(RvSipSecObjHandle)sonew);
            }
          }
        }
#endif
//SPIRENT_END

        pTrx->destAddr.eTransport = RVSIP_TRANSPORT_UNDEFINED;
		strSigcompId.hPool  = pTrx->pTrxMgr->hGeneralPool;
		strSigcompId.hPage  = pTrx->hPage;
		strSigcompId.offset = UNDEFINED;
        rv = SipSecObjGetSecureLocalAddressAndConnection(
                pTrx->pTrxMgr->hSecMgr, pTrx->hSecObj,
                RvSipMsgGetMsgType((const RvSipMsgHandle)pTrx->hMsgToSend),
                &pTrx->destAddr.eTransport, &hSecureLocalAddr,
                &pTrx->destAddr.addr, &pTrx->hSecureConn,
				peCompType,
				&strSigcompId,
				RV_TRUE);
	if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterSetInUseLocalAddrByDestAddr - pTrx=0x%p: failed to get Secure Local Address and Connection. SecObj=%p(rv=%d:%s)",
                pTrx, pTrx->hSecObj, rv, SipCommonStatus2Str(rv)));
            return RV_ERROR_UNKNOWN;
        }
#ifdef RV_SIGCOMP_ON
		pTrx->sigcompIdOffset	= strSigcompId.offset;
#endif /* #ifdef RV_SIGCOMP_ON */

        /* Set Secure Local Address into the Transmitter Local Addresses */
        eSipAddrType = SipTransportConvertCoreAddrTypeToSipAddrType(
                                    RvAddressGetType(&pTrx->destAddr.addr));
        rv = TransmitterSetLocalAddressHandleByTypes(pTrx,
                pTrx->destAddr.eTransport, eSipAddrType, hSecureLocalAddr);
        if(rv != RV_OK)
        {
            return rv;
        }
        RvLogDebug(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterSetInUseLocalAddrByDestAddr - pTrx=0x%p: Secure Local Address %p, Secure Connection %p",
            pTrx, hSecureLocalAddr, pTrx->hSecureConn));
    }
#endif /*#ifdef RV_SIP_IMS_ON*/


    eSipAddrType = SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pTrx->destAddr.addr));

    if(pTrx->destAddr.eTransport == RVSIP_TRANSPORT_UNDEFINED ||
       eSipAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED)
    {
        return RV_ERROR_NOT_FOUND;
    }
    rv = TransmitterGetLocalAddressHandleByTypes(pTrx,
                                                 pTrx->destAddr.eTransport,
                                                 eSipAddrType,
                                                 &phLocalAddr);
    if(rv != RV_OK)
    {
        return rv;
    }

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)

    if(NULL == *phLocalAddr && pTrx->destAddr.eTransport != pTrx->outboundAddr.ipAddress.eTransport && 
       pTrx->outboundAddr.ipAddress.eTransport != RVSIP_TRANSPORT_UNDEFINED) {
      rv = TransmitterGetLocalAddressHandleByTypes(pTrx,
						   pTrx->outboundAddr.ipAddress.eTransport,
						   eSipAddrType,
						   &phLocalAddr);
      if(rv != RV_OK) {
	return rv;
      }
      if(NULL != *phLocalAddr) {
	pTrx->destAddr.eTransport = pTrx->outboundAddr.ipAddress.eTransport;
      } else if(pTrx->destAddr.addr.addrtype != pTrx->outboundAddr.ipAddress.addr.addrtype &&
		pTrx->outboundAddr.ipAddress.addr.addrtype != RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED) {
	eSipAddrType = SipTransportConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pTrx->outboundAddr.ipAddress.addr));
	rv = TransmitterGetLocalAddressHandleByTypes(pTrx,
						     pTrx->outboundAddr.ipAddress.eTransport,
						     eSipAddrType,
						     &phLocalAddr);
	if(rv != RV_OK) {
	  return rv;
	}
	if(NULL != *phLocalAddr) {
	  pTrx->destAddr.eTransport = pTrx->outboundAddr.ipAddress.eTransport;
	  pTrx->destAddr.addr.addrtype = pTrx->outboundAddr.ipAddress.addr.addrtype;
	}
      }
    }
#endif
//SPIRENT_END

#if (RV_NET_TYPE & RV_NET_SCTP)
#if (RV_NET_TYPE & RV_NET_IPV6)
    /* If pure IPv4 address of SCTP type was not found, try to use mixed
       Local Address, bound to IPv4 and IPv6 multihoming addresses */
    if (NULL == *phLocalAddr && NULL != pTrx->localAddr.hLocalSctpIpv6 &&
        RVSIP_TRANSPORT_SCTP==pTrx->destAddr.eTransport &&
        RVSIP_TRANSPORT_ADDRESS_TYPE_IP == eSipAddrType)
    {
        RvUint32 numOfIpv4, numOfIpv6;
        numOfIpv4=0; numOfIpv6=0;
        rv = SipTransportLocalAddressGetNumIpv4Ipv6Ips(
            pTrx->localAddr.hLocalSctpIpv6, &numOfIpv4, &numOfIpv6);
        if (RV_OK==rv  &&  0<numOfIpv4)
        {
            phLocalAddr = &pTrx->localAddr.hLocalSctpIpv6;
        }
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
#endif /*(RV_NET_TYPE & RV_NET_SCTP)*/

	if (*phLocalAddr == NULL)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterSetInUseLocalAddrByDestAddr - pTrx=0x%p: Failed to find local addr for dest addr type %d",
            pTrx, eSipAddrType));
		return RV_ERROR_NOT_FOUND;
	}

    pTrx->localAddr.hAddrInUse = phLocalAddr;
    return RV_OK;
}

/***************************************************************************
* TransmitterUpdateViaByFinalLocalAddress
* ------------------------------------------------------------------------
* General: Update the via header with the local address that is suitable for
*          the destination address.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input: pTrx             - The transmitter handle.
*        bFixVia          - The function should fix the via header
***************************************************************************/
RvStatus TransmitterUpdateViaByFinalLocalAddress(
                                IN Transmitter* pTrx,
                                IN RvBool       bFixVia)
{
    RvStatus                    rv             = RV_OK;
    RvSipViaHeaderHandle        hVia           = NULL;
    RvSipHeaderListElemHandle   hListElem      = NULL;
    RvSipTransport              eTransportType;
#ifdef RV_SIP_IMS_ON
    RvUint16                    secureServerPort = 0;
#endif /* #ifdef RV_SIP_IMS_ON */
    RvUint16*                   pSecureServerPort = NULL;

    if(pTrx->bSkipViaProcessing == RV_TRUE)
    {
        return RV_OK;
    }

    if(*(pTrx->localAddr.hAddrInUse) == NULL)
    {
		return RV_ERROR_NOT_FOUND;
    }

	if(RVSIP_MSG_REQUEST != RvSipMsgGetMsgType(pTrx->hMsgToSend))
	{
		return RV_OK;
	}

	rv = SipTransportLocalAddressGetTransportType(*(pTrx->localAddr.hAddrInUse),&eTransportType);
    if (RV_OK != rv  ||  RVSIP_TRANSPORT_UNDEFINED == eTransportType)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to get transport type for local address 0x%p",pTrx, *(pTrx->localAddr.hAddrInUse)));
        return ((RV_OK != rv) ? rv : RV_ERROR_UNKNOWN);
    }

    hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                            pTrx->hMsgToSend,
                                            RVSIP_HEADERTYPE_VIA,
                                            RVSIP_FIRST_HEADER,
                                            RVSIP_MSG_HEADERS_OPTION_ALL,
                                            &hListElem);
    if (hVia == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to find via header in msg 0x%p, via will no be updated (rv=%d)",
            pTrx,pTrx->hMsgToSend,rv));
        return RV_OK;
    }

    /*set the transport in any case*/
    RvSipViaHeaderSetTransport(hVia,eTransportType,NULL);

    /*set the rest of the parameters only if requested*/
#ifdef RV_SIP_IMS_ON
    /* If Security is supported, the parameters of the secure Local Address,
       provided by the Security Object, should be loaded into the VIA. */
    if (NULL != pTrx->hSecObj)
    {
        bFixVia = RV_TRUE;
        rv = SipSecObjGetSecureServerPort(pTrx->pTrxMgr->hSecMgr,
                pTrx->hSecObj, eTransportType, &secureServerPort);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to find secure Server Port (rv=%d:%s). Continue",
                pTrx, rv, SipCommonStatus2Str(rv)));
            /* Don't return here, continue to build VIA header */
        }
        pSecureServerPort = &secureServerPort;
    }
#endif
    if (bFixVia == RV_TRUE)
    {
		rv = SetViaSentByParamByLocalAddress(pTrx,
                *(pTrx->localAddr.hAddrInUse), hVia, pSecureServerPort);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to set sent by in via 0x%p (rv=%d:%s)",
                pTrx, hVia, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* remove the alias parameter in case it remained from previous sending - can be omitted? */
        RvSipViaHeaderSetAliasParam(hVia, RV_FALSE);
    }

    /*add rport in UDP*/
    if (RVSIP_TRANSPORT_UDP == eTransportType                       &&
        RVSIP_MSG_REQUEST   == RvSipMsgGetMsgType(pTrx->hMsgToSend) &&
        RV_TRUE             == pTrx->pTrxMgr->bUseRportParamInVia)
    {
        rv = RvSipViaHeaderSetRportParam(hVia,UNDEFINED,RV_TRUE);
    }

    return rv;
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RvSipTransmitterUpdateViaByFinalLocalAddress(IN RvSipTransmitterHandle hTrx,
                                                      RvSipMsgHandle hMsgToSend,
                                                      RvSipTransport eTransportType) {

  Transmitter* pTrx=(Transmitter*)hTrx;
  RvStatus                    rv             = RV_OK;
  RvSipViaHeaderHandle        hVia           = NULL;
  RvSipHeaderListElemHandle   hListElem      = NULL;
#ifdef RV_SIP_IMS_ON
  RvUint16                    secureServerPort = 0;
#endif /* #ifdef RV_SIP_IMS_ON */ 
  
  hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByTypeExt(
    hMsgToSend,
    RVSIP_HEADERTYPE_VIA,
    RVSIP_FIRST_HEADER,
    RVSIP_MSG_HEADERS_OPTION_ALL,
    &hListElem);
  if (hVia == NULL)
  {
    RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
      "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to find via header in msg 0x%p, via will no be updated (rv=%d)",
      pTrx,hMsgToSend,rv));
    return RV_OK;
  }
  
  /*set the transport in any case*/
  RvSipViaHeaderSetTransport(hVia,eTransportType,NULL);
  
  /*set the rest of the parameters only if requested*/
#ifdef RV_SIP_IMS_ON
  /* If Security is supported, the parameters of the secure Local Address,
  provided by the Security Object, should be loaded into the VIA. */
  if (NULL != pTrx->hSecObj)
  {
    rv = SipSecObjGetSecureServerPort(pTrx->pTrxMgr->hSecMgr,
      pTrx->hSecObj, eTransportType, &secureServerPort);
    if (RV_OK != rv)
    {
      RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
        "TransmitterUpdateViaByFinalLocalAddress - trx=0x%p: Failed to find secure Server Port (rv=%d:%s). Continue",
        pTrx, rv, SipCommonStatus2Str(rv)));
      /* Don't return here, continue to build VIA header */
    }
  }
#endif
  
  RvSipViaHeaderSetPortNum(hVia, secureServerPort);

  /* remove the alias parameter in case it remained from previous sending - can be omitted? */
  RvSipViaHeaderSetAliasParam(hVia, RV_FALSE);
  
  /*add rport in UDP*/
  if (RVSIP_TRANSPORT_UDP == eTransportType                       &&
    RVSIP_MSG_REQUEST   == RvSipMsgGetMsgType(hMsgToSend) &&
    RV_TRUE             == pTrx->pTrxMgr->bUseRportParamInVia)
  {
    rv = RvSipViaHeaderSetRportParam(hVia,UNDEFINED,RV_TRUE);
  }
  
  return rv;
}
#endif
/* SPIRENT END */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransmitterNoOwner
 * ------------------------------------------------------------------------
 * General: Called when attaching NULL owner.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx - The trx handle
 ****************************************************************************/
RvStatus RVCALLCONV TransmitterNoOwner(IN Transmitter* pTrx)
{
    /* Set the trx's owner to NULL */
    pTrx->hAppTrx = NULL;
    memset(&(pTrx->evHandlers), 0, sizeof(pTrx->evHandlers));

    switch(pTrx->eState)
    {
        case RVSIP_TRANSMITTER_STATE_ON_HOLD:
        case RVSIP_TRANSMITTER_STATE_UNDEFINED:
        case RVSIP_TRANSMITTER_STATE_IDLE:
        case RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE:
        case RVSIP_TRANSMITTER_STATE_MSG_SENT:
        case RVSIP_TRANSMITTER_STATE_TERMINATED:
            RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "TransmitterNoOwner: Transmitter 0x%p - Terminate on state %s",
                    pTrx, TransmitterGetStateName(pTrx->eState)));
            TransmitterTerminate(pTrx);
            break;

        default:
            RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "TransmitterNoOwner: Transmitter 0x%p - No owner. state %s",
                    pTrx, TransmitterGetStateName(pTrx->eState)));

    }
    return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIGCOMP_ON
/*****************************************************************************
 * TransmitterUpdateOutboundMsgSigcompId
 * ---------------------------------------------------------------------------
 * General: Update the Trx outbound msg sigcomp-id String of the outbound destination 
 * Return Value: RvStatus.
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx       - The sending transmitter.
 *          hSrcPool   - The source pool holding the sigcomp-id
 *          hSrcPage   - The source page holding the sigcomp-id
 *          srcOffset  - The offset in the page of the sigcomp-id string
 * Output:  
 ****************************************************************************/
 RvStatus RVCALLCONV TransmitterUpdateOutboundMsgSigcompId(
                           IN Transmitter       *pTrx,
                           IN HRPOOL             hSrcPool,
						   IN HPAGE              hSrcPage,
						   IN RvInt32            srcOffset)
{
    RvUint    length;
	RvStatus  rv         = RV_OK;
	
	if (UNDEFINED == srcOffset)
	{
		return RV_OK;
	}

	length = RPOOL_Strlen(hSrcPool, hSrcPage, srcOffset);
	if (0 == length)
	{
		return RV_OK;
	}

    rv = RPOOL_Append(pTrx->hPool,pTrx->hPage, length + 1, RV_TRUE, &pTrx->sigcompIdOffset);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "UpdateTrxOutboundMsgSigcompId - trx=0x%p: Faild to allocate buffer from pool",
            pTrx));   
        return rv;
    }
	
	rv = RPOOL_CopyFrom (pTrx->hPool, pTrx->hPage, pTrx->sigcompIdOffset, hSrcPool, hSrcPage, srcOffset,length+1);
	if (RV_OK != rv)
    {
		RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "UpdateTrxOutboundMsgSigcompId - trx=0x%p: failed to copy sigcomp-id param",pTrx));
        return rv;
	}
    return RV_OK;
}
#endif
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * MsgToSendListConstruct
 * ------------------------------------------------------------------------
 * General: Allocated a list of message pages for messages that are about
 *          to be sent.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrx - The transmitter pointer
 **************************************************************************/
static RvStatus RVCALLCONV MsgToSendListConstruct(
                                                    IN Transmitter    *pTrx)
{

    pTrx->hMsgToSendList = RLIST_RPoolListConstruct(pTrx->hPool,pTrx->hPage,
                                                    sizeof(void*),
                                                    pTrx->pTrxMgr->pLogSrc);

    if(pTrx->hMsgToSendList == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    return RV_OK;
}


/***************************************************************************
 * MsgToSendListDestruct
 * ------------------------------------------------------------------------
 * General: Free all pages in the list. The list is on the object page
 *          and will be destructed when the object is destructed.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx - Pointer to the transmitter
 ***************************************************************************/
static void RVCALLCONV MsgToSendListDestruct(
                              IN Transmitter    *pTrx)
{
    if(pTrx->hMsgToSendList == NULL)
    {
        return;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "MsgToSendListDestruct - transmitter 0x%p destructing msg-to-send-list..." ,pTrx));

    MsgToSendListFreeAllPages(pTrx);
    pTrx->hMsgToSendList = NULL;
}

/***************************************************************************
 * MsgToSendListFreeAllPages
 * ------------------------------------------------------------------------
 * General: Free all pages in the list.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx - Pointer to the transmitter
 ***************************************************************************/
static void RVCALLCONV MsgToSendListFreeAllPages(
                              IN Transmitter    *pTrx)
{
    RLIST_ITEM_HANDLE hListElement;
    HPAGE             hPage;
    RvInt32           safeCounter = 0;

    if(pTrx->hMsgToSendList == NULL)
    {
        return;
    }

    RLIST_GetHead(NULL, pTrx->hMsgToSendList, &hListElement);

    while (hListElement != NULL &&
           safeCounter < pTrx->pTrxMgr->maxMessages)
    {
        hPage = *((HPAGE*)hListElement);
        if (hPage != NULL_PAGE)
        {
            RPOOL_FreePage(pTrx->pTrxMgr->hMessagePool, hPage);
			if (pTrx->hEncodedOutboundMsg == hPage)
			{
				pTrx->hEncodedOutboundMsg = NULL_PAGE;
			}
#ifdef RV_SIGCOMP_ON
			if (pTrx->hSigCompOutboundMsg == hPage)
			{
				pTrx->hSigCompOutboundMsg = NULL_PAGE;
			}
#endif
            hPage = NULL_PAGE;
        }
        RLIST_Remove(NULL, pTrx->hMsgToSendList, hListElement);

        hListElement = NULL;

        RLIST_GetHead(NULL, pTrx->hMsgToSendList, &hListElement);

        safeCounter++;
    }
}
/***************************************************************************
 * SetViaSentByParamByLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the sent-by host and port in the given via header according
 *          to the information placed in the local address header.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx             - Pointer to the transmitter
 *           hLocalAddr       - The local address handle
 *           hVia             - The via header handle
 *           secureServerPort - in IMS network VIA should contain protected
 *                              server port, where the response shuold be sent
 *                              by peer in case of UDP (3GPP TS 24.229)
 ***************************************************************************/
static RvStatus SetViaSentByParamByLocalAddress(
                        IN Transmitter                   *pTrx,
						IN RvSipTransportLocalAddrHandle hLocalAddr,
						IN RvSipViaHeaderHandle          hVia,
                        IN RvUint16*                     pSecureServerPort)
{

	RPOOL_Ptr          sentByRpoolPtr;
	RvChar*            strIp          = NULL;
	RvInt32            sentByPort     = UNDEFINED;
	RvUint16           localPort      = 0;
	RvStatus           rv             = RV_OK;

	sentByRpoolPtr.hPool  = NULL;
	sentByRpoolPtr.hPage  = NULL_PAGE;
	sentByRpoolPtr.offset = UNDEFINED;

	rv = SipTransportLocalAddressGetBufferedAddressByHandle(hLocalAddr,
											&sentByRpoolPtr, &sentByPort,
											&strIp,&localPort);
	if (RV_OK != rv)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
			"SetViaSentByParamByLocalAddress - trx=0x%p: Failed to get info for local address 0x%p (rv=%d)",
			pTrx, hLocalAddr, rv));
		return rv;
	}
	/*if there is no sent by host, use the local IP*/
	if(sentByRpoolPtr.hPool != NULL)
	{
		strIp = NULL;  /*the rpool info will be used*/
	}
	else if (NULL != pSecureServerPort)
    {
        sentByPort = *pSecureServerPort;
    }
    else
	{
		sentByPort = localPort; /*the ip info will be used*/
	}
	rv = SipViaHeaderSetHost(hVia,strIp,sentByRpoolPtr.hPool,
		sentByRpoolPtr.hPage,sentByRpoolPtr.offset);
	if (RV_OK != rv)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
			"SetViaSentByParamByLocalAddress - trx=0x%p: Failed to set host in via header (rv=%d)",
			pTrx, rv));
		return rv;
	}
	RvSipViaHeaderSetPortNum(hVia, sentByPort);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTrx);
#endif

	return RV_OK;
}

/*-----------------LOCKING FUNCTIONS------------------------*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/***************************************************************************
 * TransmitterLock
 * ------------------------------------------------------------------------
 * General: Lock the transmitter
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  pTrx           - The transmitter to lock.
 *            pTrxMgr        - pointer to the transmitter manager
 *            pTripleLock     - triple lock of the transmitter
 ***************************************************************************/
static RvStatus RVCALLCONV TransmitterLock(
                         IN Transmitter          *pTrx,
                         IN TransmitterMgr       *pTrxMgr,
                         IN SipTripleLock        *pTripleLock,
                         IN RvInt32               identifier)
{

    if ((pTrx == NULL) || (pTrxMgr == NULL) || (pTripleLock == NULL))
    {
        return RV_ERROR_NULLPTR;
    }
    RvMutexLock(&pTripleLock->hLock,pTrxMgr->pLogMgr);
    if (pTrx->trxUniqueIdentifier == identifier &&
        pTrx->trxUniqueIdentifier != 0)
    {
        /*The transmitter is valid*/
        return RV_OK;
    }
    else
    {
        RvMutexUnlock(&pTripleLock->hLock,pTrxMgr->pLogMgr);
        RvLogWarning(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
            "TransmitterLock - Transmitter 0x%p: RLIST_IsElementVacant returned TRUE!!!",
            pTrx));
    }
    return RV_ERROR_DESTRUCTED;
}


/***************************************************************************
 * TransmitterUnLock
 * ------------------------------------------------------------------------
 * General: Unlocks the transmitter's lock.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hLock - The lock to un-lock.
 ***************************************************************************/
static void RVCALLCONV TransmitterUnLock(
                                IN RvMutex *pLock,
                                IN RvLogMgr* logMgr)
{
    if (NULL != pLock)
    {
        RvMutexUnlock(pLock,logMgr);
    }
}

#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

/***************************************************************************
 * TerminateTransmitterEventHandler
 * ------------------------------------------------------------------------
 * General: Terminates a Transmitter object, Free the resources taken by it and
 *          remove it from the manager list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   obj       - pointer to Transmitter to be deleted.
 *          reason    - reason of the termination
 ***************************************************************************/
static RvStatus RVCALLCONV TerminateTransmitterEventHandler(
                                         IN void*      obj,
                                         IN RvInt32  reason)
{
    Transmitter*     pTrx = (Transmitter *)obj;
    RvStatus         rv;
    RvLogSource*     logSrc;
    RvMutex*         pObjLock;
    RvMutex*         pProcessingLock;

    if (pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransmitterLockEvent(pTrx);
    if (rv != RV_OK)
    {
        return rv;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TerminateTransmitterEventHandler - pTrx 0x%p - event is out of the termination queue", pTrx));

    logSrc          = pTrx->pTrxMgr->pLogSrc;
    pProcessingLock = &pTrx->pTripleLock->hProcessLock;
    pObjLock        = &pTrx->pTripleLock->hLock;

    /*The transmitter always terminates due to a user command*/
    TransmitterChangeState(pTrx,
                          RVSIP_TRANSMITTER_STATE_TERMINATED,
                          RVSIP_TRANSMITTER_REASON_USER_COMMAND,
                          NULL,
                          NULL);
    TransmitterDestruct(pTrx);
    TransmitterTerminateUnlock(pTrx->pTrxMgr->pLogMgr,logSrc,pObjLock,pProcessingLock);
    RV_UNUSED_ARG(reason);
    return rv;
}

/***************************************************************************
 * ChooseLocalAddressForViaFromLocalAddresses
 * ------------------------------------------------------------------------
 * General: Choose the address to set in the via header from the 6 local addresses
 *          that are kept in the transmitter.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:         pTrx - The transmitter handle.
 *                eTransportType - The function should prefered addresses
 *                                 with the given transport but it is not
 *                                 must.
 * Output:        phLocalAddr - handle of the chosen local address.
 ***************************************************************************/
static RvStatus RVCALLCONV ChooseLocalAddressForViaFromLocalAddresses(
                       IN    Transmitter*                   pTrx,
                       IN    RvSipTransport                 eTransportType,
                       OUT   RvSipTransportLocalAddrHandle* phLocalAddr)

{
    RvSipTransportLocalAddrHandle hLocalAddr;

    hLocalAddr = NULL;
    *phLocalAddr = NULL;

    /*if we have an address in use - we use it to set the via header
      other wise we guess a local address according to transport*/

    if(pTrx->localAddr.hAddrInUse != NULL)
    {
        hLocalAddr = *(pTrx->localAddr.hAddrInUse);
    }

    /* 1. If transport is specified, choose that transport.
       2. If transport is undefined, it can resolve to any of the transports.
          So if there was no UDP address, we keep on trying */
    if(hLocalAddr == NULL) /*local address was not set yet*/
    {
        if(eTransportType == RVSIP_TRANSPORT_UDP ||
           eTransportType == RVSIP_TRANSPORT_UNDEFINED)
        {
            hLocalAddr =  pTrx->localAddr.hLocalUdpIpv4;
#if(RV_NET_TYPE & RV_NET_IPV6)
            if(hLocalAddr == NULL)
            {
                hLocalAddr =  pTrx->localAddr.hLocalUdpIpv6;
            }
#endif /*RV_NET_IPV6*/
        }
        if((eTransportType == RVSIP_TRANSPORT_TCP)                          ||
           (eTransportType == RVSIP_TRANSPORT_UNDEFINED && hLocalAddr == NULL))
        {
            hLocalAddr =  pTrx->localAddr.hLocalTcpIpv4;
#if(RV_NET_TYPE & RV_NET_IPV6)
            if(hLocalAddr == NULL)
            {
                hLocalAddr =  pTrx->localAddr.hLocalTcpIpv6;
            }
#endif /*RV_NET_IPV6*/
        }
#if (RV_TLS_TYPE != RV_TLS_NONE)
        if ( (eTransportType == RVSIP_TRANSPORT_TLS)                           ||
            (eTransportType == RVSIP_TRANSPORT_UNDEFINED && hLocalAddr == NULL))
        {
            hLocalAddr =  pTrx->localAddr.hLocalTlsIpv4;
#if(RV_NET_TYPE & RV_NET_IPV6)
            if(hLocalAddr == NULL)
            {
                hLocalAddr =  pTrx->localAddr.hLocalTlsIpv6;
            }
#endif /*RV_NET_IPV6*/
        }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
/* SPIRENT_BEGIN */
/*
* COMMENTS: possible bugfix for situation when we have TLS transport in Req URI but have Route
*           header with UDP transport. No Local TLS address configured. 
*/
#if defined(UPDATED_BY_SPIRENT)
    if(hLocalAddr == NULL)
    {
        hLocalAddr =  pTrx->localAddr.hLocalUdpIpv4;
#if(RV_NET_TYPE & RV_NET_IPV6)
        if(hLocalAddr == NULL)
        {
            hLocalAddr =  pTrx->localAddr.hLocalUdpIpv6;
        }
#endif /*RV_NET_IPV6*/
    }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
#if (RV_NET_TYPE & RV_NET_SCTP)
        if ( (eTransportType == RVSIP_TRANSPORT_SCTP)                          ||
             (eTransportType == RVSIP_TRANSPORT_UNDEFINED && hLocalAddr == NULL))
        {
            hLocalAddr =  pTrx->localAddr.hLocalSctpIpv4;
#if(RV_NET_TYPE & RV_NET_IPV6)
            if(hLocalAddr == NULL)
            {
                hLocalAddr =  pTrx->localAddr.hLocalSctpIpv6;
            }
#endif /*RV_NET_IPV6*/
        }
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    }

    /* there are two ways for this "if" to be true:
       1. The transport was specified, but the stack does not hold that transport.
       2. the transport was some how not one of the RVSIP_TRANSPORT_XXX enums.
       In both cases its an error.
    */
    if(hLocalAddr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    *phLocalAddr = hLocalAddr;
    return RV_OK;
}

/***************************************************************************
 * ConvertRvToTrxReason
 * ------------------------------------------------------------------------
 * General: Converts a return value to a transmitter reason.
 * Return Value: The transmitter reason.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:       rv  - The return value to convert.
 * Output:  (-)
 ***************************************************************************/
static RvSipTransmitterReason ConvertRvToTrxReason(
                              IN RvStatus        rv)
{

    switch (rv)
    {
    case RV_ERROR_NETWORK_PROBLEM:
        return RVSIP_TRANSMITTER_REASON_NETWORK_ERROR;
    case RV_ERROR_OUTOFRESOURCES:
        return RVSIP_TRANSMITTER_REASON_OUT_OF_RESOURCES;
    default:
        return RVSIP_TRANSMITTER_REASON_UNDEFINED;
    }

}

/******************************************************************************
 * AddScopeToDestIpv6AddrIfNeed
 * ----------------------------------------------------------------------------
 * General: Destination IPv6 address of local-link level should contain
 *          Scope ID, which tells to the Operation System, through which
 *          interface the SIP message for UDP, or SIN packet for TCP should be
 *          sent. If it the Transmitter's destination address has no Scope Id,
 *          this function sets it Scope ID to the value, set for Local Address,
 *          used by the Transmitter.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx - The pointer to the Transmitter object.
 * Output:  (-)
 *****************************************************************************/
static void AddScopeToDestIpv6AddrIfNeed(IN Transmitter* pTrx)
{
    SipTransportFixIpv6DestinationAddr(*pTrx->localAddr.hAddrInUse,
                                       &pTrx->destAddr.addr);
    return;
}

#ifdef RV_SIP_OTHER_URI_SUPPORT
/******************************************************************************
 * FixMsgSendingAddrIfNeed
 * ----------------------------------------------------------------------------
 * General: When sending to a 'pres: or 'im:' address, the stack has to change
 *          the address to 'sip:' after resolution.
 *          In this function, we check which address of the message was used for
 *          sending. if it is request-uri or first-hop, we convert this address
 *          to sip. if it is something else (such as outbound proxy) we do not
 *          need to convert anything...
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx - The pointer to the Transmitter object.
 * Output:  (-)
 *****************************************************************************/
static void FixMsgSendingAddrIfNeed(IN Transmitter* pTrx)
{
    RvSipAddressHandle hSendingAddr = NULL;
    RvSipAddressType   eAddrType;

    if(RVSIP_MSG_REQUEST != RvSipMsgGetMsgType(pTrx->hMsgToSend))
	{
		return;
	}
    if(pTrx->eMsgAddrUsedForSending == TRANSMITTER_MSG_FIRST_ROUTE)
    {
        RvSipRouteHopHeaderHandle    hRouteHop = NULL;
        RvSipRouteHopHeaderType      routeHopType = RVSIP_ROUTE_HOP_UNDEFINED_HEADER;
        RvSipHeaderListElemHandle    listElem  = NULL;

        /* find the first route header */
        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByType(
                                 pTrx->hMsgToSend, RVSIP_HEADERTYPE_ROUTE_HOP, RVSIP_FIRST_HEADER, &listElem);
        while(hRouteHop != NULL)
        {
            routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);

            if(routeHopType == RVSIP_ROUTE_HOP_ROUTE_HEADER)
            {
                 break; /* found the first route */
            }
            /*get the next route hop header*/
            hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByType(
                                pTrx->hMsgToSend, RVSIP_HEADERTYPE_ROUTE_HOP, RVSIP_NEXT_HEADER, &listElem);
        }

        if(routeHopType != RVSIP_ROUTE_HOP_ROUTE_HEADER)
        {

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
           //let's not display the following exception for ACK requests on 3xx-6xx because it's not an error ... it's just on the construction of any
           //'ACK' request message in responding to a 3xx-6xx response-code, Radvision doesn't
           // add any route header to such message (please see the function 'AddRouteListToRequestMsgIfNeed' for more detail)
           //
           if(RvSipMsgGetRequestMethod(pTrx->hMsgToSend) == RVSIP_METHOD_ACK)
           {
               return;  // don't display error in this case.
           }
           // otherwise, display the exception error.
#endif
/* SPIRENT_END */

            /* we did not find the first route */
            RvLogExcep(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "FixMsgSendingAddrIfNeed: Failed to find the first route header"));
            return;
        }

        /* Take the address from the first route */
        hSendingAddr = RvSipRouteHopHeaderGetAddrSpec(hRouteHop);
        if(hSendingAddr == NULL)
        {
            /* we did not find the first route */
            RvLogExcep(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "FixMsgSendingAddrIfNeed: Failed to find the address of the first route header"));
            return;
        }
    }
    else if (pTrx->eMsgAddrUsedForSending == TRANSMITTER_MSG_REQ_URI)
    {
        hSendingAddr = RvSipMsgGetRequestUri(pTrx->hMsgToSend);
        if(hSendingAddr == NULL)
        {
            /* we did not find the first route */
            RvLogExcep(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "FixMsgSendingAddrIfNeed: Failed to find the request-uri"));
            return;
        }
    }

    /* fix the sending address */
    if(hSendingAddr != NULL)
    {
        eAddrType = RvSipAddrGetAddrType(hSendingAddr);
        if(eAddrType == RVSIP_ADDRTYPE_PRES || eAddrType == RVSIP_ADDRTYPE_IM)
        {
            SipAddrConvertOtherAddrToUrl(hSendingAddr);
        }
    }
}
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

#ifdef RV_SIGCOMP_ON
/***************************************************************************
* LookForSigCompUrnInSentMsg
* ------------------------------------------------------------------------
* General: Searches for SigComp identifier URN (Uniform Resource Name)
*          that uniquely identifies the application. SIP/SigComp identifiers
*          are carried in the 'sigcomp-id' SIP URI (Uniform Resource
*          Identifier) in outgoing request messages (and incoming responses)
*          or within Via header field parameter within outgoing response
*          messages.
*
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTrx         - A pointer to the Transmitter object.
*          hMsg         - Handle to the received request message.
* Output:  pbUrnFound   - Indication if the URN was found.
*          pUrnRpoolPtr - Pointer to the page, pool and offset which
*                         contains a copy of the
*                         found URN  (might be NULL in case of missing URN).
***************************************************************************/
static RvStatus LookForSigCompUrnInSentMsg(IN   Transmitter      *pTrx,
                                           IN   RvSipMsgHandle    hMsgToSend,
                                           OUT  RvBool           *pbUrnFound,
                                           OUT  RPOOL_Ptr        *pUrnRpoolPtr)
{
    RvStatus        rv;
    RvSipMsgType    eMsgType = RvSipMsgGetMsgType(hMsgToSend);

    *pbUrnFound = RV_FALSE;

    RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
        "LookForSigCompUrnInSentMsg - Trx 0x%p is looking for URN in hMsg 0x%x (type=%d)",
        pTrx, hMsgToSend, eMsgType));

    switch (eMsgType)
    {
    case RVSIP_MSG_REQUEST:
		if (pTrx->sigcompIdOffset > UNDEFINED)
		{
			pUrnRpoolPtr->hPool  = pTrx->hPool;
			pUrnRpoolPtr->hPage  = pTrx->hPage;
			pUrnRpoolPtr->offset = pTrx->sigcompIdOffset;
			*pbUrnFound = RV_TRUE;
		}
		rv = RV_OK; /* even if sigcomp-id was not found */
        break;
    case RVSIP_MSG_RESPONSE:
        /* Retrieve the URN from the Transaction URI */
        rv = SipCommonLookForSigCompUrnInTopViaHeader(
                hMsgToSend,pTrx->pTrxMgr->pLogSrc,pbUrnFound,pUrnRpoolPtr);
        break;
    default:
        rv = RV_ERROR_UNKNOWN;
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "LookForSigCompUrnInRcvdMsg - Trx 0x%p is sending undefined type of hMsg 0x%x",
            pTrx, hMsgToSend));
    }
    if (rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "LookForSigCompUrnInRcvdMsg - Trx 0x%p couldn't retrieve URN from hMsg 0x%x",
            pTrx, hMsgToSend));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
* convertTransportTypeToSigcompTransport
* ------------------------------------------------------------------------
* General: convert transport type into bitmask.
*          One bit represents the transport reliability, and the 2nd bit
*          represents its type.
* Return Value: 
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTrx           - A pointer to the Transmitter object.
* Output:  pTransportType - The bit mask
**************************************************************************/
static void	convertTransportTypeToSigcompTransport(IN Transmitter *pTrx, OUT RvInt *pTransportType)
{
	*pTransportType = 0;
	switch (pTrx->destAddr.eTransport)
	{
	case RVSIP_TRANSPORT_TCP:            
	case RVSIP_TRANSPORT_TLS:
		*pTransportType |= RVSIGCOMP_STREAM_TRANSPORT;
		*pTransportType |= RVSIGCOMP_UNRELIABLE_TRANSPORT;/*RVSIGCOMP_RELIABLE_TRANSPORT;*/
		break;

	case RVSIP_TRANSPORT_SCTP:
		*pTransportType |= RVSIGCOMP_MESSAGE_TRANSPORT;
		*pTransportType |= RVSIGCOMP_UNRELIABLE_TRANSPORT; /*RVSIGCOMP_RELIABLE_TRANSPORT;*/
		break;

	default:
		*pTransportType |= RVSIGCOMP_MESSAGE_TRANSPORT;
		*pTransportType |= RVSIGCOMP_UNRELIABLE_TRANSPORT;
		break;
	}
}

/***************************************************************************
* populateTransportAddessFromLocalAddr
* ------------------------------------------------------------------------
* General: fill SipTransportAddress type from current transmitter local address
*
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTrx         - A pointer to the Transmitter object.
* Output:  address      - The filled address
***************************************************************************/
static void populateTransportAddessFromLocalAddr (IN  Transmitter         *pTrx, 
												  OUT SipTransportAddress *address)
{
    RvChar                    strLocalAddress[RVSIP_TRANSPORT_LEN_STRING_IP] = {'\0'};
#if defined(UPDATED_BY_SPIRENT)
    RvChar                    if_name[RVSIP_TRANSPORT_IFNAME_SIZE] = {'\0'};
    RvUint16                  vdevblock;
#endif
    RvSipTransport            eTransportType;
    RvSipTransportAddressType eAddressType;
    RvUint16                  localPort;
	RvInt                     addrType;

	RvSipTransmitterGetCurrentLocalAddress((RvSipTransmitterHandle)pTrx, &eTransportType, &eAddressType, 
					       strLocalAddress, &localPort
#if defined(UPDATED_BY_SPIRENT)
					       ,if_name
					       ,&vdevblock
#endif

					       );
	address->eTransport = eTransportType;
	switch (eAddressType)
	{
	case RVSIP_TRANSPORT_ADDRESS_TYPE_IP:
		addrType = RV_ADDRESS_TYPE_IPV4;
		break;
	case RVSIP_TRANSPORT_ADDRESS_TYPE_IP6:
			addrType = RV_ADDRESS_TYPE_IPV6;
			break;
	default:
		addrType = RV_ADDRESS_TYPE_NONE;
		break;
	}

	RvAddressConstruct(addrType, &address->addr);
	RvAddressSetString(strLocalAddress, &address->addr);
	RvAddressSetIpPort(&address->addr, localPort);
}

#endif /* #ifdef RV_SIGCOMP_ON */

