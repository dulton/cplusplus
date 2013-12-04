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
 *                          <CallLegHighAvailability_3_0.c>
 *
 *  The following functions are for 3.0 version High Availability use.
 *  It means that user can get all the nessecary parameters of
 *  a connected call, (which is not in the middle of a refer or re-invite proccess),
 *  and copy it to another list of calls - of another call manager.
 *
 * IMPORTANT: This file MUSTN'T changed since in order to support backward
 *            compatability. If new data has to be stored it should be updated
 *            ONLY in CallLegHighAvailability.h,c files.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 November 2001
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef  RV_SIP_HIGH_AVAL_3_0
#ifndef RV_SIP_PRIMITIVES
#ifdef  RV_SIP_HIGHAVAL_ON

#include "rvrandomgenerator.h"
#include "RvSipCallLeg.h"
#include "CallLegHighAvailability.h"
#include "CallLegObject.h"
#include "rpool_API.h"
#include "RvSipAddress.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipBody.h"
#include "_SipAddress.h"
#include "_SipTransport.h"
#include "_SipPartyHeader.h"
#include "_SipMsg.h"
#include "RvSipMsg.h"
#include "RvSipHeader.h"
#include "RvSipSubscriptionTypes.h"
#include "RvSipSubscription.h"
#include "_SipSubs.h"
#include "_SipSubsMgr.h"
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* ConnectedCallParams
  ---------------------
   This structure contains all parameters offsets in the storage buffer.
  */

typedef struct
{
    RvInt32               fromHeaderOffset;
    RvInt32               toHeaderOffset;
    RvInt32               strCallIdOffset;

    RvInt32               localContactOffset;
    RvInt32               remoteContactOffset;

    RvInt32               localCSeq;
    RvInt32               remoteCSeq;
    RvSipCallLegDirection eDirection;

    RvInt32               numOfRoutes;
    RvInt32               routeListOffset;

    RvInt32               nonceCount401;
    RvInt32               nonceCount407;

    RvInt32               wwwAuthHeaderOffset;
    RvInt32               proxyAuthHeaderOffset;

    RvInt32               localAddrOffset;
    RvInt32               outboundProxyOffset;

    RvInt32               outboundMsgOffset;

    RvInt32               numOfSubscriptions;
    RvInt32               subscriptionListOffset;

    RvInt32               localAddrOfset;
}ConnectedCallParams;

/* subscriptionParams
   ------------------
   This structure contains a subscription object offsets in the storage buffer
   */
typedef struct
{
    RvSipSubsState        eSubsState;
    RvBool                bOutOfBand;
    RvSipSubscriptionType eSubsType;
    RvInt32               expiresVal;
    RvInt32               eventHeaderOffset;
    RvInt32               nextSubsOffset;

}subscriptionParams;

typedef enum
{
    CALL_LEG_HIGH_EVAL_NO_START_LINE,
    CALL_LEG_HIGH_EVAL_REQUEST,
    CALL_LEG_HIGH_EVAL_RESPONSE
}CallLegHighEvalStartLineType;

/* outboundMsgParams
   -----------------
   This structure contains the outbound message parameters offsets in the storage buffer.
   We must keep the message parameters manualy, because the outbound message is usually
   not finished, so encode and parse function receiveing this messge will fail.
  */
typedef struct
{
    CallLegHighEvalStartLineType eStartLineType;
    RvInt32 startLineEnum; /* eMethod for a request, statusCode for a response */
    RvInt32 startLineStrOffset; /* strMehod offset for a request,
                                    reasonPhrase offset for a response */
    RvSipAddressType eReqUriType; /* request-URI type. we need it for parsing the
                                  string in Restore function */
    RvInt32 requestUriOffset; /* request-URI of a request start-line */
    RvInt32 fromHeaderOffset;
    RvInt32 toHeaderOffset;
    RvInt32 cseqHeaderOffset;
    RvInt32 callIdOffset;
    RvInt32 numOfHeaders;
    RvInt32 headersListOffset;
    RvInt32 contentLength;
    RvInt32 bodyLength;
    RvInt32 bodyObjectOffset;
}outboundMsgParams;

/* localAddrParams
   ------------------
   This structure contains a trasport-local-address object offsets in the storage buffer
   */
typedef struct {
      RvInt32 localUdpIpv4Offset;
      RvInt32 localUdpIpv4PortOffset;
      RvInt32 localTcpIpv4Offset;
      RvInt32 localTcpIpv4PortOffset;
      RvInt32 localUdpIpv6Offset;
      RvInt32 localUdpIpv6PortOffset;
      RvInt32 localTcpIpv6Offset;
      RvInt32 localTcpIpv6PortOffset;
#if (RV_TLS_TYPE != RV_TLS_NONE)
      RvInt32 localTlsIpv4Offset;
      RvInt32 localTlsIpv4PortOffset;
      RvInt32 localTlsIpv6Offset;
      RvInt32 localTlsIpv6PortOffset;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

      RvInt32 addrInUseOffset;
} localAddrParams;

/* nonTLSToTLSRestoreLocalAddrParams
-------------------------------------
This structure contains a trasport-local-address object offsets in the storage buffer.
This structure was added in order to support the restoring of a Call-Leg, which was
stored in non-TLS SIP Stack application
*/
typedef struct {
	RV_INT32 localUdpIpv4Offset;  
	RV_INT32 localUdpIpv4PortOffset;  
	RV_INT32 localTcpIpv4Offset;
	RV_INT32 localTcpIpv4PortOffset;
	RV_INT32 localUdpIpv6Offset;
	RV_INT32 localUdpIpv6PortOffset;
	RV_INT32 localTcpIpv6Offset;
	RV_INT32 localTcpIpv6PortOffset;
	
	RV_INT32 addrInUseOffset;
} nonTLSToTLSRestoreLocalAddrParams;


/* outboundProxyParams
   ------------------
   This structure contains outbound proxy object offsets in the storage buffer
   */
typedef struct {
    RvInt32                rpoolHostNameOffset;
    RvInt32                bIpAddressExistsOffset;
    RvInt32                ipAddressOffset;
    RvInt32                bUseHostNameOffset;
} outboundProxyParams;


/* The obsolete RV_LI_AddressType that is stored in the 3.0 HighAval buffer */
typedef enum
{
  Obsolete_RV_LI_AddrUndefined = -1,
  Obsolete_RV_LI_Ip,
  Obsolete_RV_LI_Ip6
} Obsolete_RV_LI_AddressType;

