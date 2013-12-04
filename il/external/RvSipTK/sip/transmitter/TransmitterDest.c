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
 *                              <TransmitterDest.c>
 *
 *  The TransmitterDest.c file contains relevant for Address resolution
 *  
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                 January 2004
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvaddress.h"
#include "RvSipTransportTypes.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipViaHeader.h"
#include "TransmitterObject.h"
#include "RvSipAddress.h"
#include "_SipMsg.h"
#include "_SipTransport.h"
#include "_SipTransmitter.h"
#include "TransmitterCallbacks.h"
#include "_SipResolver.h"
#include "TransmitterDest.h"
#include "TransmitterControl.h"
#include "RvSipTransportDNS.h"
#include "_SipAddress.h"
#include "_SipCommonConversions.h"
#include "_SipCommonUtils.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#ifdef RV_SIP_IMS_ON
#include "RvSipSecurity.h"
#endif
#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV GetAddressDetails(
             IN  RvSipAddressHandle hAddress,
             OUT RvSipTransport*    pTransportType,
             OUT RvBool*            pbIsSecure);

static RvStatus RVCALLCONV CheckForOutboundAddress(
               IN  Transmitter*       pTrx,
               OUT RvChar**           pHostName,
               OUT RvInt32*           pHostNameOffset,
               OUT RvInt32*           pPort,
               OUT RvSipTransport*    peTransportType,
               OUT RvSipCompType*     peCompType,
			   OUT RPOOL_Ptr*         pSigcompId,
               OUT RvBool*            pbAddrFound);

static RvStatus RVCALLCONV ConstructSipUrlAddrByTrx(
              IN  Transmitter*        pTrx,
              IN  RvSipMsgHandle      hMsg,
              IN  RvSipAddressHandle  hAddress,
              OUT RvSipAddressHandle* phSipAddr,
              OUT RvBool*             pbAddressResolved);

static RvStatus RVCALLCONV GetViaParameter(
            IN  Transmitter*               pTrx,
            IN  RvSipViaHeaderHandle       hViaHeader,
            IN  RvSipViaHeaderStringName   eParameterName,
            OUT RvChar**                   pStrHostName,
            OUT RvInt32*                   pHostNameOffset);

static RvStatus RVCALLCONV DestGetResponseAddressFromMsg(
                           IN  Transmitter*       pTrx,
                           IN  RvSipMsgHandle     hMsgObject,
                           OUT RvChar**           pHostName,
                           OUT RvInt32*           pHostNameOffset,
                           OUT RvInt32*           pPort,
                           OUT RvSipTransport*    pTransportType);

static RvStatus RVCALLCONV DestGetRequestAddressFromMsg(
                                       IN  Transmitter*        pTrx,
                                       IN  RvSipMsgHandle      hMsgObject,
                                       IN  RvBool              bSendToFirstRouteHeader,
                                       OUT RvChar**            pHostName,
                                       OUT RvInt32*            pHostNameOffset,
                                       OUT RvInt32*            pPort,
                                       OUT RvSipTransport*     pTransportType,
                                       OUT RvBool*             pbIsSecure);

#ifdef RV_SIGCOMP_ON
static void RVCALLCONV UpdateTrxOutboundMsgCompType(
                           IN Transmitter   *pTrx,
                           IN RvSipCompType  eCompType);

#endif /* RV_SIGCOMP_ON */

static RvStatus RVCALLCONV DestIdentifyNextHopParams(IN  Transmitter*         pTrx);

static void RVCALLCONV TransmitterDestReset (IN Transmitter*     pTrx);

static RvStatus DestProtocolDecideByMsg(
    IN Transmitter* pTrx );

static RvStatus DestProtocolFindBySrv(
    IN Transmitter* pTrx );

static RvStatus DestTryPreDnsList(
    IN Transmitter* pTrx);

static RvStatus HandleUriFoundState( IN Transmitter* pTrx );

static RvStatus DestProtocolInitialFind(
    IN Transmitter* pTrx );

static RvStatus DestHostPortDiscovery(
    IN Transmitter* pTrx);

static RvSipResolverScheme DestCreateScheme(RvSipAddressHandle hAddress);

static RvStatus DestPortIPDiscoveryByRawIP(
    IN  Transmitter* pTrx);

static RvUint16 RVCALLCONV DestChoosePort(
    IN  Transmitter* pTrx);

static void RVCALLCONV DestResolutionStateChange(
    IN Transmitter* pTrx,
    IN TransmitterResolutionState    eNewState);

static RvStatus RVCALLCONV DestDataReportedEvHandler(
                              IN RvSipResolverHandle       hResolver,
                              IN RvSipAppResolverHandle    hOwner,
                              IN RvBool                  bError,
                              IN RvSipResolverMode         eMode);

static RvStatus RVCALLCONV DestResolveIpByHost(
								   IN  Transmitter *pTrx,
								   IN  RvChar      *strHost,
								   IN  RvUint16	   port);

static RvStatus DestStateResolved(
    IN Transmitter* pTrx);

#ifdef RV_SIP_TEL_URI_SUPPORT
static RvStatus DestProtocolHandleEnumResult(IN  Transmitter        *pTrx,
											 IN  RvChar			    *pStrRegExp,
											 OUT RvSipAddressHandle *phUri); 

static RvStatus DestProtocolHandleUriResult(IN Transmitter* pTrx);
#else 
#define DestProtocolHandleUriResult(_t)  RV_OK
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */

static RvStatus RVCALLCONV DestHandleResolutionStateUndefined(IN  Transmitter*         pTrx);

#ifdef RV_SIP_TEL_URI_SUPPORT
static RvStatus RVCALLCONV DestResolveNaptrRegexp(
             IN  Transmitter* pTrx,
             IN  RvChar*   strRepExp,
             IN  RvChar*   strAus,
             IN  RvSipTransmitterRegExpMatch* pMatches,
             OUT RvChar*   strResolved);

static RvStatus RVCALLCONV DestGetRegexpGetCallbackStrings(
             IN  Transmitter* pTrx,
             IN  RvChar*   strFullString,
             OUT RvChar*   strEre,
             OUT RvChar*   strReplecment,
             OUT RvSipTransmitterRegExpFlag* peFlags);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */ 

static RvStatus RVCALLCONV TryToUseOutboundAddress(
               IN  Transmitter*       pTrx,
               OUT RvChar**           pHostName,
               OUT RvInt32*           pHostNameOffset,
               OUT RvInt32*           pPort,
               OUT RvSipTransport*    peTransportType,
               OUT RvSipCompType*     peCompType,
               OUT RvBool*            pbIsSecure,
               OUT RvBool*            pbAddrFound);

static RvStatus SearchConnectionByAlias(IN Transmitter* pTrx,
                                        OUT RvSipTransportConnectionHandle *phConn);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"
#include "RvSipSecAgree.h"

#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER DEST FUNCTIONS                         */
/*-----------------------------------------------------------------------*/
/*****************************************************************************
 * TransmitterDestDiscovery
 * ---------------------------------------------------------------------------
 * General: Identify the destination address.
 *          This function is a focal point for all nex hop decisions.
 *          it forks to :
 *          - discover the next hop host
 *          - convert TEL URLs to SIP URLS
 *          - find the next hop protocol (transport)
 *          - find the next hop ip and port
 *          - proceed with sending after resolution is complete
 *          - report on a msg sent failre if DNS resolution is not completed
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_BADPARAM
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx          - pointer to transmitter
 *          bReachedFromApp - indicates whether this function was called
 *                            from the application or from a callback.
 * Output:  -
 *****************************************************************************/
RvStatus RVCALLCONV TransmitterDestDiscovery(
                IN  Transmitter*          pTrx,
                IN  RvBool                bReachedFromApp)
{
    RvStatus rv = RV_OK;

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "TransmitterDestDiscovery - pTrx=0x%p: bReachedFromApp=%d",pTrx,bReachedFromApp));

    /* If Security is supported, transport and destination address,
    protected by the Security Object should be used. It means there is no need
    to resolve destination. */
#ifdef RV_SIP_IMS_ON
    if (NULL != pTrx->hSecObj)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc  ,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterDestDiscovery - pTrx=0x%p: no need in destination resolution - use Security Object %p",
            pTrx, pTrx->hSecObj));
        rv = TransmitterControlSend(pTrx,bReachedFromApp);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterDestDiscovery - pTrx=0x%p: failed to send secure message(rv=%d:%s)",
                pTrx, rv, SipCommonStatus2Str(rv)));
        }
        return rv;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    if (TRANSMITTER_RESOLUTION_STATE_UNDEFINED == pTrx->eResolutionState)
    {
        TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR,
            RVSIP_TRANSMITTER_REASON_UNDEFINED,pTrx->hMsgToSend,NULL);
        if (RVSIP_TRANSMITTER_STATE_TERMINATED == pTrx->eState)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterDestDiscovery - pTrx=0x%p: terminated returning RV_OK",pTrx));
            return RV_OK;
        }
    }
    rv = DestTryPreDnsList(pTrx);
    if (RV_ERROR_NOT_FOUND == rv)
    {
        /* if we did not discover anything before, go to try and resolve by state*/
        rv = RV_OK;
    }
    else if (RV_ERROR_TRY_AGAIN == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterDestDiscovery - pTrx=0x%p: waiting for DNS response from prediscovered",pTrx));
        return RV_OK;
    }

    while (RV_OK == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterDestDiscovery - pTrx=0x%p: resolution state=%s",
            pTrx,TransmitterGetResolutionStateName(pTrx->eResolutionState)));

        switch (pTrx->eResolutionState)
        {
        case TRANSMITTER_RESOLUTION_STATE_UNDEFINED:
            rv = DestHandleResolutionStateUndefined(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI:
            rv = DestProtocolHandleUriResult(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_3WAY_SRV: 
            rv = DestProtocolDecideByMsg(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR: 
            rv = DestProtocolFindBySrv(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_RESOLVING_IP: 
            DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_UNRESOLVED);
            break;
        case TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT: 
            rv = DestResolveIpByHost(pTrx,pTrx->msgParams.strHostName,DestChoosePort(pTrx));
            break;
        case TRANSMITTER_RESOLUTION_STATE_URI_FOUND: 
            rv = HandleUriFoundState(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND:
            rv = DestHostPortDiscovery(pTrx);
            break;
        case TRANSMITTER_RESOLUTION_STATE_RESOLVED:
            if (RVSIP_TRANSMITTER_STATE_ON_HOLD != pTrx->eState)
            {
                rv = DestStateResolved(pTrx);
                if (RV_ERROR_NOT_FOUND == rv)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: state %s rv-not found. continue",
                        pTrx, TransmitterGetStateName(pTrx->eState)));
                    /* Bug Fix, in order to enable continuation the rv should be set to RV_OK */ 
                    rv = RV_OK;
                    continue;
                }
                /* If we were not asked to hold sending, we can go ahead and send the message */
                if (RVSIP_TRANSMITTER_STATE_TERMINATED == pTrx->eState)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: Transmitter was terminated, returning",pTrx));
                    return RV_OK;
                }
                else if (RVSIP_TRANSMITTER_STATE_ON_HOLD == pTrx->eState)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: Transmitter was set on hold, returning",pTrx));
                    return RV_OK;
                }
                /* Bug Fix, the application has decided to hold and resume the sending process. */
                /* Thus, instead of being in state Final Dest Resolved the Transmitter has turned to    */
                /* state Resolving Addr. Consequently, this loop is irrelevant. */ 
                else if (RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR == pTrx->eState)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: Transmitter has returned to Resolving Addr, returning",pTrx));
                    return RV_OK;
                }
                else if (RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE == pTrx->eState)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: Transmitter in msgsend failure, returning",pTrx));
                    return RV_OK;
                }
                else if(pTrx->eState == RVSIP_TRANSMITTER_STATE_MSG_SENT)
                {
                    /* If the application holds and resumes from the DEST_RESOLVED
                    callback, called above from the DestStateResolved(),
                    the message will be sent before this point. So nothing should
                    be done further. Just return. The Transmitter will be
                    terminated on the Transaction termination. */
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                        "TransmitterDestDiscovery - Transmitter 0x%p, message was sent already. Return",
                        pTrx));
                    return RV_OK;
                }
                else if (TRANSMITTER_RESOLUTION_STATE_UNDEFINED == pTrx->eResolutionState)
                {
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "TransmitterDestDiscovery - pTrx=0x%p: address was reset, trying resolution again",pTrx));
                }
                else
                {
                    rv = TransmitterControlSend(pTrx,bReachedFromApp);
                    return rv;
                }
            }
            else
            {
                rv = TransmitterControlSend(pTrx,bReachedFromApp);
                return rv;
            }
            break;
            
        case TRANSMITTER_RESOLUTION_STATE_UNRESOLVED:
            return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,RV_ERROR_NETWORK_PROBLEM,
                                                   RVSIP_TRANSMITTER_REASON_UNDEFINED,
                                                   bReachedFromApp);
        default:
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "TransmitterDestDiscovery - pTrx=0x%p: unexpected resolution state %d",pTrx,pTrx->eResolutionState));
            return RV_ERROR_UNKNOWN;
        }
        
		switch (rv)
		{
		case RV_ERROR_TRY_AGAIN:
			RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"TransmitterDestDiscovery - pTrx=0x%p: waiting for DNS response",pTrx));
			return RV_OK;
			/* The 'break' is meaningless in this case */ 
		case RV_DNS_ERROR_NOTFOUND:
			/* Promote to the next query due to the inline failure (next iteration in the while loop) */ 
			rv = DestTryPreDnsList(pTrx);
			RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				"TransmitterDestDiscovery - pTrx=0x%p: Tired Pre-DNS list (rv=%d)",pTrx,rv));
			if (RV_ERROR_NOT_FOUND == rv)
			{
				/* if we did not discover anything before, go to try and resolve by state*/
				rv = RV_OK;
			}
			else if (RV_ERROR_TRY_AGAIN == rv)
			{
				RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
					"TransmitterDestDiscovery - pTrx=0x%p: during promotion - waiting for DNS response from prediscovered",pTrx));
				return RV_OK;
			} 
			break; 
		case RV_OK: 
			break; /* Nothing special should be done. Moving on to the next iteration */ 
		default:
			return TransmitterMoveToMsgSendFaiureIfNeeded(pTrx,rv,
				RVSIP_TRANSMITTER_REASON_UNDEFINED,
				bReachedFromApp);
		}
    } /* while */
    return rv;
}

