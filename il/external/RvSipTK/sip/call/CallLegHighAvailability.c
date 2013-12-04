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
 *                              <CallLegHighAvailability.c>
 *
 *  The following functions are for High Availability use. It means that user can get all
 *  the nessecary parameters of a connected call, (which is not in the middle of
 *  a refer or re-invite proccess), and copy it to another list of calls - of another
 *  call manager. The format of each stored parameter is:
 *  <total param len><internal sep><param ID><internal sep><param data><external sep>
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 November 2001
 *********************************************************************************/
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_HIGHAVAL_ON

#include "rvrandomgenerator.h"
#include "rvansi.h"
#include "RvSipCallLeg.h"
#include "CallLegHighAvailability.h"
#include "CallLegObject.h"
#include "rpool_API.h"
#include "RvSipAddress.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipBody.h"
#include "_SipAddress.h"
#include "_SipTransport.h"
#include "_SipPartyHeader.h"
#include "_SipMsg.h"
#include "_SipCallLeg.h"
#include "RvSipMsg.h"
#include "RvSipHeader.h"
#include "RvSipSubscriptionTypes.h"
#include "RvSipSubscription.h"
#include "_SipSubs.h"
#include "_SipSubsMgr.h"
#include "_SipCommonHighAval.h"
#include "_SipCommonCore.h"
#include "_SipCommonCSeq.h"

#ifdef RV_SIP_HIGH_AVAL_3_0
#include "CallLegHighAvailability_3_0.h"
#endif /* RV_SIP_HIGH_AVAL_3_0 */
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define LOCAL_ADDR_IP_ADDR_FORMAT        "%s %hu"
#define OUTBOUND_PROXY_ADDRESS_FORMAT    "%s %hu %s %s"
#define MAX_CALL_LEG_PARAM_IDS_NUM       36
#define MAX_LOCAL_ADDR_PARAM_IDS_NUM     9
#define MAX_OUTBOUND_PROXY_PARAM_IDS_NUM 5
#if (RV_NET_TYPE & RV_NET_SCTP)
#define MAX_SCTP_PARAM_IDS_NUM           2
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

/* CallLeg HighAval Params IDs
   -----------------------------
   These values MUSTN'T be changed in order to assure backward compatibility.
   Moreover, these IDs MUSTN'T include the HIGH_AVAL_PARAM_INTERNAL_SEP or
   the HIGH_AVAL_PARAM_EXTERNAL_SEP.
*/
#define FROM_HEADER_ID        "FromHeader"
#define TO_HEADER_ID          "ToHeader"
#define CALL_ID_ID            "CallID"
#define LOCAL_CONTACT_ID      "LocalContact"
#define REMOTE_CONTACT_ID     "RemoteContact"
#define LOCAL_CSEQ_ID         "LocalCSeq"
#define REMOTE_CSEQ_ID        "RemoteCSeq"
#ifdef RV_SIP_UNSIGNED_CSEQ
#define IS_LOCAL_CSEQ_INIT_ID	"IsLocalCSeqInitialized"
#define IS_REMOTE_CSEQ_INIT_ID	"IsRemoteCSeqInitialized"
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
#define DIRECTION_ID          "Direction"
#define STATE_ID              "State"
#define ROUTE_ID              "Route"
#define FORKING_ENABLED_ID    "EnableForking"
#ifdef RV_SIGCOMP_ON
#define IS_SIGCOMP_ENABLED_ID "IsSigCompEnabled"
#endif /* #ifdef RV_SIGCOMP_ON */

#ifdef RV_SIP_IMS_ON
#define INITIAL_AUTHORIZATION_ID "InitialAuthorization"
#endif /*#ifdef RV_SIP_IMS_ON*/
 
#define LOCAL_ADDR_ID        "LocalAddr"        /* An extended parameter */
#define OUTBOUND_PROXY_ID    "OutboundProxy"    /* An extended parameter */
#define CALL_SUBS_ID         "CallSubscription" /* An extended parameter */
#if (RV_NET_TYPE & RV_NET_SCTP)
#define SCTP_ID              "SCTP"             /* An extended parameter */
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

/* LocalAddr HighAval Params IDs
   ------------------------------
   These values MUSTN'T be changed in order to assure backward compatibility.
   Moreover, these IDs MUSTN'T include the HIGH_AVAL_PARAM_SEP_CHAR.
*/
#define LOCAL_ADDR_UDP_IPV4_ID      "LocalUdpIpv4"
#define LOCAL_ADDR_TCP_IPV4_ID      "LocalTcpIpv4"
#define LOCAL_ADDR_UDP_IPV6_ID      "LocalUdpIpv6"
#define LOCAL_ADDR_TCP_IPV6_ID      "LocalTcpIpv6"
#if (RV_TLS_TYPE != RV_TLS_NONE)
#define LOCAL_ADDR_TLS_IPV4_ID      "LocalTlsIpv4"
#define LOCAL_ADDR_TLS_IPV6_ID      "LocalTlsIpv6"
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
#define LOCAL_ADDR_SCTP_IPV4_ID      "LocalSctpIpv4"
#define LOCAL_ADDR_SCTP_IPV6_ID      "LocalSctpIpv6"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#define LOCAL_ADDR_ADDR_IN_USE_ID   "AddrInUse"

/* OutboundProxy HighAval Params IDs
   ------------------------------
   These values MUSTN'T be changed in order to assure backward compatibility.
   Moreover, these IDs MUSTN'T include the HIGH_AVAL_PARAM_SEP_CHAR.
*/
#define OUTBOUND_PROXY_RPOOL_HOST_NAME_ID "RpoolHostName"
#define OUTBOUND_PROXY_IP_ADDR_EXISTS_ID  "IpAddressExists"
#define OUTBOUND_PROXY_IP_ADDR_ID         "IpAddress"
#define OUTBOUND_PROXY_USE_HOST_NAME_ID   "UseHostName"
#ifdef RV_SIGCOMP_ON
#define OUTBOUND_PROXY_COMP_TYPE_ID       "CompType"
#define OUTBOUND_PROXY_RPOOL_SIGCOMPID_ID "RpoolSigCompId"
#endif /* RV_SIGCOMP_ON */

#if (RV_NET_TYPE & RV_NET_SCTP)
#define SCTP_STREAM_ID           "SctpStream"
#define SCTP_MSGSENDFLAGS_ID     "SctpMsgSendFlags"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV GetRouteListSize(IN    CallLeg  *pCallLeg,
                                            OUT   RvInt32  *pLen);

static RvStatus RVCALLCONV GetSubsListSize(IN    CallLeg       *pCallLeg,
                                           OUT   RvInt32       *pLen);

static RvStatus RVCALLCONV StoreRouteList(IN    CallLeg     *pCallLeg,
                               IN    RvUint32     buffLen,
                               INOUT RvChar     **ppCurrPos,
                               INOUT RvUint32    *pCurrLen);

static RvStatus RVCALLCONV RouteListAddElement(IN  CallLeg      *pCallLeg,
                                    IN RvChar        *pEncodedRoute,
                                    IN RLIST_HANDLE   hList);

static RvStatus RVCALLCONV RestoreCall(IN  RvChar         *pBuff,
                                       IN  RvUint32        buffLen,
                                       IN  RvBool          bStandAlone,
                                       OUT CallLeg        *pCallLeg);

static RvStatus RVCALLCONV ResetCall(IN CallLeg               *pCallLeg,
                          IN RvSipCallLegDirection  eOriginDirection);

static RvStatus RVCALLCONV StoreSubsList(IN    CallLeg      *pCallLeg,
                              IN    RvUint32      buffLen,
                              INOUT RvChar      **ppCurrPos,
                              INOUT RvUint32     *pCurrLen);

#ifdef RV_SIP_SUBS_ON
static RvStatus RVCALLCONV GetActiveNonReferSubs(
                              IN    CallLeg         *pCallLeg,
                              IN    RvSipSubsHandle  hRelativeSubs,
                              OUT   RvSipSubsHandle *phActiveSubs);
#endif /* #ifdef RV_SIP_SUBS_ON */

static RvStatus RVCALLCONV RestoreCallLegAuthObj(
                                           IN    RvChar   *pAuthDataID,
                                           IN    RvUint32  authDataIDLen,
                                           IN    RvChar   *pAuthDataVal,
                                           IN    RvUint32  authDataValLen,
                                           IN    void     *pParamObj);

static RvStatus RVCALLCONV RestoreCallLegSubs(
                                           IN    RvChar   *pCallSubsID,
                                           IN    RvUint32  callSubsIDLen,
                                           IN    RvChar   *pCallSubsValue,
                                           IN    RvUint32  callSubsValueLen,
                                           INOUT void     *pParamObj);

static RvStatus RVCALLCONV GetLocalAddrSize(IN    CallLeg      *pCallLeg,
                                 OUT   RvInt32      *pLen);

static RvStatus RVCALLCONV StoreLocalAddr(
                               IN    CallLeg     *pCallLeg,
                               IN    RvUint32     buffLen,
                               INOUT RvChar     **ppCurrPos,
                               INOUT RvUint32    *pCurrLen);

static RvStatus RVCALLCONV StoreLocalAddrParams(
                        IN    CallLeg     *pCallLeg,
                        IN    RvUint32     buffLen,
                        INOUT RvChar     **ppCurrPos,
                        INOUT RvUint32    *pCurrLen);


static RvStatus RVCALLCONV GetIpAndPortSize(
                                 IN  RvSipTransportMgrHandle        hTransport,
                                 IN  RvSipTransportLocalAddrHandle  hAddr,
                                 IN  RvChar                        *strAddrID,
                                 OUT RvInt32                       *pLen);

static RvStatus RVCALLCONV StoreIpAndPort(
                               IN    RvSipTransportMgrHandle       hTransport,
                               IN    RvSipTransportLocalAddrHandle hAddr,
                               IN    RvChar                       *strAddrID,
                               IN    RvUint32                      buffLen,
                               INOUT RvChar                      **ppCurrPos,
                               INOUT RvUint32                     *pCurrLen);

static RvStatus RVCALLCONV RestoreIpAndPort(IN    RvSipTransportMgrHandle        hTransport,
                                 IN    RvSipTransport                 eTransport,
                                 IN    RvSipTransportAddressType      eAddrType,
                                 IN    RvChar                        *strAddr,
                                 OUT   RvSipTransportLocalAddrHandle *phAddr);

static RvStatus RVCALLCONV GetOutboundProxyAddrSize(IN  CallLeg      *pCallLeg,
                                         OUT RvInt32      *pLen);

static RvStatus RVCALLCONV StoreOutboundProxyAddr(
                   IN    CallLeg     *pCallLeg,
                   IN    RvUint32     buffLen,
                   INOUT RvChar     **ppCurrPos,
                   INOUT RvUint32    *pCurrLen);

static RvStatus RVCALLCONV StoreOutboundProxyAddrParams(
                   IN    SipTransportOutboundAddress   *pOutboundAddr,
                   IN    RvUint32                       buffLen,
                   INOUT RvChar                       **ppCurrPos,
                   INOUT RvUint32                      *pCurrLen);

#if (RV_NET_TYPE & RV_NET_SCTP)
static RvStatus RVCALLCONV StoreSctp(
                    IN    CallLeg     *pCallLeg,
                    IN    RvUint32     buffLen,
                    INOUT RvChar     **ppCurrPos,
                    INOUT RvUint32    *pCurrLen);
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

static RvStatus RVCALLCONV GetCallLegHeadersStorageSize(IN    CallLeg *pCallLeg,
                                             OUT   RvInt32 *pLen);

static RvStatus RVCALLCONV GetCallLegContactsStorageSize(IN    CallLeg *pCallLeg,
                                              OUT   RvInt32 *pLen);

static RvStatus RVCALLCONV GetCallLegSingleHeaderStorageSize(
                              IN     CallLeg         *pCallLeg,
                              IN     RvSipHeaderType  eHeaderType,
                              IN     RvBool           bProxy,
                              INOUT  RvInt32         *pCurrLen);

static RvStatus RVCALLCONV GetCallLegSingleAddrStorageSize(
                              IN     CallLeg         *pCallLeg,
                              IN     RvBool           bLocal,
                              INOUT  RvInt32         *pCurrLen);

#if (RV_NET_TYPE & RV_NET_SCTP)
static RvStatus RVCALLCONV GetSctpParamsSize(IN  CallLeg   *pCallLeg,
                                             OUT RvInt32   *pLen);
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

static RvStatus RVCALLCONV EncodeCallLegSingleHeader(
                              IN     CallLeg         *pCallLeg,
                              IN     RvSipHeaderType  eHeaderType,
                              IN     RvBool           bProxy,
                              OUT    HPAGE           *pEncodedPage,
                              OUT    RvUint32        *pEncodedLen,
                              OUT    RvChar         **pStrHeaderID);

static RvStatus RVCALLCONV EncodeCallLegSingleAddr(
                              IN     CallLeg         *pCallLeg,
                              IN     RvBool           bLocal,
                              OUT    HPAGE           *pEncodedPage,
                              OUT    RvUint32        *pEncodedLen,
                              OUT    RvChar         **pStrAddrID);

static RvStatus RVCALLCONV StoreCallLegHeaders(
                                 IN    CallLeg                  *pCallLeg,
                                 IN    RvUint32                  buffLen,
                                 INOUT RvChar                  **ppCurrPos,
                                 INOUT RvUint32                 *pCurrLen);

static RvStatus RVCALLCONV StoreCallLegContacts(
                             IN    CallLeg      *pCallLeg,
                             IN    RvUint32      buffLen,
                             INOUT RvChar      **ppCurrPos,
                             INOUT RvUint32     *pCurrLen);

static RvStatus RVCALLCONV StoreCallLegSingleHeader(
                                 IN    CallLeg             *pCallLeg,
                                 IN    RvSipHeaderType      eHeaderType,
                                 IN    RvBool               bProxy,
                                 IN    RvUint32             buffLen,
                                 INOUT RvChar             **ppCurrPos,
                                 INOUT RvUint32            *pCurrLen);

static RvStatus RVCALLCONV StoreCallLegCallID(
                                 IN    CallLeg     *pCallLeg,
                                 IN    RvUint32     buffLen,
                                 INOUT RvChar     **ppCurrPos,
                                 INOUT RvUint32    *pCurrLen);

static RvStatus RVCALLCONV StoreCallLegSingleAddr(
                                 IN    CallLeg                  *pCallLeg,
                                 IN    RvBool                    bLocal,
                                 IN    RvUint32                  buffLen,
                                 INOUT RvChar                  **ppCurrPos,
                                 INOUT RvUint32                 *pCurrLen);

static RvStatus RVCALLCONV StoreAddrInUse(IN    CallLeg      *pCallLeg,
                                          IN    RvUint32      buffLen,
                                          INOUT RvChar      **ppCurrPos,
                                          INOUT RvUint32     *pCurrLen);

static RvStatus RVCALLCONV GetCallLegGeneralParamsSize(
                                    IN  CallLeg  *pCallLeg,
                                    OUT RvInt32  *pCurrLen);

static RvStatus RVCALLCONV StoreCallLegGeneralParams(
                             IN    CallLeg      *pCallLeg,
                             IN    RvUint32      buffLen,
                             INOUT RvChar      **ppCurrPos,
                             INOUT RvUint32     *pCurrLen);

static RvChar *RVCALLCONV GetAddrInUseID(IN CallLeg  *pCallLeg);

