/************************************************************************
 File Name	   : rvsrtp.h
 Description   :
*************************************************************************
 Copyright (c)	2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#include "rv64ascii.h"
#include "rvsrtp.h"
#include "rvsrtplog.h"
#include "rv_srtp.h"
#include "rtputil.h"
#include "srtpProfileRfc3711.h"
#include "RtpProfileRfc3550.h"

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)
static  RvLogSource*     localLogSource = NULL;
#define RTP_SOURCE      (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_SRTP_MODULE)))
#define rvLogPtr        (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_SRTP_MODULE)))
#else
#define rvLogPtr        (NULL)
#endif
#include "rtpLogFuncs.h"
#undef FUNC_NAME
#define FUNC_NAME(name) "RvSrtp" #name


#ifdef __cplusplus
extern "C" {
#endif

    /* Logger static variables */
#if(RV_LOGMASK != RV_LOGLEVEL_NONE)
 static RvRtpLogger rtpLogManager = NULL;
#define logMngr (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMngr (NULL)
#endif /* (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/* Basic operations */

/************************************************************************************
 * RvSrtpConstruct
 * description: constructs and allocates SRTP plugin for RTP session from Dynamic Memory.
 *              The same plugin will be used for RTCP session.
 * input: session - RTP session handle
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Note:  This plugin is configured with default parameters.
 *        Changing the configuration parameters must be done before calling to
 *        RvSrtpInit().
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpConstruct(IN RvRtpSession session
#ifdef UPDATED_BY_SPIRENT
        ,IN RvSrtpMasterKeyEventCB masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpSessionInfo*     s              = (RvRtpSessionInfo*) session;
    RvStatus              result         = RV_OK;
    RtpProfileRfc3711*    srtpProfilePtr = NULL;

    RTPLOG_ENTER(Construct);

    if (s == NULL)
    {
        RTPLOG_ERROR_LEAVE(Construct, "NULL session handle");
        return RV_ERROR_NULLPTR;
    }
    if (s->profilePlugin != NULL &&
        s->profilePlugin->funcs != NULL &&
        s->profilePlugin->funcs->release != NULL)
    {
        s->profilePlugin->funcs->release(s->profilePlugin, session);
    }
    result = RvMemoryAlloc(NULL, sizeof(RtpProfileRfc3711), logMngr, (void**)&srtpProfilePtr);

    if (result == RV_OK)
    {
        s->sequenceNumber &= 0x7FFF; /* to avoid SRTP packet lost problem */
        RtpProfileRfc3711Construct(srtpProfilePtr
#ifdef UPDATED_BY_SPIRENT
        						  , masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
								  );
        s->profilePlugin = RtpProfileRfc3711GetPlugIn(srtpProfilePtr);
        RvRtcpSetProfilePlugin(s->hRTCP, s->profilePlugin);