/***************************************************************************
 * TransmitterDestAllocateHostBuffer
 * ------------------------------------------------------------------------
 * General: allocates the buffer for the host member in the msg params.
 *          we need this parameter in a consecutive buffer for resolver operations
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTrx           - transmitter
 *        length         - size to allocate
 * Output:pStrHostName   - allocated consecutive buffer
 *        pHostNameOffset - The offset of the buffer on the transmitter page.
 *                          (may be NULL)
 ***************************************************************************/
RvStatus RVCALLCONV TransmitterDestAllocateHostBuffer(
            IN  Transmitter*    pTrx,
            IN  RvUint          length,
            OUT RvChar**        pStrHostName,
            OUT RvInt32*        pHostNameOffset)
{
    RvStatus       rv       = RV_OK;
    RvInt32        offset   = 0;

    rv = RPOOL_Append(pTrx->hPool,pTrx->hPage, length, RV_TRUE, &offset);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TransmitterDestAllocateHostBuffer - trx=0x%p: Faild to allocate buffer from pool",
            pTrx));   
        return rv;
    }
    *pStrHostName = RPOOL_GetPtr(pTrx->hPool,pTrx->hPage, offset);
    if(pHostNameOffset != NULL)
    {
        *pHostNameOffset = offset;
    }
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                            STATIC FUNCTIONS                           */
/*-----------------------------------------------------------------------*/

/*****************************************************************************
 * DestHandleResolutionStateUndefined
 * ---------------------------------------------------------------------------
 * General: see if we already have a host name (from previous lops of the 
 *          DNS-continue operations).
 *          if no, we take those param from the message
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_BADPARAM
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx          - the transmitter to identify address of
 * Output:  -
 *****************************************************************************/
static RvStatus RVCALLCONV DestHandleResolutionStateUndefined(IN  Transmitter*         pTrx)
{
    RvStatus rv = RV_OK;
    
    TransmitterDestReset(pTrx);
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLUTION_STARTED);
/*  bug fix - the msgParams are not used in initial resolution*/
/*    if (NULL != pTrx->msgParams.strHostName)
    {
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_URI_FOUND);
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestHandleResolutionStateUndefined - pTrx=0x%p: Uri already exist (uri offset=%d)",
            pTrx,pTrx->msgParams.hostNameOffset));
        return rv;

    }
*/
    rv = DestIdentifyNextHopParams(pTrx);

    if (RV_OK != rv && RV_ERROR_TRY_AGAIN != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestHandleResolutionStateUndefined - pTrx=0x%p: error in discovery process rv=%d",pTrx,rv));
        return rv;
    }

    /* before starting the address resolution, give application the opportunity
    to resolve it by itself */
    if(pTrx->pTrxMgr->mgrEvents.pfnMgrResolveAddressEvHandler != NULL)
    {
        RvSipTransportAddr addrToResolve;
        RvChar             hostnameToResolve[RVSIP_MAX_HOST_NAME_LEN+1];
        RvBool             bAddrResolved = RV_FALSE;
        RvBool             bContinueResolution = RV_TRUE;
        
		if (pTrx->msgParams.strHostName != NULL)
		{
			strcpy((RvChar*)hostnameToResolve, pTrx->msgParams.strHostName);
		}
		/*If the port is undefined we are picking the default port according to the transport type*/
		if (pTrx->msgParams.port == UNDEFINED)
		{
			addrToResolve.port = (RvUint16)((pTrx->msgParams.bIsSecure == RV_TRUE || pTrx->msgParams.transportType == RVSIP_TRANSPORT_TLS) ? RVSIP_TRANSPORT_DEFAULT_TLS_PORT : RVSIP_TRANSPORT_DEFAULT_PORT);
		}
		else
		{
			/*Else we need to pick the predefined port*/
			addrToResolve.port = (RvUint16)pTrx->msgParams.port;
		}
        memset(addrToResolve.strIP, 0, RVSIP_TRANSPORT_LEN_STRING_IP);
        addrToResolve.eTransportType = pTrx->msgParams.transportType;

        /* non relevant parameters */
        addrToResolve.eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;
        addrToResolve.Ipv6Scope = UNDEFINED;
        
        TransmitterCallbackMgrResolveAddressEv(pTrx, hostnameToResolve, &addrToResolve, &bAddrResolved, &bContinueResolution);
        
        /* act according to the cb results */
        if(bContinueResolution == RV_FALSE && bAddrResolved == RV_FALSE)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestHandleResolutionStateUndefined - pTrx=0x%p: application ordered to stop resolution", 
                pTrx));
            return RV_ERROR_NOT_FOUND;
        }
        else
        {
            if(bAddrResolved == RV_TRUE) /* application made the resolution */
            {
                RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestHandleResolutionStateUndefined - pTrx=0x%p: application made the resolution.", 
                    pTrx));
                if(addrToResolve.strIP[0] == '\0')
                {
                    /* the ip string was not set. do not use the 'resolved' information */
                    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestHandleResolutionStateUndefined - pTrx=0x%p: application claimed to resolve address. but ip was not supplied!!!", 
                        pTrx));
                    return RV_ERROR_UNKNOWN;
                }
                /* set address params back to trx */
                pTrx->msgParams.port = addrToResolve.port;

                rv = TransmitterDestAllocateHostBuffer(pTrx, (RvUint)(strlen(addrToResolve.strIP) + 1), /* 1 - for the '\0' */
                                                       &pTrx->msgParams.strHostName, 
                                                       &pTrx->msgParams.hostNameOffset);
                if (RV_OK != rv)
                {
                    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestHandleResolutionStateUndefined - pTrx=0x%p: can not allocate space for URI: (rv=%d)",
                        pTrx,rv));
                    return RV_ERROR_BADPARAM;
                }

                strcpy(pTrx->msgParams.strHostName,addrToResolve.strIP);
                pTrx->msgParams.transportType = addrToResolve.eTransportType;      
            }
            else /* need to continue with the resolution process */
            {
                RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestHandleResolutionStateUndefined - pTrx=0x%p: application did not resolved. continue in resolution", 
                    pTrx));
                /* do nothing. continue as usual */
            }
        }
    }
    
    
    return rv;
}

/*****************************************************************************
 * DestIdentifyNextHopParams
 * ---------------------------------------------------------------------------
 * General: Identify the next hop address according to the message type:
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_BADPARAM
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx          - the transmitter to identify address of
 * Output:  -
 *****************************************************************************/
static RvStatus RVCALLCONV DestIdentifyNextHopParams(IN  Transmitter*         pTrx)
{
    RvStatus                        rv = RV_OK;

    pTrx->msgParams.msgType = RvSipMsgGetMsgType(pTrx->hMsgToSend);

    /* Determinate target host name, port & transport
       as specified in the message */
    switch (pTrx->msgParams.msgType)
    {
    case RVSIP_MSG_REQUEST:
        rv = DestGetRequestAddressFromMsg(pTrx,
                                      pTrx->hMsgToSend,
                                      pTrx->bSendToFirstRoute,
                                      &pTrx->msgParams.strHostName,
                                      &pTrx->msgParams.hostNameOffset,
                                      &pTrx->msgParams.port,
                                      &pTrx->msgParams.transportType,
                                      &pTrx->msgParams.bIsSecure);
        break;
    case RVSIP_MSG_RESPONSE: /* Server transaction */
        rv = DestGetResponseAddressFromMsg(pTrx,
                                       pTrx->hMsgToSend,
                                       &pTrx->msgParams.strHostName,
                                       &pTrx->msgParams.hostNameOffset,
                                       &pTrx->msgParams.port,
                                       &pTrx->msgParams.transportType);
        break;
    default:
        /* Wrong message type, error should be returned. */
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestIdentifyNextHopParams - pTrx=0x%p: wrong message type",pTrx));
        rv = RV_ERROR_UNKNOWN;
        break;
    }
    return rv;
}

/*****************************************************************************
 * DestGetRequestAddressFromMsg
 * ---------------------------------------------------------------------------
 * General: get next hop address from the message (from route, uri and proxy)
 *          if next hop is proxy - take proxy details
 *          if next hop is SIP URL - take URL details
 *          if next hop is TEL URL - resolve URL (or call other URL cb)
 *          if next hop is something else - call other URL cb
 *          
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx        - The sending transmitter.
 *          bSendToFirstRouteHeader     - Determines weather to send a request to to a loose router
 *                                (to the first in the route list) or to a strict router
 *                                (to the request URI).
 *          hMsgObject          - Handle of the message object
 * Output:  strHostPtr          - host name inside rpool.
 *          pHostName            -    The destination host name computed according
 *                                    to the Record-Route, outbound proxy and
 *                                    Request-Uri.
 *            port                -    The destination port computed according
 *                                    to the Record-Route, outbound proxy and
 *                                    Request-Uri.
 *            pTransportType        -    The transport type computed according
 *                                    to the Record-Route, outbound proxy and
 *                                    Request-Uri.
 *          pbIsSecure          -   The is secure flag computed according
 *                                    to the Record-Route, outbound proxy and
 *                                    Request-Uri.

 ****************************************************************************/
static RvStatus RVCALLCONV DestGetRequestAddressFromMsg(
                                       IN  Transmitter*        pTrx,
                                       IN  RvSipMsgHandle      hMsgObject,
                                       IN  RvBool              bSendToFirstRouteHeader,
                                       OUT RvChar**            pHostName,
                                       OUT RvInt32*            pHostNameOffset,
                                       OUT RvInt32*            pPort,
                                       OUT RvSipTransport*     pTransportType,
                                       OUT RvBool*             pbIsSecure)
{

    RvSipRouteHopHeaderHandle       hRouteHop           = NULL;
    RvUint                          length              = 0;
    RvUint                          actSize             = 0;
    RvStatus                        rv                  = RV_OK;
    RvSipHeaderListElemHandle       listElem            = NULL;
    RvChar*                         pStrHostName        = NULL;
    RvSipAddressHandle              hSipUrlAddress      = NULL;
    RvBool                          bAddressResolved    = RV_FALSE;
    TransmitterMgr*                 pMgr                = pTrx->pTrxMgr;
    RvBool                          bIsReqUri           = RV_FALSE;              
    RvSipCompType                   eCompType           = RVSIP_COMP_UNDEFINED;
    RvSipAddressType                eAddrType           = RVSIP_ADDRTYPE_UNDEFINED;

    pMgr                = pTrx->pTrxMgr;

    if(pTrx->bForceOutboundAddr == RV_TRUE)
    {
        RvBool   bOutboundAddressFound = RV_FALSE; /*indicates if we will use an outbound address*/
        pTrx->hNextMessageHop          = RvSipMsgGetRequestUri(hMsgObject);

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p - outbound address is forced, will try to use it",pTrx));

        rv = TryToUseOutboundAddress(pTrx,
                                     pHostName,
                                     pHostNameOffset,
                                     pPort,
                                     pTransportType,
                                     &eCompType,
                                     pbIsSecure,
                                     &bOutboundAddressFound);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p: Failed to get outbound address (rv=%d)",pTrx,rv));
            return rv;
        }
        
        if(bOutboundAddressFound == RV_TRUE)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p: outbound address will be used (host=%s)",
                pTrx,*pHostName));
#ifdef RV_SIP_OTHER_URI_SUPPORT
            pTrx->eMsgAddrUsedForSending = TRANSMITTER_MSG_OUTBOUND_ADDR;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
            return RV_OK;
        }
        else
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p - outbound address is forced but not found continue with regular behavior",pTrx));
        }
    }
    

    /* Check if there is a route list */
    hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByType(
                                           hMsgObject,
                                           RVSIP_HEADERTYPE_ROUTE_HOP,
                                           RVSIP_FIRST_HEADER,
                                           &listElem);
    while(hRouteHop != NULL)
    {
        RvSipRouteHopHeaderType      routeHopType;
        routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);

        if(routeHopType == RVSIP_ROUTE_HOP_ROUTE_HEADER)
        {
             break; /* There is at least one route header ==> There is a route list */
        }
        /*get the next route hop header*/
        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByType(
                             hMsgObject,
                             RVSIP_HEADERTYPE_ROUTE_HOP,
                             RVSIP_NEXT_HEADER,
                             &listElem);
    }

    if((bSendToFirstRouteHeader == RV_TRUE) && (hRouteHop != NULL))
    { /* Send the message to the first route header */

        pTrx->hNextMessageHop = RvSipRouteHopHeaderGetAddrSpec(hRouteHop);
#ifdef RV_SIP_OTHER_URI_SUPPORT
        pTrx->eMsgAddrUsedForSending = TRANSMITTER_MSG_FIRST_ROUTE;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
    }
    else /* The message is not sent to the first route header take information from request uri */
    {
        pTrx->hNextMessageHop  = RvSipMsgGetRequestUri(hMsgObject);
        bIsReqUri = RV_TRUE;
#ifdef RV_SIP_OTHER_URI_SUPPORT
        pTrx->eMsgAddrUsedForSending = TRANSMITTER_MSG_REQ_URI;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/

        /* If there is no route list and the application instructed us not to ignore the
           outbound proxy we will send the message to the outbound proxy if it exists */
        if((hRouteHop == NULL) && (pTrx->bIgnoreOutboundProxy == RV_FALSE))
        {
            RvBool   bOutboundAddressFound = RV_FALSE; /*indicates if we will use an outbound address*/
            
            rv = TryToUseOutboundAddress(pTrx,
                                         pHostName,
                                         pHostNameOffset,
                                         pPort,
                                         pTransportType,
                                         &eCompType,
                                         pbIsSecure,
                                         &bOutboundAddressFound);
            if(rv != RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "DestGetRequestAddressFromMsg - trx=0x%p: Failed to get outbound address (rv=%d)",pTrx,rv));
                return rv;
            }

            if(bOutboundAddressFound == RV_TRUE)
            {
                RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                        "DestGetRequestAddressFromMsg - trx=0x%p: outbound address will be used (host=%s)",
                        pTrx,*pHostName));
#ifdef RV_SIP_OTHER_URI_SUPPORT
                pTrx->eMsgAddrUsedForSending = TRANSMITTER_MSG_OUTBOUND_ADDR;
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
                return RV_OK;
            }
        }
    }

    if (pTrx->hNextMessageHop == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "DestGetRequestAddressFromMsg - trx=0x%p: hNextAddress is NULL",pTrx));
       return RV_ERROR_UNKNOWN;
    }

    /* checking if the Address type is not SIP URL */
    eAddrType = RvSipAddrGetAddrType(pTrx->hNextMessageHop);
    if (RVSIP_ADDRTYPE_URL == eAddrType 
#ifdef RV_SIP_OTHER_URI_SUPPORT
        || RVSIP_ADDRTYPE_IM == eAddrType
        || RVSIP_ADDRTYPE_PRES == eAddrType
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
        )
    {
        hSipUrlAddress = pTrx->hNextMessageHop;
    }
    