static RvStatus RVCALLCONV RestoreCallLegPartyHeader(
                                          IN    RvChar   *pHeaderID,
                                          IN    RvUint32  headerIDLen,
                                          IN    RvChar   *pHeaderValue,
                                          IN    RvUint32  headerValueLen,
                                          INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegCallID(IN    RvChar   *pCallIDID,
                                     IN    RvUint32  callIDLen,
                                     IN    RvChar   *pCallIDValue,
                                     IN    RvUint32  callIDValueLen,
                                     INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegContact(
                                   IN    RvChar   *pContactID,
                                   IN    RvUint32  contactIDLen,
                                   IN    RvChar   *pContactValue,
                                   IN    RvUint32  contactValueLen,
                                   INOUT void     *pObj);
#ifdef RV_SIP_IMS_ON
static RvStatus RVCALLCONV RestoreCallLegInitialAuthorization(
                                 IN    RvChar   *pParamID,
                                 IN    RvUint32  paramIDLen,
                                 IN    RvChar   *pHeaderValue,
                                 IN    RvUint32  headerValueLen,
                                 INOUT void     *pObj);
#endif /*#ifdef RV_SIP_IMS_ON*/

static RvStatus RVCALLCONV RestoreCallLegRoute(IN    RvChar   *pRouteID,
                                    IN    RvUint32  routeIDLen,
                                    IN    RvChar   *pRouteValue,
                                    IN    RvUint32  routeValueLen,
                                    INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegLocalAddr(
                                 IN    RvChar   *pLocalAddrID,
                                 IN    RvUint32  localAddrIDLen,
                                 IN    RvChar   *pLocalAddrValue,
                                 IN    RvUint32  localAddrValueLen,
                                 INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegLocalAddrInUse(
                                 IN    RvChar   *pAddrInUseID,
                                 IN    RvUint32  addrInUseIDLen,
                                 IN    RvChar   *pAddrInUseVal,
                                 IN    RvUint32  addrInUseValLen,
                                 INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegOutboundProxyHostName(
                                        IN    RvChar   *pHostNameID,
                                        IN    RvUint32  hostNameIDLen,
                                        IN    RvChar   *pHostNameVal,
                                        IN    RvUint32  hostNameValLen,
                                        INOUT void     *pObj);

#ifdef RV_SIGCOMP_ON
static RvStatus RVCALLCONV RestoreCallLegOutboundProxyCompType(
                                        IN    RvChar   *pCompTypeID,
                                        IN    RvUint32  compTypeIDLen,
                                        IN    RvChar   *pCompTypeVal,
                                        IN    RvUint32  compTypeValLen,
                                        INOUT void     *pObj);
#endif /* RV_SIGCOMP_ON */

static RvStatus RVCALLCONV RestoreCallLegState(
                                        IN    RvChar   *pStateID,
                                        IN    RvUint32  stateIDLen,
                                        IN    RvChar   *pStateVal,
                                        IN    RvUint32  stateValLen,
                                        INOUT void     *pObj);

static RvStatus RVCALLCONV RestoreCallLegDirection(
                                        IN    RvChar   *pDirectionID,
                                        IN    RvUint32  directionIDLen,
                                        IN    RvChar   *pDirectionVal,
                                        IN    RvUint32  directionValLen,
                                        INOUT void     *pObj);

static RvStatus RVCALLCONV PrepareLocalAddrIDsArray(
                                 IN     CallLeg                    *pCallLeg,
                                 INOUT  RvInt32                    *pArraySize,
                                 OUT    SipCommonHighAvalParameter *pIDsArray);

static RvStatus RVCALLCONV PrepareOutboundProxyAddrIDsArray(
                                 IN    CallLeg                    *pCallLeg,
                                 INOUT RvInt32                    *pArraySize,
                                 OUT   SipCommonHighAvalParameter *pIDsArray);

#if (RV_NET_TYPE & RV_NET_SCTP)
static RvStatus RVCALLCONV PrepareSctpIDsArray(
                                IN    CallLeg                    *pCallLeg,
                                INOUT RvInt32                    *pArraySize,
                                OUT   SipCommonHighAvalParameter *pIDsArray);
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

static RvStatus RVCALLCONV GetOutboundProxyIpAddrStr(
                                IN  SipTransportAddress *pIpAddr,
                                IN  RvUint32             maxAddrStrLen,
                                OUT RvChar              *strIpAddr);

static RvStatus RVCALLCONV AddRestoredCallLegToMgrHash(
                                  IN CallLeg               *pCallLeg,
                                  IN RvSipCallLegDirection  eOriginDirection);

static RvBool RVCALLCONV AreEssentialsRestored(IN CallLeg *pCallLeg); 

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegGetCallStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of
 *          connected call.
 *          (The size of buffer should be supply in
 *          RvSipCallLegStoreConnectedCall()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR     - hCallleg or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hCallLeg     - Handle to the call-leg.
 *          bStandAlone  - Indicates if the CallLeg is independent.
 *                         This parameter should be RV_FALSE if
 *                         a subscription object retrieves its CallLeg
 *                         storage size, in order to avoid infinity recursion.
 * Output:  pLen     - size of buffer. will be UNDEFINED (-1) in case of failure.
 ***************************************************************************/
RvStatus CallLegGetCallStorageSize(IN  RvSipCallLegHandle hCallLeg,
                                   IN  RvBool             bStandAlone,
                                   OUT RvInt32           *pLen)
{
    CallLeg    *pCallLeg = (CallLeg*)hCallLeg;
    RvInt32     currLen  = 0;
    RvInt32     tempLen;
    RvStatus    rv;

    *pLen = UNDEFINED;
    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalGetGeneralStoredBuffPrefixLen(&tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;
    }

    /* CallLeg Headers size */
    rv = GetCallLegHeadersStorageSize(pCallLeg,&tempLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    currLen += tempLen;

    /* Contact size */
    rv = GetCallLegContactsStorageSize(pCallLeg,&tempLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    currLen += tempLen;

    /* Route List size */
    if(pCallLeg->hRouteList != NULL)
    {
        rv = GetRouteListSize(pCallLeg, &tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;
    }

    /* The subs list is stored only if it shouldn't be ignored */
    if(bStandAlone == RV_TRUE)
    {
        rv = GetSubsListSize(pCallLeg,&tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;
    }

#ifdef RV_SIP_AUTH_ON
    if (NULL != pCallLeg->hListAuthObj)
    {
        rv = SipAuthenticatorHighAvailGetAuthObjStorageSize(
          pCallLeg->pMgr->hAuthMgr,pCallLeg->hListAuthObj,&tempLen);
        if (RV_OK != rv)
        {
            return rv;
        }
        currLen += tempLen;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
    
    rv = GetLocalAddrSize(pCallLeg, &tempLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    /* Add the main LocalAddr ID string length */
    currLen += SipCommonHighAvalGetTotalStoredParamLen(
                                   tempLen,(RvUint32)strlen(LOCAL_ADDR_ID));

    rv = GetOutboundProxyAddrSize(pCallLeg, &tempLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    /* Add the main OutboundProxy ID string length */
    currLen += SipCommonHighAvalGetTotalStoredParamLen(
                                    tempLen,(RvUint32)strlen(OUTBOUND_PROXY_ID));

    /* CallLeg General Left Parameters */
    rv = GetCallLegGeneralParamsSize(pCallLeg,&tempLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currLen += tempLen;

#if (RV_NET_TYPE & RV_NET_SCTP)
    /* CallLeg SCTP Parameters */
    rv = GetSctpParamsSize(pCallLeg,&tempLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    /* Add the main OutboundProxy ID string length */
    currLen += SipCommonHighAvalGetTotalStoredParamLen(
        tempLen,(RvUint32)strlen(SCTP_ID));
    
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

	if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalGetGeneralStoredBuffSuffixLen(&tempLen);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += tempLen;
    }
	
    *pLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * CallLegStoreCall
 * ------------------------------------------------------------------------
 * General: Copies all call-leg parameters from a given call-leg to a given buffer.
 *          This buffer should be supplied when restoring the call leg.
 *          In order to store call-leg information the call leg must be in the
 *          connceted state.
 * Return Value:RV_ERROR_INVALID_HANDLE    - The handle to the callLeg is invalid.
 *              RV_ERROR_NULLPTR       - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION    - If the state is not on state CONNECTED, or the
 *                                    refer state is not REFER_IDLE.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES   - Resource problems.
 *              RV_OK          - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCallLeg    - Handle to the call-leg .
 *        maxBuffLen  - The length of the given buffer.
 *        bStandAlone - Indicates if the CallLeg is independent .
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg
 *                      storage size, in order to avoid infinity recursion.
 * Output: memBuff    - The buffer that will store the CallLeg's parameters.
 *         pStoredLen - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus CallLegStoreCall(IN  RvSipCallLegHandle hCallLeg,
                          IN  RvBool             bStandAlone,
                          IN  RvUint32           maxBuffLen,
                          OUT void              *memBuff,
                          OUT RvInt32           *pStoredLen)
{
    CallLeg     *pCallLeg = (CallLeg*)hCallLeg;
    RvChar      *currPos  = (RvChar*)memBuff;
    RvUint32     currLen  = 0;
    RvStatus     rv;

    *pStoredLen = UNDEFINED;

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalStoreGeneralPrefix(maxBuffLen,&currPos,&currLen);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, SipCommonHighAvalStoreGeneralPrefix() failed. status %d",
                pCallLeg, rv));

            return rv;
        }
    }
    rv = StoreCallLegHeaders(pCallLeg,maxBuffLen,&currPos,&currLen);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, StoreCallLegHeaders failed. status %d",
                pCallLeg, rv));
        return rv;
    }

    rv = StoreCallLegContacts(pCallLeg,maxBuffLen,&currPos,&currLen);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, StoreCallLegContacts failed. status %d",
                pCallLeg, rv));
        return rv;
    }

    /* route list */
    if(pCallLeg->hRouteList != NULL)
    {
        rv = StoreRouteList(pCallLeg, maxBuffLen, &currPos, &currLen);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, StoreRouteList failed. status %d",
                pCallLeg, rv));
            return rv;
        }
    }

    /* Authentication data */
#ifdef RV_SIP_AUTH_ON
    if (NULL != pCallLeg->hListAuthObj)
    {
        rv = SipAuthenticatorHighAvailStoreAuthObj(
            pCallLeg->pMgr->hAuthMgr, pCallLeg->hListAuthObj,
            maxBuffLen, &currPos, &currLen);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, hAuthData 0x%x - StoreAuthObj failed (rv=%d:%s)",
                pCallLeg, pCallLeg->hListAuthObj,
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

    if (bStandAlone == RV_TRUE)
    {
        rv = StoreSubsList(pCallLeg,maxBuffLen, &currPos, &currLen);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, StoreSubsList failed. status %d",
                pCallLeg, rv));
            return rv;
        }
    }

    rv = StoreLocalAddr(pCallLeg,maxBuffLen,&currPos,&currLen);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegStoreCall: Call-Leg 0x%p, StoreLocalAddr failed. status %d",
                  pCallLeg, rv));
        return rv;
    }

    rv = StoreOutboundProxyAddr(
            pCallLeg, maxBuffLen, &currPos, &currLen);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "CallLegStoreCall: Call-Leg 0x%p, StoreOutboundProxyAddr failed. status %d",
                  pCallLeg, rv));
        return rv;
    }

#if (RV_NET_TYPE & RV_NET_SCTP)
    rv = StoreSctp(
            pCallLeg, maxBuffLen, &currPos, &currLen);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegStoreCall: Call-Leg 0x%p, StoreSctp failed. status %d",
            pCallLeg, rv));
        return rv;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
    /* We store the state parameters at the end of the buffer , because */
    /* some of the restore API functions check the IDLE state (for      */
    /* example - you can't set call-id to a connected call-leg) */
    rv = StoreCallLegGeneralParams(pCallLeg,maxBuffLen,&currPos,&currLen);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                   "CallLegStoreCall: Call-Leg 0x%p, StoreCallLegGeneralParams failed. status %d",
                   pCallLeg, rv));
        return rv;
    }

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalStoreGeneralSuffix(maxBuffLen,&currPos,&currLen);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegStoreCall: Call-Leg 0x%p, SipCommonHighAvalStoreGeneralSuffix() failed. status %d",
                pCallLeg, rv));

            return rv;
        }
    }

    *pStoredLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * CallLegRestoreCall
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
 * Input: memBuff     - The buffer that stores all the callLeg's parameters.
 *        buffLen     - The buffer size
 *        bStandAlone - Indicates if the CallLeg is independent.
 *                      This parameter should be RV_FALSE if
 *                      a subscription object retrieves its CallLeg
 *                      storage size, in order to avoid infinity recursion.
 * InOut: pCallLeg    - Pointer to the call-leg.
 ***************************************************************************/
RvStatus CallLegRestoreCall(IN    void     *memBuff,
                            IN    RvUint32  buffLen,
                            IN    RvBool    bStandAlone,
                            IN    RvSipCallLegHARestoreMode eHAmode,
                            OUT   CallLeg  *pCallLeg)
{
    RvStatus                  rv               = RV_OK;
    RvChar                   *pBuff            = (RvChar*)memBuff;
    RvUint32                  prefixLen        = 0;
    RvBool                    bUseOldVer       = RV_FALSE;
    RvSipCallLegDirection     eOriginDirection = pCallLeg->eDirection;

    /* The callLeg must be on IDLE state */
    /* Besides, the callLeg should be 'empty' - that was not inserted to the
       hash yet.(A callLeg that was created by refer request could be on idle
        state, but not empty)*/
    if(pCallLeg->eState         != RVSIP_CALL_LEG_STATE_IDLE ||
       pCallLeg->hashElementPtr != NULL)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    if (bStandAlone == RV_TRUE)
    {
        rv = SipCommonHighAvalHandleStandAloneBoundaries(
            buffLen,&pBuff,&prefixLen,&bUseOldVer);
        if (rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegRestoreCall - CallLeg 0x%p Failed in SipCommonHighAvalHandleStandAloneBoundaries()",
                pCallLeg));

            return rv;
        }
    }

    if (bUseOldVer == RV_FALSE)
    {
        rv = RestoreCall(pBuff, buffLen-prefixLen, bStandAlone, pCallLeg);
    }
    else
    {
#ifdef RV_SIP_HIGH_AVAL_3_0
        rv = CallLegRestoreCall_3_0(pCallLeg,memBuff, eHAmode);        
#else  /* #ifdef RV_SIP_HIGH_AVAL_3_0 */
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "CallLegRestoreCall - Restored Call-leg 0x%p probably got old storage buffer, which cannot be restored",
            pCallLeg));

        return RV_ERROR_ILLEGAL_ACTION;
    
#endif /* #ifdef RV_SIP_HIGH_AVAL_3_0 */
    }
    RV_UNUSED_ARG(eHAmode)
    if(rv != RV_OK)
    {
        ResetCall(pCallLeg, eOriginDirection);
        return RV_ERROR_UNKNOWN;
    }

    /* insert the call leg to the callMgr hash */
    rv = AddRestoredCallLegToMgrHash(pCallLeg,eOriginDirection);

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RestoreCall
 * ------------------------------------------------------------------------
 * General: The function restores call-leg parameters from a given buffer
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pBuff        - The buffer that contains a stored call-leg.
 *         buffLen      - The buffer size.
 *         bStandAlone  - Indicates if the CallLeg is independent.
 *                        This parameter should be RV_FALSE if
 *                        a subscription object retrieves its CallLeg
 *                        storage size, in order to avoid infinity recursion.
 * Output: pCallLeg     - Pointer to the callLeg to restore
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCall(IN  RvChar         *pBuff,
                                       IN  RvUint32        buffLen,
                                       IN  RvBool          bStandAlone,
                                       OUT CallLeg        *pCallLeg)
{
    RvStatus                    rv;
    SipCommonHighAvalParameter  callIDsArray         [MAX_CALL_LEG_PARAM_IDS_NUM];
    SipCommonHighAvalParameter  localAddrIDsArray    [MAX_LOCAL_ADDR_PARAM_IDS_NUM];
    SipCommonHighAvalParameter  outboundProxyIDsArray[MAX_OUTBOUND_PROXY_PARAM_IDS_NUM];
#if (RV_NET_TYPE & RV_NET_SCTP)
    SipCommonHighAvalParameter  sctpIDsArray         [MAX_SCTP_PARAM_IDS_NUM];
    RvInt32                     actualSctpIDsNo      = MAX_SCTP_PARAM_IDS_NUM;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    SipCommonHighAvalParameter *pCurrCallID          = callIDsArray;
    RvInt32                     actualLocalAddrIDsNo = MAX_LOCAL_ADDR_PARAM_IDS_NUM;
    RvInt32                     actualOutboundIDsNo  = MAX_OUTBOUND_PROXY_PARAM_IDS_NUM;

    memset(callIDsArray,         0,sizeof(callIDsArray));
    memset(localAddrIDsArray,    0,sizeof(localAddrIDsArray));
    memset(outboundProxyIDsArray,0,sizeof(outboundProxyIDsArray));

    /* Firstly initialize the OutBoundAddr before reading from the buffer */
    SipTransportInitObjectOutboundAddr(pCallLeg->pMgr->hTransportMgr,&pCallLeg->outboundAddr);

    pCurrCallID->strParamID       = FROM_HEADER_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegPartyHeader;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = TO_HEADER_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegPartyHeader;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = CALL_ID_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegCallID;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = LOCAL_CONTACT_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegContact;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = REMOTE_CONTACT_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegContact;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    /* Route List */
    pCurrCallID->strParamID       = ROUTE_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegRoute;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    /* Local Address */
    rv = PrepareLocalAddrIDsArray(pCallLeg,&actualLocalAddrIDsNo,localAddrIDsArray);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreCall - CallLeg 0x%p failed in PrepareLocalAddrIDsArray",pCallLeg));

        return rv;
    }
    pCurrCallID->strParamID             = LOCAL_ADDR_ID;
    pCurrCallID->pParamObj              = pCallLeg;
    pCurrCallID->subHighAvalParamsArray = localAddrIDsArray;
    pCurrCallID->paramsArrayLen         = (RvInt32)actualLocalAddrIDsNo;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    /* Outbound Proxy Address */
    rv = PrepareOutboundProxyAddrIDsArray(
            pCallLeg,&actualOutboundIDsNo,outboundProxyIDsArray);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreCall - CallLeg 0x%p failed in PrepareOutboundProxyAddrIDsArray",
                  pCallLeg));

        return rv;
    }
    pCurrCallID->strParamID             = OUTBOUND_PROXY_ID;
    pCurrCallID->pParamObj              = pCallLeg;
    pCurrCallID->subHighAvalParamsArray = outboundProxyIDsArray;
    pCurrCallID->paramsArrayLen         = (RvUint32)actualOutboundIDsNo;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