/* The obsolete RV_LI_AddressUnion that is stored in the 3.0 HighAval buffer */
typedef union
{
    /* ipAddress */
    struct
    {
        RvInt32  ip;
    } Obsolete_ipAddr;

#if(RV_NET_TYPE & RV_NET_IPV6)
    /* ip6Address */
    struct
    {
        RvChar      ip[16];  /* the address in byte format */
        RvInt      scopeId; /* the scope id (interface) of the address */
    } Obsolete_ip6Addr;
#endif /* RV_CFLAG_IPV6 */

} Obsolete_RV_LI_AddressUnion;

/* The obsolete RV_LI_ConnectionType that is stored in the 3.0 HighAval buffer
   These definitions are used when a connection is opened in order to attach
   the type of protocol to it */

typedef enum
{
    Obsolete_RV_LI_ConnUndefined = -1,
    Obsolete_RV_LI_Udp,
    Obsolete_RV_LI_Tcp,
    Obsolete_RV_LI_Tls
}   Obsolete_RV_LI_ConnectionType;

/* The obsolete RV_LI_Address that is stored in the 3.0 HighAval buffer
   These definitions are used when a connection is opened in order to attach
   the type of protocol to it
   This is the Address struct. The data union should be the corresponding
   to the AddressType. */
typedef struct
{
    Obsolete_RV_LI_AddressType    type;
    Obsolete_RV_LI_AddressUnion   data;
    RvUint16                      port;
    Obsolete_RV_LI_ConnectionType transport;
} Obsolete_RV_LI_Address;

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RestoreRouteList(CallLeg*       pCallLeg,
                              RvInt32          numOfRoutes ,
                              RvChar*          routeList);

static RvStatus ListAddElement(IN  CallLeg      *pCallLeg,
                               IN RvChar*      pEncodedRoute,
                               IN RLIST_HANDLE  hList);

static RvStatus RestoreOutboundMsg(IN CallLeg             *pCallLeg,
                                   IN ConnectedCallParams *pCallParams,
                                   IN RvSipMsgHandle       hMsg,
                                   IN RvChar              *pBuff);

#ifdef RV_SIP_SUBS_ON
static RvStatus RestoreSubsList(IN CallLeg*       pCallLeg,
                                IN ConnectedCallParams* pCallParams,
                                IN RvChar*       pBuff);
#endif /* #ifdef RV_SIP_SUBS_ON */ 

static RvStatus RestoreLocalAddr(IN CallLeg*             pCallLeg,
                                 IN ConnectedCallParams* pCallParams,
                                 IN RvChar*             pBuff,
                                 IN RvSipCallLegHARestoreMode eHAmode);

static RvStatus RestoreIpAndPort(IN    RvSipTransportMgrHandle        hTransport,
                                 IN    RvSipTransport                 eTransport,
                                 IN    RvSipTransportAddressType      eAddrType,
                                 IN    RvChar                        *pIp,
                                 IN    RvChar                        *pPort,
                                 OUT   RvSipTransportLocalAddrHandle *phAddr);

static RvStatus RestoreOutboundProxyAddr(
                                  IN CallLeg             *pCallLeg,
                                  IN ConnectedCallParams *pCallParams,
                                  IN RvChar              *pBuff);

static RvStatus RestoreOutboundProxyIpAddr(
                                  IN CallLeg             *pCallLeg,
                                  IN outboundProxyParams *pOutboundParams,
                                  IN RvChar              *pBuff);

static RvInt ConvertObsoleteAddressTypeToNew(
                                  IN Obsolete_RV_LI_AddressType addrType);

static RvSipTransport ConvertObsoleteConnectionTypeToNew(
                                  IN Obsolete_RV_LI_ConnectionType transport);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegRestoreCall_3_0
 * ------------------------------------------------------------------------
 * General: Restore all call-leg information into a given call-leg. The call-leg
 *          will assume the connceted state and all call-leg parameters will be
 *          initialized from the given buffer.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not on state CONNECTED, or the
 *                                 refer state is not REFER_IDLE.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg    - Handle to the call-leg.
 *            memBuff     - The buffer that stores all the callLeg's parameters.
 ***************************************************************************/
RvStatus CallLegRestoreCall_3_0(IN CallLeg              *pCallLeg,
                                IN void                 *memBuff,
                                IN RvSipCallLegHARestoreMode eHAmode)
{
    RvStatus                         status;
    ConnectedCallParams              callParams;
    RvChar                          *pBuff        = (RvChar*)memBuff;
    HRPOOL                           hPool        = pCallLeg->pMgr->hGeneralPool;
    RvSipAddressType                 eStrAddrType = RVSIP_ADDRTYPE_UNDEFINED;

    memcpy(&callParams, memBuff, sizeof(ConnectedCallParams));

    /* from header */
    status = RvSipPartyHeaderConstruct(pCallLeg->pMgr->hMsgMgr, hPool, pCallLeg->hPage, &pCallLeg->hFromHeader);
    if(status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to construct from header Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    status = SipPartyHeaderSetType(pCallLeg->hFromHeader, RVSIP_HEADERTYPE_FROM);
    if(status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to set from type in party header Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    status = RvSipPartyHeaderParse(pCallLeg->hFromHeader,
                                  RV_FALSE,
                                  pBuff + callParams.fromHeaderOffset);
    if (status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to parse FROM header. Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    /* to header */
    status = RvSipPartyHeaderConstruct(pCallLeg->pMgr->hMsgMgr, hPool, pCallLeg->hPage, &pCallLeg->hToHeader);
    if(status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to construct to header Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    status = SipPartyHeaderSetType(pCallLeg->hToHeader, RVSIP_HEADERTYPE_TO);
    if(status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to set to type in party header Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    status = RvSipPartyHeaderParse(pCallLeg->hToHeader,
                                  RV_TRUE,
                                  pBuff + callParams.toHeaderOffset);
    if (status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to parse TO header. Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    /* strCallId */
    status = RvSipCallLegSetCallId((RvSipCallLegHandle)pCallLeg, pBuff + callParams.strCallIdOffset);
    if (status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to set callId. Call-leg 0x%p",
            pCallLeg));
        return status;
    }

    /* local contact */
    if(callParams.localContactOffset > 0)
    {
        eStrAddrType = SipAddrGetAddressTypeFromString(pCallLeg->pMgr->hMsgMgr, pBuff + callParams.localContactOffset);
        status = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr,
                                    hPool,
                                    pCallLeg->hPage,
                                    eStrAddrType,
                                    &pCallLeg->hLocalContact);
        if (status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Failed to construct local contact address. Call-leg 0x%p",
                pCallLeg));
            return status;
        }

        status = RvSipAddrParse(pCallLeg->hLocalContact,
                                pBuff + callParams.localContactOffset);
        if (status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Failed to parse local contact address. Call-leg 0x%p",
                pCallLeg));
            return status;
        }
    }

    /* remote contact */
    if(callParams.remoteContactOffset > 0)
    {
        eStrAddrType = SipAddrGetAddressTypeFromString(pCallLeg->pMgr->hMsgMgr, pBuff + callParams.remoteContactOffset);
        status = RvSipAddrConstruct(pCallLeg->pMgr->hMsgMgr,
                                    hPool,
                                    pCallLeg->hPage,
                                    eStrAddrType,
                                    &pCallLeg->hRemoteContact);
        if (status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Failed to construct remote contact address. Call-leg 0x%p",
                pCallLeg));
            return status;
        }

        status = RvSipAddrParse(pCallLeg->hRemoteContact,
                                pBuff + callParams.remoteContactOffset);
        if (status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Failed to parse remote contact address. Call-leg 0x%p",
                pCallLeg));
            return status;
        }
    }

    /* route list */
    if(callParams.numOfRoutes > 0)
    {
        status = RestoreRouteList(pCallLeg,
                              callParams.numOfRoutes,
                              pBuff + callParams.routeListOffset);
        if (status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Failed to set route list. Call-leg 0x%p",
                pCallLeg));
            return status;
        }
    }

