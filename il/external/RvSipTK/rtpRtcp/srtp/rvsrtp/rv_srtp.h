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
#if !defined(RV_SRTP_H)
#define RV_SRTP_H

#include "rvtypes.h"
#include "rtp.h"
#include "rvnetaddress.h"
#include "rvsrtpaesplugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Source/Destination types */
typedef enum
{
   RvSrtpStreamTypeSRTCP = RV_FALSE,
   RvSrtpStreamTypeSRTP  = RV_TRUE,

} RvSrtpStreamType;


/* Master Key defaults */
#define RV_SRTP_DEFAULT_MKISIZE 4
#define RV_SRTP_DEFAULT_MASTERKEYSIZE 16
#define RV_SRTP_DEFAULT_SALTSIZE 14

#ifdef UPDATED_BY_SPIRENT
/* Master Key destination types*/
#ifndef RV_SRTP_KEY_LOCAL
#define RV_SRTP_KEY_LOCAL 1
#endif
#ifndef RV_SRTP_KEY_REMOTE
#define RV_SRTP_KEY_REMOTE 0
#endif
#endif // UPDATED_BY_SPIRENT


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
);

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
RvStatus RVCALLCONV RvSrtpInit(IN RvRtpSession session, RvSrtpAesPlugIn* plugin);

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
RvStatus RVCALLCONV RvSrtpDestruct(IN RvRtpSession session);

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
 *        mki     - Pointer to the MKI identifier for the key.
 *        key     - Pointer to the master key.
 *        salt    - Pointer to the master salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
#ifdef UPDATED_BY_SPIRENT
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyAdd(
       IN RvRtpSession session,
       IN RvUint8      *mki,
       IN RvUint8      *key,
       IN RvUint8      *salt,
       IN RvUint64      lifetime,
       IN RvUint64      threshold,
       IN RvUint8       direction
);
#define RvSrtpMasterKeyAddLocal(thisPtr, mki, key, salt, lifetime, threshold) \
    RvSrtpMasterKeyAdd(thisPtr, mki, key, salt, lifetime, threshold, RV_SRTP_KEY_LOCAL) 
#define RvSrtpMasterKeyAddRemote(thisPtr, mki, key, salt, lifetime, threshold) \
    RvSrtpMasterKeyAdd(thisPtr, mki, key, salt, lifetime, threshold, RV_SRTP_KEY_REMOTE) 
#else
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyAdd(
       IN RvRtpSession session,
       IN RvUint8      *mki,
       IN RvUint8      *key,
       IN RvUint8      *salt);
#endif // UPDATED_BY_SPIRENT

/************************************************************************************
 * RvSrtpMasterKeyRemove
 * description: This function removes a master key (and salt) from the session.
 * input: session - RTP session handle
 *        mki     - Pointer to the MKI identifier for the key.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
#ifdef UPDATED_BY_SPIRENT
RVAPI
RvStatus RVCALLCONV RvSrtpMasterKeyRemove(
       IN RvRtpSession session,
       IN RvUint8     *mki,
       IN RvUint8       direction
);
#define RvSrtpMasterKeyRemoveLocal(thisPtr, mki) \
    RvSrtpMasterKeyRemove(thisPtr, mki, RV_SRTP_KEY_LOCAL)
#define RvSrtpMasterKeyRemoveRemote(thisPtr, mki) \
    RvSrtpMasterKeyRemove(thisPtr, mki, RV_SRTP_KEY_REMOTE)
#else
RvStatus RVCALLCONV RvSrtpMasterKeyRemove(
       IN RvRtpSession session,
       IN RvUint8     *mki);
#endif // UPDATED_BY_SPIRENT

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
       IN RvRtpSession session);

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
#ifdef UPDATED_BY_SPIRENT
RVAPI
RvSize_t RVCALLCONV RvSrtpMasterKeyGetContextCount(
       IN RvRtpSession session,
       IN RvUint8      *mki,
       IN RvUint8       direction
);
#define RvSrtpMasterKeyGetContextCountLocal(thisPtr, mki) \
    rvSrtpMasterKeyGetContextCount(thisPtr, mki, RV_SRTP_KEY_LOCAL)
#define RvSrtpMasterKeyGetContextCountRemote(thisPtr, mki) \
    rvSrtpMasterKeyGetContextCount(thisPtr, mki, RV_SRTP_KEY_REMOTE)
#else
RVAPI
RvSize_t RVCALLCONV RvSrtpMasterKeyGetContextCount(
       IN RvRtpSession session,
       IN RvUint8      *mki);
#endif

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
        IN RvBool           shareTrigger);

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
 *                        this destination context switches, RV_FALSE otherwise.
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
        IN RvBool           shareTrigger);

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
        IN RvRtpSession  session,
        IN RvNetAddress* address);

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
        IN    RvRtpSession  session,
        IN    RvNetAddress* address,
        INOUT RvUint64*     indexPtr);

/************************************************************************************
 *                            Remote Source Management
 ************************************************************************************/

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
        IN RvUint32     sequenceNum);

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
        IN RvUint32     index);

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
        IN RvRtpSession       session,
        IN RvUint32           ssrc,
        IN RvSrtpStreamType   sourceType);

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
         IN RvRtpSession session);


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
 -----------------------------------------------------------------------------------
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
         IN RvUint8           *mki,
         IN RvBool           shareTrigger);

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
 *               appropriate range and values for the type of destination
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
 *        mki           - Pointer to the MKI identifier for the master key to use.
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
         IN RvRtpSession      session,
         IN RvUint32          ssrc,
         IN RvSrtpStreamType  sourceType,
         IN RvUint8           *mki,
         IN RvUint64          index,
         IN RvBool            shareTrigger);


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
         IN RvSrtpStreamType sourceType);

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
         IN RvRtpSession     session,
         IN RvUint32         ssrc,
         IN RvSrtpStreamType sourceType,
         INOUT RvUint64      *indexPtr);

#ifdef __cplusplus
}
#endif

#endif