#if (RV_NET_TYPE & RV_NET_SCTP)
    /* SCTP Parameters */
    rv = PrepareSctpIDsArray(
            pCallLeg,&actualSctpIDsNo,sctpIDsArray);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCall - CallLeg 0x%p failed in PrepareSctpIDsArray",
            pCallLeg));
        return rv;
    }
    pCurrCallID->strParamID             = SCTP_ID;
    pCurrCallID->pParamObj              = pCallLeg;
    pCurrCallID->subHighAvalParamsArray = sctpIDsArray;
    pCurrCallID->paramsArrayLen         = (RvUint32)actualSctpIDsNo;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

    pCurrCallID->strParamID       = HIGHAVAIL_AUTHDATA_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegAuthObj;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

#ifdef RV_SIP_IMS_ON
    pCurrCallID->strParamID       = INITIAL_AUTHORIZATION_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegInitialAuthorization;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* subs list */
    if(bStandAlone == RV_TRUE)
    {
        pCurrCallID->strParamID       = CALL_SUBS_ID;
        pCurrCallID->pParamObj        = pCallLeg;
        pCurrCallID->pfnSetParamValue = RestoreCallLegSubs;
        pCurrCallID += 1; /* Promote the pointer to next call ID*/
    }

    pCurrCallID->strParamID       = STATE_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegState;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = DIRECTION_ID;
    pCurrCallID->pParamObj        = pCallLeg;
    pCurrCallID->pfnSetParamValue = RestoreCallLegDirection;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = LOCAL_CSEQ_ID;
    pCurrCallID->pParamObj        = &pCallLeg->localCSeq.val;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

    pCurrCallID->strParamID       = REMOTE_CSEQ_ID;
    pCurrCallID->pParamObj        = &pCallLeg->remoteCSeq.val;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

#ifdef RV_SIP_UNSIGNED_CSEQ
	pCurrCallID->strParamID       = IS_LOCAL_CSEQ_INIT_ID;
    pCurrCallID->pParamObj        = &pCallLeg->localCSeq.bInitialized;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/
	
	pCurrCallID->strParamID       = IS_REMOTE_CSEQ_INIT_ID;
    pCurrCallID->pParamObj        = &pCallLeg->remoteCSeq.bInitialized;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

    pCurrCallID->strParamID       = FORKING_ENABLED_ID;
    pCurrCallID->pParamObj        = &pCallLeg->bForkingEnabled;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/

#ifdef RV_SIGCOMP_ON
	pCurrCallID->strParamID       = IS_SIGCOMP_ENABLED_ID;
    pCurrCallID->pParamObj        = &pCallLeg->bSigCompEnabled;
    pCurrCallID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrCallID += 1; /* Promote the pointer to next call ID*/
#endif /* #ifdef RV_SIGCOMP_ON */ 

	if (pCurrCallID-callIDsArray > MAX_CALL_LEG_PARAM_IDS_NUM)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreCall - CallLeg 0x%p call IDs data (%u items) was written beyond the array boundary",
                  pCallLeg,pCurrCallID-callIDsArray));

        return RV_ERROR_ILLEGAL_ACTION;
    }
    rv = SipCommonHighAvalRestoreFromBuffer(
            pBuff,buffLen,callIDsArray,(RvUint32)(pCurrCallID-callIDsArray),pCallLeg->pMgr->pLogSrc);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "RestoreCall - CallLeg 0x%p failed in SipCommonHighAvalRestoreFromBuffer",
                  pCallLeg));
        return rv;
    }

    return RV_OK;

}

/***************************************************************************
 * ResetCall
 * ------------------------------------------------------------------------
 * General: The function clears call-leg parameters
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg    - Pointer to the callLeg to clear.
 *         eOriginDirection - direction of the call-leg.
 ***************************************************************************/
static RvStatus RVCALLCONV ResetCall(IN CallLeg*              pCallLeg,
                          IN RvSipCallLegDirection eOriginDirection)
{
    HRPOOL    hPool = pCallLeg->pMgr->hGeneralPool;
    RvStatus status;

    pCallLeg->eState        = RVSIP_CALL_LEG_STATE_IDLE;
    pCallLeg->hToHeader     = NULL;
    pCallLeg->hFromHeader   = NULL;
    pCallLeg->hLocalContact = NULL;
    pCallLeg->hRemoteContact= NULL;
    pCallLeg->eDirection    = eOriginDirection;
    pCallLeg->strCallId     = UNDEFINED;
    pCallLeg->hListPage     = NULL_PAGE;
	SipCommonCSeqSetStep(0,&pCallLeg->localCSeq); 
	SipCommonCSeqReset(&pCallLeg->remoteCSeq); 
    pCallLeg->hOutboundMsg  = NULL;

#ifdef RV_SIP_SUBS_ON
    pCallLeg->hActiveReferSubs = NULL;
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef SIP_DEBUG
    pCallLeg->pCallId          =  NULL;
#endif

#ifdef RV_SIP_AUTH_ON
    /* Authentication Data */
    if(NULL != pCallLeg->hListAuthObj)
    {
        SipAuthenticatorAuthListResetPasswords(pCallLeg->hListAuthObj);
        SipAuthenticatorAuthListDestruct(pCallLeg->pMgr->hAuthMgr,pCallLeg->pAuthListInfo, pCallLeg->hListAuthObj);
        pCallLeg->hListAuthObj = NULL;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_IMS_ON
    pCallLeg->hInitialAuthorization = NULL;
#endif /*#ifdef RV_SIP_IMS_ON*/

    /* route list */
    if(pCallLeg->hRouteList != NULL)
    {
        pCallLeg->hRouteList = NULL;
        RPOOL_FreePage(hPool, pCallLeg->hListPage);
    }

    /* free the callLeg page, and get a new 'clean' one */
    RPOOL_FreePage(hPool, pCallLeg->hPage);
    status = RPOOL_GetPage(hPool,0,&(pCallLeg->hPage));
    if(status != RV_OK)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    /* Set the local address as the default one */
    status = SipTransportLocalAddressInitDefaults(pCallLeg->pMgr->hTransportMgr,
                                                  &(pCallLeg->localAddr));
    if (RV_OK != status)
    {
        RPOOL_FreePage(hPool, pCallLeg->hPage);
        return status;
    }
    /* outboundProxy */
    memset(&pCallLeg->outboundAddr.ipAddress, 0, sizeof(pCallLeg->outboundAddr.ipAddress));

    /* SigComp data */
#ifdef RV_SIGCOMP_ON
    pCallLeg->bSigCompEnabled = RV_TRUE;
#endif /* RV_SIGCOMP_ON */

    /* Set unique identifier only on initialization success */
    RvRandomGeneratorGetInRange(pCallLeg->pMgr->seed, RV_INT32_MAX,
        (RvRandom*)&pCallLeg->callLegUniqueIdentifier);

    return RV_OK;
}

/***************************************************************************
 * RouteListAddElement
 * ------------------------------------------------------------------------
 * General: Construct a new RouteHopHeader, fill it, and add it to the list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg      - Pointer to the call-leg
 *            pEncodedRoute - String with encoded route header
 *          hList         - Handle to the list of routes.
 ***************************************************************************/
static RvStatus RVCALLCONV RouteListAddElement(IN  CallLeg      *pCallLeg,
                                    IN RvChar        *pEncodedRoute,
                                    IN RLIST_HANDLE   hList)
{
    RvSipRouteHopHeaderHandle  hNewRouteHop;
    RvSipRouteHopHeaderHandle *phNewRouteHop;
    RLIST_ITEM_HANDLE          listItem;
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
                  "RouteListAddElement - Failed for Call-leg 0x%p, Failed to construct new RouteHop.",
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
                  "RouteListAddElement - Failed for Call-leg 0x%p, Failed to get new list element",pCallLeg));
        return rv;
    }

    phNewRouteHop = (RvSipRouteHopHeaderHandle*)listItem;
    *phNewRouteHop = hNewRouteHop;
    return RV_OK;
}

/***************************************************************************
 * StoreRouteList
 * ------------------------------------------------------------------------
 * General: The function stores the route list from call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreRouteList(IN    CallLeg     *pCallLeg,
                               IN    RvUint32     buffLen,
                               INOUT RvChar     **ppCurrPos,
                               INOUT RvUint32    *pCurrLen)
{
    RLIST_ITEM_HANDLE         pItem  = NULL; /*current item*/
    RvSipRouteHopHeaderHandle hRoute = NULL;
    HRPOOL                    hPool  = pCallLeg->pMgr->hGeneralPool;
    HPAGE                     tempPage;
    RvUint32                  encodedLen;
    RvStatus                  rv;

    RLIST_GetHead(NULL, pCallLeg->hRouteList, &pItem);
    while(pItem != NULL)
    {
        hRoute = *(RvSipRouteHopHeaderHandle*)pItem;

        /* Encode the Route Hop Header */
        rv = RvSipRouteHopHeaderEncode(hRoute,hPool,&tempPage,&encodedLen);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = SipCommonHighAvalStoreSingleParamFromPage(
                ROUTE_ID,hPool,tempPage,0,RV_TRUE,buffLen, encodedLen, ppCurrPos,pCurrLen);
        if (rv != RV_OK)
        {
            return rv;
        }

        RLIST_GetNext(NULL, pCallLeg->hRouteList, pItem, &pItem);
    }

    return RV_OK;
}

/***************************************************************************
 * GetRouteListSize
 * ------------------------------------------------------------------------
 * General: The function retrives the length of the route list needed
 *          allocations
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg that holds the route list.
 * Output:    len      - The size which is required for allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetRouteListSize(IN    CallLeg  *pCallLeg,
                                            OUT   RvInt32  *pLen)
{
    RvStatus                  rv;
    HPAGE                     tempPage;
    RvUint32                  encodedLen;
    HRPOOL                    hPool       = pCallLeg->pMgr->hGeneralPool;
    RLIST_ITEM_HANDLE         pItem       = NULL; /*current item*/
    RvSipRouteHopHeaderHandle hRoute      = NULL;

    *pLen = 0;
    RLIST_GetHead(NULL, pCallLeg->hRouteList, &pItem);

    while(pItem != NULL)
    {
        hRoute = *(RvSipRouteHopHeaderHandle*)pItem;

        /* allocate new list item in params */
        rv = RvSipRouteHopHeaderEncode(hRoute,hPool,&tempPage,&encodedLen);
        if(rv != RV_OK)
        {
            *pLen = UNDEFINED;
            return RV_ERROR_UNKNOWN;
        }
        *pLen += SipCommonHighAvalGetTotalStoredParamLen(encodedLen,(RvUint32)strlen(ROUTE_ID));
        RPOOL_FreePage(hPool, tempPage);

        RLIST_GetNext(NULL, pCallLeg->hRouteList, pItem, &pItem);
    }

    return RV_OK;
}


/***************************************************************************
 * GetSubsListSize
 * ------------------------------------------------------------------------
 * General: The function retrives the length of the subs list which is
 *          required for allocations.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Handle to the call-leg that holds the subs list.
 * Output:    len      - The size which is required for allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetSubsListSize(IN    CallLeg       *pCallLeg,
                                           OUT   RvInt32       *pLen)
{
#ifdef RV_SIP_SUBS_ON
    RvSipSubsHandle hSubs         = NULL;
    RvSipSubsHandle hRelativeSubs = NULL;
    RvInt32         currSubsLen;
    RvStatus        rv;

    *pLen = 0;

    if (pCallLeg->hSubsList == NULL)
    {
        return RV_OK;
    }

    rv = GetActiveNonReferSubs(pCallLeg,hRelativeSubs,&hSubs);
    if (rv != RV_OK)
    {
        *pLen = UNDEFINED;
        return rv;
    }

    while (hSubs != NULL)
    {
        rv = SipSubsGetActiveSubsStorageSize(hSubs,RV_FALSE,&currSubsLen);
        if (rv != RV_OK)
        {
            *pLen = UNDEFINED;
            return rv;
        }
        /* The subs length includes all it parameters and separators */
        /* no additional calculations has to be done */
        *pLen += SipCommonHighAvalGetTotalStoredParamLen(currSubsLen,(RvUint32)strlen(CALL_SUBS_ID));

        hRelativeSubs = hSubs;
        rv            = GetActiveNonReferSubs(pCallLeg,hRelativeSubs,&hSubs);
        if(rv != RV_OK)
        {
            *pLen = UNDEFINED;
            return rv;
        }
    }

    return rv;
#else  /* #ifdef RV_SIP_SUBS_ON */
    *pLen = 0;

    RV_UNUSED_ARG(pCallLeg)
    RV_UNUSED_ARG(pLen)

    return RV_OK;
#endif /* #ifdef RV_SIP_SUBS_ON */
}

/***************************************************************************
 * GetLocalAddrSize
 * ------------------------------------------------------------------------
 * General: The function retrives the length of the LocalAddress information
 *          (to be stored) - not including the main LocalAddress string
 *          which should be extended.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Handle to the call-leg that holds the outbound msg.
 * InOut:   pLen       - An updated integer, with the size of so far needed allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetLocalAddrSize(IN    CallLeg      *pCallLeg,
                                 OUT   RvInt32      *pLen)
{
    RvStatus rv;
    RvInt32  currAddrLen;
    RvInt32  currSumLen = 0;
    RvChar  *strAddrInUseID;

    *pLen = UNDEFINED;

    /* Udp - Ipv4 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalUdpIpv4,
                          LOCAL_ADDR_UDP_IPV4_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;

    /* Tcp - Ipv4 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalTcpIpv4,
                          LOCAL_ADDR_TCP_IPV4_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;


    /* Udp - Ipv6 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalUdpIpv6,
                          LOCAL_ADDR_UDP_IPV4_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;


    /* Tcp - Ipv6 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalTcpIpv6,
                          LOCAL_ADDR_TCP_IPV6_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Tls - Ipv4 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalTlsIpv4,
                          LOCAL_ADDR_TLS_IPV4_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;

    /* Tls - Ipv6 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
                          pCallLeg->localAddr.hLocalTlsIpv6,
                          LOCAL_ADDR_TLS_IPV6_ID,
                          &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE)*/
	
#if (RV_NET_TYPE & RV_NET_SCTP)
	/* Sctp - Ipv4 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
						  pCallLeg->localAddr.hLocalSctpIpv4,
						  LOCAL_ADDR_SCTP_IPV4_ID,
						  &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;
	
    /* Sctp - Ipv6 */
    rv = GetIpAndPortSize(pCallLeg->pMgr->hTransportMgr,
						  pCallLeg->localAddr.hLocalSctpIpv6,
						  LOCAL_ADDR_SCTP_IPV6_ID,
						  &currAddrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    currSumLen += currAddrLen;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
	
    /* Indication of the address in use */
    strAddrInUseID = GetAddrInUseID(pCallLeg);
    if (NULL != strAddrInUseID)
    {
        currSumLen += SipCommonHighAvalGetTotalStoredParamLen(
                                            (RvUint32)strlen(strAddrInUseID),
                                            (RvUint32)strlen(LOCAL_ADDR_ADDR_IN_USE_ID));
    }

    *pLen = currSumLen;

    return RV_OK;
}


/***************************************************************************
 * GetOutboundProxyAddrSize
 * ------------------------------------------------------------------------
 * General: The function retrives the length of the outbound-proxy address
 *          INTERNAL information (to be stored) - not including the main
 *          OutboundProxy string ID.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Handle to the call-leg that holds the outbound msg.
 * Output:  pLen       - The size of Outbound Proxy Addr required for allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetOutboundProxyAddrSize(IN  CallLeg      *pCallLeg,
                                                    OUT RvInt32      *pLen)
{
    RvChar   strIpAddr[RV_ADDRESS_MAXSTRINGSIZE+(2*SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN)];
#ifdef RV_SIGCOMP_ON
    RvChar  *strComp;
#endif /*#ifdef RV_SIGCOMP_ON*/
    RvInt32  currLen = 0;
    RvStatus rv;

    *pLen = UNDEFINED;

    if (UNDEFINED != pCallLeg->outboundAddr.rpoolHostName.offset)
    {
        currLen += SipCommonHighAvalGetTotalStoredParamLen(
                        RPOOL_Strlen(pCallLeg->outboundAddr.rpoolHostName.hPool,
                        pCallLeg->outboundAddr.rpoolHostName.hPage,
                        pCallLeg->outboundAddr.rpoolHostName.offset),
                        (RvUint32)strlen(OUTBOUND_PROXY_RPOOL_HOST_NAME_ID));
    }

    /* bIpAddressExists */
    currLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
                        pCallLeg->outboundAddr.bIpAddressExists,
                        (RvUint32)strlen(OUTBOUND_PROXY_IP_ADDR_EXISTS_ID));
    /* Only if there's a valid IP address it's length is calculated */
    if (RV_TRUE == pCallLeg->outboundAddr.bIpAddressExists)
    {
        rv = GetOutboundProxyIpAddrStr(
                &pCallLeg->outboundAddr.ipAddress,sizeof(strIpAddr),strIpAddr);
        if (rv != RV_OK)
        {
            return rv;
        }
        currLen += SipCommonHighAvalGetTotalStoredParamLen(
                        (RvUint32)strlen(strIpAddr),(RvUint32)strlen(OUTBOUND_PROXY_IP_ADDR_ID));
    }

    /* bUseHostName */
    currLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
                            pCallLeg->outboundAddr.bUseHostName,
                            (RvUint32)strlen(OUTBOUND_PROXY_USE_HOST_NAME_ID));