#ifdef RV_SIP_AUTH_ON
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    status = SipAuthenticatorHighAvailRestoreAuthObj3_0(
        pCallLeg->pMgr->hAuthMgr, pBuff,
        callParams.wwwAuthHeaderOffset,   callParams.nonceCount401,
        callParams.proxyAuthHeaderOffset, callParams.nonceCount407,
        pCallLeg->hPage, pCallLeg,
        (ObjLockFunc)CallLegLockAPI, (ObjUnLockFunc)CallLegUnLockAPI,
        &pCallLeg->pAuthListInfo, &pCallLeg->hListAuthObj);
#else
    status = SipAuthenticatorHighAvailRestoreAuthObj3_0(
        pCallLeg->pMgr->hAuthMgr, pBuff,
        callParams.wwwAuthHeaderOffset,   callParams.nonceCount401,
        callParams.proxyAuthHeaderOffset, callParams.nonceCount407,
        pCallLeg->hPage, pCallLeg,
        NULL, NULL,
        &pCallLeg->pAuthListInfo, &pCallLeg->hListAuthObj);
#endif
    if (status != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall_3_0 - Failed to resotre authentication data. Call-leg 0x%p",
            pCallLeg));
        return status;
    }
#endif /* RV_SIP_AUTH_ON */ 

    /* outbound msg */
    if(callParams.outboundMsgOffset > 0)
    {
        RvSipMsgHandle hTempMsg;

        status = RvSipCallLegGetOutboundMsg((RvSipCallLegHandle)pCallLeg, &hTempMsg);
        if(status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Call 0x%p: Failed to get outbound msg",
                pCallLeg));
            return status;
        }
        status = RestoreOutboundMsg(pCallLeg, &callParams, hTempMsg, pBuff);
        if(status != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall_3_0 - Call 0x%p: RestoreOutboundMsg failed status %d",
                pCallLeg, status));
            return status;
        }
    }

    /* local address */
    status = RestoreLocalAddr(pCallLeg, &callParams, pBuff, eHAmode);
    if (RV_OK != status)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegRestoreCall_3_0 - Call 0x%p: RestoreLocalAddr failed status %d",
                  pCallLeg, status));
        return status;
    }

    /* outbound proxy address */
    status = RestoreOutboundProxyAddr(pCallLeg, &callParams, pBuff);
    if (RV_OK != status)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegRestoreCall_3_0 - Call 0x%p: RestoreOutboundProxyAddr failed status %d",
                  pCallLeg, status));
        return status;
    }

#ifdef RV_SIP_SUBS_ON
    /* subs list */
    if(callParams.numOfSubscriptions > 0)
    {
        status = RestoreSubsList(pCallLeg, &callParams, pBuff);
        if (status != RV_OK)
            return status;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */ 


    /* we set the state parameters at the end, because some of the set API functions
    check the IDLE state (for example - you can't set call-id to a connected call-leg)*/
    pCallLeg->eState         = RVSIP_CALL_LEG_STATE_CONNECTED;
    pCallLeg->eDirection     = callParams.eDirection;
    pCallLeg->localCSeq.val  = callParams.localCSeq;
    pCallLeg->remoteCSeq.val = callParams.remoteCSeq;

    return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RestoreRouteList
 * ------------------------------------------------------------------------
 * General: The function restores the route list, in a callLeg
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg    - Pointer to the callLeg to fill with route list.
 *         numOfRoutes - Number of route headers in list.
 *         routeList   - Pointer to the beginning of encoded route header list.
 ***************************************************************************/
static RvStatus RestoreRouteList(IN CallLeg*  pCallLeg,
                                  IN RvInt32  numOfRoutes ,
                                  IN RvChar*  routeList)
{
    RvStatus      rv;
    RvInt         counter;
    RvChar*       routeHeader = routeList;
    /*--------------------------------------
       Construct the Route list if needed
     ----------------------------------------*/
    if(pCallLeg->hRouteList == NULL)
    {
        pCallLeg->hRouteList = RLIST_RPoolListConstruct(
                                            pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage,
                                            sizeof(void*), pCallLeg->pMgr->pLogSrc);
        if(pCallLeg->hRouteList == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreRouteList - Failed for Call-leg 0x%p, Failed to construct a new list",pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    }

    for(counter = 0; counter < numOfRoutes; ++counter)
    {
        rv = ListAddElement(pCallLeg, routeHeader, pCallLeg->hRouteList);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreRouteList - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
            return rv;
        }
        /* go to the next route header (+1 for the NULL) */
        routeHeader += strlen(routeHeader)+1;

    }

    return RV_OK;

}

/***************************************************************************
 * ListAddElement
 * ------------------------------------------------------------------------
 * General: Construct a new RouteHopHeader, fill it, and add it to the list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - Pointer to the call-leg
 *            pEncodedRoute - String with encoded route header
 *          hList         - Handle to the list of routes.
 ***************************************************************************/
static RvStatus ListAddElement(IN  CallLeg      *pCallLeg,
                                IN RvChar*      pEncodedRoute,
                                IN RLIST_HANDLE  hList)
{
    RvSipRouteHopHeaderHandle hNewRouteHop;
    RvSipRouteHopHeaderHandle *phNewRouteHop;
    RLIST_ITEM_HANDLE  listItem;
    RvStatus          rv;

    /*add the Route hop to the list - first copy the header to the given page
      and then insert the new handle to the list*/
    rv = RvSipRouteHopHeaderConstruct(pCallLeg->pMgr->hMsgMgr,
                                      pCallLeg->pMgr->hGeneralPool,
                                      pCallLeg->hPage,
                                      &hNewRouteHop);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "ListAddElement - Failed for Call-leg 0x%p, Failed to construct new RouteHop.",
                  pCallLeg));
        return rv;
    }
    /* parse the given encoded route header */
    rv = RvSipRouteHopHeaderParse(hNewRouteHop, pEncodedRoute);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = RLIST_InsertTail(NULL, hList,
                          (RLIST_ITEM_HANDLE *)&listItem);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "ListAddElement - Failed for Call-leg 0x%p, Failed to get new list element",pCallLeg));
        return rv;
    }

    phNewRouteHop = (RvSipRouteHopHeaderHandle*)listItem;
    *phNewRouteHop = hNewRouteHop;
    return RV_OK;
}