#ifdef RV_SIP_TEL_URI_SUPPORT
    /*this is tel URL and we want tel URL to be resolved by NAPTR */
    else if ((pMgr->bResolveTelUrls == RV_TRUE ) && (RVSIP_ADDRTYPE_TEL == eAddrType)) 
    {
        /* checking enumdi according to draft-stastny-iptel-tel-enumdi-00.txt [5] */
        if (RVSIP_ENUMDI_TYPE_EXISTS_EMPTY == RvSipAddrTelUriGetEnumdiParamType(pTrx->hNextMessageHop))
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p: TEL URL has enumdi",pTrx));

            rv = ConstructSipUrlAddrByTrx(
                pTrx,hMsgObject,pTrx->hNextMessageHop,&hSipUrlAddress,&bAddressResolved);
            
            if (RV_OK != rv || RV_TRUE != bAddressResolved)
            {
                rv = (rv != RV_OK) ? rv : RV_ERROR_UNKNOWN;
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: (enumdi) couldn't construct SIP URL Address. bAddressResolved=%d, rv=%d",
                    pTrx,bAddressResolved,rv));
                return rv;
            }
        }
        else
        {
            RvChar  strENUMQuery[MAX_HOST_NAME_LEN+1];
            RvInt32 allocationOffset    = 0;
            RvInt32 ausLen              = 0; 
#ifdef UPDATED_BY_SPIRENT
            RvSipTransportAddr localAddr;
#endif

            strENUMQuery[0]='\0';
        
            rv = SipResolverCreateAUSFromTel(pTrx->hNextMessageHop,(RvChar*)strENUMQuery);
            ausLen = (RvInt32)strlen(strENUMQuery)+1;
            pTrx->strQuery = RPOOL_AlignAppend(pTrx->hPool,pTrx->hPage,ausLen,&allocationOffset);
			if (pTrx->strQuery == NULL)
			{
				RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: failed to append new page.",pTrx));
                return RV_ERROR_UNKNOWN;
			}
            strcpy(pTrx->strQuery,strENUMQuery);
#ifdef UPDATED_BY_SPIRENT
            rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
            if (RV_OK != rv)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: failed to get vdev block info",pTrx));
                return rv;
            }

#endif
            rv = SipResolverResolve(pTrx->hRslv,
                    RVSIP_RESOLVER_MODE_FIND_URI_BY_NAPTR,
                    pTrx->strQuery,
                    RVSIP_RESOLVER_SCHEME_UNDEFINED,RV_FALSE,
                    0,RVSIP_TRANSPORT_UNDEFINED,
                    pTrx->hDnsList,DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

			/* The resolution state is changed in 2 cases: */ 
			/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
			/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
			/*    In this case the transmitter is ready to progress to the next query according to */ 
			/*    relevant state - it shouldn't wait to any report data callback. */ 
            if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
            {
                DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI);
                RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: resolving TEL Url To URI by DNS",pTrx));
                return rv;
            }
            else if (RV_OK != rv)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: failed to resolve TEL Url",pTrx));
                return RV_ERROR_UNKNOWN;
            }
        }
    }    
#if defined(UPDATED_BY_SPIRENT)    
    else if ((pMgr->bResolveTelUrls == RV_FALSE ) && (RVSIP_ADDRTYPE_TEL == eAddrType))
    {
       rv = ConstructSipUrlAddrByTrx(
             pTrx,hMsgObject,pTrx->hNextMessageHop,&hSipUrlAddress,&bAddressResolved);

       if (RV_OK != rv || RV_TRUE != bAddressResolved)
       {
          rv = (rv != RV_OK) ? rv : RV_ERROR_UNKNOWN;
          RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                   "DestGetRequestAddressFromMsg - trx=0x%p: (enumdi) couldn't construct SIP URL Address. bAddressResolved=%d, rv=%d", pTrx,bAddressResolved,rv));
          return rv;
       }
    }
#endif    
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */	
    else /* other URL, or no tel url support*/
    {
        rv = ConstructSipUrlAddrByTrx(
            pTrx,hMsgObject,pTrx->hNextMessageHop,&hSipUrlAddress,&bAddressResolved);

        if (RV_OK != rv || RV_TRUE != bAddressResolved)
        {
            rv = (rv != RV_OK) ? rv : RV_ERROR_UNKNOWN;
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetRequestAddressFromMsg - trx=0x%p: couldn't construct SIP URL Address. bAddressResolved=%d, rv=%d",
                pTrx,bAddressResolved,rv));
            return rv;
        }
    }
    
    rv  = GetAddressDetails(hSipUrlAddress,pTransportType,pbIsSecure);
    /* RFC 3261 defines the following behavior for both proxies [16.6.7] and UACs [8.1.2]:
       Independent of which URI is used as input to the procedures of [4], 
       if the Request-URI specifies a SIPS resource, the UAC MUST follow the procedures 
       of [4] as if the input URI were a SIPS URI.
       in the following code segment we override the secure parameter or any non-req-URI 
       with the req-URI secure parameter.
    */
    if (RV_FALSE == bIsReqUri && *pbIsSecure == RV_FALSE)
    {
        RvSipAddressHandle hUri;
        hUri = RvSipMsgGetRequestUri(hMsgObject);
        if (RVSIP_ADDRTYPE_URL == eAddrType)
        {
            *pbIsSecure = RvSipAddrUrlIsSecure(hUri);
        }
    }

    /* Check if the Request-Uri contains maddr parameter. In that case this is
    the address to send the message to */
    length = RvSipAddrGetStringLength(hSipUrlAddress, RVSIP_ADDRESS_MADDR_PARAM);
    if (0 != length)
    {
        /* There is a maddr parameter. Append it to the transmitter's page to gain
           consecutive memory. */
       rv = TransmitterDestAllocateHostBuffer(pTrx,length,&pStrHostName,pHostNameOffset);
       if (RV_OK != rv)
       {
           RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: failed to append consecutive memory for maddr param",pTrx));
           return rv;
       }
       rv = RvSipAddrUrlGetMaddrParam(hSipUrlAddress, pStrHostName,length, &actSize);
       if (RV_OK != rv)
       {
           RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: failed to get maddr param",pTrx));
           return rv;
       }
    }
    else /* no maddr */
    {
        length = RvSipAddrGetStringLength(hSipUrlAddress,
                                          RVSIP_ADDRESS_HOST);
        if (0 == length)
        {
            /* There must be a host in a legal Request-Uri */
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: no host in hSipUrlAddress. must be",pTrx));
            return RV_ERROR_UNKNOWN;
        }
        /* Append the host string to the transmitter's page */
        rv = TransmitterDestAllocateHostBuffer(pTrx,length,&pStrHostName, pHostNameOffset);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: Failed to appent consecutive memory for host string",pTrx));
            return rv;
        }
        rv = RvSipAddrUrlGetHost(hSipUrlAddress, pStrHostName,length, &actSize);
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: failed the get host param",pTrx));
            return rv;
        }
    }

    if (NULL == pStrHostName)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* check if the pStrHostName length is not greater than pHostName size */
	/* length includes the null at the end. We need to check only the address part */
    if((length - 1)> MAX_HOST_NAME_LEN)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
              "DestGetRequestAddressFromMsg - trx=0x%p: not enough space to copy host name",pTrx));
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    *pHostName=pStrHostName;
    
    /* the port in the address structure is int32, and the local port parameter is int16.
        --> we can't make a direct casting... */
    *pPort = RvSipAddrUrlGetPortNum(hSipUrlAddress); 
    
    
#ifdef RV_SIGCOMP_ON
    /* Retrieve the compression type from the SIP URL address and store */
    /* it in the current Transmitter object */
    eCompType = RvSipAddrUrlGetCompParam(hSipUrlAddress);
    UpdateTrxOutboundMsgCompType(pTrx,eCompType);
	rv = TransmitterUpdateOutboundMsgSigcompId(pTrx, SipAddrGetPool(hSipUrlAddress), SipAddrGetPage(hSipUrlAddress),
			SipAddrUrlGetSigCompIdParam(hSipUrlAddress));

#endif /* RV_SIGCOMP_ON */
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_URI_FOUND);

    return rv;
}

/*****************************************************************************
 * DestGetResponseAddressFromMsg
 * ---------------------------------------------------------------------------
 * General: Get host address from the Via header
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgObject              - handle of the message object
 *            pTrx                  - The sending transaction. if the pointer is NULL
 *                                    the sending object is the pTranscMgr.
 * Output:  pHostName               - The host name that the response is sent to.
 *          pPort                   - The port that the response is sent to.
 *          pTransportType          - The type of the response transport.
 ****************************************************************************/
static RvStatus RVCALLCONV DestGetResponseAddressFromMsg(
                           IN  Transmitter*       pTrx,
                           IN  RvSipMsgHandle     hMsgObject,
                           OUT RvChar**           pHostName,
                           OUT RvInt32*           pHostNameOffset,
                           OUT RvInt32*           pPort,
                           OUT RvSipTransport*    pTransportType)
{
    RvSipViaHeaderHandle        hViaHeader;
    RvSipHeaderListElemHandle   hListElem;
    RvStatus                    rv = RV_OK;
    TransmitterMgr*             pMgr;
    RvChar*                     pStrHostName;
    RvInt32                     hostNameOffset;
    RvBool                      bUseRport;
    RvInt32                     rport;
#ifdef RV_SIGCOMP_ON
    RvSipCompType               eCompType;
#endif /* RV_SIGCOMP_ON */
#if defined(UPDATED_BY_SPIRENT)
#else
    RvUint16                    port = 0;
#endif


    hListElem = NULL;
    pMgr = pTrx->pTrxMgr;
    
    /* getting the top via header */
    hViaHeader = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByTypeExt(
                                            hMsgObject,
                                            RVSIP_HEADERTYPE_VIA,
                                            RVSIP_FIRST_HEADER,
                                            RVSIP_MSG_HEADERS_OPTION_ALL,
                                            &hListElem);

    if (hViaHeader == NULL)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetResponseAddressFromMsg - trx 0x%p: No Via header in msg, returning failure", pTrx));
        return RV_ERROR_UNKNOWN;
    }

    /* checking that the transport type of the via header is udp */
    *pTransportType = RvSipViaHeaderGetTransport(hViaHeader);
    if(*pTransportType == RVSIP_TRANSPORT_UNDEFINED)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "DestGetResponseAddressFromMsg - trx 0x%p: transport in Via header is UNDEFINED (bad-syntax via?), returning failure", pTrx));
        return RV_ERROR_UNKNOWN;
    }

    /* Retrieve the destination address from the message Via header */
    if ((RVSIP_TRANSPORT_TCP == *pTransportType) ||
        (RVSIP_TRANSPORT_SCTP == *pTransportType) ||
        (RVSIP_TRANSPORT_TLS == *pTransportType)) /* TCP, SCTP or TLS */
    {
        rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_RECEIVED_PARAM, &pStrHostName, &hostNameOffset);
        if (RV_ERROR_NOT_FOUND == rv)
        {
            rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_MADDR_PARAM, &pStrHostName, &hostNameOffset);
            if (RV_ERROR_NOT_FOUND == rv)
            {
                rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_HOST, &pStrHostName, &hostNameOffset);
            }
        }
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetResponseAddressFromMsg - trx 0x%p: Could not get parameters from Via (rv=%d)", pTrx, rv));
            return rv;
        }
    }
    else /* UDP */
    {
        rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_MADDR_PARAM, &pStrHostName, &hostNameOffset);
        if (RV_ERROR_NOT_FOUND == rv)
        {
            rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_RECEIVED_PARAM, &pStrHostName, &hostNameOffset);
            if (RV_ERROR_NOT_FOUND == rv)
            {
                rv = GetViaParameter(pTrx, hViaHeader,RVSIP_VIA_HOST, &pStrHostName, &hostNameOffset);
            }
        }
        if (RV_OK != rv)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "DestGetResponseAddressFromMsg - trx=0x%p:: Could not get parameters from Via (rv=%d)", pTrx, rv));
            return rv;
        }
    }

    if (NULL == pStrHostName)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Retrieve the 'rport' parameter value from Via Header. In case of
      non existing or undefined 'rport' the used port is retrieved
      from the 'sent-by' section. */
    rv = RvSipViaHeaderGetRportParam(hViaHeader,&rport,&bUseRport);
#if defined(UPDATED_BY_SPIRENT)
    if (RV_OK == rv && RV_TRUE == bUseRport && UNDEFINED  != rport)
    {
        *pPort = rport;
    }
    else
    {
        *pPort = RvSipViaHeaderGetPortNum(hViaHeader);
    }
#else
    if (RV_OK == rv && RV_TRUE == bUseRport && UNDEFINED  != rport)
    {
        port = (RvUint16)rport;
    }
    else
    {
        port = (RvUint16)RvSipViaHeaderGetPortNum(hViaHeader);
    }
    *pPort = port;
#endif

    *pHostName = pStrHostName;
    *pHostNameOffset = hostNameOffset;
#ifdef RV_SIGCOMP_ON
    /* Retrieve the compression type from the Via Header and store */
    /* it in the current Transmitter object */
    eCompType = RvSipViaHeaderGetCompParam(hViaHeader);
    UpdateTrxOutboundMsgCompType(pTrx,eCompType);
#endif /* RV_SIGCOMP_ON */
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_URI_FOUND);    
    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*****************************************************************************
 * GetAddressDetails
 * ---------------------------------------------------------------------------
 * General: Retrieving address details in case of known RVSIP_ADDRTYPE_URL
 *            address .
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress            - The address handle that contains the details.
 *
 * Output:  pTransportType        -    The retrieved transport type.
 *          pbIsSecure          -   The retrieved secure flag.
 ****************************************************************************/
static RvStatus RVCALLCONV GetAddressDetails(
             IN  RvSipAddressHandle  hAddress,
             OUT RvSipTransport*    pTransportType,
             OUT RvBool*            pbIsSecure)
{
    if (NULL == hAddress)
    {
        return RV_ERROR_BADPARAM;
    }
    
    switch (RvSipAddrGetAddrType(hAddress))
    {
        /* checking if the Address type is SIP URL */
    case RVSIP_ADDRTYPE_URL:
        /* getting the transport type */
        *pTransportType = RvSipAddrUrlGetTransport(hAddress);
        *pbIsSecure     = RvSipAddrUrlIsSecure(hAddress);
        break;
    case RVSIP_ADDRTYPE_IM:
    case RVSIP_ADDRTYPE_PRES:
        /* getting the transport type */
        *pTransportType = RvSipAddrUrlGetTransport(hAddress);
        *pbIsSecure     = RvSipAddrUrlIsSecure(hAddress);
        break;
    default:
    /* setting default values since the address
        handle doesn't include the details */
        *pTransportType = RVSIP_TRANSPORT_UNDEFINED;
        *pbIsSecure     = RV_FALSE;
        break;
    }

    return RV_OK;
}