#ifdef RV_SIGCOMP_ON
    if (RVSIP_COMP_UNDEFINED != pCallLeg->outboundAddr.eCompType)
    {
        strComp  = SipMsgGetCompTypeName(pCallLeg->outboundAddr.eCompType);
        currLen += SipCommonHighAvalGetTotalStoredParamLen(
                                    (RvUint32)strlen(strComp),
                                    (RvUint32)strlen(OUTBOUND_PROXY_COMP_TYPE_ID));
    }
#endif /* RV_SIGCOMP_ON */
    *pLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * GetIpAndPortSize
 * ------------------------------------------------------------------------
 * General: Compute the length in string that is required for this address
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransport    - Handle to the transport manager.
 *          hAddr         - Handle to the address object.
 *          strAddrID     - The string that identifies the address type in
 *                          the storing buffer.
 * Output:  pLen          - The size of required allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetIpAndPortSize(
                                 IN  RvSipTransportMgrHandle        hTransport,
                                 IN  RvSipTransportLocalAddrHandle  hAddr,
                                 IN  RvChar                        *strAddrID,
                                 OUT RvInt32                       *pLen)
{
    RvStatus rv;
    RvUint16 port;
    RvChar   strIp  [RVSIP_TRANSPORT_LEN_STRING_IP] = "";
#if defined(UPDATED_BY_SPIRENT)
    RvChar   if_name  [RVSIP_TRANSPORT_IFNAME_SIZE] = "";
    RvUint16 vdevblock = 0;
#endif
    RvChar   strAddr[RVSIP_TRANSPORT_LEN_STRING_IP+SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];


    *pLen = 0;

    if (hAddr == NULL)
    {
        return RV_OK;
    }
    rv = SipTransportLocalAddressGetAddressByHandle(
                    hTransport,hAddr,RV_FALSE,RV_TRUE,strIp,&port
#if defined(UPDATED_BY_SPIRENT)
                    ,if_name,&vdevblock
#endif
           );
    if(rv != RV_OK)
    {
        return rv;
    }

    RvSprintf(strAddr,LOCAL_ADDR_IP_ADDR_FORMAT, strIp,port);
    /* Add the Ip address + Port length */
    *pLen += SipCommonHighAvalGetTotalStoredParamLen((RvUint32)strlen(strAddr),(RvUint32)strlen(strAddrID));

    return RV_OK;
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * GetSctpParamsSize
 * ------------------------------------------------------------------------
 * General: The function retrives the length of the SCTP data
 *          INTERNAL information (to be stored).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Handle to the call-leg.
 * Output:  pLen       - The size of SCTP data required for allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV GetSctpParamsSize(IN  CallLeg   *pCallLeg,
                                             OUT RvInt32   *pLen)
{
    RvInt32  currLen;

    currLen = SipCommonHighAvalGetTotalStoredInt32ParamLen(
            pCallLeg->sctpParams.sctpStream,
            (RvUint32)strlen(SCTP_STREAM_ID));

    currLen += SipCommonHighAvalGetTotalStoredInt32ParamLen(
        pCallLeg->sctpParams.sctpMsgSendingFlags,
        (RvUint32)strlen(SCTP_MSGSENDFLAGS_ID));

    *pLen = currLen;

    return RV_OK;
}
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

/***************************************************************************
 * StoreLocalAddr
 * ------------------------------------------------------------------------
 * General: The function stores the CallLeg LocalAddress struct to be
 *          extended by sub-params.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Handle to the call-leg that holds the outbound msg.
 *          buffLen     - Length of the whole given buffer.
 * InOut:   ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreLocalAddr(
                               IN    CallLeg     *pCallLeg,
                               IN    RvUint32     buffLen,
                               INOUT RvChar     **ppCurrPos,
                               INOUT RvUint32    *pCurrLen)
{
    RvStatus  rv;
    RvInt32  localAddrLen;

    /* Firstly set the Local Address parameter prefix */
    /* to be extended by additional parameters        */
    rv = GetLocalAddrSize(pCallLeg,&localAddrLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            LOCAL_ADDR_ID,buffLen,localAddrLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = StoreLocalAddrParams(pCallLeg,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* Lastly the Local Address parameter suffix */
    rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen);

    return rv;
}

/***************************************************************************
 * StoreLocalAddrParams
 * ------------------------------------------------------------------------
 * General: The function stores a LocalAddr parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Pointer the CallLeg that contains the LocalAddr
 *          buffLen     - Length of the whole given buffer.
 * InOut:   ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreLocalAddrParams(
                        IN    CallLeg     *pCallLeg,
                        IN    RvUint32     buffLen,
                        INOUT RvChar     **ppCurrPos,
                        INOUT RvUint32    *pCurrLen)
{
    RvStatus                       rv;
    RvSipTransportMgrHandle        hTransportMgr = pCallLeg->pMgr->hTransportMgr;
    SipTransportObjLocalAddresses *pLocalAddr    = &(pCallLeg->localAddr);

    /* Udp - Ipv4 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalUdpIpv4,
                        LOCAL_ADDR_UDP_IPV4_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* Tcp - Ipv4 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalTcpIpv4,
                        LOCAL_ADDR_TCP_IPV4_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* Udp - Ipv6 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalUdpIpv6,
                        LOCAL_ADDR_UDP_IPV6_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }


    /* Tcp - Ipv6 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalTcpIpv6,
                        LOCAL_ADDR_TCP_IPV6_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Tls - Ipv4 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalTlsIpv4,
                        LOCAL_ADDR_TLS_IPV4_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }


    /* Tls - Ipv6 */
    rv = StoreIpAndPort(hTransportMgr,
                        pLocalAddr->hLocalTlsIpv6,
                        LOCAL_ADDR_TLS_IPV6_ID,
                        buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

#if (RV_NET_TYPE & RV_NET_SCTP)
	/* Sctp - Ipv4 */
    rv = StoreIpAndPort(hTransportMgr,
						pLocalAddr->hLocalSctpIpv4,
						LOCAL_ADDR_SCTP_IPV4_ID,
						buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
	
    /* Sctp - Ipv6 */
    rv = StoreIpAndPort(hTransportMgr,
						pLocalAddr->hLocalSctpIpv6,
						LOCAL_ADDR_SCTP_IPV6_ID,
						buffLen, ppCurrPos, pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    /* Store the Address In Use */
    rv = StoreAddrInUse(pCallLeg,buffLen,ppCurrPos,pCurrLen);

    return rv;
}

/***************************************************************************
 * StoreOutboundProxyAddr
 * ------------------------------------------------------------------------
 * General: The function stores the outbound-proxy parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg  - Handle to the call-leg that holds the outbound msg.
 *          buffLen   - Length of the whole given buffer.
 * InOut:   ppCurrPos - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen  - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreOutboundProxyAddr(
                   IN    CallLeg     *pCallLeg,
                   IN    RvUint32     buffLen,
                   INOUT RvChar     **ppCurrPos,
                   INOUT RvUint32    *pCurrLen)
{
    RvStatus   rv;
    RvInt32   outboundProxyLen;

    rv = GetOutboundProxyAddrSize(pCallLeg,&outboundProxyLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            OUTBOUND_PROXY_ID,buffLen,outboundProxyLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = StoreOutboundProxyAddrParams(
            &pCallLeg->outboundAddr,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen);

    return rv;
}

/***************************************************************************
 * StoreOutboundProxyAddrParams
 * ------------------------------------------------------------------------
 * General: The function stores the OutboundProxy extending parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg  - Handle to the call-leg that holds the outbound msg.
 *          buffLen   - Length of the whole given buffer.
 * InOut:   ppCurrPos - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen  - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreOutboundProxyAddrParams(
                   IN    SipTransportOutboundAddress   *pOutboundAddr,
                   IN    RvUint32                       buffLen,
                   INOUT RvChar                       **ppCurrPos,
                   INOUT RvUint32                      *pCurrLen)
{
    RvChar     strIpAddr[RV_ADDRESS_MAXSTRINGSIZE+(2*SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN)];
#ifdef RV_SIGCOMP_ON
    RvChar    *strCompType;
#endif /*#ifdef RV_SIGCOMP_ON*/
    RvStatus   rv;

    /* rpoolHostName */
    if (UNDEFINED != pOutboundAddr->rpoolHostName.offset)
    {
        RvUint32    strHostNameLen = 0;

        strHostNameLen = RPOOL_Strlen(pOutboundAddr->rpoolHostName.hPool,
                                      pOutboundAddr->rpoolHostName.hPage,
                                      pOutboundAddr->rpoolHostName.offset);
        rv = SipCommonHighAvalStoreSingleParamFromPage(
                            OUTBOUND_PROXY_RPOOL_HOST_NAME_ID,
                            pOutboundAddr->rpoolHostName.hPool,
                            pOutboundAddr->rpoolHostName.hPage,
                            pOutboundAddr->rpoolHostName.offset,
                            RV_FALSE,
                            buffLen,
                            strHostNameLen,
                            ppCurrPos,
                            pCurrLen);
        if (rv != RV_OK)
        {
            return rv;
        }
    }

    /* bIpAddressExistsOffset */
    rv = SipCommonHighAvalStoreSingleBoolParam(
                        OUTBOUND_PROXY_IP_ADDR_EXISTS_ID,
                        pOutboundAddr->bIpAddressExists,
                        buffLen,
                        ppCurrPos,
                        pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* ipAddress */
    /* Only if there's a valid IP address it is stored */
    if (RV_TRUE == pOutboundAddr->bIpAddressExists)
    {
        rv = GetOutboundProxyIpAddrStr(
                    &(pOutboundAddr->ipAddress),sizeof(strIpAddr),strIpAddr);
        if (rv != RV_OK)
        {
            return rv;
        }
        rv = SipCommonHighAvalStoreSingleParamFromStr(
               OUTBOUND_PROXY_IP_ADDR_ID,strIpAddr,buffLen,ppCurrPos,pCurrLen);
        if (rv != RV_OK)
        {
            return rv;
        }
    }

    /* bUseHostName */
    rv = SipCommonHighAvalStoreSingleBoolParam(
                            OUTBOUND_PROXY_USE_HOST_NAME_ID,
                            pOutboundAddr->bUseHostName,
                            buffLen,
                            ppCurrPos,
                            pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }
#ifdef RV_SIGCOMP_ON
    /* compType */
    if (pOutboundAddr->eCompType != RVSIP_COMP_UNDEFINED)
    {
        strCompType = SipMsgGetCompTypeName(pOutboundAddr->eCompType);
        rv          = SipCommonHighAvalStoreSingleParamFromStr(
                                        OUTBOUND_PROXY_COMP_TYPE_ID,
                                        strCompType,
                                        buffLen,
                                        ppCurrPos,
                                        pCurrLen);
        if (rv != RV_OK)
        {
            return rv;
        }
    }

    /* rpoolSigcompId */
    if (UNDEFINED != pOutboundAddr->rpoolSigcompId.offset)
    {
        RvUint32    strSigcompIdLen = 0;

        strSigcompIdLen = RPOOL_Strlen(pOutboundAddr->rpoolSigcompId.hPool,
                                      pOutboundAddr->rpoolSigcompId.hPage,
                                      pOutboundAddr->rpoolSigcompId.offset);
        rv = SipCommonHighAvalStoreSingleParamFromPage(
                            OUTBOUND_PROXY_RPOOL_SIGCOMPID_ID,
                            pOutboundAddr->rpoolSigcompId.hPool,
                            pOutboundAddr->rpoolSigcompId.hPage,
                            pOutboundAddr->rpoolSigcompId.offset,
                            RV_FALSE,
                            buffLen,
                            strSigcompIdLen,
                            ppCurrPos,
                            pCurrLen);
        if (rv != RV_OK)
        {
            return rv;
        }
    }

#endif /* RV_SIGCOMP_ON */

    return RV_OK;
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * StoreSctp
 * ------------------------------------------------------------------------
 * General: The function stores the SCTP parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg  - Handle to the call-leg that holds the outbound msg.
 *          buffLen   - Length of the whole given buffer.
 * InOut:   ppCurrPos - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen  - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreSctp(
                                           IN    CallLeg     *pCallLeg,
                                           IN    RvUint32     buffLen,
                                           INOUT RvChar     **ppCurrPos,
                                           INOUT RvUint32    *pCurrLen)
{
    RvStatus  rv;
    RvInt32   sctpLen;

    rv = GetSctpParamsSize(pCallLeg,&sctpLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            SCTP_ID,buffLen,sctpLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* sctpStream */
    rv = SipCommonHighAvalStoreSingleInt32Param(
                                    SCTP_STREAM_ID,
                                    pCallLeg->sctpParams.sctpStream,
                                    buffLen,
                                    ppCurrPos,
                                    pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    /* sctpMsgSendingFlags */
    rv = SipCommonHighAvalStoreSingleInt32Param(
                                    SCTP_MSGSENDFLAGS_ID,
                                    pCallLeg->sctpParams.sctpMsgSendingFlags,
                                    buffLen,
                                    ppCurrPos,
                                    pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen);

    return rv;
}
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

/***************************************************************************
 * StoreIpAndPort
 * ------------------------------------------------------------------------
 * General: The function stores Ip and Prot into the file.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransport  - Handle to the transport manager.
 *          hAddr       - Handle to the address.
 *          strAddrID   - The ID of the stored IP address.
 *          buffLen     - Length of the whole given buffer.
 * InOut:   ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *          pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreIpAndPort(
                               IN    RvSipTransportMgrHandle       hTransport,
                               IN    RvSipTransportLocalAddrHandle hAddr,
                               IN    RvChar                       *strAddrID,
                               IN    RvUint32                      buffLen,
                               INOUT RvChar                      **ppCurrPos,
                               INOUT RvUint32                     *pCurrLen)
{
    RvStatus     rv;
    RvUint16     port;
    RvChar       strIP  [RVSIP_TRANSPORT_LEN_STRING_IP];
#if defined(UPDATED_BY_SPIRENT)
    RvChar       if_name  [RVSIP_TRANSPORT_IFNAME_SIZE] = "";
    RvUint16     vdevblock = 0;
#endif
    RvChar       strAddr[RVSIP_TRANSPORT_LEN_STRING_IP+SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN];

    if (hAddr == NULL)
    {
        return RV_OK;
    }
    memset(strIP  , 0, sizeof(strIP));
    memset(strAddr, 0, sizeof(strAddr));
    rv = SipTransportLocalAddressGetAddressByHandle(
            hTransport,hAddr,RV_FALSE,RV_TRUE,strIP,&port
#if defined(UPDATED_BY_SPIRENT)
	    ,if_name,&vdevblock
#endif
     );
    if(rv != RV_OK)
    {
        return rv;
    }

    /* Concatenate the port string to the whole address string */
    RvSprintf(strAddr,LOCAL_ADDR_IP_ADDR_FORMAT, strIP,port);

    /* Store the IP address string */
    rv = SipCommonHighAvalStoreSingleParamFromStr(
                    strAddrID,strAddr,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
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
 *          strAddr    - The string that contains the restored IP and Port.
 * Out:     phAddress  - The resulting address object
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreIpAndPort(
                                 IN    RvSipTransportMgrHandle        hTransport,
                                 IN    RvSipTransport                 eTransport,
                                 IN    RvSipTransportAddressType      eAddrType,
                                 IN    RvChar                        *strAddr,
                                 OUT   RvSipTransportLocalAddrHandle *phAddr)
{
    RvStatus      rv;
    RvChar        strIp[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvUint16      port;
    RvSipTransportLocalAddrHandle hAddr = NULL;
    RvSscanf(strAddr,LOCAL_ADDR_IP_ADDR_FORMAT,strIp,&port);

    rv = SipTransportLocalAddressGetHandleByAddress(hTransport,
                                                    eTransport,
                                                    eAddrType,
                                                    strIp, port,
#if defined(UPDATED_BY_SPIRENT)
						    "", 0, /* we are not going to use high-availability in embedded apps */
#endif
                                                    &hAddr);
    if(hAddr != NULL)
    {
        *phAddr = hAddr;
    }
    return rv;
}


/***************************************************************************
* StoreSubsList
* ------------------------------------------------------------------------
* General: The function stores the subscription list.
*
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pCallLeg    - Handle to the call-leg that holds the outbound msg.
*          buffLen     - Length of the whole given buffer.
* InOut:   ppCurrPos   - An updated pointer to the end of allocation in the buffer.
*          pCurrLen    - An updated integer, with the size of so far allocation.
***************************************************************************/
static RvStatus RVCALLCONV StoreSubsList(IN    CallLeg      *pCallLeg,
                              IN    RvUint32      buffLen,
                              INOUT RvChar      **ppCurrPos,
                              INOUT RvUint32     *pCurrLen)
{
#ifdef RV_SIP_SUBS_ON
    RvStatus           rv;
    RvSipSubsHandle    hSubs         = NULL;
    RvSipSubsHandle    hRelativeSubs = NULL;
    RvInt32            currSubsLen;
    RvInt32            actualSubsLen;

    if (pCallLeg->hSubsList == NULL)
    {
        return RV_OK;
    }

    /* Get first active subscription one by one, and set it in call-leg */
    rv = GetActiveNonReferSubs(pCallLeg, hRelativeSubs, &hSubs);
    if(rv != RV_OK)
    {
        return rv;
    }

    while(hSubs != NULL)
    {
        /* First, retrieve the subscription length */
        rv = SipSubsGetActiveSubsStorageSize(hSubs,RV_FALSE,&currSubsLen);
        if (RV_OK != rv)
        {
            return rv;
        }
        /* Store the Subscription prefix */
        rv = SipCommonHighAvalStoreSingleParamPrefix(
                CALL_SUBS_ID,buffLen,currSubsLen,ppCurrPos,pCurrLen);
        if (RV_OK != rv)
        {
            return rv;
        }
        /* Store the Subscription itself */
        rv = SipSubsStoreActiveSubs(
                hSubs,RV_FALSE,buffLen-(*pCurrLen),*ppCurrPos,&actualSubsLen);
        if (RV_OK != rv)
        {
            return rv;
        }
        /* Verify that the current Subscription length and */
        /* the actual length equal */
        if (actualSubsLen != currSubsLen)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                  "StoreSubsList - Call-leg 0x%p got different Subs length (%d, %d)",
                  pCallLeg,currSubsLen,actualSubsLen));
            return RV_ERROR_UNKNOWN;
        }
        /* Promote the current position and length */
        *ppCurrPos += actualSubsLen;
        *pCurrLen  += actualSubsLen;
        /* Store the Subscription suffix */
        rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen) ;
        if (RV_OK != rv)
        {
            return rv;
        }

        /* get next active subscription */
        hRelativeSubs = hSubs;
        rv            = GetActiveNonReferSubs(pCallLeg, hRelativeSubs, &hSubs);
        if(rv != RV_OK)
        {
            return rv;
        }
    }
#else
    RV_UNUSED_ARG(pCallLeg);
    RV_UNUSED_ARG(buffLen);
    RV_UNUSED_ARG(ppCurrPos);
    RV_UNUSED_ARG(pCurrLen);
#endif /* #ifdef RV_SIP_SUBS_ON */

    return RV_OK;
}

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * GetActiveNonReferSubs
 * ------------------------------------------------------------------------
 * General: The function retrieves active subscriptions (which are triggered
 *          by a REFER request) from the subscriptions list
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg      - Handle to the call-leg.
 *          hRelativeSubs - The subscription to start the search from or NULL for
 *                          the begining.
 *          phActiveSubs  - The found active subscription.
 ***************************************************************************/
static RvStatus RVCALLCONV GetActiveNonReferSubs(
                              IN    CallLeg         *pCallLeg,
                              IN    RvSipSubsHandle  hRelativeSubs,
                              OUT   RvSipSubsHandle *phActiveSubs)
{
    RvStatus                    rv = RV_OK;
    RvSipSubsState              eSubsState;
    RvSipSubsEventPackageType   eEventPack;
    RvSipListLocation           eLocation;

    eLocation    = (hRelativeSubs == NULL) ? RVSIP_FIRST_ELEMENT: RVSIP_NEXT_ELEMENT;
    *phActiveSubs = NULL;
    do
    {
        rv = RvSipCallLegGetSubscription((RvSipCallLegHandle)pCallLeg,
                                         eLocation,
                                         hRelativeSubs,
                                         phActiveSubs);
        if(rv != RV_OK)
        {
            *phActiveSubs = NULL;
            return rv;
        }
        if (*phActiveSubs != NULL)
        {
            hRelativeSubs = *phActiveSubs;
            eLocation     = RVSIP_NEXT_ELEMENT;
            rv            = RvSipSubsGetCurrentState(*phActiveSubs, &eSubsState);
            if(rv != RV_OK)
            {
                return rv;
            }
            rv            = RvSipSubsGetEventPackageType(*phActiveSubs,&eEventPack);
            if (rv != RV_OK)
            {
                return rv;
            }
            if(eSubsState == RVSIP_SUBS_STATE_ACTIVE &&
               eEventPack != RVSIP_SUBS_EVENT_PACKAGE_TYPE_REFER)
            {
                return RV_OK;
            }
        }

    } while(*phActiveSubs != NULL); /*end of do {*/

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

/******************************************************************************
 * RestoreCallLegAuthObj
 * ----------------------------------------------------------------------------
 * General: restores the list of Authentication Data structures
 *          from the High Availability record buffer into the provided list.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  pAuthDataID    - prefix of the Authetnication Data in buffer
 *         authDataIDLen  - lenght of the prefix string string.
 *         pAuthDataVal   - string with Authetnication Data in buffer.
 *         authDataValLen - length of the Authetnication Data string in buffer.
 * Output: pParamObj      - handle of the CallLeg, for which the data is restor
 *****************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegAuthObj(
                                                IN    RvChar   *pAuthDataID,
                                                IN    RvUint32  authDataIDLen,
                                                IN    RvChar   *pAuthDataVal,
                                                IN    RvUint32  authDataValLen,
                                                IN    void     *pParamObj)
{
    CallLeg* pCallLeg = (CallLeg *)pParamObj;
#ifdef RV_SIP_AUTH_ON
    RvStatus rv;

    if (RV_TRUE != SipCommonMemBuffExactlyMatchStr(
        pAuthDataID, authDataIDLen, HIGHAVAIL_AUTHDATA_ID))
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegAuthObj - Call-leg 0x%p received illegal ID",
            pCallLeg));
        return RV_ERROR_BADPARAM;
    }

    rv = SipAuthenticatorHighAvailRestoreAuthObj(
        pCallLeg->pMgr->hAuthMgr, pAuthDataVal, authDataValLen, 
        pCallLeg->hPage, pCallLeg,
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
        (ObjLockFunc)CallLegLockAPI, (ObjUnLockFunc)CallLegUnLockAPI,
#else
        NULL, NULL,
#endif
        &pCallLeg->pAuthListInfo, &pCallLeg->hListAuthObj);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegAuthObj - Call-leg 0x%p failed to restore. Buff Len=%d (rv=%d:%s)",
            pCallLeg, authDataValLen, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
#else
    RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RestoreCallLegAuthObj - Cannot restore Authentocation Data in Call-leg 0x%p (not supported)",
        pCallLeg));
    RV_UNUSED_ARG(pAuthDataID);
    RV_UNUSED_ARG(authDataIDLen);
    RV_UNUSED_ARG(pAuthDataVal);
    RV_UNUSED_ARG(authDataValLen);
    RV_UNUSED_ARG(pParamObj);
#endif /* #ifdef RV_SIP_AUTH_ON */
    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegSubs
 * ------------------------------------------------------------------------
 * General: The function restores a CallLeg Subscription.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallSubsID      - The ID of the configured parameter.
 *        callSubsIDLen    - The length of the parameter ID.
 *        pCallSubsValue   - The value to be set in the paramObj data member.
 *        callSubsValueLen - The length of the parameter value.
 * InOut: pParamObj        - A reference to an object that might be
 *                           affected by the parameter value.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegSubs(
                                           IN    RvChar   *pCallSubsID,
                                           IN    RvUint32  callSubsIDLen,
                                           IN    RvChar   *pCallSubsValue,
                                           IN    RvUint32  callSubsValueLen,
                                           INOUT void     *pParamObj)
{

    CallLeg           *pCallLeg = (CallLeg *)pParamObj;
#ifdef RV_SIP_SUBS_ON
    RvStatus           rv       = RV_OK;
    RvSipSubsHandle    hSubs;

    if (SipCommonMemBuffExactlyMatchStr(pCallSubsID,callSubsIDLen,CALL_SUBS_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegSubs - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    /* 1. create subscription */
    rv = SipSubsMgrSubscriptionCreate(
                                pCallLeg->pMgr->hSubsMgr,
                                (RvSipCallLegHandle)pCallLeg,
                                NULL, /*hAppSubs */
                                RVSIP_SUBS_TYPE_SUBSCRIBER,
                                RV_FALSE, /*bOutOfBand*/
                                &hSubs
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
, -1 /* cctContext*/, 0xFF /*URI_cfg*/ /*we do not support high-availability*/
#endif
//SPIRENT_END
                                );
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegSubs - Call-leg 0x%p failed to create subs.",
            pCallLeg));

        return rv;
    }
    /* 2. set subscription parameters */
    rv = SipSubsRestoreActiveSubs(pCallSubsValue,callSubsValueLen,RV_FALSE,hSubs);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegSubs - Failed to restore subs 0x%p. Call-leg 0x%p",
            hSubs, pCallLeg));
        return rv;
    }
#else  /* #ifdef RV_SIP_SUBS_ON */
    RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
        "RestoreCallLegSubs - Cannot restore subscription in Call-leg 0x%p (not supported)",
        pCallLeg));

    RV_UNUSED_ARG(pCallSubsID);
    RV_UNUSED_ARG(callSubsIDLen);
    RV_UNUSED_ARG(pCallSubsValue);
    RV_UNUSED_ARG(callSubsValueLen);
    RV_UNUSED_ARG(pParamObj);