/***************************************************************************
 * RestoreLocalAddr
 * ------------------------------------------------------------------------
 * General: The function restores one local-address parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Handle to the call-leg that outbound msg is restore in.
 *          pCallParams - Pointer to the call-leg parameters structure in the buffer.
 *          pBuff    - Pointer to the given buffer.
 ***************************************************************************/
static void RestoreOneLocalAddr(IN CallLeg*                  pCallLeg,
                                IN RvInt32                   localAddrOffset,
                                IN RvSipTransport            eTransport,
                                IN RvSipTransportAddressType eAddrType,
                                IN RvChar                    *pIp,
                                IN RvChar                    *pPort,
                                OUT RvSipTransportLocalAddrHandle  *phAddr)
{
    RvStatus rv;
    if (UNDEFINED != localAddrOffset)
    {
        rv = RestoreIpAndPort(pCallLeg->pMgr->hTransportMgr,
                            eTransport,
                            eAddrType,
                            pIp,
                            pPort,
                            phAddr);
        if(rv != RV_OK)
        {
            RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOneLocalAddr - Call-leg 0x%p failed to restore %s %s address, the default is used",
                pCallLeg, SipCommonConvertEnumToStrTransportType(eTransport), SipCommonConvertEnumToStrTransportAddressType(eAddrType)));
        }
    }
    else
    {
        *phAddr = NULL;
    }
}
/***************************************************************************
 * RestoreLocalAddr
 * ------------------------------------------------------------------------
 * General: The function restores the local-address parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg that outbound msg is restore in.
 *          pCallParams - Pointer to the call-leg parameters structure in the buffer.
 *          pBuff    - Pointer to the given buffer.
 ***************************************************************************/