/*****************************************************************************
 * ConstructSipUrlAddrByTrx
 * ---------------------------------------------------------------------------
 * General: Construct SIP URL address by using other URL address data.
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc        - The transaction that handles the new SIP URL
 *                                  address construction .
 *          hMsg                - Handle of the message that contains the Other
 *                                  URL address.
 *            hAddress            - The other URL address handle to be "converted"
 *                                  to SIP URL address .
 *
 * Output:  hSipAddr            - Handle to the constructed SIP URL address.
 *            bAddressResolved    - Indication of a successful/failed address
 *                                  resolving.
 *
 ****************************************************************************/
static RvStatus RVCALLCONV ConstructSipUrlAddrByTrx(
              IN  Transmitter*        pTrx,
              IN  RvSipMsgHandle      hMsg,
              IN  RvSipAddressHandle  hAddress,
              OUT RvSipAddressHandle* phSipAddr,
              OUT RvBool*             pbAddressResolved)
{
    RvStatus  rv  = RV_ERROR_UNKNOWN;

    /* Initialize the indication of the resolving completion */
    *pbAddressResolved = RV_FALSE;

    if (NULL == pTrx || NULL == hAddress || NULL == phSipAddr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Check if the application can handle the address conversion */
    if (NULL != pTrx->evHandlers.pfnOtherURLAddressFoundEvHandler)
    {
        rv = RvSipAddrConstruct(pTrx->pTrxMgr->hMsgMgr,
                               SipMsgGetPool(hMsg),
                               SipMsgGetPage(hMsg),
                               RVSIP_ADDRTYPE_URL,
                               phSipAddr);

        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                      "ConstructSipUrlAddrByTrx - trx=0x%p: couldn't construct new SIP URL address (rv=%d)",pTrx,rv));
            return rv;
        }

        /* Converting the other URL address to SIP URL address with callback*/
        rv = TransmitterCallbackOtherURLAddressFoundEv(
                                   pTrx,
                                    hMsg,
                                    hAddress,
                                    *phSipAddr,
                                    pbAddressResolved);

        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                       "ConstructSipUrlAddrByTrx - trx=0x%p: failed to handle unknown URL address(rv=%d)",pTrx,rv));
            return rv;
        }
    }

    return rv;
}

/*****************************************************************************
 * TryToUseOutboundAddress
 * ---------------------------------------------------------------------------
 * General: See if there is an outbound address to use for this request if 
 *          there is one: use it!
 * Return Value: RvStatus.
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx                - The sending transmitter.
 * Output:  pHostName           - Outbound proxy host or ip address
 *          pPort               - Outbound proxy port
 *          peTransportType     - outbound proxy transport
 *          pbIsSecure          - is address secure
 *          peCompType          - the compression type
 *          bAddressFound       - indicates if an outbound address was found.
 ****************************************************************************/
static RvStatus RVCALLCONV TryToUseOutboundAddress(
               IN  Transmitter*       pTrx,
               OUT RvChar**           pHostName,
               OUT RvInt32*           pHostNameOffset,
               OUT RvInt32*           pPort,
               OUT RvSipTransport*    peTransportType,
               OUT RvSipCompType*     peCompType,
               OUT RvBool*            pbIsSecure,
               OUT RvBool*            pbAddrFound)
{
    RvStatus   rv;
	RPOOL_Ptr  rpoolSigcompId;

	rpoolSigcompId.offset = UNDEFINED;

    rv = CheckForOutboundAddress(pTrx,pHostName,pHostNameOffset,pPort,peTransportType,peCompType,
		&rpoolSigcompId,pbAddrFound);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TryToUseOutboundAddress - trx=0x%p: Failed to get outbound address (rv=%d)",pTrx,rv));
        return rv;
    }
    if (RV_FALSE == *pbAddrFound)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TryToUseOutboundAddress - trx=0x%p: no outbound address, returning",pTrx));
        return rv;
    }
    if (RVSIP_ADDRTYPE_URL == RvSipAddrGetAddrType(pTrx->hNextMessageHop))
    {
        *pbIsSecure = RvSipAddrUrlIsSecure(pTrx->hNextMessageHop);
    }
    else
    {
        *pbIsSecure = RV_FALSE;
    }
#ifdef RV_SIGCOMP_ON
    UpdateTrxOutboundMsgCompType(pTrx,*peCompType);
	rv = TransmitterUpdateOutboundMsgSigcompId(pTrx,rpoolSigcompId.hPool,rpoolSigcompId.hPage,rpoolSigcompId.offset);
	if (rv != RV_OK)
	{
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "TryToUseOutboundAddress - trx=0x%p: Failed to get update sigcom-id (rv=%d)",pTrx,rv));
        return rv;
	}
#endif /* RV_SIGCOMP_ON */
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_URI_FOUND);
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "TryToUseOutboundAddress - trx=0x%p: outbound address used: secure=%d, comp=%d,transport=%s",
        pTrx,*pbIsSecure,*peCompType,SipCommonConvertEnumToStrTransportType(*peTransportType)));
    return RV_OK;
}

/*****************************************************************************
 * CheckForOutboundAddress
 * ---------------------------------------------------------------------------
 * General: See if there is an outbound address to use for this request. First
 *          look for an outbound address in the transaction object. If there
 *          is no outbound address in the transaction, look for an address in
 *          the transaction manager. (This function can be called for an
 *          out of context message. in this case the transaction handle will be
 *          NULL and the address will be taken from the transaction manager.
 * Return Value: RvStatus.
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx                - The sending transmitter.
 * Output:  pHostName           - Outbound proxy host or ip address
 *          pPort               - Outbound proxy port
 *          peTransportType     - outbound proxy transport
 *          peCompType          - compression type
 *          pSigcompId          - the sigcomp-id string
 *          bAddressFound       - indicates if an outbound address was found.
 ****************************************************************************/
static RvStatus RVCALLCONV CheckForOutboundAddress(
               IN  Transmitter*       pTrx,
               OUT RvChar**           pHostName,
               OUT RvInt32*           pHostNameOffset,
               OUT RvInt32*           pPort,
               OUT RvSipTransport*    peTransportType,
               OUT RvSipCompType*     peCompType,
			   OUT RPOOL_Ptr*         pSigcompId,
               OUT RvBool*            pbAddrFound)
{
    TransmitterMgr*         pMgr            = pTrx->pTrxMgr;
    RvChar*                 pLocalHostName  = NULL;
    RvInt32                 localHostNameOffset = UNDEFINED;
    RvStatus                rv              = RV_OK;
    RvChar  strIp[MAX_HOST_NAME_LEN];

    *pbAddrFound = RV_TRUE;
    /*look for outbound ip in the transmitter*/
    if ((pTrx->outboundAddr.bUseHostName == RV_FALSE)     &&
        (pTrx->outboundAddr.bIpAddressExists == RV_TRUE)   )
    {
        /* there is no need for an IPv6 section, because the address is hidden in core layer */
        RvAddressGetString(&pTrx->outboundAddr.ipAddress.addr,MAX_HOST_NAME_LEN,(RvChar*)strIp);
        rv = TransmitterDestAllocateHostBuffer(pTrx,(RvUint)strlen(strIp)+1/*1 for '\0'*/,&pLocalHostName, &localHostNameOffset );
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CheckForOutboundAddress- trx=0x%p: allocate buf (rv=%d)", pTrx,rv));
            return rv;
        }
        strcpy(pLocalHostName,strIp);
        *pHostName       = pLocalHostName;
        *pHostNameOffset = localHostNameOffset;
        *pPort           = RvAddressGetIpPort(&pTrx->outboundAddr.ipAddress.addr);
        *peTransportType = pTrx->outboundAddr.ipAddress.eTransport;
#ifdef RV_SIGCOMP_ON    
        *peCompType      = pTrx->outboundAddr.eCompType;
		*pSigcompId      = pTrx->outboundAddr.rpoolSigcompId;
#endif /*#ifdef RV_SIGCOMP_ON */   

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckForOutboundAddress - returning IP String %s:%d;transport=%s",*pHostName,*pPort,SipCommonConvertEnumToStrTransportType(*peTransportType)));
        return RV_OK;
    }
    /*look for outbound host in the transmitter*/
    if( pTrx->outboundAddr.bUseHostName == RV_TRUE &&
        pTrx->outboundAddr.rpoolHostName.hPage != NULL_PAGE &&
        pTrx->outboundAddr.rpoolHostName.hPage != NULL &&
        pTrx->outboundAddr.rpoolHostName.offset != UNDEFINED)
    {

        RvInt32 length = RPOOL_Strlen(pTrx->outboundAddr.rpoolHostName.hPool,
                              pTrx->outboundAddr.rpoolHostName.hPage,
                              pTrx->outboundAddr.rpoolHostName.offset);
        rv = TransmitterDestAllocateHostBuffer(pTrx,length+1,pHostName, pHostNameOffset);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CheckForOutboundAddress- trx=0x%p: allocate buf (rv=%d)", pTrx,rv));
            return rv;
        }

        rv = RPOOL_CopyToExternal(pTrx->outboundAddr.rpoolHostName.hPool,
                              pTrx->outboundAddr.rpoolHostName.hPage,
                              pTrx->outboundAddr.rpoolHostName.offset,
                              *pHostName,
                              length+1);
        if(rv != RV_OK)
        {
            RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                "CheckForOutboundAddress- trx=0x%p: Failed to copy to buff (rv=%d)", pTrx,rv));
            return rv;
        }
        *pPort           = RvAddressGetIpPort(&pTrx->outboundAddr.ipAddress.addr);
        *peTransportType = pTrx->outboundAddr.ipAddress.eTransport;
#ifdef RV_SIGCOMP_ON    
        *peCompType      = pTrx->outboundAddr.eCompType;
		*pSigcompId      = pTrx->outboundAddr.rpoolSigcompId;
#endif /*#ifdef RV_SIGCOMP_ON*/    

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "CheckForOutboundAddress - trx=0x%p: returning transaction outbound host %s:%d",pTrx,*pHostName,*pPort));
        return RV_OK;
    }
    {
        /* if an outbound address was not found in the specific transmitter see if
           the stack has a configured outbound address*/
        RvChar*        strHostName           = NULL;
        RvChar*        strCfgOutboundIp      = NULL;
        RvChar*        strCfgOutboundHost    = NULL;
        RvUint16       cfgOutboundPort       = 0;
        RvSipTransport eCfgOutboundTransport = RVSIP_TRANSPORT_UNDEFINED;
        RvSipCompType  eCfgOutboundCompType  = RVSIP_COMP_UNDEFINED;
		RvChar*        strSigcompId          = NULL;
        SipTransportGetOutboundAddress(pMgr->hTransportMgr,
                                       &strCfgOutboundIp,
                                       &strCfgOutboundHost,
                                       &cfgOutboundPort,
                                       &eCfgOutboundTransport,
                                       &eCfgOutboundCompType,
									   &strSigcompId);
            
        /*look for outbound ip in the transport manager*/
        if (strCfgOutboundIp != NULL)
        {
            strHostName = strCfgOutboundIp;
        }
        /*look for outbound host in the transaction manager*/
        if(strCfgOutboundHost != NULL)
        {
            strHostName = strCfgOutboundHost;
        }
        if (NULL != strHostName)
        {
            rv = TransmitterDestAllocateHostBuffer(pTrx, (RvUint)strlen(strHostName)+1,
                                                   &pLocalHostName, &localHostNameOffset );
            if(rv != RV_OK)
            {
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "CheckForOutboundAddress- trx=0x%p: allocate buf (rv=%d)", pTrx,rv));
                return rv;
            }
            strcpy(pLocalHostName,strHostName);
            *pHostName       = pLocalHostName;
            *pHostNameOffset = localHostNameOffset;
			*pPort           = cfgOutboundPort;
			*peTransportType = eCfgOutboundTransport;
			*peCompType      = eCfgOutboundCompType;
			if (strSigcompId != NULL)
			{
				rv = RPOOL_Append(pTrx->hPool, pTrx->hPage, (RvInt)strlen(strSigcompId)+1,
					RV_TRUE,&pSigcompId->offset);
				if(rv != RV_OK)
				{
					RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
						"CheckForOutboundAddress- trx=0x%p: append buf (rv=%d)", pTrx,rv));
					return rv;
				}
				pSigcompId->hPool = pTrx->hPool;
				pSigcompId->hPage = pTrx->hPage;
			}
			else
			{
				pSigcompId->offset = UNDEFINED;
			}
			return RV_OK;
        }
    }
    *pbAddrFound = RV_FALSE; /*no outbound address to use for this request*/
    return RV_OK;
}


/***************************************************************************
 * GetViaParameter
 * ------------------------------------------------------------------------
 * General: Get address parameter from Via header. This is either Receives,
 *          maddr or host parameter.
 * Return Value: -  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pMgr           - The transaction manager (to retrieve the memory
 *                         pool from).
 *        hDestAddrPage  - Memory page to use for memory appending.
 *          hViaHeader     - The Via header to retrieve the parameter from.
 *        eParameterName - The name of the parameter to retrieve.
 *        pStrHostName   - Host name retrieved from the chosen parameter.
 ***************************************************************************/