#endif /* #ifdef RV_SIP_SUBS_ON */
    return RV_OK;
}

/***************************************************************************
 * GetCallLegHeadersStorageSize
 * ------------------------------------------------------------------------
 * General: Retrieve headers storage size of the CallLeg to be stored
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Handle to the call-leg.
 * Output:  pLen     - The current Size of the required storage buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV GetCallLegHeadersStorageSize(IN    CallLeg *pCallLeg,
                                                        OUT   RvInt32 *pLen)
{
    RvStatus rv;
    RvInt32  currLen = 0;
    RvChar   tempChar;
    RvInt32  tempLen = 0;

    *pLen = UNDEFINED;

    /* From header */
    rv = GetCallLegSingleHeaderStorageSize(
            pCallLeg,RVSIP_HEADERTYPE_FROM,RV_TRUE,&currLen);
    if (rv != RV_OK)
    {
        return rv;
    }
    /* To header */
    rv = GetCallLegSingleHeaderStorageSize(
            pCallLeg,RVSIP_HEADERTYPE_TO,RV_TRUE,&currLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* strCallId */
    rv = CallLegGetCallId(pCallLeg,sizeof(tempChar),&tempChar,&tempLen);
    currLen += SipCommonHighAvalGetTotalStoredParamLen(tempLen,(RvUint32)strlen(CALL_ID_ID));

    /* initial authorization */
#ifdef RV_SIP_IMS_ON
    if(pCallLeg->hInitialAuthorization != NULL)
    {
        rv = GetCallLegSingleHeaderStorageSize(
            pCallLeg,RVSIP_HEADERTYPE_AUTHORIZATION,RV_TRUE,&currLen);
        if (rv != RV_OK)
        {
            return rv;
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    *pLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * GetCallLegContactsStorageSize
 * ------------------------------------------------------------------------
 * General: Retrieve contacts storage size of the CallLeg to be stored
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg - Handle to the call-leg.
 * Output:  pLen     - The current Size of the required storage buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV GetCallLegContactsStorageSize(IN    CallLeg *pCallLeg,
                                                         OUT   RvInt32 *pLen)
{
    RvStatus rv;
    RvInt32  currLen = 0;

    *pLen = UNDEFINED;

    /* local contact */
    rv = GetCallLegSingleAddrStorageSize(pCallLeg,RV_TRUE,&currLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* remote contact */
    rv = GetCallLegSingleAddrStorageSize(pCallLeg,RV_FALSE,&currLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    *pLen = currLen;

    return RV_OK;
}

/***************************************************************************
 * GetCallLegSingleHeaderStorageSize
 * ------------------------------------------------------------------------
 * General: Retrieves a single header storage size of the CallLeg to be stored
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Handle to the call-leg.
 *          eHeaderType - The header type.
 *          bProxy      - Indication for authentication header type.
 * Output:  pCurrLen    - The current Size of the required storage buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV GetCallLegSingleHeaderStorageSize(
                              IN     CallLeg         *pCallLeg,
                              IN     RvSipHeaderType  eHeaderType,
                              IN     RvBool           bProxy,
                              INOUT  RvInt32         *pCurrLen)
{
    HRPOOL   hCallPool  = pCallLeg->pMgr->hGeneralPool;
    HPAGE    tempPage;
    RvUint32 encodedLen;
    RvChar  *strHeaderID;
    RvStatus rv;

    /* Add the current header content length */
    rv = EncodeCallLegSingleHeader(
            pCallLeg,eHeaderType,bProxy,&tempPage,&encodedLen,&strHeaderID);
    if(rv != RV_OK)
    {
        return rv;
    }

    if (encodedLen > 0)
    {
        RPOOL_FreePage(hCallPool, tempPage);
        /* Find the total length of the current header */
        *pCurrLen += SipCommonHighAvalGetTotalStoredParamLen(
                                   encodedLen,(RvUint32)strlen(strHeaderID));
    }

    return rv;
}

/***************************************************************************
 * GetCallLegSingleAddrStorageSize
 * ------------------------------------------------------------------------
 * General: Retrieves a single header storage size of the CallLeg to be stored
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg   - Handle to the call-leg.
 *          bLocal      - Indication for header type that is duplicated
 *                        in a CallLeg.
 * Output:  pCurrLen    - The current Size of the required storage buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV GetCallLegSingleAddrStorageSize(
                              IN     CallLeg         *pCallLeg,
                              IN     RvBool           bLocal,
                              INOUT  RvInt32         *pCurrLen)
{
    HRPOOL   hCallPool  = pCallLeg->pMgr->hGeneralPool;
    RvUint32 encodedLen = 0;
    RvChar  *strAddrID;
    HPAGE    tempPage;
    RvStatus rv;

    rv = EncodeCallLegSingleAddr(pCallLeg,bLocal,&tempPage,&encodedLen,&strAddrID);
    if(rv != RV_OK)
    {
        return rv;
    }

    if (encodedLen > 0)
    {
        RPOOL_FreePage(hCallPool, tempPage);
        *pCurrLen += SipCommonHighAvalGetTotalStoredParamLen(
                                    encodedLen,(RvUint32)strlen(strAddrID));
    }

    return rv;
}

/***************************************************************************
 * EncodeCallLegSingleHeader
 * ------------------------------------------------------------------------
 * General: Encodes a single header of the CallLeg to be stored and find its
 *          header ID to be stored.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg     - Handle to the call-leg.
 *          eHeaderType  - the header type.
 *          bProxy       - Indication for authentication header type.
 * Output:  pCurrLen     - The current Size of the required storage buffer.
 *          pStrHeaderID - The encoded Header ID.
 ***************************************************************************/
static RvStatus RVCALLCONV EncodeCallLegSingleHeader(
                              IN     CallLeg         *pCallLeg,
                              IN     RvSipHeaderType  eHeaderType,
                              IN     RvBool           bProxy,
                              OUT    HPAGE           *pEncodedPage,
                              OUT    RvUint32        *pEncodedLen,
                              OUT    RvChar         **pStrHeaderID)
{
    HRPOOL   hCallPool = pCallLeg->pMgr->hGeneralPool;
    void    *hHeader;
    RvStatus rv;

    *pEncodedLen  = 0;
    *pStrHeaderID = NULL;

    switch (eHeaderType)
    {
    case RVSIP_HEADERTYPE_TO:
        hHeader       = (void *)pCallLeg->hToHeader;
        *pStrHeaderID = TO_HEADER_ID;
        break;
    case RVSIP_HEADERTYPE_FROM:
        hHeader       = (void *)pCallLeg->hFromHeader;
        *pStrHeaderID = FROM_HEADER_ID;
        break;
#ifdef RV_SIP_IMS_ON
    case RVSIP_HEADERTYPE_AUTHORIZATION:
        hHeader = (void*)pCallLeg->hInitialAuthorization;
        *pStrHeaderID = INITIAL_AUTHORIZATION_ID;
        break;
#endif /*#ifdef RV_SIP_IMS_ON*/
    default:
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "EncodeCallLegSingleHeader: Call-leg 0x%p doesn't contain header type %d",
                pCallLeg, eHeaderType));
        return RV_ERROR_BADPARAM;
    }

    rv = RvSipHeaderEncode(hHeader,eHeaderType,hCallPool,pEncodedPage,pEncodedLen);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "EncodeCallLegSingleHeader: Call-leg 0x%p failed to encode header type %d",
                pCallLeg, eHeaderType));
        return rv;
    }
    RV_UNUSED_ARG(bProxy)
    return rv;
}