static RvStatus RestoreLocalAddr( IN CallLeg*             pCallLeg,
                                  IN ConnectedCallParams* pCallParams,
                                  IN RvChar*             pBuff,
                                  IN RvSipCallLegHARestoreMode eHAmode)
{
    RvUint16                          addrInUse;
    RvChar  *plocalUdpIpv4, *plocalTcpIpv4, *plocalUdpIpv6, *plocalTcpIpv6;
    RvChar *localUdpIpv4Port, *localTcpIpv4Port,*localUdpIpv6Port,*localTcpIpv6Port;
    RvInt32  localUdpIpv4Offset, localTcpIpv4Offset, localUdpIpv6Offset, localTcpIpv6Offset;
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvChar *plocalTlsIpv4 = NULL, *plocalTlsIpv6 = NULL;
    RvChar *localTlsIpv4Port = NULL,*localTlsIpv6Port = NULL;
    RvInt32  localTlsIpv4Offset = UNDEFINED, localTlsIpv6Offset = UNDEFINED;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
    RvChar* pAddrInUse;
    RvInt32 addrInUseOffset;

	/* If no HA mode was set, then a default value is used */
	if (eHAmode == RVSIP_CALL_LEG_H_A_RESTORE_MODE_UNDEFINED) {
#if (RV_TLS_TYPE != RV_TLS_NONE)
		eHAmode = RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITH_TLS; 
#else
		eHAmode = RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITHOUT_TLS;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */ 
	}
    /* if the file was stored in version 3.0 with TLS, then the information was
       stored in a localAddrParams structure.
       if the file was stored in version 3.0 without TLS, then the information was
       stored in a nonTLSToTLSRestoreLocalAddrParams structure. */
    if(eHAmode == RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITH_TLS)
    {
        localAddrParams		pLocalAddrParams;
                              
        memcpy(&pLocalAddrParams, pBuff+pCallParams->localAddrOffset, sizeof(pLocalAddrParams));

        plocalUdpIpv4          = pBuff+pLocalAddrParams.localUdpIpv4Offset;
        localUdpIpv4Port       = pBuff+pLocalAddrParams.localUdpIpv4PortOffset;
        localUdpIpv4Offset     = pLocalAddrParams.localUdpIpv4Offset;
        
        plocalTcpIpv4          = pBuff+pLocalAddrParams.localTcpIpv4Offset;
        localTcpIpv4Port       = pBuff+pLocalAddrParams.localTcpIpv4PortOffset;
        localTcpIpv4Offset     = pLocalAddrParams.localTcpIpv4Offset;
        
        plocalUdpIpv6          = pBuff+pLocalAddrParams.localUdpIpv6Offset;
        localUdpIpv6Port       = pBuff+pLocalAddrParams.localUdpIpv6PortOffset;
        localUdpIpv6Offset     = pLocalAddrParams.localUdpIpv6Offset;
        
        plocalTcpIpv6          = pBuff+pLocalAddrParams.localTcpIpv6Offset;
        localTcpIpv6Port       = pBuff+pLocalAddrParams.localTcpIpv6PortOffset;
        localTcpIpv6Offset     = pLocalAddrParams.localTcpIpv6Offset;
        
#if (RV_TLS_TYPE != RV_TLS_NONE)
        plocalTlsIpv4     = pBuff+pLocalAddrParams.localTlsIpv4Offset;
        localTlsIpv4Port       = pBuff+pLocalAddrParams.localTlsIpv4PortOffset;
        localTlsIpv4Offset     = pLocalAddrParams.localTlsIpv4Offset;
        
        plocalTlsIpv6     = pBuff+pLocalAddrParams.localTlsIpv6Offset;
        localTlsIpv6Port       = pBuff+pLocalAddrParams.localTlsIpv6PortOffset;
        localTlsIpv6Offset     = pLocalAddrParams.localTlsIpv6Offset;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

        pAddrInUse = pBuff+pLocalAddrParams.addrInUseOffset;
        addrInUseOffset = pLocalAddrParams.addrInUseOffset;
    }
    else if (eHAmode == RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITHOUT_TLS)
    {
        nonTLSToTLSRestoreLocalAddrParams pLocalAddrParams;
        memcpy(&pLocalAddrParams, pBuff+pCallParams->localAddrOffset, sizeof(pLocalAddrParams));
        
        plocalUdpIpv4          = pBuff+pLocalAddrParams.localUdpIpv4Offset;
        localUdpIpv4Port       = pBuff+pLocalAddrParams.localUdpIpv4PortOffset;
        localUdpIpv4Offset     = pLocalAddrParams.localUdpIpv4Offset;
        
        plocalTcpIpv4          = pBuff+pLocalAddrParams.localTcpIpv4Offset;
        localTcpIpv4Port       = pBuff+pLocalAddrParams.localTcpIpv4PortOffset;
        localTcpIpv4Offset     = pLocalAddrParams.localTcpIpv4Offset;
        
        plocalUdpIpv6          = pBuff+pLocalAddrParams.localUdpIpv6Offset;
        localUdpIpv6Port       = pBuff+pLocalAddrParams.localUdpIpv6PortOffset;
        localUdpIpv6Offset     = pLocalAddrParams.localUdpIpv6Offset;
        
        plocalTcpIpv6          = pBuff+pLocalAddrParams.localTcpIpv6Offset;
        localTcpIpv6Port       = pBuff+pLocalAddrParams.localTcpIpv6PortOffset;
        localTcpIpv6Offset     = pLocalAddrParams.localTcpIpv6Offset;
        
        pAddrInUse      = pBuff+pLocalAddrParams.addrInUseOffset;
        addrInUseOffset = pLocalAddrParams.addrInUseOffset;
    }
    else
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreLocalAddr - unknown eHAmode %d. Call-leg %x",
            eHAmode ,pCallLeg));
        return RV_ERROR_ILLEGAL_ACTION;
    }
    
    /* Restore the addresses */

    /* Udp - Ipv4 */
    RestoreOneLocalAddr(pCallLeg, 
                        localUdpIpv4Offset,
                        RVSIP_TRANSPORT_UDP,
                        RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                        plocalUdpIpv4,
                        localUdpIpv4Port,
                        &pCallLeg->localAddr.hLocalUdpIpv4);
   
    /* Tcp - Ipv4 */
    RestoreOneLocalAddr(pCallLeg, 
                        localTcpIpv4Offset,
                          RVSIP_TRANSPORT_TCP,
                          RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                          plocalTcpIpv4,
                          localTcpIpv4Port,
                          &pCallLeg->localAddr.hLocalTcpIpv4);

    /* Udp - Ipv6 */
    RestoreOneLocalAddr(pCallLeg, 
                        localUdpIpv6Offset,
                          RVSIP_TRANSPORT_UDP,
                          RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                          plocalUdpIpv6,
                          localUdpIpv6Port,
                          &pCallLeg->localAddr.hLocalUdpIpv6);

    /* Tcp - Ipv6 */
    RestoreOneLocalAddr(pCallLeg, 
                        localTcpIpv6Offset,
                          RVSIP_TRANSPORT_TCP,
                          RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                          plocalTcpIpv6,
                          localTcpIpv6Port,
                          &pCallLeg->localAddr.hLocalTcpIpv6);

#if (RV_TLS_TYPE != RV_TLS_NONE) 
    if(eHAmode == RVSIP_CALL_LEG_H_A_RESTORE_MODE_FROM_3_0_WITH_TLS)
    {
        /* Tls - Ipv4 */
        RestoreOneLocalAddr(pCallLeg, 
                            localTlsIpv4Offset,
                              RVSIP_TRANSPORT_TLS,
                              RVSIP_TRANSPORT_ADDRESS_TYPE_IP,
                              plocalTlsIpv4,
                              localTlsIpv4Port,
                              &pCallLeg->localAddr.hLocalTlsIpv4);
            

        /* Tls - Ipv6 */
        RestoreOneLocalAddr(pCallLeg, 
                        localTlsIpv6Offset,
                          RVSIP_TRANSPORT_TLS,
                          RVSIP_TRANSPORT_ADDRESS_TYPE_IP6,
                          plocalTlsIpv6,
                          localTlsIpv6Port,
                          &pCallLeg->localAddr.hLocalTlsIpv6);
    }
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

    /* Indication to the address in use */
    if (UNDEFINED != addrInUseOffset)
    {
        sscanf(pAddrInUse,"%hu ",&addrInUse);
        switch (addrInUse)
        {
        case 1:
            pCallLeg->localAddr.hAddrInUse =  &pCallLeg->localAddr.hLocalUdpIpv4;
            break;
        case 2:
            pCallLeg->localAddr.hAddrInUse = &pCallLeg->localAddr.hLocalTcpIpv4;
            break;
        case 3:
            pCallLeg->localAddr.hAddrInUse = &pCallLeg->localAddr.hLocalUdpIpv6;
            break;
        case 4:
            pCallLeg->localAddr.hAddrInUse = &pCallLeg->localAddr.hLocalTcpIpv6;
            break;
#if (RV_TLS_TYPE != RV_TLS_NONE)
        case 5:
            pCallLeg->localAddr.hAddrInUse = &pCallLeg->localAddr.hLocalTlsIpv4;
            break;
        case 6:
            pCallLeg->localAddr.hAddrInUse = &pCallLeg->localAddr.hLocalTlsIpv6;
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
        default:
			RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreLocalAddr - AddrInUse(%hu) is invalid cannot move on. Call-leg %x",
                addrInUse,pCallLeg));
            return RV_ERROR_UNKNOWN;
        }
    }

    return RV_OK;
}