static RvStatus RVCALLCONV GetViaParameter(
            IN  Transmitter*               pTrx,
            IN  RvSipViaHeaderHandle       hViaHeader,
            IN  RvSipViaHeaderStringName   eParameterName,
            OUT RvChar**                   pStrHostName,
            OUT RvInt32*                   pHostNameOffset)
{
    RvStatus       rv       = RV_OK;
    RvUint         length   = 0;
    RvUint         actSize  = 0;

    switch (eParameterName)
    {
    case RVSIP_VIA_MADDR_PARAM:
        {
            length = RvSipViaHeaderGetStringLength(hViaHeader, RVSIP_VIA_MADDR_PARAM);
            if (0 == length)
            {
                return RV_ERROR_NOT_FOUND;
            }

            /* The Via header contains maddr parameter.
               Append the maddr parameter to the transmitter's memory
               page to gain consecutive memory */
            rv = TransmitterDestAllocateHostBuffer(pTrx,length +1,pStrHostName, pHostNameOffset);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (MADDR): Faild to allocate buffer from pool",
                    pTrx));      
                return rv;
            }

            rv = RvSipViaHeaderGetMaddrParam(hViaHeader, *pStrHostName,length+1, &actSize);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (MADDR): Faild to get maddr",pTrx));      
                return rv;
            }
            break;
        }
    case RVSIP_VIA_RECEIVED_PARAM:
        {
            length = RvSipViaHeaderGetStringLength(hViaHeader,RVSIP_VIA_RECEIVED_PARAM);
            if (0 == length)
            {
                return RV_ERROR_NOT_FOUND;
            }
            /* The Via header contains received parameter.
               Append the parameter to the transaction's memory
               page to gain consecutive memory */
            rv = TransmitterDestAllocateHostBuffer(pTrx,length +1,pStrHostName, pHostNameOffset);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (RECEIVED): Faild to allocate buffer from pool",pTrx));      
                return rv;
            }

            rv = RvSipViaHeaderGetReceivedParam(hViaHeader, *pStrHostName,length+1, &actSize);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (RECEIVED): Faild to get received",pTrx));      
                return rv;
            }
            break;
        }
    case RVSIP_VIA_HOST:
        {
            length = RvSipViaHeaderGetStringLength(hViaHeader, RVSIP_VIA_HOST);
            if (0 == length)
            {
                return RV_ERROR_NOT_FOUND;
            }
            /* Append the address to the transaction's memory page to gain
               consecutive memory */
            rv = TransmitterDestAllocateHostBuffer(pTrx,length +1,pStrHostName, pHostNameOffset);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (HOST): Faild to allocate buffer from pool",pTrx));      
                return rv;
            }
            rv = RvSipViaHeaderGetHost(hViaHeader, *pStrHostName,length+1, &actSize);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "GetViaParameter - trx=0x%p (HOST): Faild to get host",pTrx));      
                return rv;
            }
            break;
        }
    default:
        {
            RvLogExcep(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc,
                "GetViaParameter - Requested unknown address parameter"));
            return RV_ERROR_UNKNOWN;
        }
    }

    return RV_OK;
}

#ifdef RV_SIGCOMP_ON
/*****************************************************************************
 * UpdateTrxOutboundMsgCompType
 * ---------------------------------------------------------------------------
 * General: Update the Trx outbound msg compression type by a general 
 *          compression type.
 * Return Value: RvStatus.
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx      - The sending transmitter.
 *          eCompType - The compression type to be translated to mesg 
 *                      compression type.                      
 * Output:  
 ****************************************************************************/
static void RVCALLCONV UpdateTrxOutboundMsgCompType(
                           IN Transmitter   *pTrx,
                           IN RvSipCompType  eCompType)
{
    switch (eCompType)
    {
    case RVSIP_COMP_SIGCOMP:
        pTrx->eOutboundMsgCompType = 
            RVSIP_TRANSMITTER_MSG_COMP_TYPE_SIGCOMP_COMPRESSED;
        break;
    default:
        pTrx->eOutboundMsgCompType = 
            RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNCOMPRESSED;
    }
}

#endif /* RV_SIGCOMP_ON */

/************************************************************************************
 * TransmitterDestReset
 * ----------------------------------------------------------------------------------
 * General: Reset transmitter to the initial state.
 *          Call this function when you want to go for another round with
 *          the DNS procedure
 *
 * Return Value: RvStatus - RV_OK or RV_ERROR_BADPARAM
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    pTrx-      - Transmitter
 * Output:  none
 * ---------------------------------------------------------------------------------
 * NOTE: The transmitter does not destruct messages after they are sent.
 *       It is the responsibility of the calling object to know when the message is
 *       no longer needed and destruct it
 ***********************************************************************************/
static void RVCALLCONV TransmitterDestReset (IN Transmitter*     pTrx)
{
    RvSipTransportDNSSRVElement      srvElement;
    RvSipTransportDNSHostNameElement hostElement;

    if (RV_TRUE == pTrx->bAddrManuallySet)
    {
        /* this address was set by the application we dont want to overwrite it*/
        return;
    }
    /*Sarit - bug fix - reseting the temporary msgParams befor address resolution*/    
	/* resetting msg params */
    pTrx->msgParams.strHostName     = NULL;
    pTrx->msgParams.hostNameOffset  = UNDEFINED;
    pTrx->msgParams.transportType   = RVSIP_TRANSPORT_UNDEFINED;
    pTrx->msgParams.bIsSecure       = RV_FALSE;
    pTrx->msgParams.port            = UNDEFINED;
	pTrx->msgParams.msgType         = RVSIP_MSG_UNDEFINED;

	
	memset(&pTrx->destAddr,0,sizeof(pTrx->destAddr));
    RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&pTrx->destAddr.addr);
    
    pTrx->destAddr.eTransport    = RVSIP_TRANSPORT_UNDEFINED;
    
    /*setting used elements */
    memset(&srvElement,0,sizeof(srvElement));
    memset(&hostElement,0,sizeof(hostElement));
    RvSipTransportDNSListSetUsedSRVElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&srvElement);
    RvSipTransportDNSListSetUsedHostElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&hostElement);
}


/***********************************************************************************************************************
 * DestTryPreDnsList
 * purpose : retrieves protocol by already discovered IP, Host or SRV list entries
 * input   : pTrx   - transmitter that keeps the DNS list
 * output  : none
 * return  : RvStatus - RV_OK or RV_ERROR_UNKNOWN
 *************************************************************************************************************************/
static RvStatus DestTryPreDnsList(
    IN Transmitter* pTrx)
{
    RvStatus                            rv = RV_OK;
    RvSipTransportDNSIPElement          ipElement;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvSipTransportDNSHostNameElement    hostElement;
    RvSipTransportDNSSRVElement         srvElement;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    TransmitterMgr*                     pTrxMgr = pTrx->pTrxMgr;

		RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
			"DestTryPreDnsList - pTrx=0x%p: host offset %d: trying to find prediscovered elements",
			pTrx, pTrx->msgParams.hostNameOffset)); 

    /* First check if there are host name elements or IP address elements
    discovered during previous iterations. In that case no SRV resolution
    should be applied */
    if (TRANSMITTER_RESOLUTION_STATE_RESOLVED != pTrx->eResolutionState)
    {
        rv = RvSipTransportDNSListPopIPElement(pTrxMgr->hTransportMgr,
            pTrx->hDnsList,&ipElement);
        if (rv == RV_OK)
        {
            pTrx->destAddr.eTransport = ipElement.protocol;
            SipTransportIPElementToSipTransportAddress(&pTrx->destAddr,&ipElement);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
            {
                RvChar  ip[RVSIP_TRANSPORT_LEN_STRING_IP];
                
                RvAddressGetString(&pTrx->destAddr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,(RvChar*)ip);
                RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - pTrx=0x%p: host offset %d: IP element already in list (listip=%s)",
                    pTrx, pTrx->msgParams.hostNameOffset,ip));
            }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
            DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVED);
            return RV_OK;
        }
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if (TRANSMITTER_RESOLUTION_STATE_UNDEFINED                     == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_RESOLUTION_STARTED            == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_URI_FOUND                     == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI               == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT        == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_3WAY_SRV == pTrx->eResolutionState)
    {
        rv = RvSipTransportDNSListPopHostElement(pTrx->pTrxMgr->hTransportMgr,
            pTrx->hDnsList,&hostElement);
        if (rv == RV_OK)
        {
            RvInt32 allocationOffset = 0;
            
            pTrx->destAddr.eTransport = hostElement.protocol;
			/* The hostElement port number is NOT set within pTrx->destAddr.addr */
			/* in this early stage, since the pTrx->destAddr.addr type is not    */
			/* known yet add the storage operation might fail (return NULL)		 */
            RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                "DestTryPreDnsList - pTrx=0x%p: host %d: host element already in list. (list host =%s)",
                pTrx, pTrx->msgParams.hostNameOffset,hostElement.hostName));
            RvSipTransportDNSListSetUsedHostElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&hostElement);
            /* saving the host string */
            pTrx->strQuery = RPOOL_AlignAppend(pTrx->hPool,pTrx->hPage,(RvInt32)strlen(hostElement.hostName)+1,&allocationOffset);
			if (pTrx->strQuery == NULL)
			{
				RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - trx=0x%p: failed to append new page.",pTrx));
                return RV_ERROR_UNKNOWN;
			}
            strcpy(pTrx->strQuery,hostElement.hostName);
            rv = DestResolveIpByHost(pTrx,pTrx->strQuery,hostElement.port);
            return rv;
        }
    }
    if (TRANSMITTER_RESOLUTION_STATE_UNDEFINED            == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_URI_FOUND            == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_WAITING_FOR_URI      == pTrx->eResolutionState ||
        TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR == pTrx->eResolutionState)
    {
        rv = RvSipTransportDNSListPopSrvElement(pTrx->pTrxMgr->hTransportMgr,
            pTrx->hDnsList,&srvElement);
        if (rv == RV_OK)
        {
#ifdef UPDATED_BY_SPIRENT
            RvSipTransportAddr localAddr;
#endif
            pTrx->destAddr.eTransport = srvElement.protocol;
            RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                "DestTryPreDnsList - pTrx=0x%p: host offset %d: SRV element already in list. (list srv =%s)",
                pTrx, pTrx->msgParams.hostNameOffset,srvElement.srvName));
            RvSipTransportDNSListSetUsedSRVElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&srvElement);

            /*-------------------------------------------------------*/
            /* Bug Fix - set the Transmitter's msgParams.strHostName , since:
               1. In state RESOLVING_URI_HOSTPORT the msgParams.strHostName is referred 
               2. In the following called function SipResolverResolve(), the srvElement.srvName
                  is referred asynchronously, however it's freed right after exiting this 
                  function scope. */ 

            /* A memory should be allocated for keeping the last query. The 'srvElement.srvName'
               should be copied since it might be accessed when returning synchronously from 
               the ARES module */ 
            rv = TransmitterDestAllocateHostBuffer(pTrx, (RvUint)strlen(srvElement.srvName) +1, /* 1- for the '\0' */
                                                   &pTrx->msgParams.strHostName, 
                                                   &pTrx->msgParams.hostNameOffset);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - pTrx=0x%p: can not allocate space for URI: (rv=%d)",
                    pTrx,rv));
                return RV_ERROR_BADPARAM;
            }
            strcpy(pTrx->msgParams.strHostName,srvElement.srvName);
#ifdef UPDATED_BY_SPIRENT
            rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
            if (RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestGetRequestAddressFromMsg - trx=0x%p: failed to get vdev block info",pTrx));
                return rv;
            }

#endif
            /*-------------------------------------------------------*/
            rv = SipResolverResolve(pTrx->hRslv,RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_SRV_STRING,
                pTrx->msgParams.strHostName,RVSIP_RESOLVER_SCHEME_UNDEFINED,
                pTrx->msgParams.bIsSecure,0,
                srvElement.protocol,
                pTrx->hDnsList, DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

			/* The resolution state is changed in 2 cases: */ 
			/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
			/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
			/*    In this case the transmitter is ready to progress to the next query according to */ 
			/*    relevant state - it shouldn't wait to any report data callback. */ 
            if (/* Fix it should return in async processing case RV_ERROR_TRY_AGAIN == rv ||*/ 
                RV_DNS_ERROR_NOTFOUND == rv)
            {
                RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - trx=0x%p: (host name offset %d) resolveing IP by host after NOTFOUND",
                    pTrx,pTrx->msgParams.hostNameOffset));

				/* The memory allocation has been moved above (before the 'if' condition) */ 
   
                DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT);
                return rv;
            }
            /* Bug fix the RV_ERROR_TRY_AGAIN is a valid return value (async mode) */
            else if (RV_ERROR_TRY_AGAIN == rv) 
            {
                RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - trx=0x%p: (host name offset %d) resolving IP by host after TRY_AGAIN",
                    pTrx,pTrx->msgParams.hostNameOffset));

                return rv; 
            }
            else 
            {
                RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
                    "DestTryPreDnsList - trx=0x%p: (message name offset %d) error resolving host and port by srv",
                    pTrx,pTrx->msgParams.hostNameOffset));
                return rv;
            }
        }
    }

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
		RvLogDebug(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
        "DestTryPreDnsList - pTrx=0x%p: host offset %d: no prediscovered elements",
        pTrx, pTrx->msgParams.hostNameOffset));
    
    return RV_ERROR_NOT_FOUND;
}

/***********************************************************************************************************************
 * DestProtocolGetExplicitlySpecified
 * purpose : retrieves protocol explicitly defined in the SIP URI (ie; ;transport=xxx or sips scheme)
 * input   : pTrx   - transmitter that keeps the DNS list
 * output  : none
 * return  : RvStatus - RV_OK or RV_ERROR_UNKNOWN
 *************************************************************************************************************************/
static RvStatus DestProtocolGetExplicitlySpecified(
    IN Transmitter* pTrx)
{
    /*  in case, transport type is specified, it should be used */
    if (pTrx->msgParams.transportType != RVSIP_TRANSPORT_UNDEFINED)
    {
        if (pTrx->msgParams.bIsSecure == RV_TRUE)
        {
            if (pTrx->msgParams.transportType == RVSIP_TRANSPORT_UDP)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestProtocolGetExplicitlySpecified - pTrx=0x%p: host offset %d: Secure address can not use UDP (rv=%d)",
                    pTrx, pTrx->msgParams.hostNameOffset,RV_ERROR_BADPARAM));
                return RV_ERROR_BADPARAM;
            }
            pTrx->destAddr.eTransport = RVSIP_TRANSPORT_TLS;
        }


        /* transport may have been set by secure flag */
        if (pTrx->destAddr.eTransport == RVSIP_TRANSPORT_UNDEFINED)
        {
            pTrx->destAddr.eTransport = pTrx->msgParams.transportType;
        }

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolGetExplicitlySpecified - pTrx=0x%p: host offset %d: Transport type specified explicitly to be %s",
            pTrx, pTrx->msgParams.hostNameOffset,
            SipCommonConvertEnumToStrTransportType(pTrx->msgParams.transportType)));

        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND);
        return RV_OK;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolGetExplicitlySpecified - pTrx=0x%p: host offset %d: Transport not explicitly specified",
        pTrx, pTrx->msgParams.hostNameOffset));

    return RV_ERROR_NOT_FOUND;
}


/***********************************************************************************************************************
 * DestProtocolGetImplicitlySpecified
 * purpose : retrieves protocol if specified implicitly in the SIP URI
 *           by using IP address (not host name) or specifying port
 * input   : pTrx   - transmitter that keeps the DNS list
 * output  : none
 * return  : RvStatus - RV_OK or RV_ERROR_UNKNOWN
 *************************************************************************************************************************/