/*      RtpProfileRfc3711Construct calls  result = rvSrtpConstruct(srtpPtr);*/
        RTPLOG_INFO((RTP_SOURCE, "Constructed SRTP plugin for RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(Construct);
        return RV_OK;

    }
    RTPLOG_ERROR((RTP_SOURCE, "cannot allocate memory for SRTP plugin for RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(Construct);
    return RV_ERROR_OUTOFRESOURCES;
}

/************************************************************************************
 * RvSrtpInit
 * description: Initializes (registers) the SRTP plugin with the RTP/RTCP sessions and
 *              allocates SRTP plugin hashes and pools and databases.
 * input: session - RTP session handle
 *        plugin  - external AES plugin
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Note:  This function must be called after RvSrtpConstruct() and after
 *        completed configuring of SRTP by functions from rvsrtpconfig.h
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpInit(IN RvRtpSession session, RvSrtpAesPlugIn* plugin)
{
    RvRtpSessionInfo*  s                    = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*            srtpPtr              = NULL;
    RvStatus           result               = RV_OK;

    RTPLOG_ENTER(Init);
    if (s == NULL || plugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(Init, "NULL session handle or SRTP plugin");
        return RV_ERROR_NULLPTR;
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRegister(srtpPtr, session, plugin);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "Activated SRTP plugin for RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(Init);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpInit - Failed to register SRTP plugin for RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(Init);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpDestruct
 * description: Deallocates and unregisters all SRTP plugin data and destruct plugin
 *              objects.
 * input: session - RTP session handle
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 * Note:  This function must be called after RvSrtpInit().
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpDestruct(IN RvRtpSession session)
{
    RvRtpSessionInfo*  s                    = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*            srtpPtr              = NULL;
    RvStatus           res                  = RV_ERROR_UNKNOWN;

    RTPLOG_ENTER(Destruct);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(Destruct, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    RvLockGet(&s->lock, logMngr);
    if (s == NULL || s->profilePlugin == NULL) /* knowingly second checking */
    {
        RvLockRelease(&s->lock, logMngr);
        RTPLOG_ERROR_LEAVE(Destruct, "SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    /* has rvSrtpUnregister() inside */
    res = rvSrtpDestruct(srtpPtr);
    s->profilePlugin = NULL;
    RvLockRelease(&s->lock, logMngr);
    RTPLOG_INFO((RTP_SOURCE, "RvSrtpDestruct - destructed SRTP plugin for RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(Destruct);
    return res;
}

/* Master key management */
/************************************************************************************
 * RvSrtpMasterKeyAdd
 * description: This function adds a new master key (and salt) to the session.
 *              The length of the mki, key, and salt must be the lengths
 *              configured with the RvSrtpSetMasterKeySizes() function or taken
 *              by default.
 *              Even if the MKI is not to be included in the SRTP/SRTCP
 *              packets, a unique value must be specified since this value
 *              will be used to identify this key when it is to be
 *              removed.
 *              The mki, key, and salt will be copied, so the user does not
 *              need to maintain the original copies of this data.
 * input: session - RTP session handle
 *        mki     - Pointer to the MKI indentifier for the key.
 *        key     - Pointer to the master key.
 *        salt    - Pointer to the master salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyAdd(
       IN RvRtpSession session,
       IN RvUint8      *mki,
       IN RvUint8      *key,
       IN RvUint8      *salt
#ifdef UPDATED_BY_SPIRENT
      ,IN RvUint64      lifetime
      ,IN RvUint64      threshold
      ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvStatus          result  = RV_OK;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*            srtpPtr              = NULL;

    RTPLOG_ENTER(MasterKeyAdd);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(MasterKeyAdd, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpMasterKeyAdd(srtpPtr , mki, key, salt
#ifdef UPDATED_BY_SPIRENT
            , lifetime
            , threshold
            , direction
#endif // UPDATED_BY_SPIRENT
    );
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpMasterKeyAdd - added master key to the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(MasterKeyAdd);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpMasterKeyAdd - failed to add master key for RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(MasterKeyAdd);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpMasterKeyRemove
 * description: This function removes a master key (and salt) from the session.
 * input: session - RTP session handle
 *        mki     - Pointer to the MKI identifier for the key.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyRemove(
       IN RvRtpSession session,
       IN RvUint8      *mki
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
)
{

    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*            srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(MasterKeyRemove);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(MasterKeyRemove, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;
    result = rvSrtpMasterKeyRemove(srtpPtr, mki
#ifdef UPDATED_BY_SPIRENT
            , direction
#endif // UPDATED_BY_SPIRENT
    );
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpMasterKeyRemove - removed master key from the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(MasterKeyRemove);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpMasterKeyRemove - failed to remove master key for RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(MasterKeyRemove);
    return RV_ERROR_UNKNOWN;

}

/************************************************************************************
 * RvSrtpMasterKeyRemoveAll
 * description: This function removes all master keys (and salts) from the session.
 * input: session - RTP session handle
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyRemoveAll(
       IN RvRtpSession session)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(MasterKeyRemoveAll);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(MasterKeyRemoveAll, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpMasterKeyRemoveAll(srtpPtr);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpMasterKeyRemoveAll - removed all master keys from the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(MasterKeyRemoveAll);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpMasterKeyRemoveAll - failed to remove all master keys from the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(MasterKeyRemoveAll);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpMasterKeyGetContextCount
 * description: This function returns the number of contexts that currently
 *              refer to the specified master key. This includes contexts
 *              that are scheduled to be created in the future along with
 *              those remaining in the history.
 * input: session - RTP session handle
 *        mki     - Pointer to the MKI identifier for the key.
 * output: none.
 * return value:  number of contexts that currently
 *                refer to the specified master key
 ***********************************************************************************/
RVAPI
RvSize_t RVCALLCONV RvSrtpMasterKeyGetContextCount(
       IN RvRtpSession session,
       IN RvUint8 *mki
#ifdef UPDATED_BY_SPIRENT
       ,IN RvUint8       direction
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpSessionInfo*  s                    = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(MasterKeyGetContextCount);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(MasterKeyGetContextCount, "NULL RTP session or SRTP plugin has already been destructed");
        return 0; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;
    result = rvSrtpMasterKeyGetContextCount(srtpPtr, mki
#ifdef UPDATED_BY_SPIRENT
            ,direction
#endif // UPDATED_BY_SPIRENT
    );
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpMasterKeyGetContextCount - got context count from the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(MasterKeyGetContextCount);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpMasterKeyGetContextCount - failed to get context count from the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(MasterKeyGetContextCount);
    return 0;
}

/***********************************************************************************
 *                              Destination Management                             *
 ***********************************************************************************
 * Use regular functions:
 *  RvRtpAddRemoteAddress()          for adding destination for RTP for RvRtpWrite sending
 *  RvRtcpAddRemoteAddress()         for adding destination for RTCP sending
 *  RvRtpRemoveRemoteAddress()       for removing destination address from RTP RvRtpWrite sending
 *  RvRtcpRemoveRemoteAddress()      for removing destination address from RTCP
 *  RvRtpRemoveAllRemoteAddresses()  for removing all destination addresses for RTP
 *  RvRtcpRemoveAllRemoteAddresses() for removing all destination addresses for RTCP
 ***********************************************************************************/


/************************************************************************************
 * RvSrtpAddDestinationContext
 * description: This function changes the master key that should be used to
 *              send to the specified destination. The new key will be used
 *              until changed, either by another call to this function or at a
 *              key change point specified by the RvSrtpAddDestinationContextAt
 *              function.
 *              If shareTrigger is set to RV_TRUE, every source and
 *              destination in the session that shares the destinations old
 *              master key will switch to this same new master key when
 *              this destination switches to this new key.
 *              If mki is set to NULL, that indicates that the destination
 *              has no key and any packets to be sent to that destination can
 *              not be sent.
 *              If the ssrc of the master local source changes, all key
 *              mappings for the destination will be cleared and the destination
 *              will be reinitialized to use the new ssrc.
 * input: session       - RTP session handle     -
 *        address       - destination address
 *        sourceType    - sourceType    - RvSrtpStreamTypeSRTCP or RvSrtpStreamTypeSRTP
 *        mki           - Pointer to the MKI identifier for the master key to use.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys when
 *                        this destination switches, RV_FALSE otherwise.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpAddDestinationContext(
        IN RvRtpSession     session,
        IN RvNetAddress*    address,
        IN RvSrtpStreamType sourceType,
        IN RvUint8*         mki,
        IN RvBool           shareTrigger)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvStatus          result  = RV_OK;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;

    RTPLOG_ENTER(AddDestinationContext);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(AddDestinationContext, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
    {
        RTPLOG_ERROR_LEAVE(AddDestinationContext, "wrong address parameter");
        return RV_ERROR_UNKNOWN;
    }
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        RTPLOG_ERROR_LEAVE(AddDestinationContext, "Unsupported IPV6 in current configuration. See manual.");
        return RV_ERROR_NOTSUPPORTED;
#endif
    }

    if (sourceType)
        result = rvSrtpDestinationAddRtp(srtpPtr, addrString, port);
    else
        result = rvSrtpDestinationAddRtcp(srtpPtr, addrString, port);

    result = rvSrtpDestinationChangeKey(srtpPtr, addrString, port, mki, shareTrigger);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpAddDestinationContext - added destination context for the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(AddDestinationContext);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpAddDestinationContext - failed to add destination context for the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(AddDestinationContext);
    return RV_ERROR_UNKNOWN;
}


/************************************************************************************
 * RvSrtpAddDestinationContextAt
 * description: This function sets a point in time (based on the index) at
 *      which the specified master key should be used to send to
 *      the specified destination. The new key will be used until
 *      changed, either by another change specified by a call to
 *      this function or by a call RvSrtpAddDestinationContext once
 *      the key is in use.
 *      If shareTrigger is set to RV_TRUE, every source and
 *      destination in the session that shares the destinations old
 *      master key will switch to this same new master key when
 *      this destination switches to this new key.
 *      Remember that the index is different for an SRTP
 *      destination as opposed to an SRTCP destination. Use the
 *      appropriate range and values for the type of destination
 *      specified.
 *      IMPORTANT: This function is designed to set new keys for
 *      future use. However, since the current index value may be
 *      constantly changing, it is possible that an index value for
 *      a future key change may not be received until after the
 *      point of the switchover. Thus, a historic index value is
 *      allowed, however, because index values are allowed to wrap,
 *      the user must insure that the index value specified is
 *      never older than the maximum history length configured for
 *      the session, otherwise the "historic" key change will
 *      actually be set for a time in the distant future. That is
 *      why it is important to set the history length (using
 *      RvSrtpSetHistory) to a period of time long enough to
 *      safely allow for any delay in key exchanges.
 *      If mki is set to NULL, that indicates that the destination
 *      has no key and any packets to be sent to that destination can
 *      not be sent.
 *      If the ssrc of the master local source changes, all key
 *      mappings for the destination will be cleared and the destination
 *      will be reinitialized to use the new ssrc.
--------------------------------------------------------------------
 * input: session       - RTP session handle
 *        address       - destination address
 *        sourceType    - sourceType    - RvSrtpStreamTypeSRTCP or RvSrtpStreamTypeSRTP
 *        mki           - Pointer to the MKI identifier for the master key to use.
 *        index         - The index at which the key should change.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys when
 *                        this destination switches, RV_FALSE otherwise.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpAddDestinationContextAt(
        IN RvRtpSession     session,
        IN RvNetAddress*    address,
        IN RvSrtpStreamType sourceType,
        IN RvUint8*         mki,
        IN RvUint64         index,
        IN RvBool           shareTrigger)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;
    RvChar            indexString[64] = {0};

    RTPLOG_ENTER(AddDestinationContextAt);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(AddDestinationContextAt, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
    {
        RTPLOG_ERROR_LEAVE(AddDestinationContextAt, "wrong address parameter");
        return RV_ERROR_UNKNOWN;
    }
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        RTPLOG_ERROR_LEAVE(AddDestinationContextAt, "Unsupported IPV6 in current configuration. See manual.");
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    if (sourceType)
        result = rvSrtpDestinationAddRtp(srtpPtr, addrString, port);
    else
        result = rvSrtpDestinationAddRtcp(srtpPtr, addrString, port);

    result = rvSrtpDestinationChangeKeyAt(srtpPtr, addrString, port, mki, index, shareTrigger);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        Rv64toA(index, indexString);
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpAddDestinationContextAt - added destination context at index (%s) for the RTP session (%#x)", indexString, (RvUint32)s));
        RTPLOG_LEAVE(AddDestinationContextAt);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpAddDestinationContextAt - failed to add destination context for the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(AddDestinationContextAt);
    return RV_ERROR_UNKNOWN;
}


/************************************************************************************
 * RvSrtpDestinationClearAllKeys
 * description:  This function removes all key mappings for a destination.
 * input: session       - RTP session handle
 *        address       - destination address
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpDestinationClearAllKeys(
        IN RvRtpSession session,
        IN RvNetAddress* address)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(DestinationClearAllKeys);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(DestinationClearAllKeys, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
    {
        RTPLOG_ERROR_LEAVE(DestinationClearAllKeys, "wrong address parameter");
        return RV_ERROR_UNKNOWN;
    }
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        RTPLOG_ERROR_LEAVE(DestinationClearAllKeys, "Unsupported IPV6 in current configuration. See manual.");
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    result = rvSrtpDestinationClearAllKeys(srtpPtr, addrString, port);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpDestinationClearAllKeys - removed all key mappings for a destination for the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(DestinationClearAllKeys);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpDestinationClearAllKeys - failed to remove all key mappings for a destination for the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(DestinationClearAllKeys);
    return RV_ERROR_UNKNOWN;
}


/************************************************************************************
 * RvSrtpDestinationGetIndex
 * description:  This function gets the highest index value that has been
 *               encrypted for a message to be sent to the specified
 *               destination. Note that this value may be constantly
 *               changing and should only be used for status or for
 *               estimating future index values for key changes.
 *               Remember that the index is different for an SRTP
 *               destination as opposed to an SRTCP destination.
 * input: session       - the RTP session handle
 *        address       - the destination address
 *
 * output: indexPtr     - the pointer to location where index should be stored.
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpDestinationGetIndex(
        IN RvRtpSession  session,
        IN RvNetAddress* address,
        OUT RvUint64 *   indexPtr)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RvNetAddressType  type;
    RvChar            addrString[32];
    RvInt16           port    = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;
    RvChar            indexStr[36] = {0};

    RTPLOG_ENTER(DestinationGetIndex);

    type = RvNetGetAddressType(address);
    if (type == RVNET_ADDRESS_NONE)
    {
        RTPLOG_ERROR_LEAVE(DestinationGetIndex, "wrong address parameter");
        return RV_ERROR_UNKNOWN;
    }
    if (type == RVNET_ADDRESS_IPV4)
    {
        RvNetIpv4 ipv4;
        RvNetGetIpv4(&ipv4 ,address);
        RvAddressIpv4ToString(addrString, sizeof(addrString), ipv4.ip);
        port = ipv4.port;
    }
    else
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvNetIpv6 ipv6;
        RvNetGetIpv6(&ipv6 ,address);
        RvAddressIpv6ToString(addrString, sizeof(addrString), ipv6.ip);
        port = ipv6.port;
#else
        RTPLOG_ERROR_LEAVE(DestinationGetIndex, "Unsupported IPV6 in current configuration. See manual.");
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    if (s == NULL)
    {
        RTPLOG_ERROR_LEAVE(DestinationGetIndex, "NULL RTP session");
        return RV_ERROR_NULLPTR;
    }

    if (s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(DestinationGetIndex, "SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpDestinationGetIndex(srtpPtr, addrString, port, indexPtr);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpDestinationGetIndex - got highest index(%s) that has been encrypted to destination from the RTP session (%#x)", Rv64UtoA(*indexPtr, indexStr) ,(RvUint32)s));
        RTPLOG_LEAVE(DestinationGetIndex);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpDestinationGetIndex - failed to get highest index that has been encrypted to destination for the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(DestinationGetIndex);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 *                              Source Management
 ***********************************************************************************/

/************************************************************************************
 * RvSrtpRemoteSourceAddRtp
 * description:  This function adds a new remote RTP source to the session.
 *               A source needs to be created for each ssrc that the session
 *               will be receiving RTP from. If SRTCP is also going to be
 *               received for a given ssrc, a separate remote source needs
 *               to be added for it using rvSrtpRemoteSourceAddRtcp.
 *               If the starting rollover counter value (roc) and/or
 *               sequence number is not available, set them to zero, but be
 *               aware of the possible implications as explained in the
 *               SRTP/SRTCP RFC.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        roc           - The initial rollover counter value that will be received.
 *        sequenceNum   - The initial sequence number value that will be received.
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceAddRtp(
        IN RvRtpSession session,
        IN RvUint32     ssrc,
        IN RvUint32     roc,
        IN RvUint32     sequenceNum)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceAddRtp);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceAddRtp, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceAddRtp(srtpPtr, ssrc, roc, sequenceNum);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceAddRtp - added a new remote RTP source to the RTP session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceAddRtp);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceAddRtp - failed to add a new remote RTP source to the RTP session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceAddRtp);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpRemoteSourceAddRtcp
 * description:  This function adds a new remote RTCP source to the session.
 *               A source needs to be created for each ssrc that the session
 *               will be receiving RTP from. If SRTP is also going to be
 *               received for a given ssrc, a separate remote source needs
 *               to be added for it using rvSrtpRemoteSourceAddRtp.
 *               If the starting index number is not available, set it to
 *               zero, but be aware of the implications as explained in the
 *               SRTP/SRTCP RFC.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        index         - The initial sequence number value that will be received.
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceAddRtcp(
        IN RvRtpSession session,
        IN RvUint32     ssrc,
        IN RvUint32     index)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceAddRtcp);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceAddRtcp, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceAddRtcp(srtpPtr, ssrc,  index);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceAddRtcp - added a new remote RTCP source (ssrc=%#x, index=%#x) to the RTCP session (%#x)", ssrc, index, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceAddRtcp);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceAddRtcp - failed to add a new remote RTCP source (ssrc=%#x, index=%#x) to the RTCP session (%#x)", ssrc, index, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceAddRtcp);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpRemoteSourceRemove
 * description:  This function removes a remote source from the session.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        sourceType    - sourceType    - RvSrtpStreamTypeSRTCP or RvSrtpStreamTypeSRTP
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceRemove(
        IN RvRtpSession      session,
        IN RvUint32          ssrc,
        IN RvSrtpStreamType  sourceType)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceRemove);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceRemove, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceRemove(srtpPtr, ssrc,  sourceType);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceRemove - removed a remote source (ssrc=%#x) from the session (%#x)", ssrc, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceRemove);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceRemove - failed to remove a remote source (ssrc=%#x) from the RTCP session (%#x)", ssrc, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceRemove);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpRemoteSourceRemoveAll
 * description:  This function remove all remote sources from the session.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceRemoveAll(
         IN RvRtpSession session)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceRemoveAll);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceRemoveAll, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceRemoveAll(srtpPtr);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceRemoveAll - removed all remote sources from the session (%#x)", (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceRemoveAll);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceRemoveAll - failed to remove all remote sources from the session (%#x)", (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceRemoveAll);
    return RV_ERROR_UNKNOWN;
}


/************************************************************************************
 * RvSrtpRemoteSourceChangeKey
 * description:  This function changes the master key that should be used to
 *               receive from the specified remote source. The new key will be used
 *               until changed, either by another call to this function or at a
 *               key change point specified by the RvSrtpRemoteSourceChangeKeyAt()
 *               function.
 *               If shareTrigger is set to RV_TRUE, every source and
 *               destination in the session that shares the remote source's old
 *               master key will switch to this same new master key when
 *               this remote source switches to this new key.
 *               If mki is set to NULL, that indicates that the remote source
 *               has no key and any packets to be received from that
 *               source can not be decrypted.
 *               A remote source of the appropriate type (RTP or RTCP) for
 *               the specified ssrc must have been created using the
 *               RvSrtpRemoteSourceAddRtp or RvSrtpRemoteSourceAddRtcp function
 *               before keys can be set for that remote source.
 ---------------------------------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        sourceType    - RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP
 *        mki           - Pointer to the MKI identifier for the master key to use.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys
 *                        when this source switches, RV_FALSE otherwise.
 *                        This trigger (RV_TRUE) is used in case of multi-unicast or multicast.
 *                        All contexts that share the same key will be switched
 *                        to the new source context, that created by this function.
 * output: none
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceChangeKey(
         IN RvRtpSession     session,
         IN RvUint32         ssrc,
         IN RvSrtpStreamType sourceType,
         IN RvUint8          *mki,
         IN RvBool           shareTrigger)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceChangeKey);
    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceChangeKey, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceChangeKey(srtpPtr, ssrc, sourceType, mki, shareTrigger);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceChangeKey - changed the master key, which is used to receive from the remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceChangeKey);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceChangeKey - failed to change the master key, which is used to receive from the remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceChangeKey);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpRemoteSourceChangeKeyAt
 * description:  This function sets a point in time (based on the index) at
 *               which the specified master key should be used to receive
 *               from the specified source. The new key will be used until
 *               changed, either by another change specified by a call to
 *               this function or by a call rvSrtpRemoteSourceChangeKey once
 *               the key is in use.
 *               If shareTrigger is set to RV_TRUE, every source and
 *               destination in the session that shares the remote source's old
 *               master key will switch to this same new master key when
 *               this remote source switches to this new key.}
 *               Remember that the index is different for an SRTP
 *               destination as opposed to an SRTCP destination. Use the
 *               approprate range and values for the type of destination
 *               specified.
 *               IMPORTANT: This function is designed to set new keys for
 *               future use. However, since the current index value may be
 *               constantly changing, it is possible that an index value for
 *               a future key change may not be received until after the
 *               point of the switchover. Thus, a historic index value is
 *               allowed, however, because index values are allowed to wrap,
 *               the user must insure that the index value specified is
 *               never older than the maximum history length configured for
 *               the session, otherwise the "historic" key change will
 *               actually be set for a time in the distant future. That is
 *               why it is important to set the history length (using
 *               RvSrtpSetHistory) to a period of time long enough to
 *               safely allow for any delay in key exchanges.
 *               If mki is set to NULL, that indicates that the remote source
 *               has no key and any packets to be received from that source can
 *               not be decrypted.
 *               A remote source of the appropriate type (RTP or RTCP) for
 *               the specified ssrc must have been created using the
 *               RvSrtpRemoteSourceAddRtp or RvSrtpRemoteSourceAddRtcp function
 *               before keys can be set for that remote source.
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        sourceType    - RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP
 *        mki           - Pointer to the MKI indentifier for the master key to use.
 *        index         - The index at which the key should change.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys
 *                        when this source switches, RV_FALSE otherwise.
 *                        This trigger (RV_TRUE) is used in case of multi-unicast or multicast.
 *                        All contexts that share the same key will be switched
 *                        to the new source context, that created by this function.
 * output: none
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceChangeKeyAt(
         IN RvRtpSession     session,
         IN RvUint32         ssrc,
         IN RvSrtpStreamType sourceType,
         IN RvUint8          *mki,
         IN RvUint64         index,
         IN RvBool           shareTrigger)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;
    RvChar            indexString[64] = {0};

    RTPLOG_ENTER(RemoteSourceChangeKeyAt);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceChangeKeyAt, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceChangeKeyAt(srtpPtr, ssrc, sourceType, mki, index, shareTrigger);

    Rv64toA(index, indexString);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceChangeKeyAt - changed the master key,"
            " which is used to receive from the remote source (ssrc = %#x) at time (index=%s), the session (%#x)",
            ssrc, indexString, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceChangeKeyAt);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceChangeKeyAt - failed to change the master key,"
        " which is used to receive from the remote source (ssrc = %#x) at time (index=%s), the session (%#x)",
        ssrc, indexString, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceChangeKeyAt);
    return RV_ERROR_UNKNOWN;
}


/************************************************************************************
 * RvSrtpRemoteSourceClearAllKeys
 * description:  This function removes all key mappings for a remote source.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        sourceType    - RvSrtpStreamTypeSRTCP or RvSrtpStreamTypeSRTP
 * output: none
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceClearAllKeys(
         IN RvRtpSession     session,
         IN RvUint32         ssrc,
         IN RvSrtpStreamType sourceType)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceClearAllKeys);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceClearAllKeys, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceClearAllKeys(srtpPtr, ssrc, sourceType);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceClearAllKeys - removed all key mappings for a remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceClearAllKeys);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceClearAllKeys - failed to remove all key mappings for a remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceClearAllKeys);
    return RV_ERROR_UNKNOWN;
}

/************************************************************************************
 * RvSrtpRemoteSourceGetIndex
 * description: This function gets the highest index value that has been
 *              decrypted for a message received from the specified
 *              remote source. Note that this value may be constantly
 *              changing and should only be used for status or for
 *              estimating future index values for key changes.
 *              Remember that the index is different for an SRTP
 *              source as opposed to an SRTCP source.
 -----------------------------------------------------------
 * input: session       - the RTP session handle
 *        ssrc          - The ssrc of the remote source.
 *        sourceType    - RvSrtpStreamTypeSRTCP or RvSrtpStreamTypeSRTP
 * output: indexPtr     - The pointer to location where index should be stored.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpRemoteSourceGetIndex(
         IN    RvRtpSession     session,
         IN    RvUint32         ssrc,
         IN    RvSrtpStreamType sourceType,
         INOUT RvUint64         *indexPtr)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    RvStatus          result  = RV_OK;

    RTPLOG_ENTER(RemoteSourceGetIndex);

    if (s == NULL || s->profilePlugin == NULL)
    {
        RTPLOG_ERROR_LEAVE(RemoteSourceGetIndex, "NULL RTP session or SRTP plugin has already been destructed");
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    }

    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp;

    result = rvSrtpRemoteSourceGetIndex(srtpPtr, ssrc, sourceType, indexPtr);
    if (result == RV_RTPSTATUS_Succeeded)
    {
        RTPLOG_INFO((RTP_SOURCE, "RvSrtpRemoteSourceGetIndex - got highest index value that has been decrypted for a message received from the "
            "remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
        RTPLOG_LEAVE(RemoteSourceGetIndex);
        return RV_OK;
    }
    RTPLOG_ERROR((RTP_SOURCE, "RvSrtpRemoteSourceGetIndex - failed to get highest index value that has been decrypted for a message received from the "
        "remote source (ssrc = %#x), the session (%#x)", ssrc, (RvUint32)s));
    RTPLOG_LEAVE(RemoteSourceGetIndex);
    return RV_ERROR_UNKNOWN;
}

#if 0 /* translators */
/* Destination management for translators only */
RvRtpStatus RvSrtpForwardDestinationAddRtp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 roc, RvUint32 sequenceNum);
RvRtpStatus rvSrtpForwardDestinationAddRtcp(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint32 index);
RvRtpStatus rvSrtpForwardDestinationRemove(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpForwardDestinationChangeKey(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvBool shareTrigger);
RvRtpStatus rvSrtpForwardDestinationChangeKeyAt(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvChar *mki, RvUint64 index, RvBool shareTrigger);
RvRtpStatus rvSrtpForwardDestinationClearAllKeys(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port);
RvRtpStatus rvSrtpForwardDestinationGetIndex(RvSrtp *thisPtr, RvUint32 ssrc, RvChar *address, RvUint16 port, RvUint64 *indexPtr);
RvRtpStatus rvSrtpForwardDestinationSsrcChanged(RvSrtp *thisPtr, RvUint32 oldSsrc, RvUint32 newSsrc);
/* Translator forwarding functions (use instead of RTP/RTCP forwarding functions) */
RvRtpStatus rvSrtpForwardRtp(RvSrtp *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvRtpParam *headerPtr, RvUint32 roc);
RvRtpStatus rvSrtpForwardRtcp(RvSrtp *thisPtr, RvChar *bufStartPtr, RvUint32 bufSize, RvChar *dataStartPtr, RvUint32 dataSize, RvUint32 index);
#endif

RvStatus rvRtpSessionRegisterEncryptionPlugIn(
       IN RvRtpSession session,
       IN RvSrtpEncryptionPlugIn *plugInPtr)
{
    RvRtpSessionInfo* s       = (RvRtpSessionInfo*) session;

    rvSrtpLogEnter((rvSrtpLogSourcePtr, "rvRtpSessionRegisterEncryptionPlugIn"));

    /* Make sure that plugin callbacks are all set (NULL plugin means unregister) */
    if(plugInPtr != NULL) {
        if((plugInPtr->registerPlugin == NULL) || (plugInPtr->unregisterPlugin == NULL) ||
            (plugInPtr->sessionOpen == NULL) ||  (plugInPtr->sessionClose == NULL) ||
            (plugInPtr->encryptRtp == NULL) || (plugInPtr->encryptRtcp == NULL) ||
            (plugInPtr->decryptRtp == NULL) || (plugInPtr->decryptRtcp == NULL) ||
            (plugInPtr->validateSsrc == NULL) || (plugInPtr->ssrcChanged == NULL) ||
            (plugInPtr->getPaddingSize == NULL) || (plugInPtr->getHeaderSize == NULL) ||
            (plugInPtr->getFooterSize == NULL) || (plugInPtr->getTransmitSize == NULL) ||
            (plugInPtr->rtpSeqNumChanged == NULL) || (plugInPtr->rtcpIndexChanged == NULL)) {
            rvSrtpLogError((rvSrtpLogSourcePtr, "rvRtpSessionRegisterEncryptionPlugIn: All PlugIn callbacks not set"));
            rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvRtpSessionRegisterEncryptionPlugIn"));
            return RV_ERROR_BADPARAM;
        }
    }

    /* Use special lock because we can't reconfigure while close is in progress */


    /* Unregister any old Plugin and Register new one (if any) */
    if(plugInPtr != NULL)
        plugInPtr->registerPlugin(
             &session,
             plugInPtr->userData,
             /*thisPtr->rtpEnabled*/ RV_TRUE,
             /*thisPtr->rtcpEnabled*/RV_TRUE,
             RvRtpGetSSRC(session),
             s->sequenceNumber,
             s->roc,
             rtcpSessionGetIndex(s->hRTCP));

    rvSrtpLogLeave((rvSrtpLogSourcePtr, "rvRtpSessionRegisterEncryptionPlugin"));
    return RV_OK;
}


#ifdef __cplusplus
}
#endif