/***************************************************************************
 * RestoreOutboundProxyAddr
 * ------------------------------------------------------------------------
 * General: The function restores the outbound-proxy parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg that outbound msg is restore in.
 *          pCallParams - Pointer to the call-leg parameters structure in the buffer.
 *          pBuff    - Pointer to the given buffer.
 ***************************************************************************/
static RvStatus RestoreOutboundProxyAddr(
                                  IN CallLeg             *pCallLeg,
                                  IN ConnectedCallParams *pCallParams,
                                  IN RvChar              *pBuff)
{
    outboundProxyParams    outboundParams;
    RvStatus               rv;
    RvUint16               tempBool;

    memcpy(&outboundParams, pBuff+pCallParams->outboundProxyOffset, sizeof(outboundProxyParams));

    SipTransportInitObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,&pCallLeg->outboundAddr);

    /* rpoolHostName */
    if (UNDEFINED != outboundParams.rpoolHostNameOffset)
    {
        RvInt32            strLen;
        RvChar            *strHostName = pBuff+outboundParams.rpoolHostNameOffset;
        RPOOL_ITEM_HANDLE   item;

        strLen = (RvInt32)strlen(strHostName);

        strLen += 1;

        pCallLeg->outboundAddr.rpoolHostName.hPool = pCallLeg->pMgr->hGeneralPool;
        pCallLeg->outboundAddr.rpoolHostName.hPage = pCallLeg->hPage;

        rv = RPOOL_AppendFromExternal(pCallLeg->outboundAddr.rpoolHostName.hPool,
                                      pCallLeg->outboundAddr.rpoolHostName.hPage,
                                      strHostName,
                                      strLen, RV_FALSE,
                                      &(pCallLeg->outboundAddr.rpoolHostName.offset),
                                      &item);
        if (RV_OK != rv)
        {
            return rv;
        }

#ifdef SIP_DEBUG
        pCallLeg->outboundAddr.strHostName = RPOOL_GetPtr(pCallLeg->outboundAddr.rpoolHostName.hPool,
                                                          pCallLeg->outboundAddr.rpoolHostName.hPage,
                                                          pCallLeg->outboundAddr.rpoolHostName.offset);
#endif

    }

    /* bIpAddressExistsOffset */
    sscanf(pBuff+outboundParams.bIpAddressExistsOffset, "%hu ", &tempBool);
    if (1 == tempBool)
    {
        pCallLeg->outboundAddr.bIpAddressExists = RV_TRUE;
    }
    else
    {
        pCallLeg->outboundAddr.bIpAddressExists = RV_FALSE;
    }


    /* ipAddress */
    rv = RestoreOutboundProxyIpAddr(pCallLeg,&outboundParams,pBuff);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreOutboundProxyAddr - Call-leg 0x%p failed in RestoreOutboundProxyIpAddr()",
                  pCallLeg));
        return rv;
    }

    /* bUseHostName */
    sscanf(pBuff+outboundParams.bUseHostNameOffset, "%hu ", &tempBool);
    if (1 == tempBool)
    {
        pCallLeg->outboundAddr.bUseHostName = RV_TRUE;
    }
    else
    {
        pCallLeg->outboundAddr.bUseHostName = RV_FALSE;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreOutboundProxyAddr
 * ------------------------------------------------------------------------
 * General: The function restores the outbound-proxy Ip address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg        - Handle to the call-leg that outbound msg is
 *                            restore in.
 *          pOutboundParams - Pointer to the call-leg OutboundProxy
 *                            parameters structure in the buffer.
 *          pBuff           - Pointer to the given buffer.
 ***************************************************************************/
static RvStatus RestoreOutboundProxyIpAddr(
                                  IN CallLeg             *pCallLeg,
                                  IN outboundProxyParams *pOutboundParams,
                                  IN RvChar              *pBuff)
{
    Obsolete_RV_LI_Address obsAddr;
    SipTransportAddress   *pCallLegIpAddr = &pCallLeg->outboundAddr.ipAddress;

    memcpy(&obsAddr,pBuff+pOutboundParams->ipAddressOffset, sizeof(Obsolete_RV_LI_Address));

    pCallLegIpAddr->eTransport    = ConvertObsoleteConnectionTypeToNew(obsAddr.transport);
    pCallLegIpAddr->addr.addrtype = ConvertObsoleteAddressTypeToNew(obsAddr.type);
    if (pCallLegIpAddr->addr.addrtype == RV_ADDRESS_TYPE_NONE)
    {
        /* No outbound address is set */
        return RV_OK;
    }
    if (RvAddressSetIpPort(&pCallLegIpAddr->addr,obsAddr.port) == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreOutboundProxyIpAddr - Call-leg 0x%p failed in RvAddressSetIpPort()",
                  pCallLeg));

        return RV_ERROR_UNKNOWN;
    }

    if (pCallLegIpAddr->addr.addrtype == RV_ADDRESS_TYPE_IPV4)
    {
        if (RvAddressIpv4SetIp(&pCallLegIpAddr->addr.data.ipv4,
                               obsAddr.data.Obsolete_ipAddr.ip) == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreOutboundProxyIpAddr - Call-leg 0x%p failed in RvAddressIpv4SetIp()",
                  pCallLeg));

            return RV_ERROR_UNKNOWN;
        }
    }
#if (RV_NET_TYPE & RV_NET_IPV6)
    else if (pCallLegIpAddr->addr.addrtype == RV_ADDRESS_TYPE_IPV6)
    {
        if (RvAddressIpv6SetIp(&pCallLegIpAddr->addr.data.ipv6,
                               (RvUint8*)obsAddr.data.Obsolete_ip6Addr.ip) == NULL ||
            RvAddressSetIpv6Scope(&pCallLegIpAddr->addr,
                                  obsAddr.data.Obsolete_ip6Addr.scopeId) == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreOutboundProxyIpAddr - Call-leg 0x%p failed in RvAddressIpv6SetIp() or in RvAddressIpv6SetScope()",
                  pCallLeg));

            return RV_ERROR_UNKNOWN;
        }
    }