static RvStatus DestProtocolGetImplicitlySpecified(
    IN Transmitter* pTrx)
{
    RvStatus             rv;
    SipTransportAddress  addr;

    /* In case host name is numeric IP address, we should use UDP */
    rv = TransportMgrConvertString2Address(pTrx->msgParams.strHostName,
                                           &addr, RV_TRUE,RV_TRUE);
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (rv != RV_OK)
    {
        rv = TransportMgrConvertString2Address(pTrx->msgParams.strHostName,
                                              &addr, RV_FALSE,RV_TRUE);
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    if((rv == RV_OK) || pTrx->msgParams.port != UNDEFINED)
    {
        if (pTrx->msgParams.bIsSecure == RV_FALSE)
        { 
            RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestProtocolGetImplicitlySpecified - pTrx=0x%p: host offset %d: destination is rawIP/port exists (p=%u) and 'sip' Uri -> transport is UDP",
                pTrx, pTrx->msgParams.hostNameOffset,pTrx->msgParams.port));
            pTrx->destAddr.eTransport = RVSIP_TRANSPORT_UDP;
        }
        else
        {
            RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestProtocolGetImplicitlySpecified - pTrx=0x%p: host offset %d: destination is rawIP/port exists (p=%u) and 'sips' Uri -> transport is TLS",
                pTrx, pTrx->msgParams.hostNameOffset,pTrx->msgParams.port));
            pTrx->destAddr.eTransport = RVSIP_TRANSPORT_TLS;
        }

        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND);
        return RV_OK;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolGetImplicitlySpecified - pTrx=0x%p: host offset %d: Transport not implicitly specified",
        pTrx, pTrx->msgParams.hostNameOffset));

    return RV_ERROR_NOT_FOUND;
}

/*****************************************************************************
* HandleUriFoundState
* ---------------------------------------------------------------------------
* General: checks if the URI that was found is an ip or not.
*          if not - try to locate a server connection with this URI as alias.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
* Output:  none
*****************************************************************************/
static RvStatus HandleUriFoundState( IN Transmitter* pTrx )
{
    RvStatus            rv;
    SipTransportAddress addr;
    RvSipTransportConnectionHandle hConn = NULL;

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "HandleUriFoundState - pTrx=0x%p: (host name offset %d)",
        pTrx,pTrx->msgParams.hostNameOffset));

    if (RVSIP_MSG_RESPONSE == pTrx->msgParams.msgType)
    {
        pTrx->destAddr.eTransport = pTrx->msgParams.transportType;  
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "HandleUriFoundState - pTrx=0x%p: (host name offset %d) response - transport taken from msg (t=%s)",
            pTrx,pTrx->msgParams.hostNameOffset, SipCommonConvertEnumToStrTransportType(pTrx->destAddr.eTransport)));
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND);
        return RV_OK;        
    }

    /* check if the URI is an IP or not */
    rv = TransportMgrConvertString2Address(pTrx->msgParams.strHostName,
        &addr, RV_TRUE,RV_TRUE);
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (rv != RV_OK)
    {
        rv = TransportMgrConvertString2Address(pTrx->msgParams.strHostName,
            &addr, RV_FALSE,RV_TRUE);
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
    
    if(rv != RV_OK)
    {
        /*in connection reuse feature, we may have inserted a connection with an alias
         equals to the host name. 
         In this case we have to check first if such an alias exists in the connection hash table. */
        rv = SearchConnectionByAlias(pTrx, &hConn);
        if(RV_OK == rv && NULL != hConn)
        {
            /* we found a connection for this host name. can stop the dns process */
            pTrx->bSendByAlias = RV_TRUE;
            rv = TransmitterAttachToConnection(pTrx, hConn);
            if(RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                    "HandleUriFoundState - pTrx=0x%p: Failed to attach on connection 0x%p",
                    pTrx, hConn));
            }
            rv = SipTransportConnectionGetRemoteAddress(hConn, &pTrx->destAddr);
            if(RV_OK != rv)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                    "HandleUriFoundState - pTrx=0x%p: Failed to get remote address from connection 0x%p",
                    pTrx, hConn));
            }

            /* set port to -1, because this is an ephemeral port */ 
            RvAddressSetIpPort(&pTrx->destAddr.addr, 0);

            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "HandleUriFoundState - pTrx=0x%p: found server conn 0x%p with alias offset %d",
                pTrx,hConn,pTrx->msgParams.hostNameOffset));
    
            DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVED);
            return RV_OK;
        }
    }

    /* activate the 'locating sip servers' DNS procedure */
    return DestProtocolInitialFind(pTrx);

}
/*****************************************************************************
* DestProtocolInitialFind
* ---------------------------------------------------------------------------
* General: Identifies the destination transport:
*          1. If found in list -> take from list
*          2. If Explicitly specified -> choose that (i.e. ";transport=xxx" or "sips"
*          3. If Implicitly specified -> choose that (i.e. port specified/raw ip)
*          4. Try to send NAPTR -> take result
*          5. If no NAPTR -> UDP for "sip", TLS for "sips"
*          transport or NAPTR.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestProtocolInitialFind(
    IN Transmitter* pTrx )
{
    RvStatus                           rv;
#ifdef UPDATED_BY_SPIRENT
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvSipTransportAddr localAddr;
#endif
#endif

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) protocol discovery procedure started",
        pTrx,pTrx->msgParams.hostNameOffset));

    /* Check if protocol was specified explicitly */
    rv = DestProtocolGetExplicitlySpecified(pTrx);
    if (rv == RV_OK)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) protocol was set explicitly in the URI",
            pTrx,pTrx->msgParams.hostNameOffset));
        return RV_OK;
    }
    else if (rv == RV_ERROR_BADPARAM)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) SIP URI contains incorrect protocol",
            pTrx,pTrx->msgParams.hostNameOffset));
        return RV_ERROR_UNKNOWN;
    }

    /* Check if protocol was specified implicitly in the SIP URI by
    using IP address or specifying port */
    rv = DestProtocolGetImplicitlySpecified(pTrx);
    if (rv == RV_OK)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) protocol was set implicitly",
            pTrx,pTrx->msgParams.hostNameOffset));
        return RV_OK;
    }


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    /*  In case no transport protocol specified & host name is alpha-numeric
        string, NAPTR DNS query should be used to discover protocol */
    rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestProtocolInitialFind - trx=0x%p: failed to get vdev block info",pTrx));
        return rv;
    }
    rv = SipResolverResolve(pTrx->hRslv,
                            RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_NAPTR,
                            pTrx->msgParams.strHostName,
                            RVSIP_RESOLVER_SCHEME_UNDEFINED,pTrx->msgParams.bIsSecure,0,RVSIP_TRANSPORT_UNDEFINED,
                            pTrx->hDnsList, DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

	/* The resolution state is changed in 2 cases: */ 
	/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
	/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
	/*    In this case the transmitter is ready to progress to the next query according to */ 
	/*    relevant state - it shouldn't wait to any report data callback. */ 
    if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) protocol by NAPTR initiated",
            pTrx,pTrx->msgParams.hostNameOffset));
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_NAPTR);
        return rv;
    }
    else
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) failed to resolve protocol by NAPTR",
            pTrx,pTrx->msgParams.hostNameOffset));
        return rv;
    }
    
#else /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolInitialFind - pTrx=0x%p: (host name offset %d) No EDNS, no specific protocol. deciding by Msg",
        pTrx,pTrx->msgParams.hostNameOffset));
    /* If we got here, the message contains a host name.
    In case host name is specified, but NAPTR/SRV procedure is
    disabled, UDP/TLS is chosen as the transport protocol, depending on the scheme*/
    rv = DestProtocolDecideByMsg(pTrx);
    return RV_OK;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/*****************************************************************************
* DestProtocolHandleEnumResult
* ---------------------------------------------------------------------------
* General: resolves the regular expression retrieve from the ENUM NAPTR query.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
*		   pStrRegExp  - The regular expression that was returned as enum
*						 result. 
* Output:  phUri       - The bulit URI according to the ENUM result
*****************************************************************************/
static RvStatus DestProtocolHandleEnumResult(IN  Transmitter        *pTrx,
											 IN  RvChar			    *pStrRegExp,
											 OUT RvSipAddressHandle *phUri) 
{
	RvStatus					rv;
	RvChar						strEre[RV_NAPTR_REGEXP_LEN];
    RvChar						strReplecment[RV_NAPTR_REGEXP_LEN];
	RvSipTransmitterRegExpMatch matches[TRANSMITTER_DNS_REG_MAX_MATCHES];
	RvSipTransmitterRegExpFlag  eFlags		= RVSIP_TRANSMITTER_REGEXP_FLAG_NONE;
	RvSipAddressType			eResultType = RVSIP_ADDRTYPE_UNDEFINED;
	RvChar                      strResolvedAddr[RV_DNS_MAX_NAME_LEN];

	*phUri = NULL;
	memset(matches,-1,sizeof(matches));
	
	rv = DestGetRegexpGetCallbackStrings(pTrx,pStrRegExp,strEre,strReplecment,&eFlags);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleEnumResult - pTrx=0x%p: can not break regexp into seperate values(rv=%d)",
            pTrx,rv));
        return RV_ERROR_BADPARAM;
    }
	
    rv = TransmitterCallbacksResolveApplyRegExpCb(
		pTrx,strEre,pTrx->strQuery,TRANSMITTER_DNS_REG_MAX_MATCHES,eFlags,matches);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleEnumResult - pTrx=0x%p: could not apply regexpp(rv=%d)",
            pTrx,rv));
        return RV_ERROR_BADPARAM;
    }

    rv = DestResolveNaptrRegexp(pTrx,strReplecment,pTrx->strQuery,matches,strResolvedAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleEnumResult - pTrx=0x%p: could not apply regexpp(rv=%d)",
            pTrx,rv));
        return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolHandleEnumResult - pTrx=0x%p: resolved ENUM = %s",
        pTrx,strResolvedAddr));
    eResultType = SipAddrGetAddressTypeFromString(pTrx->pTrxMgr->hMsgMgr, strResolvedAddr);
    if (RVSIP_ADDRTYPE_URL != eResultType)
    {
        RvSipAddressHandle  hResultTel = NULL;
        RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleEnumResult - pTrx=0x%p: can only resolve ENUM to URL",
            pTrx));
        if (RVSIP_ADDRTYPE_TEL == eResultType)
        {
            rv = RvSipAddrConstruct(
				pTrx->pTrxMgr->hMsgMgr,pTrx->hPool,pTrx->hPage,RVSIP_ADDRTYPE_TEL,&hResultTel);
            if (RV_OK == rv)
            {
                rv = RvSipAddrParse(hResultTel,strResolvedAddr);
                if (RV_OK != rv)
                {
                    RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestProtocolHandleEnumResult - pTrx=0x%p: could not parse tel address for enumdi checks(rv=%d)",
                        pTrx,rv));
                }
                if (RV_OK == rv && RV_TRUE == RvSipAddrIsEqual(hResultTel,pTrx->hNextMessageHop))
                {
                /* TEL URL that is the same as the one we sent, we add the enumdi param as per
                    draft-stastny-iptel-tel-enumdi-00.txt [4.2] bullet 1*/
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestProtocolHandleEnumResult - pTrx=0x%p:adding enumdi to tel url (same TEL URL received)",pTrx));
                    RvSipAddrTelUriSetEnumdiParamType(pTrx->hNextMessageHop,RVSIP_ENUMDI_TYPE_EXISTS_EMPTY);
                }
                if (RV_OK == rv && RVSIP_ENUMDI_TYPE_EXISTS_EMPTY == RvSipAddrTelUriGetEnumdiParamType(hResultTel))
                {
                /* TEL URL with enumdi, we add the enumdi param as per
                    draft-stastny-iptel-tel-enumdi-00.txt [4.2] bullet 2*/
                    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestProtocolHandleEnumResult - pTrx=0x%p:ading enumdi to tel url (enumdi in TEL received)",pTrx));
                    RvSipAddrTelUriSetEnumdiParamType(pTrx->hNextMessageHop,RVSIP_ENUMDI_TYPE_EXISTS_EMPTY);
                }
            }
            else
            {
                RvLogWarning(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestProtocolHandleEnumResult - pTrx=0x%p: could not construct tel address for enumdi checks (rv=%d)",
                    pTrx,rv));
            }
        }
		
		/* Return error code since the result was not URL */
        return RV_ERROR_BADPARAM;
    }

    if (NULL == *phUri)
    {
        rv = RvSipAddrConstruct(
				pTrx->pTrxMgr->hMsgMgr,pTrx->hPool,pTrx->hPage,RVSIP_ADDRTYPE_URL,phUri);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestProtocolHandleEnumResult - pTrx=0x%p: could not build Uri(rv=%d)",
                pTrx,rv));
            return RV_ERROR_BADPARAM;
        }
        rv = RvSipAddrParse(*phUri,strResolvedAddr);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestProtocolHandleEnumResult - pTrx=0x%p: could not parse address %s(rv=%d)",
                pTrx,strResolvedAddr, rv));
            return RV_ERROR_BADPARAM;
        }
    }

	return RV_OK;
}