/***************************************************************************
 * EncodeCallLegSingleAddr
 * ------------------------------------------------------------------------
 * General: Encodes a single addr of the CallLeg to be stored and finds
 *          the encoded parameter ID to be stored as well.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pCallLeg    - Handle to the call-leg.
 *          bLocal      - Indication for Local/Remote contact addr.
 * Output:  pCurrLen    - The current Size of the required storage buffer.
 *          pStrAddrID  - The encoded parameter ID.
 ***************************************************************************/
static RvStatus RVCALLCONV EncodeCallLegSingleAddr(
                              IN     CallLeg         *pCallLeg,
                              IN     RvBool           bLocal,
                              OUT    HPAGE           *pEncodedPage,
                              OUT    RvUint32        *pEncodedLen,
                              OUT    RvChar         **pStrAddrID)
{
    RvSipAddressHandle hAddr;
    RvStatus           rv;

    *pEncodedLen = 0;
    if (bLocal == RV_TRUE)
    {
        hAddr       = pCallLeg->hLocalContact;
        *pStrAddrID = LOCAL_CONTACT_ID;
    }
    else
    {
        hAddr       = pCallLeg->hRemoteContact;
        *pStrAddrID = REMOTE_CONTACT_ID;
    }

    if (hAddr == NULL)
    {
        return RV_OK;
    }

    rv = RvSipAddrEncode(hAddr, pCallLeg->pMgr->hGeneralPool,pEncodedPage,pEncodedLen);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "EncodeCallLegSingleAddr: Call-leg 0x%p failed to encode addr (bLocal=%d)",
                pCallLeg, bLocal));
        return rv;
    }

    return rv;

}

/***************************************************************************
 * StoreCallLegHeaders
 * ------------------------------------------------------------------------
 * General: The function stores the headers of a call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegHeaders(
                                 IN    CallLeg                  *pCallLeg,
                                 IN    RvUint32                  buffLen,
                                 INOUT RvChar                  **ppCurrPos,
                                 INOUT RvUint32                 *pCurrLen)
{
    RvStatus    rv;

    /* From Header */
    rv = StoreCallLegSingleHeader(
            pCallLeg,RVSIP_HEADERTYPE_FROM,RV_TRUE,buffLen,ppCurrPos,pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* To Header */
    rv = StoreCallLegSingleHeader(
            pCallLeg,RVSIP_HEADERTYPE_TO,RV_TRUE,buffLen,ppCurrPos,pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* strCallId */
    rv = StoreCallLegCallID(pCallLeg,buffLen,ppCurrPos,pCurrLen);
    if(rv != RV_OK)
    {
        return rv;
    }

#ifdef RV_SIP_IMS_ON
    if(pCallLeg->hInitialAuthorization != NULL)
    {
        rv = StoreCallLegSingleHeader(pCallLeg,RVSIP_HEADERTYPE_AUTHORIZATION,
                                      RV_TRUE,buffLen,ppCurrPos,pCurrLen);
        if(rv != RV_OK)
        {
            return rv;
        }
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    return RV_OK;
}

/***************************************************************************
 * StoreCallLegContacts
 * ------------------------------------------------------------------------
 * General: The function stores the addrs of a call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegContacts(
                             IN    CallLeg      *pCallLeg,
                             IN    RvUint32      buffLen,
                             INOUT RvChar      **ppCurrPos,
                             INOUT RvUint32     *pCurrLen)
{
    RvStatus    rv;

    /* Local Contact */
    rv = StoreCallLegSingleAddr(
            pCallLeg,RV_TRUE,buffLen,ppCurrPos,pCurrLen);
    if(rv!= RV_OK)
    {
        return rv;
    }

    /* Remote Contact */
    rv = StoreCallLegSingleAddr(
            pCallLeg,RV_FALSE,buffLen,ppCurrPos,pCurrLen);
    if(rv!= RV_OK)
    {
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * StoreCallLegSingleHeader
 * ------------------------------------------------------------------------
 * General: The function stores a single header of a call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        bLocal      - Indication for authentication header type.
 *        eHeaderType - The type of the CallLeg header to store.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegSingleHeader(
                                 IN    CallLeg                  *pCallLeg,
                                 IN    RvSipHeaderType           eHeaderType,
                                 IN    RvBool                    bProxy,
                                 IN    RvUint32                  buffLen,
                                 INOUT RvChar                  **ppCurrPos,
                                 INOUT RvUint32                 *pCurrLen)
{
    HPAGE       tempPage;
    RvUint32    encodedLen = 0;
    HRPOOL      hCallPool  = pCallLeg->pMgr->hGeneralPool;
    RvChar     *strHeaderID;
    RvStatus    rv;

    rv = EncodeCallLegSingleHeader(
            pCallLeg,eHeaderType,bProxy,&tempPage,&encodedLen,&strHeaderID);
    if(rv != RV_OK)
    {
        return rv;
    }
    if (encodedLen > 0)
    {
        rv = SipCommonHighAvalStoreSingleParamFromPage(
              strHeaderID,hCallPool,tempPage,0,RV_TRUE,buffLen,encodedLen,ppCurrPos,pCurrLen);
    }

    return rv;
}

/***************************************************************************
 * StoreCallLegCallID
 * ------------------------------------------------------------------------
 * General: The function stores a single header of a call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        bLocal      - Indication for authentication header type.
 *        eHeaderType - The type of the CallLeg header to store.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegCallID(
                                 IN    CallLeg     *pCallLeg,
                                 IN    RvUint32     buffLen,
                                 INOUT RvChar     **ppCurrPos,
                                 INOUT RvUint32    *pCurrLen)
{
    RvChar   tempChar;
    RvInt32  callIDLen;
    RvStatus rv;

    /* Retrieve the CallLeg ID length */
    rv = CallLegGetCallId(pCallLeg,sizeof(tempChar),&tempChar,&callIDLen);
    if (*pCurrLen + callIDLen > buffLen)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    /* Store the parameter prefix */
    rv = SipCommonHighAvalStoreSingleParamPrefix(
            CALL_ID_ID,buffLen,callIDLen,ppCurrPos,pCurrLen);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Store the parameter content */
    rv = CallLegGetCallId(pCallLeg,buffLen-*pCurrLen,*ppCurrPos,&callIDLen);
    if(rv != RV_OK)
    {
        return rv;
    }
    *ppCurrPos += callIDLen;
    *pCurrLen  += callIDLen;

    /* Store the parameter suffix */
    rv = SipCommonHighAvalStoreSingleParamSuffix(buffLen,ppCurrPos,pCurrLen);

    return rv;
}

/***************************************************************************
 * StoreCallLegSingleAddr
 * ------------------------------------------------------------------------
 * General: The function stores a single addr of a call-leg to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        bLocal      - Indication for header type that is duplicated
 *                      in a CallLeg.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegSingleAddr(
                                 IN    CallLeg                  *pCallLeg,
                                 IN    RvBool                    bLocal,
                                 IN    RvUint32                  buffLen,
                                 INOUT RvChar                  **ppCurrPos,
                                 INOUT RvUint32                 *pCurrLen)
{
    HPAGE       tempPage;
    RvUint32    encodedLen = 0;
    HRPOOL      hCallPool  = pCallLeg->pMgr->hGeneralPool;
    RvChar     *strAddrID;
    RvStatus    rv;

    rv = EncodeCallLegSingleAddr(pCallLeg,bLocal,&tempPage,&encodedLen,&strAddrID);
    if(rv != RV_OK)
    {
        return rv;
    }
    if (encodedLen > 0)
    {
        rv = SipCommonHighAvalStoreSingleParamFromPage(
                strAddrID,hCallPool,tempPage,0,RV_TRUE,buffLen,encodedLen,ppCurrPos,pCurrLen);
    }

    return rv;
}

/***************************************************************************
 * StoreAddrInUse
 * ------------------------------------------------------------------------
 * General: The function stores address in use parameter of a call-leg
 *          to buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg that holds the route list.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreAddrInUse(IN    CallLeg      *pCallLeg,
                                          IN    RvUint32      buffLen,
                                          INOUT RvChar      **ppCurrPos,
                                          INOUT RvUint32     *pCurrLen)
{
    RvChar   *strAddrInUse;
    RvStatus  rv;

    strAddrInUse = GetAddrInUseID(pCallLeg);
    if (NULL == strAddrInUse)
    {
        return RV_OK;
    }

    rv = SipCommonHighAvalStoreSingleParamFromStr(
            LOCAL_ADDR_ADDR_IN_USE_ID,strAddrInUse,buffLen,ppCurrPos,pCurrLen);

    return rv;
}

/***************************************************************************
 * GetCallLegGeneralParamsSize
 * ------------------------------------------------------------------------
 * General: The function calculates the CallLeg stored general parameters
 *          length.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg  - A pointer to the CallLeg.
 * Output: pCurrLen  - The total length which is required for the storage
 *                     of the CallLeg general parameters.
 ***************************************************************************/
static RvStatus RVCALLCONV GetCallLegGeneralParamsSize(
                                    IN  CallLeg  *pCallLeg,
                                    OUT RvInt32  *pCurrLen)
{
    const RvChar *strDirection;
    const RvChar *strState;

    *pCurrLen = 0;

    /* local cseq */
    *pCurrLen += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                        pCallLeg->localCSeq.val,(RvUint32)strlen(LOCAL_CSEQ_ID));

    /* remote cseq */
    *pCurrLen += SipCommonHighAvalGetTotalStoredInt32ParamLen(
                        pCallLeg->remoteCSeq.val,(RvUint32)strlen(REMOTE_CSEQ_ID));

#ifdef RV_SIP_UNSIGNED_CSEQ
	/* is local cseq initialized */ 
	*pCurrLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
					pCallLeg->localCSeq.bInitialized,(RvUint32)strlen(IS_LOCAL_CSEQ_INIT_ID));

	/* is remote cseq initialized */ 
	*pCurrLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
					pCallLeg->remoteCSeq.bInitialized,(RvUint32)strlen(IS_REMOTE_CSEQ_INIT_ID));
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

    /* eDirection */
    strDirection = SipCallLegGetDirectionName(pCallLeg->eDirection);
    *pCurrLen   += SipCommonHighAvalGetTotalStoredParamLen((RvUint32)strlen(strDirection),
                                                           (RvUint32)strlen(DIRECTION_ID));

    /* state that might not be CONNECTED in state owning subs */
    strState   = SipCallLegGetStateName(pCallLeg->eState);
    *pCurrLen += SipCommonHighAvalGetTotalStoredParamLen((RvUint32)strlen(strState),
                                                         (RvUint32)strlen(STATE_ID));
    /* bForkingEnabled */
    *pCurrLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
                        pCallLeg->bForkingEnabled,(RvUint32)strlen(FORKING_ENABLED_ID));

#ifdef RV_SIGCOMP_ON
	/* bSigCompEnabled */
    *pCurrLen += SipCommonHighAvalGetTotalStoredBoolParamLen(
						pCallLeg->bSigCompEnabled,(RvUint32)strlen(IS_SIGCOMP_ENABLED_ID));
#endif /* #ifdef RV_SIGCOMP_ON */ 
	
    return RV_OK;
}

/***************************************************************************
 * StoreCallLegGeneralParams
 * ------------------------------------------------------------------------
 * General: The function stores the CallLeg general parameters.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg    - Handle to the call-leg.
 *        buffLen     - Length of the whole given buffer.
 * InOut: ppCurrPos   - An updated pointer to the end of allocation in the buffer.
 *        pCurrLen    - An updated integer, with the size of so far allocation.
 ***************************************************************************/
static RvStatus RVCALLCONV StoreCallLegGeneralParams(
                             IN    CallLeg      *pCallLeg,
                             IN    RvUint32      buffLen,
                             INOUT RvChar      **ppCurrPos,
                             INOUT RvUint32     *pCurrLen)
{
    const RvChar  *strDirection;
    const RvChar  *strState;
    RvStatus       rv;

    /* local cseq */
    rv = SipCommonHighAvalStoreSingleInt32Param(
            LOCAL_CSEQ_ID,pCallLeg->localCSeq.val,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* remote cseq */
    rv = SipCommonHighAvalStoreSingleInt32Param(
            REMOTE_CSEQ_ID,pCallLeg->remoteCSeq.val,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

#ifdef RV_SIP_UNSIGNED_CSEQ
	/* is local cseq initialized */
    rv = SipCommonHighAvalStoreSingleBoolParam(
		IS_LOCAL_CSEQ_INIT_ID,pCallLeg->localCSeq.bInitialized,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

	/* is remote cseq initialized */
    rv = SipCommonHighAvalStoreSingleBoolParam(
		IS_REMOTE_CSEQ_INIT_ID,pCallLeg->remoteCSeq.bInitialized,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

    /* eDirection */
    strDirection = SipCallLegGetDirectionName(pCallLeg->eDirection);
    rv           = SipCommonHighAvalStoreSingleParamFromStr(
                        DIRECTION_ID,strDirection,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* state that might not be CONNECTED in state owning subs */
    strState = SipCallLegGetStateName(pCallLeg->eState);
    rv       = SipCommonHighAvalStoreSingleParamFromStr(
                    STATE_ID,strState,buffLen,ppCurrPos,pCurrLen);
    if (rv != RV_OK)
    {
        return rv;
    }

    /* bForkingEnabled */
    rv = SipCommonHighAvalStoreSingleBoolParam(
            FORKING_ENABLED_ID,pCallLeg->bForkingEnabled,buffLen,ppCurrPos,pCurrLen);
	if (rv != RV_OK)
    {
        return rv;
    }
	
#ifdef RV_SIGCOMP_ON
	/* bSigCompEnabled */
    rv = SipCommonHighAvalStoreSingleBoolParam(
		IS_SIGCOMP_ENABLED_ID,pCallLeg->bSigCompEnabled,buffLen,ppCurrPos,pCurrLen);
	if (rv != RV_OK)
    {
        return rv;
    }
#endif /* #ifdef RV_SIGCOMP_ON */

    return RV_OK;
}

/***************************************************************************
 * GetAddrInUseID
 * ------------------------------------------------------------------------
 * General: The function retrieves the string that identifies the address
 *          in use type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - The pointer to the CallLeg.
 ***************************************************************************/
static RvChar *RVCALLCONV GetAddrInUseID(IN CallLeg  *pCallLeg)
{
    if (NULL == pCallLeg->localAddr.hAddrInUse ||
        NULL == *pCallLeg->localAddr.hAddrInUse)
    {
        return NULL;
    }

    if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalUdpIpv4)
    {
        return LOCAL_ADDR_UDP_IPV4_ID;
    }
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalUdpIpv6)
    {
        return LOCAL_ADDR_UDP_IPV6_ID;
    }
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalTcpIpv4)
    {
        return LOCAL_ADDR_TCP_IPV4_ID;
    }
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalTcpIpv6)
    {
        return LOCAL_ADDR_TCP_IPV6_ID;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalTlsIpv4)
    {
        return LOCAL_ADDR_TLS_IPV4_ID;
    }
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalTlsIpv6)
    {
        return LOCAL_ADDR_TLS_IPV6_ID;
    }
#endif /* (RV_TLS_TYPE != RV_TLS_NONE) */
#if (RV_NET_TYPE & RV_NET_SCTP)
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalSctpIpv4)
    {
        return LOCAL_ADDR_SCTP_IPV4_ID;
    }
    else if (*pCallLeg->localAddr.hAddrInUse == pCallLeg->localAddr.hLocalSctpIpv6)
    {
        return LOCAL_ADDR_SCTP_IPV6_ID;
    }
#endif /* (RV_NET_TYPE & RV_NET_SCTP) */
    
    RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
               "GetAddrInUseID - Call-leg 0x%p LocalAddr AddrInUse is illegal",
               pCallLeg));

    return NULL;
}

/***************************************************************************
 * RestoreCallLegPartyHeader
 * ------------------------------------------------------------------------
 * General: The function restores a CallLeg party header from a given buffer
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pHeaderID       - The party header ID.
 *         headerIDLen     - The party header ID length.
 *         pHeaderValue    - The party header value string.
 *         headerValueLen  - Length of the party header value string.
 * InOut:  pObj            - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegPartyHeader(
                                          IN    RvChar   *pHeaderID,
                                          IN    RvUint32  headerIDLen,
                                          IN    RvChar   *pHeaderValue,
                                          IN    RvUint32  headerValueLen,
                                          INOUT void     *pObj)
{
    CallLeg                *pCallLeg       = (CallLeg *)pObj;
    HRPOOL                  hCallPool      = pCallLeg->pMgr->hGeneralPool;
    RvChar                  termChar;
    RvSipPartyHeaderHandle *phPartyHeader;
    RvSipHeaderType         eHeaderType;
    RvBool                  bIsTo;
    RvStatus                rv;

    if (SipCommonMemBuffExactlyMatchStr(pHeaderID,headerIDLen,TO_HEADER_ID) == RV_TRUE)
    {
        bIsTo         = RV_TRUE;
        eHeaderType   = RVSIP_HEADERTYPE_TO;
        phPartyHeader = &(pCallLeg->hToHeader);
    }
    else if (SipCommonMemBuffExactlyMatchStr(pHeaderID,headerIDLen,FROM_HEADER_ID) == RV_TRUE)
    {
        bIsTo         = RV_FALSE;
        eHeaderType   = RVSIP_HEADERTYPE_FROM;
        phPartyHeader = &(pCallLeg->hFromHeader);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                    "RestoreCallLegPartyHeader - Call-leg 0x%p received illegal parameter ID which cannot be restored",
                    pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    rv = RvSipPartyHeaderConstruct(
            pCallLeg->pMgr->hMsgMgr, hCallPool, pCallLeg->hPage, phPartyHeader);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegPartyHeader - Call-leg 0x%p failed to construct header of type %d (to/from)",
             pCallLeg,eHeaderType));

        return rv;
    }

    rv = SipPartyHeaderSetType(*phPartyHeader, eHeaderType);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegPartyHeader - Call-leg 0x%p failed to set header type %d (to/from)",
             pCallLeg,eHeaderType));

        return rv;
    }

    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pHeaderValue,headerValueLen,&termChar);
    rv = RvSipPartyHeaderParse(*phPartyHeader,bIsTo,pHeaderValue);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,headerValueLen,pHeaderValue);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegPartyHeader - Failed to parse header (type=%d). Call-leg 0x%p",
            eHeaderType,pCallLeg));

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegCallID
 * ------------------------------------------------------------------------
 * General: The function restores a call-leg Call-ID header from a given buffer
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallIDID       - The Call-ID header ID.
 *         callIDLen       - The Call-ID header ID length.
 *         pCallIDValue    - The Call-ID header value string.
 *         callIDValueLen  - Length of the Call-ID header value string.
 * InOut:  pObj            - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegCallID(IN    RvChar   *pCallIDID,
                                     IN    RvUint32  callIDLen,
                                     IN    RvChar   *pCallIDValue,
                                     IN    RvUint32  callIDValueLen,
                                     INOUT void     *pObj)
{
    CallLeg    *pCallLeg  = (CallLeg *)pObj;
    RvChar      termChar;
    RvStatus    rv;

    if (SipCommonMemBuffExactlyMatchStr(pCallIDID,callIDLen,CALL_ID_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegCallID - Received illegal parameter ID in Call-leg 0x%p",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pCallIDValue,callIDValueLen,&termChar);
    rv = RvSipCallLegSetCallId((RvSipCallLegHandle)pCallLeg,pCallIDValue);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,callIDValueLen,pCallIDValue);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegCallID - Failed to set CallID in Call-leg 0x%p",
            pCallLeg));

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegContact
 * ------------------------------------------------------------------------
 * General: The function restores a call-leg contact addr from a given buffer
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pContactID       - The contact ID.
 *         contactIDLen     - The contact ID length.
 *         pContactValue    - The contact value string.
 *         contactValueLen  - Length of the contact value string.
 * InOut:  pObj             - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegContact(
                                   IN    RvChar   *pContactID,
                                   IN    RvUint32  contactIDLen,
                                   IN    RvChar   *pContactValue,
                                   IN    RvUint32  contactValueLen,
                                   INOUT void     *pObj)
{
    CallLeg            *pCallLeg  = (CallLeg *)pObj;
    RvChar              termChar;
    RvStatus            rv;
    RvSipAddressType    eStrAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    RvSipAddressHandle *phAddr;

    if (SipCommonMemBuffExactlyMatchStr(pContactID,
                                        contactIDLen,
                                        LOCAL_CONTACT_ID) == RV_TRUE)
    {
        phAddr = &(pCallLeg->hLocalContact);
    }
    else if (SipCommonMemBuffExactlyMatchStr(pContactID,
                                             contactIDLen,
                                             REMOTE_CONTACT_ID) == RV_TRUE)
    {
        phAddr = &(pCallLeg->hRemoteContact);
    }
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegContact - Call-leg 0x%p received illegal contact ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pContactValue,contactValueLen,&termChar);
    eStrAddrType = SipAddrGetAddressTypeFromString(pCallLeg->pMgr->hMsgMgr, pContactValue);
    rv           = RvSipAddrConstruct(
                        pCallLeg->pMgr->hMsgMgr,
                        pCallLeg->pMgr->hGeneralPool,
                        pCallLeg->hPage,
                        eStrAddrType,
                        phAddr);
    if (rv != RV_OK)
    {
        /* Restore the given buffer altough no parsing took place */
        SipCommonHighAvalRestoreParamBufferAfterParse(termChar,contactValueLen,pContactValue);
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegContact - Failed to construct contact addr. Call-leg 0x%p",pCallLeg));

        return rv;
    }

    rv = RvSipAddrParse(*phAddr, pContactValue);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,contactValueLen,pContactValue);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegContact - Failed to parse contact addr. Call-leg 0x%p",
            pCallLeg));

        return rv;
    }

    return RV_OK;
}

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RestoreCallLegInitialAuthorization
 * ------------------------------------------------------------------------
 * General: The function restores a call-leg initial authorization
 *          from a given buffer
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pContactID       - The contact ID.
 *         contactIDLen     - The contact ID length.
 *         pContactValue    - The contact value string.
 *         contactValueLen  - Length of the contact value string.
 * InOut:  pObj             - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegInitialAuthorization(
                                   IN    RvChar   *pParamID,
                                   IN    RvUint32  paramIDLen,
                                   IN    RvChar   *pHeaderValue,
                                   IN    RvUint32  headerValueLen,
                                   INOUT void     *pObj)
{
    CallLeg            *pCallLeg  = (CallLeg *)pObj;
    RvChar              termChar;
    RvStatus            rv;

    if (SipCommonMemBuffExactlyMatchStr(pParamID,
                                        paramIDLen,
                                        INITIAL_AUTHORIZATION_ID) == RV_FALSE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegInitialAuthorization - Call-leg 0x%p received illegal contact ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    /* construct new authorization header */
    rv = RvSipAuthorizationHeaderConstruct(
                    pCallLeg->pMgr->hMsgMgr, pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, 
                    &(pCallLeg->hInitialAuthorization));
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegInitialAuthorization - Call-leg 0x%p failed to construct header",
            pCallLeg));
        
        return rv;
    }
    
    /* Prepare the header value for parsing process */
    SipCommonHighAvalPrepareParamBufferForParse(pHeaderValue,headerValueLen,&termChar);
    rv = RvSipAuthorizationHeaderParse(pCallLeg->hInitialAuthorization, pHeaderValue);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,headerValueLen,pHeaderValue);
    if (rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegInitialAuthorization - Failed to parse header. Call-leg 0x%p",
            pCallLeg));
        
        return rv;
    }
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/
/***************************************************************************
 * RestoreCallLegRoute
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg route list according to the
 *          ROUTE_IDs order in the restoration buffer.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pRouteID       - The route ID.
 *         routeIDLen     - The route ID length.
 *         pRouteValue    - The route value string.
 *         routeValueLen  - Length of the route value string.
 * InOut:  pObj           - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegRoute(IN    RvChar   *pRouteID,
                                    IN    RvUint32  routeIDLen,
                                    IN    RvChar   *pRouteValue,
                                    IN    RvUint32  routeValueLen,
                                    INOUT void     *pObj)
{
    CallLeg            *pCallLeg  = (CallLeg *)pObj;
    RvChar              termChar;
    RvStatus            rv;

    if (SipCommonMemBuffExactlyMatchStr(pRouteID,routeIDLen,ROUTE_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegRoute - Call-leg 0x%p received illegal route ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }
    /*--------------------------------------
       Construct the Route list if needed
     ----------------------------------------*/
    if(pCallLeg->hRouteList == NULL)
    {
        pCallLeg->hRouteList = RLIST_RPoolListConstruct(
                                            pCallLeg->pMgr->hGeneralPool,
                                            pCallLeg->hPage,
                                            sizeof(void*),
                                            pCallLeg->pMgr->pLogSrc);
        if(pCallLeg->hRouteList == NULL)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "RestoreCallLegRoute - Call-leg 0x%p Failed to construct a new route list",pCallLeg));

            return RV_ERROR_UNKNOWN;
        }
    }

    SipCommonHighAvalPrepareParamBufferForParse(pRouteValue,routeValueLen,&termChar);
    rv = RouteListAddElement(pCallLeg,pRouteValue,pCallLeg->hRouteList);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,routeValueLen,pRouteValue);
    if(rv != RV_OK)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegRoute - Call-Leg 0x%p - Failed add route to RouteList - Record-Route mechanism will not be supported for this call-leg",pCallLeg));

        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegLocalAddr
 * ------------------------------------------------------------------------
 * General: The function restores the local-address parameters.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pLocalAddrID      - The ID of the configured parameter.
 *        localAddrIDLen    - The length of the parameter ID.
 *        pLocalAddrValue   - The parameter value string.
 *        localAddrValueLen - The length of the parameter value string.
 * InOut: pObj              - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegLocalAddr(
                                 IN    RvChar   *pLocalAddrID,
                                 IN    RvUint32  localAddrIDLen,
                                 IN    RvChar   *pLocalAddrValue,
                                 IN    RvUint32  localAddrValueLen,
                                 INOUT void     *pObj)
{
    CallLeg                       *pCallLeg   = (CallLeg *)pObj;
    SipTransportObjLocalAddresses *pLocalAddr = &pCallLeg->localAddr;
    RvChar                         termChar;
    RvStatus                       rv;
    RvSipTransportLocalAddrHandle *phAddr;
    RvSipTransport                 eTransport;
    RvSipTransportAddressType      eAddrType;

    if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                        localAddrIDLen,
                                        LOCAL_ADDR_UDP_IPV4_ID) == RV_TRUE)
    {
        /* Udp - Ipv4 */
        eTransport = RVSIP_TRANSPORT_UDP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
        phAddr     = &pLocalAddr->hLocalUdpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                             localAddrIDLen,
                                             LOCAL_ADDR_TCP_IPV4_ID) == RV_TRUE)
    {
        /* Tcp - Ipv4 */
        eTransport = RVSIP_TRANSPORT_TCP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
        phAddr     = &pLocalAddr->hLocalTcpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                             localAddrIDLen,
                                             LOCAL_ADDR_UDP_IPV6_ID) == RV_TRUE)
    {
        /* Udp - Ipv6 */
        eTransport = RVSIP_TRANSPORT_UDP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
        phAddr     = &pLocalAddr->hLocalUdpIpv6;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                             localAddrIDLen,
                                             LOCAL_ADDR_TCP_IPV6_ID) == RV_TRUE)
    {
        /* Tcp - Ipv6 */
        eTransport = RVSIP_TRANSPORT_TCP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
        phAddr     = &pLocalAddr->hLocalTcpIpv6;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                             localAddrIDLen,
                                             LOCAL_ADDR_TLS_IPV4_ID) == RV_TRUE)
    {
        /* Tls - Ipv4 */
        eTransport = RVSIP_TRANSPORT_TLS;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
        phAddr     = &pLocalAddr->hLocalTlsIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
                                             localAddrIDLen,
                                             LOCAL_ADDR_TLS_IPV6_ID) == RV_TRUE)
    {
        /* Tls - Ipv6 */
        eTransport = RVSIP_TRANSPORT_TLS;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
        phAddr     = &pLocalAddr->hLocalTlsIpv6;
    }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
	else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
											 localAddrIDLen,
											 LOCAL_ADDR_SCTP_IPV4_ID) == RV_TRUE)
    {
        /* Sctp - Ipv4 */
        eTransport = RVSIP_TRANSPORT_SCTP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
        phAddr     = &pLocalAddr->hLocalSctpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pLocalAddrID,
											 localAddrIDLen,
											 LOCAL_ADDR_SCTP_IPV6_ID) == RV_TRUE)
    {
        /* Sctp - Ipv6 */
        eTransport = RVSIP_TRANSPORT_SCTP;
        eAddrType  = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
        phAddr     = &pLocalAddr->hLocalSctpIpv6;
    }
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */ 
    else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegLocalAddr - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(
            pLocalAddrValue,localAddrValueLen,&termChar);
    rv = RestoreIpAndPort(
            pCallLeg->pMgr->hTransportMgr,eTransport,eAddrType,pLocalAddrValue,phAddr);
    SipCommonHighAvalRestoreParamBufferAfterParse(
            termChar,localAddrValueLen,pLocalAddrValue);
    if (rv != RV_OK)
    {
        RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegLocalAddr - Call-leg 0x%p failed to restore IP and port, the default is used",
            pCallLeg));
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegLocalAddrInUse
 * ------------------------------------------------------------------------
 * General: The function restores the local-address Address In Use parameter.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pAddrInUseID    - The ID of the configured parameter.
 *        addrInUseIDLen  - The length of the parameter ID.
 *        pAddrInUseVal   - The parameter value string.
 *        addrInUseValLen - The length of the parameter value string.
 * InOut: pObj            - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegLocalAddrInUse(
                                 IN    RvChar   *pAddrInUseID,
                                 IN    RvUint32  addrInUseIDLen,
                                 IN    RvChar   *pAddrInUseVal,
                                 IN    RvUint32  addrInUseValLen,
                                 INOUT void     *pObj)
{
    CallLeg                       *pCallLeg   = (CallLeg *)pObj;
    SipTransportObjLocalAddresses *pLocalAddr = &pCallLeg->localAddr;

    if (SipCommonMemBuffExactlyMatchStr(pAddrInUseID,
                                        addrInUseIDLen,
                                        LOCAL_ADDR_ADDR_IN_USE_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegLocalAddrInUse - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                        addrInUseIDLen,
                                        LOCAL_ADDR_UDP_IPV4_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse =  &pLocalAddr->hLocalUdpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                             addrInUseIDLen,
                                             LOCAL_ADDR_UDP_IPV6_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse =  &pLocalAddr->hLocalUdpIpv6;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                             addrInUseValLen,
                                             LOCAL_ADDR_TCP_IPV4_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalTcpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                             addrInUseValLen,
                                             LOCAL_ADDR_TCP_IPV6_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalTcpIpv6;
    }
#if (RV_TLS_TYPE != RV_TLS_NONE)
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                             addrInUseValLen,
                                             LOCAL_ADDR_TLS_IPV4_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalTlsIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
                                             addrInUseValLen,
                                             LOCAL_ADDR_TLS_IPV6_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalTlsIpv6;
    }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
	else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
											 addrInUseValLen,
											 LOCAL_ADDR_SCTP_IPV4_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalSctpIpv4;
    }
    else if (SipCommonMemBuffExactlyMatchStr(pAddrInUseVal,
											 addrInUseValLen,
											 LOCAL_ADDR_SCTP_IPV6_ID) == RV_TRUE)
    {
        pLocalAddr->hAddrInUse = &pLocalAddr->hLocalSctpIpv6;
    }
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */ 	
	else
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegLocalAddrInUse - Call-leg 0x%p received illegal Value",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegOutboundProxyHostName
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg OutboundProxy host name.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pHostNameID    - The ID of the configured parameter.
 *        hostNameIDLen  - The length of the parameter ID.
 *        pHostNameVal   - The parameter value string.
 *        hostNameValLen - The length of the parameter value string.
 * InOut: pObj           - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegOutboundProxyHostName(
                                        IN    RvChar   *pHostNameID,
                                        IN    RvUint32  hostNameIDLen,
                                        IN    RvChar   *pHostNameVal,
                                        IN    RvUint32  hostNameValLen,
                                        INOUT void     *pObj)
{
    CallLeg           *pCallLeg = (CallLeg *)pObj;
    RPOOL_ITEM_HANDLE  item;
    RvChar             termChar;
    RvStatus           rv;

    if (SipCommonMemBuffExactlyMatchStr(pHostNameID,
                                        hostNameIDLen,
                                        OUTBOUND_PROXY_RPOOL_HOST_NAME_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyHostName - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    pCallLeg->outboundAddr.rpoolHostName.hPool = pCallLeg->pMgr->hGeneralPool;
    pCallLeg->outboundAddr.rpoolHostName.hPage = pCallLeg->hPage;

    SipCommonHighAvalPrepareParamBufferForParse(pHostNameVal,hostNameValLen,&termChar);
    rv = RPOOL_AppendFromExternal(pCallLeg->outboundAddr.rpoolHostName.hPool,
                                  pCallLeg->outboundAddr.rpoolHostName.hPage,
                                  pHostNameVal,
                                  hostNameValLen + 1, /* +1 for '\0' */
                                  RV_FALSE,
                                  &(pCallLeg->outboundAddr.rpoolHostName.offset),
                                  &item);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,hostNameValLen,pHostNameVal);
    if (RV_OK != rv)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyHostName - Call-leg 0x%p failed to copy host name from buffer",
            pCallLeg));

        return rv;
    }

#ifdef SIP_DEBUG
    pCallLeg->outboundAddr.strHostName =
                    RPOOL_GetPtr(pCallLeg->outboundAddr.rpoolHostName.hPool,
                                 pCallLeg->outboundAddr.rpoolHostName.hPage,
                                 pCallLeg->outboundAddr.rpoolHostName.offset);
#endif /* SIP_DEBUG */

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegOutboundProxyIpAddr
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg OutboundProxy Ip Address
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompTypeID    - The ID of the configured parameter.
 *        compTypeIDLen  - The length of the parameter ID.
 *        pCompTypeVal   - The parameter value string.
 *        compTypeValLen - The length of the parameter value string.
 * InOut: pObj           - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegOutboundProxyIpAddr(
                                        IN    RvChar   *pAddressID,
                                        IN    RvUint32  addressIDLen,
                                        IN    RvChar   *pAddressVal,
                                        IN    RvUint32  addressValLen,
                                        INOUT void     *pObj)
{
    RvChar     termChar;
    RvUint16   ipAddrPort;
    RvAddress *pCallLegAddr;
    CallLeg   *pCallLeg                                         = (CallLeg *)pObj;
    RvChar     strIpAddr   [RV_ADDRESS_MAXSTRINGSIZE]           = "";
    RvChar     strAddrType [SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN] = "";
    RvChar     strTransport[SIP_COMMON_HIGH_AVAL_PARAM_STR_LEN] = "";

    if (SipCommonMemBuffExactlyMatchStr(pAddressID,
                                        addressIDLen,
                                        OUTBOUND_PROXY_IP_ADDR_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyIpAddr - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pAddressVal,addressValLen,&termChar);
    if (RvSscanf(pAddressVal,
                 OUTBOUND_PROXY_ADDRESS_FORMAT,
                 strIpAddr,
                 &ipAddrPort,
                 strAddrType,
                 strTransport) != 4)
    {
        SipCommonHighAvalRestoreParamBufferAfterParse(termChar,addressValLen,pAddressVal);
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyIpAddr - Call-leg 0x%p AddrVal doesn't contain 2 parameters",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,addressValLen,pAddressVal);

    pCallLegAddr           = &pCallLeg->outboundAddr.ipAddress.addr;
    pCallLegAddr->addrtype = SipCommonCoreGetAddressTypeInt(strAddrType);
    RvAddressSetIpPort(pCallLegAddr,ipAddrPort);
    if (RvAddressSetString(strIpAddr,pCallLegAddr) == NULL)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyIpAddr - Call-leg 0x%p failed to set string %s",
            pCallLeg,strIpAddr));

        return RV_ERROR_UNKNOWN;
    }
    pCallLeg->outboundAddr.ipAddress.eTransport = 
        SipCommonConvertStrToEnumTransportType(strTransport);

    return RV_OK;
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RestoreCallLegOutboundProxyCompType
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg OutboundProxy compression
 *          type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompTypeID    - The ID of the configured parameter.
 *        compTypeIDLen  - The length of the parameter ID.
 *        pCompTypeVal   - The parameter value string.
 *        compTypeValLen - The length of the parameter value string.
 * InOut: pObj           - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegOutboundProxyCompType(
                                        IN    RvChar   *pCompTypeID,
                                        IN    RvUint32  compTypeIDLen,
                                        IN    RvChar   *pCompTypeVal,
                                        IN    RvUint32  compTypeValLen,
                                        INOUT void     *pObj)
{
    CallLeg  *pCallLeg = (CallLeg *)pObj;
    RvChar    termChar;

    if (SipCommonMemBuffExactlyMatchStr(pCompTypeID,
                                        compTypeIDLen,
                                        OUTBOUND_PROXY_COMP_TYPE_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegOutboundProxyCompType - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pCompTypeVal,compTypeValLen,&termChar);
    pCallLeg->outboundAddr.eCompType = SipMsgGetCompTypeEnum(pCompTypeVal);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,compTypeValLen,pCompTypeVal);

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */

/***************************************************************************
 * RestoreCallLegState
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg State.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pStateID    - The ID of the configured parameter.
 *        stateIDLen  - The length of the parameter ID.
 *        pStateVal   - The parameter value string.
 *        stateValLen - The length of the parameter value string.
 * InOut: pObj        - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegState(
                                        IN    RvChar   *pStateID,
                                        IN    RvUint32  stateIDLen,
                                        IN    RvChar   *pStateVal,
                                        IN    RvUint32  stateValLen,
                                        INOUT void     *pObj)
{
    CallLeg *pCallLeg = (CallLeg *)pObj;
    RvChar   termChar;

    if (SipCommonMemBuffExactlyMatchStr(pStateID,stateIDLen,STATE_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegState - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pStateVal,stateValLen,&termChar);
    pCallLeg->eState = SipCallLegGetStateEnum(pStateVal);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,stateValLen,pStateVal);

    return RV_OK;
}

/***************************************************************************
 * RestoreCallLegDirection
 * ------------------------------------------------------------------------
 * General: The function restores the CallLeg Direction.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pDirectionID    - The ID of the configured parameter.
 *        directionIDLen  - The length of the parameter ID.
 *        pDirectionVal   - The parameter value string.
 *        directionValLen - The length of the parameter value string.
 * InOut: pObj            - Pointer to the restored CallLeg obj.
 ***************************************************************************/
static RvStatus RVCALLCONV RestoreCallLegDirection(
                                        IN    RvChar   *pDirectionID,
                                        IN    RvUint32  directionIDLen,
                                        IN    RvChar   *pDirectionVal,
                                        IN    RvUint32  directionValLen,
                                        INOUT void     *pObj)
{
    CallLeg *pCallLeg = (CallLeg *)pObj;
    RvChar   termChar;

    if (SipCommonMemBuffExactlyMatchStr(pDirectionID,directionIDLen,DIRECTION_ID) != RV_TRUE)
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "RestoreCallLegDirection - Call-leg 0x%p received illegal ID",
            pCallLeg));

        return RV_ERROR_BADPARAM;
    }

    SipCommonHighAvalPrepareParamBufferForParse(pDirectionVal,directionValLen,&termChar);
    pCallLeg->eDirection = SipCallLegGetDirectionEnum(pDirectionVal);
    SipCommonHighAvalRestoreParamBufferAfterParse(termChar,directionValLen,pDirectionVal);

    return RV_OK;
}

/***************************************************************************
 * PrepareLocalAddrIDsArray
 * ------------------------------------------------------------------------
 * General: The function fills the LocalAddr ID parameters array that
 *          might be useful when restoring the CallLeg LocalAddr struct
 *          from a buffer by the the function
 *          SipCommonHighAvalRestoreFromBuffer() .
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg   - Pointer to the restored CallLeg object.
 * InOut:  pArraySize - The number of elements in LocalAddr IDs array.
 * Output: pIDsArray  - Pointer to the LocalAddr IDs array to be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV PrepareLocalAddrIDsArray(
                                 IN     CallLeg                    *pCallLeg,
                                 INOUT  RvInt32                    *pArraySize,
                                 OUT    SipCommonHighAvalParameter *pIDsArray)
{
    SipCommonHighAvalParameter *pCurrID = pIDsArray;

    pCurrID->strParamID       = LOCAL_ADDR_UDP_IPV4_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = LOCAL_ADDR_UDP_IPV6_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = LOCAL_ADDR_TCP_IPV4_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = LOCAL_ADDR_TCP_IPV6_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

#if (RV_TLS_TYPE != RV_TLS_NONE)
    pCurrID->strParamID       = LOCAL_ADDR_TLS_IPV4_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = LOCAL_ADDR_TLS_IPV6_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/
#endif /* (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_NET_TYPE & RV_NET_SCTP)
	pCurrID->strParamID       = LOCAL_ADDR_SCTP_IPV4_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/
	
    pCurrID->strParamID       = LOCAL_ADDR_SCTP_IPV6_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */ 

    pCurrID->strParamID       = LOCAL_ADDR_ADDR_IN_USE_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegLocalAddrInUse;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    if (pCurrID - pIDsArray > *pArraySize)
    {
         RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "PrepareLocalAddrIDsArray - Call-leg 0x%p, %d params were added but the max size is %u",
            pCallLeg,pCurrID-pIDsArray,*pArraySize));

        return RV_ERROR_UNKNOWN;
    }
    *pArraySize = (RvInt32)(pCurrID - pIDsArray);

    return RV_OK;
}

/***************************************************************************
 * PrepareOutboundProxyAddrIDsArray
 * ------------------------------------------------------------------------
 * General: The function fills the OutboundProxyAddr ID parameters array
 *          that might be useful when restoring the CallLeg OutboundProxy
 *          Addr struct from a buffer by the the function
 *          SipCommonHighAvalRestoreFromBuffer() .
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg   - Pointer to the restored CallLeg object.
 * InOut:  pArraySize - The number of elements in LocalAddr IDs array.
 * Output: pIDsArray  - Pointer to the LocalAddr IDs array to be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV PrepareOutboundProxyAddrIDsArray(
                                 IN    CallLeg                    *pCallLeg,
                                 INOUT RvInt32                    *pArraySize,
                                 OUT   SipCommonHighAvalParameter *pIDsArray)
{
    SipCommonHighAvalParameter *pCurrID = pIDsArray;

    pCurrID->strParamID       = OUTBOUND_PROXY_RPOOL_HOST_NAME_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegOutboundProxyHostName;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = OUTBOUND_PROXY_IP_ADDR_EXISTS_ID;
    pCurrID->pParamObj        = &pCallLeg->outboundAddr.bIpAddressExists;
    pCurrID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = OUTBOUND_PROXY_IP_ADDR_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegOutboundProxyIpAddr;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    pCurrID->strParamID       = OUTBOUND_PROXY_USE_HOST_NAME_ID;
    pCurrID->pParamObj        = &pCallLeg->outboundAddr.bUseHostName;
    pCurrID->pfnSetParamValue = SipCommonHighAvalRestoreBoolParam;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

#ifdef RV_SIGCOMP_ON
    pCurrID->strParamID       = OUTBOUND_PROXY_COMP_TYPE_ID;
    pCurrID->pParamObj        = pCallLeg;
    pCurrID->pfnSetParamValue = RestoreCallLegOutboundProxyCompType;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/
#endif /* RV_SIGCOMP_ON */
    if (pCurrID - pIDsArray > *pArraySize)
    {
         RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "PrepareOutboundProxyAddrIDsArray - Call-leg 0x%p, %d params were added but the max size is %u",
            pCallLeg,pCurrID-pIDsArray,*pArraySize));

        return RV_ERROR_UNKNOWN;
    }
    *pArraySize = (RvInt32)(pCurrID - pIDsArray);

    return RV_OK;
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * PrepareSctpIDsArray
 * ------------------------------------------------------------------------
 * General: The function fills the SCTP ID parameters array
 *          that might be useful when restoring the CallLeg sctpParams struct
 *          from a buffer by the the function
 *          SipCommonHighAvalRestoreFromBuffer().
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg   - Pointer to the restored CallLeg object.
 * InOut:  pArraySize - The number of elements in LocalAddr IDs array.
 * Output: pIDsArray  - Pointer to the LocalAddr IDs array to be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV PrepareSctpIDsArray(
                                IN    CallLeg                    *pCallLeg,
                                INOUT RvInt32                    *pArraySize,
                                OUT   SipCommonHighAvalParameter *pIDsArray)
{
    SipCommonHighAvalParameter *pCurrID = pIDsArray;

    pCurrID->strParamID       = SCTP_STREAM_ID;
    pCurrID->pParamObj        = &pCallLeg->sctpParams.sctpStream;
    pCurrID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrID += 1; /* Promote the pointer to next ID*/

    pCurrID->strParamID       = SCTP_MSGSENDFLAGS_ID;
    pCurrID->pParamObj        = &pCallLeg->sctpParams.sctpMsgSendingFlags;
    pCurrID->pfnSetParamValue = SipCommonHighAvalRestoreInt32Param;
    pCurrID += 1; /* Promote the pointer to next LocalAddr ID*/

    if (pCurrID - pIDsArray > *pArraySize)
    {
        RvLogExcep(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "PrepareSctpIDsArray - Call-leg 0x%p, %d params were added but the max size is %u",
            pCallLeg,pCurrID-pIDsArray,*pArraySize));
        
        return RV_ERROR_UNKNOWN;
    }
    *pArraySize = pCurrID - pIDsArray;
    
    return RV_OK;
}
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */

/***************************************************************************
 * GetOutboundProxyIpAddrStr
 * ------------------------------------------------------------------------
 * General: The function returns the string that represents the
 *          OutboundProxy IP Address.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pIpAddr       - Pointer to the OutboundProxy IP Address.
 *         maxAddrStrLen - The max length of the retrieved string buffer
 * Output: strIpAddr     - Pointer to the string buffer.
 ***************************************************************************/
static RvStatus RVCALLCONV GetOutboundProxyIpAddrStr(
                                IN  SipTransportAddress *pIpAddr,
                                IN  RvUint32             maxAddrStrLen,
                                OUT RvChar              *strIpAddr)
{
    RvChar        strLocalIpAddr[RV_ADDRESS_MAXSTRINGSIZE];
    const RvChar *strLocalTransport;
    const RvChar *strAddressType;
    RvUint16      ipAddrPort;

    if (RvAddressGetString(&(pIpAddr->addr),maxAddrStrLen,strLocalIpAddr) == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    ipAddrPort        = RvAddressGetIpPort(&(pIpAddr->addr));
    strAddressType    = SipCommonCoreGetAddressTypeStr(pIpAddr->addr.addrtype);
    strLocalTransport = SipCommonConvertEnumToStrTransportType(pIpAddr->eTransport);

    if (RvSnprintf(strIpAddr,
                   maxAddrStrLen,
                   OUTBOUND_PROXY_ADDRESS_FORMAT,
                   strLocalIpAddr,
                   ipAddrPort,
                   strAddressType,
                   strLocalTransport) < 0)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    return RV_OK;
}

/***************************************************************************
 * AddRestoredCallLegToMgrHash
 * ------------------------------------------------------------------------
 * General: The function adds the newly restored CallLeg to the CallLegMgr
 *          hash table (only if it doesn't exist).
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - Pointer to the added CallLeg.
 ***************************************************************************/
static RvStatus RVCALLCONV AddRestoredCallLegToMgrHash(
                                  IN CallLeg               *pCallLeg,
                                  IN RvSipCallLegDirection  eOriginDirection)
{
    CallLeg              *pFoundCallLeg;
    SipTransactionKey     pKey;

	if (AreEssentialsRestored(pCallLeg) != RV_TRUE)
	{
		RvLogWarning(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
            "AddRestoredCallLegToMgrHash - The essentials of Call-leg 0x%p weren't restored - The call stays out of hash",
            pCallLeg));
        return RV_OK;
	}

    memset(&pKey,0,sizeof(pKey));

    pKey.hFromHeader      = pCallLeg->hFromHeader;
    pKey.hToHeader        = pCallLeg->hToHeader;
    pKey.strCallId.offset = pCallLeg->strCallId;
    pKey.strCallId.hPage  = pCallLeg->hPage;
    pKey.strCallId.hPool  = pCallLeg->pMgr->hGeneralPool;

    CallLegMgrHashFind(pCallLeg->pMgr,
                       &pKey,
                       pCallLeg->eDirection,
                       RV_FALSE /*bOnlyEstablishedCall=RV_FALSE*/,
                       (RvSipCallLegHandle**)&pFoundCallLeg);

    if(pFoundCallLeg == NULL) /* there is no such callLeg. we can insert this to the hash */
    {
        if(CallLegMgrHashInsert((RvSipCallLegHandle)pCallLeg) != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AddRestoredCallLegToMgrHash - Failed to insert Call-Leg 0x%p - to hash  - Terminating call",
                pCallLeg));
            CallLegTerminate(pCallLeg,RVSIP_CALL_LEG_REASON_CALL_TERMINATED);
            return RV_ERROR_UNKNOWN;
        }
    }
    else /* such a callLeg already exist. need to clear all it's parameters */
    {
        RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "AddRestoredCallLegToMgrHash - same CallLeg 0x%p is in the hash. reset callLeg 0x%p",
                pFoundCallLeg,pCallLeg));
        ResetCall(pCallLeg, eOriginDirection);
        return RV_ERROR_UNKNOWN;
    }

    return RV_OK;
}

/***************************************************************************
 * AreEssentialsRestored
 * ------------------------------------------------------------------------
 * General: The function checks if the call-leg essential fields were 
 *			restored. 
 *
 * Return Value: RvBool, an indication of the validity of the restore
 *			     operation.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCallLeg - Pointer to the added CallLeg.
 ***************************************************************************/
static RvBool RVCALLCONV AreEssentialsRestored(IN CallLeg *pCallLeg)
{
	/* A call-leg can be added to hash table only if it's essentials are restored */
    if(pCallLeg->hFromHeader == NULL ||
	   pCallLeg->hToHeader   == NULL ||
	   pCallLeg->strCallId   == UNDEFINED)
    {
		return RV_FALSE; 
    }

	return RV_TRUE;
}

#endif /* #ifdef RV_SIP_HIGHAVAL_ON */ 
#endif /* ifndef RV_SIP_PRIMITIVES*/