#endif /* #if (RV_NET_TYPE & RV_NET_IPV6) */
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreOutboundProxyIpAddr - Call-leg 0x%p got unknown Address Type",
                  pCallLeg));

        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreIpAndPort
 * ------------------------------------------------------------------------
 * General: The function restores Ip address and port to the file.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransport - Handle to the transport manager
 *          eTransport - The transport enumeration - Udp/Tcp
 *          eAddrType  - The address type - Ip/Ip6
 *          pIp        - The location in the buffer where the Ip address is.
 *          pPort       - The location in the buffer where the port is.
 * Out:     phAddress  - The resulting address object
 ***************************************************************************/
static RvStatus RestoreIpAndPort(IN    RvSipTransportMgrHandle       hTransport,
                                  IN    RvSipTransport                eTransport,
                                  IN    RvSipTransportAddressType     eAddrType,
                                  IN    RvChar                      *pIp,
                                  IN    RvChar                      *pPort,
                                  OUT   RvSipTransportLocalAddrHandle  *phAddr)
{
    RvStatus					  rv;
    RvChar						  strIp[64];
    RvUint16					  port;
	RvSipTransportLocalAddrHandle hAddr = NULL;

    sscanf(pIp, "%s ", strIp);
    sscanf(pPort, "%hu ", &port);

    rv = SipTransportLocalAddressGetHandleByAddress(hTransport,
                                                    eTransport,
                                                    eAddrType,
                                                    strIp, port,
                                                    &hAddr);
	if (hAddr != NULL)
	{
		*phAddr = hAddr;
	}

    return rv;
}

/***************************************************************************
 * RestoreOutboundMsg
 * ------------------------------------------------------------------------
 * General: The function restores the outbound msg parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg that outbound msg is restore in.
 *          pCallParams - Pointer to the call-leg parameters structure in the buffer.
 *          hMsg     - Handle to the outbound msg in call-leg.
 *          pBuff    - Pointer to the given buffer.
 ***************************************************************************/