/*****************************************************************************
* DestProtocolHandleUriResult
* ---------------------------------------------------------------------------
* General: resolves the regular expression retrieve from the ENUM NAPTR query,
*		   if exists or call the OtherUrlFound callback
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestProtocolHandleUriResult(IN Transmitter* pTrx)
{
    RvStatus            rv                  = RV_OK;
    RvSipAddressHandle  hUri                = NULL;
    RvUint              actualLen           = 0;
    RvChar*             pStrRegExp          = NULL;
	RvBool				bAddressResolved = RV_FALSE;
    RvChar              strUrlHost[RV_DNS_MAX_NAME_LEN];
  
    
    rv = SipTransportDnsGetEnumResult(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&pStrRegExp);
    if (RV_ERROR_NOT_FOUND == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleUriResult - pTrx=0x%p:Did not get result from URI query",
                pTrx));

        /* the DNS querey failed, we add the enumdi param as per
           draft-stastny-iptel-tel-enumdi-00.txt [4.2] bullet 0*/
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleUriResult - pTrx=0x%p:adding enumdi to tel url",pTrx));
        RvSipAddrTelUriSetEnumdiParamType(pTrx->hNextMessageHop,RVSIP_ENUMDI_TYPE_EXISTS_EMPTY);
    }
    else if (RV_OK == rv)
    {
        rv = DestProtocolHandleEnumResult(pTrx,pStrRegExp,&hUri);
    }
	else 
	{ 
		RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"DestProtocolHandleUriResult - pTrx=0x%p: failure in SipTransportDnsGetEnumResult() (rv=%d)",
			pTrx, rv));
		return RV_ERROR_BADPARAM;
	}
    
	/* In 2 cases we would like to involve the application by OtherURLFound callback: */
	/* 1. If SipTransportDnsGetEnumResult() function returned RV_ERROR_NOT_FOUND.     */
	/* 2. If the function DestProtocolHandleEnumResult() returned an error.			  */
	if (RV_OK != rv) {
		rv = ConstructSipUrlAddrByTrx(
            pTrx,pTrx->hMsgToSend,pTrx->hNextMessageHop,&hUri,&bAddressResolved);
        if (RV_OK != rv || RV_FALSE == bAddressResolved)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestProtocolHandleUriResult - pTrx=0x%p: (host name offset %d) can not determine next URI: (rv=%d,bAddressResolved=%d)",
                pTrx,pTrx->msgParams.hostNameOffset,rv,bAddressResolved));
            return RV_ERROR_BADPARAM;
        }
	}


    strUrlHost[0] = '\0';

    rv = RvSipAddrUrlGetHost(hUri,strUrlHost,RV_DNS_MAX_NAME_LEN,&actualLen);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleUriResult - pTrx=0x%p: can not get host(rv=%d)",
            pTrx, rv));
        return RV_ERROR_BADPARAM;
    }

    rv = TransmitterDestAllocateHostBuffer(pTrx, actualLen,
                                           &pTrx->msgParams.strHostName, 
                                           &pTrx->msgParams.hostNameOffset);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolHandleUriResult - pTrx=0x%p: can not allocate space for URI: (rv=%d)",
            pTrx,rv));
        return RV_ERROR_BADPARAM;
    }

    strcpy(pTrx->msgParams.strHostName,strUrlHost);
    pTrx->msgParams.port          = RvSipAddrUrlGetPortNum(hUri);
    pTrx->msgParams.bIsSecure     = RvSipAddrUrlIsSecure(hUri);
    pTrx->msgParams.transportType = RvSipAddrUrlGetTransport(hUri);
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolHandleUriResult - pTrx=0x%p: URI params: host offset =%d, port=%d,bSecure=%d,transport=%s",
        pTrx,pTrx->msgParams.hostNameOffset,pTrx->msgParams.port,pTrx->msgParams.bIsSecure,
        SipCommonConvertEnumToStrTransportType(pTrx->msgParams.transportType)));
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_URI_FOUND);
	
    return RV_OK;
}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */ 

/*****************************************************************************
* DestProtocolDecideByMsg
* ---------------------------------------------------------------------------
* General: Identifies the destination transport, according to message 
*          transport or NAPTR.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestProtocolDecideByMsg(
    IN Transmitter* pTrx )
{
    if (pTrx->msgParams.bIsSecure == RV_FALSE)
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolDecideByMsg - pTrx=0x%p: (host name offset %d) No EDNS, no spesific protocol. Transport protocol becomes UDP",
            pTrx,pTrx->msgParams.hostNameOffset));
        pTrx->destAddr.eTransport = RVSIP_TRANSPORT_UDP;
    }
    else
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolDecideByMsg - pTrx=0x%p: (host name offset %d) No EDNS, no spesific protocol, bSecure=1. Transport protocol becomes TLS",
            pTrx,pTrx->msgParams.hostNameOffset));
        pTrx->destAddr.eTransport = RVSIP_TRANSPORT_TLS;
    }
    DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_TRANSPORT_FOUND);

    return RV_OK;
}

/*****************************************************************************
* DestProtocolFindBySrv
* ---------------------------------------------------------------------------
* General: Identifies the destination transport, according to message 
*          transport or NAPTR.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transport transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestProtocolFindBySrv(
    IN Transmitter* pTrx )
{
    RvStatus rv = RV_OK;
#ifdef UPDATED_BY_SPIRENT
    RvSipTransportAddr localAddr;
#endif

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestProtocolFindBySrv - pTrx=0x%p: (host name offset %d) SRV protocol discovery",
        pTrx,pTrx->msgParams.hostNameOffset));
#ifdef UPDATED_BY_SPIRENT
    rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestProtocolFindBySrv- trx=0x%p: failed to get vdev block info",pTrx));
        return rv;
    }
#endif

    rv = SipResolverResolve(pTrx->hRslv,
                            RVSIP_RESOLVER_MODE_FIND_TRANSPORT_BY_3WAY_SRV,
                            pTrx->msgParams.strHostName,
                            DestCreateScheme(pTrx->hNextMessageHop),
                            pTrx->msgParams.bIsSecure,0,RVSIP_TRANSPORT_UNDEFINED,
                            pTrx->hDnsList, DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

	/* The resolution state is changed in 2 cases: */ 
	/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
	/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
	/*    In this case the transmitter is ready to progress to the next query according to */ 
	/*    relevant state - it shouldn't wait to any report data callback. */ 
    if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolFindBySrv - pTrx=0x%p: (host name offset %d) protocol doing 3way SRV",
            pTrx,pTrx->msgParams.hostNameOffset));
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_TRANSPORT_3WAY_SRV);
        return rv;
    }
    else
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestProtocolFindBySrv - pTrx=0x%p: (host name offset %d) failed to do 3way SRV",
            pTrx,pTrx->msgParams.hostNameOffset));
        return rv;
        
    }
}

/*****************************************************************************
* DestCreateScheme
* ---------------------------------------------------------------------------
* General: create a scheme string for DNS query.
*          Currently this function is "empty" but with more schemes, it will grow
* Return Value: RvChar* a scheme for SRV query
* ---------------------------------------------------------------------------
* Arguments:
* Input:   - (no input yet)
* Output:  none
*****************************************************************************/
static RvSipResolverScheme DestCreateScheme(RvSipAddressHandle hAddress)
{
    RvSipAddressType  eType = RvSipAddrGetAddrType(hAddress);
    switch (eType)
    {
    case RVSIP_ADDRTYPE_URL:
        return RVSIP_RESOLVER_SCHEME_SIP;
    case RVSIP_ADDRTYPE_PRES:
        return RVSIP_RESOLVER_SCHEME_PRES;
    case RVSIP_ADDRTYPE_IM:
        return RVSIP_RESOLVER_SCHEME_IM;
    default:
        return RVSIP_RESOLVER_SCHEME_SIP;
    }
}


/*****************************************************************************
* DestStateResolved
* ---------------------------------------------------------------------------
* General: notifies the application that the resolution was done
* Return Value: RvStatus - RV_ERROR_NOT_FOUND - application reset the DNS result
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestStateResolved(IN Transmitter* pTrx)
{
	RvStatus rv; 

	rv = TransmitterMoveToStateFinalDestResolved(
                            pTrx, RVSIP_TRANSMITTER_REASON_UNDEFINED,
                            pTrx->hMsgToSend, NULL);
	if (rv != RV_OK)
	{
		RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "DestStateResolved - pTrx=0x%p: Failed to promote to FinalDestResolved",pTrx));
		rv = TransmitterMoveToMsgSendFaiureIfNeeded(
			pTrx,rv,RVSIP_TRANSMITTER_REASON_NETWORK_ERROR,RV_FALSE);
		return rv;
	}
	
    if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
            "DestStateResolved - pTrx=0x%p: was terminated/set on hold from callback. returning from function",
            pTrx));
        return RV_OK;
    }
    
    if (TRANSMITTER_RESOLUTION_STATE_UNDEFINED == pTrx->eResolutionState)
    {
        /* The application requested a new DNS resolution process */
        TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR,
            RVSIP_TRANSMITTER_REASON_UNDEFINED,pTrx->hMsgToSend,NULL);
        
        /*the transmitter was terminated from callback*/
        if(pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc  ,
                "DestStateResolved - pTrx=0x%p: was terminated/set on hold from callback. returning from function",
                pTrx));
            return RV_OK;
        }
        return RV_ERROR_NOT_FOUND;
    }
    return RV_OK;
}

/*****************************************************************************
* locateLocalAddressAndConn
* ---------------------------------------------------------------------------
* General: Find a local address for a specific transport type, and then find
*          a server connection with those identifiers.
*          look for ip and ipv6 local addresses.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvStatus locateLocalAddressAndConn(
                           IN Transmitter*                  pTrx,
                           IN  RvSipTransport              eTransportType,
                           OUT RvSipTransportConnectionHandle *phConn)
{
    RvSipTransportLocalAddrHandle *phLocalAddress = NULL;
    
    /* take hLocalAddress for ipv4 */
    TransmitterGetLocalAddressHandleByTypes(pTrx, 
                                            eTransportType, 
                                            RVSIP_TRANSPORT_ADDRESS_TYPE_IP, 
                                            &phLocalAddress);
    if(NULL != phLocalAddress && NULL != *phLocalAddress)
    {
        /* found a local address - search for a connection */
        SipTransportConnectionHashFindByAlias(pTrx->pTrxMgr->hTransportMgr,
                                            eTransportType,
                                            *phLocalAddress,
                                            pTrx->hPool,
                                            pTrx->hPage,
                                            pTrx->msgParams.hostNameOffset,
                                            phConn);
        if(*phConn != NULL)
        {
            RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                "locateLocalAddressAndConn: Found connection 0x%p for host offset %d", 
                *phConn, pTrx->msgParams.hostNameOffset));
            return RV_OK;
        }
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    /* take hLocalAddress for ipv6 */
    TransmitterGetLocalAddressHandleByTypes(pTrx, 
                                            eTransportType, 
                                            RVSIP_TRANSPORT_ADDRESS_TYPE_IP6, 
                                            &phLocalAddress);
    if(NULL != phLocalAddress && NULL != *phLocalAddress)
    {
        /* found a local address - search for a connection */
        SipTransportConnectionHashFindByAlias(pTrx->pTrxMgr->hTransportMgr,
                                            eTransportType,
                                            *phLocalAddress,
                                            pTrx->hPool,
                                            pTrx->hPage,
                                            pTrx->msgParams.hostNameOffset,
                                            phConn);
        if(*phConn != NULL)
        {
            RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                "locateLocalAddressAndConn: Found connection 0x%p for host offset %d", 
                *phConn, pTrx->msgParams.hostNameOffset));
            return RV_OK;
        }
    }
#endif /*#if(RV_NET_TYPE & RV_NET_IPV6)*/
    return RV_ERROR_NOT_FOUND;
}
/*****************************************************************************
* SearchConnectionByAlias
* ---------------------------------------------------------------------------
* General: Search a connection with alias equal to the found host name.
*          if the transport type is specified - search for the specific transport.
*          else - search separatly for TCP, TLS, SCTP.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvStatus SearchConnectionByAlias(IN Transmitter* pTrx,
                                        OUT RvSipTransportConnectionHandle *phConn)
{
    RvStatus rv = RV_OK;
	TransportMgr  *trnsMgr;
    
    if(RV_FALSE == SipTransportMgrIsSupportServerConnReuse(pTrx->pTrxMgr->hTransportMgr))
    {
        return RV_ERROR_NOT_FOUND;
    }

	/* no point to look for connection, if TCP is not enabled */
	trnsMgr = (TransportMgr *)(pTrx->pTrxMgr->hTransportMgr);
	if (RV_FALSE == trnsMgr->tcpEnabled)
	{
		return RV_ERROR_NOT_FOUND;
	}

	/* no connection reuse for UDP */
	if (pTrx->msgParams.transportType == RVSIP_TRANSPORT_UDP)
	{
		return RV_ERROR_NOT_FOUND;
	}

    /* if the transport type is specified - search for such a connection */
    if(pTrx->msgParams.transportType != RVSIP_TRANSPORT_UNDEFINED)
    {
        rv = locateLocalAddressAndConn(pTrx,pTrx->msgParams.transportType, phConn);
        if(rv == RV_OK)
        {
            RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                "SearchConnectionByAlias: Found connection 0x%p for host offset %d", 
                *phConn, pTrx->msgParams.hostNameOffset));
            return RV_OK;
        }
        else
        {
            return RV_ERROR_NOT_FOUND;
        }
    }
    if(pTrx->msgParams.bIsSecure == RV_TRUE)
    {
        rv = locateLocalAddressAndConn(pTrx,RVSIP_TRANSPORT_TLS, phConn);
        if(rv == RV_OK)
        {
            RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
                "SearchConnectionByAlias: Found connection 0x%p for host offset %d", 
                *phConn, pTrx->msgParams.hostNameOffset));
            return RV_OK;
        }
        else
        {
            return RV_ERROR_NOT_FOUND;
        }
    }

        
    /* if the transport type is not specified - search a server connection 
    for TCP, TLS, CSTP */

    /* TCP */
    rv = locateLocalAddressAndConn(pTrx,RVSIP_TRANSPORT_TCP, phConn);
    if(rv == RV_OK)
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
            "SearchConnectionByAlias: Found connection 0x%p for host offset %d", 
            *phConn, pTrx->msgParams.hostNameOffset));
        return RV_OK;
    }


    /* TLS */
#if (RV_TLS_TYPE != RV_TLS_NONE)
    rv = locateLocalAddressAndConn(pTrx,RVSIP_TRANSPORT_TLS, phConn);
    if(rv == RV_OK)
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
            "SearchConnectionByAlias: Found connection 0x%p for host offset %d", 
            *phConn, pTrx->msgParams.hostNameOffset));
        return RV_OK;
    }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

    /* CSTP */
#if (RV_NET_TYPE & RV_NET_SCTP)
    rv = locateLocalAddressAndConn(pTrx,RVSIP_TRANSPORT_SCTP, phConn);
    if(rv == RV_OK)
    {
        RvLogInfo(pTrx->pTrxMgr->pLogSrc, (pTrx->pTrxMgr->pLogSrc,
            "SearchConnectionByAlias: Found connection 0x%p for host offset %d", 
            *phConn, pTrx->msgParams.hostNameOffset));
        return RV_OK;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    return RV_ERROR_NOT_FOUND;
}
/*****************************************************************************
* DestHostPortDiscovery
* ---------------------------------------------------------------------------
* General: Identifies the destination host address and port number
*          1. see if SRV records are found
*          2. see if address is raw IP
*          3. 
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_TRY_AGAIN - specifies that asynchronous DNS
*                                          procedure was applied
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvStatus DestHostPortDiscovery(IN Transmitter* pTrx)
{
    RvStatus rv = RV_OK;
#ifdef UPDATED_BY_SPIRENT
    RvSipTransportAddr localAddr;
#endif
    
    /* First try to retrieve the Next Hop for case when host name is raw IP address*/
    rv = DestPortIPDiscoveryByRawIP(pTrx);
    if(rv == RV_OK)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestHostPortDiscovery - pTrx=0x%p: (message name offset %d) IP and port discovered by Raw IP",
            pTrx,pTrx->msgParams.hostNameOffset));
            DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVED);
        return RV_OK; /* no asynchronous procedure in this case */
    }

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    

    /* In case host name is not raw IP address and port is unspecified
    SRV routine should be applied. */
    if (pTrx->msgParams.port == UNDEFINED && pTrx->msgParams.transportType != RVSIP_TRANSPORT_UNDEFINED)
    {
#ifdef UPDATED_BY_SPIRENT
        rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                        "DestHostPortDiscovery - trx=0x%p: failed to get vdev block info",pTrx));
            return rv;
        }