static RvStatus RestoreOutboundMsg(IN CallLeg             *pCallLeg,
                                   IN ConnectedCallParams *pCallParams,
                                   IN RvSipMsgHandle       hMsg,
                                   IN RvChar              *pBuff)
{
    outboundMsgParams   msgParams ;
    RvStatus           rv;

    memcpy(&msgParams, pBuff+pCallParams->outboundMsgOffset, sizeof(outboundMsgParams));

    /* start line */
    if(msgParams.eStartLineType == CALL_LEG_HIGH_EVAL_REQUEST)
    {
        /* method */
        rv = RvSipMsgSetMethodInRequestLine(hMsg,
                                            (RvSipMethodType)msgParams.startLineEnum,
                                            pBuff + msgParams.startLineStrOffset);
        if(rv != RV_OK)
        {
            return rv;
        }
        /* request URI */
        if(msgParams.requestUriOffset > 0)
        {
            RvSipAddressHandle hReqUri;

            rv = RvSipAddrConstructInStartLine(hMsg, msgParams.eReqUriType, &hReqUri);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RvSipAddrParse(hReqUri,pBuff + msgParams.requestUriOffset);
            if(rv != RV_OK)
            {
                return rv;
            }
        }
    }
    else if(msgParams.eStartLineType == CALL_LEG_HIGH_EVAL_RESPONSE)
    {
        /* code */
        rv = RvSipMsgSetStatusCode(hMsg, (RvInt16)msgParams.startLineEnum, RV_TRUE);
        if(rv != RV_OK)
        {
            return rv;
        }
        /* reason phrase */
        if(msgParams.startLineStrOffset > 0)
        {
            rv = RvSipMsgSetReasonPhrase(hMsg, pBuff + msgParams.startLineStrOffset);
            if(rv != RV_OK)
            {
                return rv;
            }
        }

    }

    /* from */
    if(msgParams.fromHeaderOffset > 0)
    {
        RvSipPartyHeaderHandle hFromHeader;

        rv = RvSipFromHeaderConstructInMsg(hMsg, &hFromHeader);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to construct from header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }

        rv = RvSipPartyHeaderParse(hFromHeader, RV_FALSE,
                                        pBuff + msgParams.fromHeaderOffset);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to parse FROM header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }

    /* to */
    if(msgParams.toHeaderOffset > 0)
    {
        RvSipPartyHeaderHandle hToHeader;

        rv = RvSipToHeaderConstructInMsg(hMsg, &hToHeader);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to construct TO header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }

        rv = RvSipPartyHeaderParse(hToHeader, RV_TRUE,
                                        pBuff + msgParams.toHeaderOffset);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to parse TO header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }

    /* cseq */
    if(msgParams.cseqHeaderOffset > 0)
    {
        RvSipCSeqHeaderHandle hCseq;

        rv = RvSipCSeqHeaderConstructInMsg(hMsg, &hCseq);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to construct CSEQ header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }

        rv = RvSipCSeqHeaderParse(hCseq, pBuff + msgParams.cseqHeaderOffset);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to parse CSEQ header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }

    /* callId */
    if(msgParams.callIdOffset > 0)
    {
        rv = RvSipMsgSetCallIdHeader(hMsg, pBuff + msgParams.callIdOffset);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to set CALL_ID header. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }
    }


    /* body object */
    if(msgParams.bodyObjectOffset > 0)
    {
        RvSipBodyHandle hBody;

        rv = RvSipBodyConstructInMsg(hMsg, &hBody);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to construct body. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }

        rv = RvSipBodySetBodyStr(hBody, pBuff + msgParams.bodyObjectOffset,
                                msgParams.bodyLength);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreOutboundMsg - Failed to set bodyStr in body. Call-leg 0x%p",
                pCallLeg));
            return rv;
        }

    }

    /* header list */
    if(msgParams.numOfHeaders > 0)
    {
        RvInt32 i;
        RvChar* pHeader = pBuff + msgParams.headersListOffset;

        for(i = 0; i < msgParams.numOfHeaders; ++i)
        {
            RvSipHeaderType eHeaderType;
            RvChar*        pHeaderStr;
            void*           hHeader;
            RvInt32        headerLen;

            /* each header is saved with it's type enumuration first. this enum is
            in size of RvInt32 */
            eHeaderType = (RvSipHeaderType)*(RvInt32*)pHeader;
            pHeaderStr = pHeader + sizeof(RvInt32);

            rv = RvSipMsgConstructHeaderInMsgByType(hMsg, eHeaderType, RV_FALSE, &hHeader);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RestoreOutboundMsg - Call-Leg 0x%p - Failed to add header",pCallLeg));
                return rv;
            }

            rv = RvSipHeaderParse(hHeader, eHeaderType, pHeaderStr);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RestoreOutboundMsg - Call-Leg 0x%p - Failed to parse header",pCallLeg));
                return rv;
            }
            /* go to the next header (+1 for the NULL) */
            headerLen = (RvInt32)strlen(pHeaderStr);
            pHeader += sizeof(RvInt32)+ headerLen + 1 ;
        }
    }

    /* content length */
    if(msgParams.contentLength > 0)
    {
        rv = RvSipMsgSetContentLengthHeader(hMsg, msgParams.contentLength);
        if(rv != RV_OK)
        {
            return rv;
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pCallLeg);
#endif

    return RV_OK;
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * RestoreSubsList
 * ------------------------------------------------------------------------
 * General: The function restores the subscription list.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg.
 *          pCallParams - Pointer to the call-leg parameters structure in the buffer.
 *          pBuff    - Pointer to the given buffer.
 ***************************************************************************/
static RvStatus RestoreSubsList(IN CallLeg*       pCallLeg,
                                 IN ConnectedCallParams* pCallParams,
                                 IN RvChar*       pBuff)
{
    RvStatus           rv;
    subscriptionParams  subsParams;
    RvSipSubsHandle     hSubs;
    RvInt32            subsOffset;

    subsOffset = pCallParams->subscriptionListOffset;

    while (subsOffset > 0)
    {
        memcpy(&subsParams, pBuff + subsOffset,sizeof(subscriptionParams));

        /* 1. create subscription */
        if(subsParams.bOutOfBand == RV_TRUE)
        {
            rv = SipSubsMgrSubscriptionCreate(pCallLeg->pMgr->hSubsMgr,
                                            (RvSipCallLegHandle)pCallLeg,
                                            NULL,
                                            subsParams.eSubsType,
                                            RV_TRUE, /*bOutOfBand*/
                                            &hSubs);
            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RestoreSubsList - Failed to create out of band subs. Call-leg 0x%p",
                    pCallLeg));
                return rv;
            }
        }
        else
        {
            rv = SipSubsMgrSubscriptionCreate(
                                pCallLeg->pMgr->hSubsMgr,
                                (RvSipCallLegHandle)pCallLeg,
                                NULL, /*hAppSubs */
                                RVSIP_SUBS_TYPE_SUBSCRIBER, 
                                RV_FALSE, /*bOutOfBand*/
                                &hSubs);

            if(rv != RV_OK)
            {
                RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RestoreSubsList - Failed to create subs. Call-leg 0x%p",
                    pCallLeg));
                return rv;
            }
        }

        SipSubsSetSubsType(hSubs, subsParams.eSubsType);

        /* 2. set subscription parameters */

        if(subsParams.eventHeaderOffset > 0)
        {
            /* set event and expires */
            RvSipEventHeaderHandle hEvent;
            HPAGE                  tempPage;

            rv = RPOOL_GetPage(pCallLeg->pMgr->hGeneralPool, 0, &tempPage);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv = RvSipEventHeaderConstruct(pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool, tempPage, &hEvent);
            if(rv != RV_OK)
            {
                RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, tempPage);
                return rv;
            }
            rv = RvSipEventHeaderParse(hEvent, pBuff + subsParams.eventHeaderOffset);
            if(rv != RV_OK)
            {
                RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, tempPage);
                return rv;
            }
            rv = RvSipSubsSetEventHeader(hSubs, hEvent);
            RPOOL_FreePage(pCallLeg->pMgr->hGeneralPool, tempPage);
            if(rv != RV_OK)
            {
                return rv;
            }

            rv = RvSipSubsSetExpiresVal(hSubs, subsParams.expiresVal);
        }
        else
        {
            /* set only expires */
            rv = RvSipSubsSetExpiresVal(hSubs, subsParams.expiresVal);
        }

        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreSubsList - Failed to set expires and event in subs 0x%p. Call-leg 0x%p",
                hSubs, pCallLeg));
            return rv;
        }

        SipSubsSetSubsCurrentState(hSubs, subsParams.eSubsState);

        subsOffset = subsParams.nextSubsOffset;

    } /* while */

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */ 

/***************************************************************************
* ConvertObsoleteAddressTypeToNew
* ------------------------------------------------------------------------
* General: Converts the obsolete AddressType to the new Common Core Type.
* Return Value: The suitable Common Core Type.
* ------------------------------------------------------------------------
* Arguments:
* Input:   addrType - The obsolete Type.
***************************************************************************/
static RvInt ConvertObsoleteAddressTypeToNew(
                                  IN Obsolete_RV_LI_AddressType addrType)
{
    switch (addrType)
    {
    case Obsolete_RV_LI_Ip:
        return RV_ADDRESS_TYPE_IPV4;
    case Obsolete_RV_LI_Ip6:
        return RV_ADDRESS_TYPE_IPV6;
    default:
        return RV_ADDRESS_TYPE_NONE;
    }
}

/***************************************************************************
* ConvertObsoleteConnectionTypeToNew
* ------------------------------------------------------------------------
* General: Converts the obsolete ConnectionType to RvSipTransport.
* Return Value: The suitable TransportType.
* ------------------------------------------------------------------------
* Arguments:
* Input:   transport - The obsolete Type.
***************************************************************************/
static RvSipTransport ConvertObsoleteConnectionTypeToNew(
                                  IN Obsolete_RV_LI_ConnectionType transport)
{
    switch (transport)
    {
    case Obsolete_RV_LI_Udp:
        return RVSIP_TRANSPORT_UDP;
    case Obsolete_RV_LI_Tcp:
        return RVSIP_TRANSPORT_TCP;
    case Obsolete_RV_LI_Tls:
        return RVSIP_TRANSPORT_TLS;
    default:
        return RVSIP_TRANSPORT_UNDEFINED;
    }
}
#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 
#endif /* ifndef RV_SIP_PRIMITIVES*/

#endif /*#ifdef  RV_SIP_HIGH_AVAL_3_0*/