#endif
        rv = SipResolverResolve(pTrx->hRslv,RVSIP_RESOLVER_MODE_FIND_HOSTPORT_BY_TRANSPORT,
                                pTrx->msgParams.strHostName,DestCreateScheme(pTrx->hNextMessageHop),
                                pTrx->msgParams.bIsSecure,0,
                                pTrx->destAddr.eTransport,
                                pTrx->hDnsList, DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

		/* The resolution state is changed in 2 cases: */ 
		/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
		/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
		/*    In this case the transmitter is ready to progress to the next query according to */ 
		/*    relevant state - it shouldn't wait to any report data callback. */ 
        if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
        {
            DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_URI_HOSTPORT);
            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestHostPortDiscovery - trx=0x%p: (message name offset %d) resolving port host by SRV",
                pTrx,pTrx->msgParams.hostNameOffset));
            return rv;
        }
        else
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "DestHostPortDiscovery - trx=0x%p: (message name offset %d) failed in resolving port host by SRV",
                pTrx,pTrx->msgParams.hostNameOffset));
            return rv;
        }
    }
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

    /* we got here:
       host is not raw IP or port is specified,
       set the port from message and ask for the name.
    */
    RvAddressSetIpPort(&pTrx->destAddr.addr,DestChoosePort(pTrx));
#ifdef UPDATED_BY_SPIRENT
    rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
    if (RV_OK != rv)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestHostPortDiscovery - trx=0x%p: failed to get vdev block info",pTrx));
        return rv;
    }
#endif
    
    rv = SipResolverResolve(pTrx->hRslv,RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST,
        pTrx->msgParams.strHostName,RVSIP_RESOLVER_SCHEME_UNDEFINED,
        pTrx->msgParams.bIsSecure,RvAddressGetIpPort(&pTrx->destAddr.addr),
        pTrx->destAddr.eTransport,
        pTrx->hDnsList, DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

	/* The resolution state is changed in 2 cases: */ 
	/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
	/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
	/*    In this case the transmitter is ready to progress to the next query according to */ 
	/*    relevant state - it shouldn't wait to any report data callback. */ 
    if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
    {
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_IP);
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestHostPortDiscovery - pTrx=0x%p: Waiting for DNS response",pTrx));
        return rv;
    }
    else 
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestHostPortDiscovery - trx=0x%p: (message name offset %d) failed resolving ip by host",
            pTrx,pTrx->msgParams.hostNameOffset));
        return rv;
    }
}

/*****************************************************************************
* DestPortIPDiscoveryByRawIP
* ---------------------------------------------------------------------------
* General: Translates raw IP address string to binary IP address and
*          calculates port & protocol.
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   strHostName              - Host name
*          transportType        - transport protocol
*                                 results
* Output:  pAddress             - IP address
*****************************************************************************/
static RvStatus DestPortIPDiscoveryByRawIP(
    IN  Transmitter* pTrx)
{
    SipTransportAddress         addr;

    RvAddressConstruct(RV_ADDRESS_TYPE_IPV4,&addr.addr);
    /* In case host name is numeric IP address, no name resolution is required */
    if (RV_OK != TransportMgrConvertString2Address(pTrx->msgParams.strHostName,&addr, RV_TRUE,RV_FALSE))
    {
#if(RV_NET_TYPE & RV_NET_IPV6)
        RvAddressDestruct(&addr.addr);
        /* the address was not ipv4 raw ip, maybe it is ipv6 raw ip? */
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV6,&addr.addr);
        if (RV_OK != TransportMgrConvertString2Address(pTrx->msgParams.strHostName,&addr, RV_FALSE,RV_FALSE))
        {
            RvAddressDestruct(&addr.addr);
            return RV_ERROR_UNKNOWN;
        }
#else /*(RV_NET_TYPE & RV_NET_IPV6)*/
        return RV_ERROR_UNKNOWN;
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/
    }


    /* Copy the resolved destination IP address */
    RvAddressCopy(&addr.addr,&pTrx->destAddr.addr);

    RvAddressSetIpPort(&pTrx->destAddr.addr,DestChoosePort(pTrx));

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestPortIPDiscoveryByRawIP - pTrx=0x%p: IP and port discovered directly by the raw IP host name. (host offset =%d,port=%d)",
        pTrx, pTrx->msgParams.hostNameOffset,
        RvAddressGetIpPort(&pTrx->destAddr.addr)));
    return RV_OK;
}

/*****************************************************************************
* DestChoosePort
* ---------------------------------------------------------------------------
* General: chosses a port from message/default for protocol.
* Return Value: port to use
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTrx        - Transmitter
* Output:  none
*****************************************************************************/
static RvUint16 RVCALLCONV DestChoosePort(
    IN  Transmitter* pTrx)
{
    /*  In case port was not specified, default port value
    should be used  */

    RvUint16 destAddrPort = 0;
	
	

    if (pTrx->msgParams.port == UNDEFINED)
    {
        destAddrPort = (RvUint16)((pTrx->msgParams.bIsSecure == RV_TRUE || pTrx->msgParams.transportType == RVSIP_TRANSPORT_TLS) ? RVSIP_TRANSPORT_DEFAULT_TLS_PORT : RVSIP_TRANSPORT_DEFAULT_PORT);
    }
    else
    {
        destAddrPort = (RvUint16)pTrx->msgParams.port;
    }
    return destAddrPort;
}

/************************************************************************************
 * DestResolutionStateChange
 * ----------------------------------------------------------------------------------
 * General: change the state of the resolution process
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN, RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx        -- transmitter pointer
 *          eNewState   -- new resolution state
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV DestResolutionStateChange(
    IN Transmitter* pTrx,
    IN TransmitterResolutionState    eNewState)
{
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestResolutionStateChange - pTrx=0x%p: host offset %d: Resolution state change %s->%s",
        pTrx, pTrx->msgParams.hostNameOffset, 
        TransmitterGetResolutionStateName(pTrx->eResolutionState),
        TransmitterGetResolutionStateName(eNewState)));
    pTrx->eResolutionState = eNewState;
}

/***************************************************************************
 * DestDataReportedEvHandler
 * ------------------------------------------------------------------------
 * General: 
 * Return Value: Ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    owner   -   The owner of the resolver
 *           
 ***************************************************************************/
static RvStatus RVCALLCONV DestDataReportedEvHandler(
                              IN RvSipResolverHandle       hResolver,
                              IN RvSipAppResolverHandle    hOwner,
                              IN RvBool                  bError,
                              IN RvSipResolverMode         eMode)
{
    Transmitter* pTrx = (Transmitter*)hOwner;
    
    TransmitterDestDiscovery(pTrx,RV_FALSE);
    RV_UNUSED_ARG(eMode);
    RV_UNUSED_ARG(bError);
    RV_UNUSED_ARG(hResolver);
    return RV_OK;
}

/***************************************************************************
 * DestResolveIpByHost
 * ------------------------------------------------------------------------
 * General: 
 * Return Value: Ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    owner   -   The owner of the resolver
 *           
 ***************************************************************************/
static RvStatus RVCALLCONV DestResolveIpByHost(
                              IN  Transmitter *pTrx,
                              IN  RvChar      *strHost,
							  IN  RvUint16	   port)
{
    RvStatus rv = RV_OK;
#ifdef UPDATED_BY_SPIRENT
    RvSipTransportAddr localAddr;
#endif
    
#ifdef UPDATED_BY_SPIRENT
    rv = SipTransmitterGetLocalAddr(pTrx,&localAddr);
    if(RV_OK != rv){
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestResolveIpByHost - trx=0x%p: SipTransmitterGetLocalAddr failed. (rv=%d)",
                    pTrx,rv));
        return rv;
    }
#endif
    rv = SipResolverResolve(pTrx->hRslv,RVSIP_RESOLVER_MODE_FIND_IP_BY_HOST,
        strHost,RVSIP_RESOLVER_SCHEME_UNDEFINED,
        pTrx->msgParams.bIsSecure,port,
        pTrx->destAddr.eTransport,
        pTrx->hDnsList,DestDataReportedEvHandler
#if defined(UPDATED_BY_SPIRENT)
		    , &localAddr
#endif
);

	/* The resolution state is changed in 2 cases: */ 
	/* 1. The resolution is NOT completed yet - it's in the middle of asynchronous session */ 
	/* 2. An inline ERROR was returned due to the Common Core negative caching response.   */
	/*    In this case the transmitter is ready to progress to the next query according to */ 
	/*    relevant state - it shouldn't wait to any report data callback. */ 
    if (RV_ERROR_TRY_AGAIN == rv || RV_DNS_ERROR_NOTFOUND == rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "DestResolveIpByHost - trx=0x%p: (message name offset %d) resolving IP by host (rv=%d)",
            pTrx,pTrx->msgParams.hostNameOffset,rv));
        DestResolutionStateChange(pTrx,TRANSMITTER_RESOLUTION_STATE_RESOLVING_IP);
        return rv;
    }
    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestResolveIpByHost - trx=0x%p: (message name offset %d) error resolving IP by host (rv=%d)",
        pTrx,pTrx->msgParams.hostNameOffset,rv));
    return rv;
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/************************************************************************************
 * DestResolveNaptrRegexp
 * ---------------------------------------------------------------------------
 * General: resolves the regular expression to a string bt applying substitutions
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   pTrx     - Transmitter
 *          strRepExp - the replacement string
 *          strAus    - the application unique string
 *          pMatches  - the matches array
 * output:  strResolved  - string after substitutions
 ***********************************************************************************/
static RvStatus RVCALLCONV DestResolveNaptrRegexp(
             IN  Transmitter* pTrx,
             IN  RvChar*   strRepExp,
             IN  RvChar*   strAus,
             IN  RvSipTransmitterRegExpMatch* pMatches,
             OUT RvChar*   strResolved)
{
    RvInt32                     i   = 0;
    RvInt32                     j   = 0;
    RvInt32                     resIndex   = 0;
    RvInt32                     ausLen     = 0;
    RvInt32                     strLen     = 0;
    RvUint32                    curMatch   = 0;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTrx);
#endif

    ausLen = (RvInt32)strlen(strAus);
    strLen = (RvInt32)strlen(strRepExp);

    /* before the match starts */
    for (i = 0;i< pMatches[0].startOffSet ;i++)
    {
        strResolved[resIndex++] = strAus[i];
    }

    for (i=0;i<strLen;i++)
    {
        if('\\' == strRepExp[i] && 0 < sscanf(strRepExp+i+1,"%d",&curMatch))
        {
            if (curMatch> 9 || curMatch < 1)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestResolveNaptrRegexp - pTrx=0x%p. regexp:%s. backref out of range:%d",
                    pTrx,strResolved, curMatch));
                return RV_ERROR_BADPARAM;
            }
            if (-1 == pMatches[curMatch].startOffSet)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                    "DestResolveNaptrRegexp - pTrx=0x%p. regexp:%s. non existing backref:%d",
                    pTrx,strResolved, curMatch));
                return RV_ERROR_BADPARAM;
            }
            for (j=pMatches[curMatch].startOffSet;j<pMatches[curMatch].endOffSet;j++)
            {
                strResolved[resIndex++] = strAus[j];
            }
            i++;
        }
        else
        {
            strResolved[resIndex++] = strRepExp[i];
        }
    }
    
    /* after the match ends */
    for (i = pMatches[0].endOffSet;i< ausLen ;i++)
    {
        strResolved[resIndex++] = strAus[i];
    }
    
    strResolved[resIndex++] = '\0';
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestResolveNaptrRegexp - pTrx=0x%p. regexp:%s",
        pTrx,strResolved));
    return RV_OK;
}


/************************************************************************************
 * DestGetRegexpGetCallbackStrings
 * ---------------------------------------------------------------------------
 * General: breaks the NAPTR regexp into seperate components 
 *          !ere!rep!flags
 * Return Value: RvStatus - RvStatus
 * Arguments:
 * Input:   pTrx     - Transmitter
 *          strFullString - full string as was received from the NAPTR record
 * output:  strEre  - the regular expression
 *          strReplecment - the replacment string
 *          peFlags       - regexp flags
 ***********************************************************************************/
static RvStatus RVCALLCONV DestGetRegexpGetCallbackStrings(
             IN  Transmitter* pTrx,
             IN  RvChar*   strFullString,
             OUT RvChar*   strEre,
             OUT RvChar*   strReplecment,
             OUT RvSipTransmitterRegExpFlag* peFlags)
{
    /* testing the validity of the received NAPTR record */
    RvChar      flags[RV_NAPTR_REGEXP_LEN];
    RvInt32     i           = 0;
    RvInt32     strlocation = 0;
    RvChar      delim       = '\0';
    
    /* resetting the strings*/
    strEre[0]           = '\0';
    strReplecment[0]    = '\0';
    flags[0]            = '\0';
    /* setting the delimiter as the first char returned from the NAPTR regexp field */
    delim = strFullString[0];
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestGetRegexpGetCallbackStrings - pTrx=0x%p. delimiter=%c",
        pTrx,delim));

    for (i=1;i<RV_NAPTR_REGEXP_LEN;i++)
    {
        if ('\0' == strFullString[i] || 
            delim == strFullString[i])
        {
            strEre[strlocation++]='\0';
            break;
        }
        else
        {
            strEre[strlocation++]=strFullString[i];
        }
    }
    i++;
    strlocation = 0;
    for (;i<RV_NAPTR_REGEXP_LEN;i++)
    {
        if ('\0' == strFullString[i] || 
            delim == strFullString[i])
        {
            strReplecment[strlocation++]='\0';
            break;
        }
        else
        {
            strReplecment[strlocation++]=strFullString[i];
        }
    }
    i++;
    strlocation = 0;
    for (;i<RV_NAPTR_REGEXP_LEN;i++)
    {
        if ('\0' == strFullString[i] || 
            delim == strFullString[i])
        {
            flags[strlocation++]='\0';
            break;
        }
        else
        {
            flags[strlocation++]=strFullString[i];
        }
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "DestGetRegexpGetCallbackStrings - pTrx=0x%p. ere=%s repstring=%s flags=%s",
        pTrx,strEre,strReplecment,flags));
    if ('i' == flags[0])
    {
        *peFlags |= RVSIP_TRANSMITTER_REGEXP_FLAG_NO_CASE;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTrx);
#endif

	return RV_OK;
}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */























