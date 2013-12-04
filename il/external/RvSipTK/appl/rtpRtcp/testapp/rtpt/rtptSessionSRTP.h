/***********************************************************************
Copyright (c) 2003 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..
RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef _RV_RTPTSESSION_SRTP_H_
#define _RV_RTPTSESSION_SRTP_H_


#include "rtptSession.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SRTP_ADD_ON)
/********************************************************************************************
 * function rtptSessionSRTPConstruct
 * purpose : constructs SRTP plugin for the session
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSRTPConstruct(IN rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionSRTPInit
 * purpose : inits SRTP plugin for the session 
 *           This function must be called after rtptSessionSRTPConstruct() and
 *           SRTP/SRTCP configuring
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSRTPInit(IN rtptSessionObj* s);

/********************************************************************************************
 * function rtptSessionSRTPDestruct
 * purpose : destructs SRTP plugin for the session 
 *           This function must be called after rtptSessionSRTPConstruct() 
 * input   : s - pointer to session object (rtptSessionObj) 
 * output  : none
 * return  : RvStatus
 ********************************************************************************************/
RvStatus rtptSessionSRTPDestruct(IN rtptSessionObj* s);


/* Master key management */
/************************************************************************************
 * rtptSessionSRTPMasterKeyAdd
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
 * input: s - pointer to session object (rtptSessionObj)
 *        mki     - Pointer to the MKI indentifier for the key.
 *        key     - Pointer to the master key.
 *        salt    - Pointer to the master salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus rtptSessionSRTPMasterKeyAdd(
       IN rtptSessionObj* s, 
       IN RvUint8*        mki,
       IN RvUint8*        key, 
       IN RvUint8*        salt);

/************************************************************************************
 * rtptSessionSRTPMasterKeyRemove
 * description: This function removes a master key (and salt) from the session.
 * input: s       - pointer to session object (rtptSessionObj)
 *        mki     - Pointer to the MKI indentifier for the key.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus rtptSessionSRTPMasterKeyRemove(
       IN rtptSessionObj* s, 
       IN RvUint8*        mki);

/************************************************************************************
 * rtptSessionSRTPMasterKeyRemoveAll
 * description: This function removes all master keys (and salts) from the session.
 * input:   s - pointer to session object (rtptSessionObj)
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus rtptSessionSRTPMasterKeyRemoveAll(IN rtptSessionObj* s);

/************************************************************************************
 * rtptSessionSRTPRemoteSourceAddRtp
 * description: This function adds a new remote RTP source to the session.
 *               A source needs to be created for each ssrc that the session
 *               will be receiving RTP from. If SRTCP is also going to be
 *               received for a given ssrc, a separate remote source needs
 *               to be added for it using rvSrtpRemoteSourceAddRtcp.
 *               If the starting rollover counter value (roc) and/or
 *               seqeunce number is not available, set them to zero, but be
 *               aware of the possible implications as explained in the
 *               SRTP/SRTCP RFC.
 * input: s             - The pointer to session object (rtptSessionObj)
 *        client        - client     
 *        session       - sessionID. SSRC from this session and client must be added
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus rtptSessionSRTPRemoteSourceAddRtp(
     IN rtptSessionObj* s,
     IN RvInt32         client, 
     IN RvInt32         session);

/************************************************************************************
 * rtptSessionSRTPRemoteSourceAddRtcp
 * description:  This function adds a new remote RTCP source to the session.
 *               A source needs to be created for each ssrc that the session
 *               will be receiving RTP from. If SRTP is also going to be
 *               received for a given ssrc, a separate remote source needs
 *               to be added for it using rvSrtpRemoteSourceAddRtp.
 *               If the starting index number is not available, set it to
 *               zero, but be aware of the implications as explained in the
 *               SRTP/SRTCP RFC.
 -----------------------------------------------------------
 * input: s             - The pointer to session object (rtptSessionObj)
 *        client        - client     
 *        session       - sessionID. SSRC from this session and client must be added
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPRemoteSourceAddRtcp(
        IN rtptSessionObj* s,
        IN RvInt32         client, 
        IN RvInt32         session);

/************************************************************************************
 * rtptSessionSRTPRemoteSourceRemove
 * description:  This function removes a remote source from the session.
 -----------------------------------------------------------
 * input: s             - The pointer to session object (rtptSessionObj)
 *        client        - client     
 *        session       - sessionID. SSRC from this session and client must be removed
 *        type          - TRUE - RTP, FALSE RTCP
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPRemoteSourceRemove(
        IN rtptSessionObj* s,
        IN RvInt32         client, 
        IN RvInt32         session,
        IN RvBool          type);

/************************************************************************************
 * rtptSessionSRTPRemoteSourceRemoveAll
 * description:  This function remove all remote sources from the session. (RTP and RTCP)
 -----------------------------------------------------------
 * input:  s  - The pointer to session object (rtptSessionObj)
 * output: none
 *
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPRemoteSourceRemoveAll(
        IN rtptSessionObj* s);

/************************************************************************************
 * rtptSessionSRTPAddDestinationContext
 * description: This function changes the master key that should be used to
 *              send to the specified destination. The new key will be used
 *              until changed, either by another call to this function or at a
 *              key change point specified by the RvSrtpDestinationChangeKeyAt
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
 * input: s             - The pointer to session object (rtptSessionObj)
 *        type          - RTP - RV_TRUE, RTCP - RV_FALSE
 *        address, port - destination address
 *        mki           - Pointer to the MKI indentifier for the master key to use.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys when 
 *                        this destination switches, RV_FALSE otherwise.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPAddDestinationContext(
        IN rtptSessionObj* s, 
        IN RvBool          type,
        IN const RvChar*   address,
        IN RvUint16        port,      
        IN RvUint8*        mki, 
        IN RvBool          shareTrigger);

/************************************************************************************
 * rtptSessionSRTPAddDestinationContextAt
 * description: This function sets a point in time (based on the index) at
 *      which the specified master key should be used to send to
 *      the specified destination. The new key will be used until
 *      changed, either by another change specified by a call to
 *      this function or by a call RvSrtpDestinationChangeKey once
 *      the key is in use.
 *      If shareTrigger is set to RV_TRUE, every source and
 *      destination in the session that shares the destinations old
 *      master key will switch to this same new master key when
 *      this destination switches to this new key.
 *      Remember that the index is different for an SRTP
 *      destination as opposed to an SRTCP destination. Use the
 *      approprate range and values for the type of destination
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
 *      If the ssrc of the master local source changes, all key
 *      mappings for the destination will be cleared and the destination
 *      will be reinitialized to use the new ssrc.
 * input: s             - The pointer to session object (rtptSessionObj)
 *        type          - RTP - RV_TRUE, RTCP - RV_FALSE
 *        address, port - destination address
 *        mki           - Pointer to the MKI identifier for the master key to use.
 *        indexoffset   - The addition to the current index at which the key should change.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys when 
 *                        this destination switches, RV_FALSE otherwise.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPAddDestinationContextAt(
     IN rtptSessionObj* s, 
     IN RvBool          type,
     IN const RvChar*   address,
     IN RvUint16        port,      
     IN RvUint8*        mki, 
     IN RvUint64        indexoffset, 
     IN RvBool          shareTrigger);

/************************************************************************************
 * rtptSessionSRTPSourceChangeKey
 * description:  This function changes the master key that should be used to
 *               receive from the specified remote source. The new key will be used
 *               until changed, either by another call to this function or at a
 *               key change point specified by the rtptSessionSRTPSourceChangeKeyAt()
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
 * input: s             - The pointer to session object (rtptSessionObj)
 *        client        - remote source client number (needed to retrive SSRC only)
 *        session       - remote source session ID (needed to retrive SSRC only)
 *        sourceType    - RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP
 *        mki           - Pointer to the MKI indentifier for the master key to use.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys 
 *                        when this source switches, RV_FALSE otherwise.
 * output: none
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPSourceChangeKey(
        IN rtptSessionObj* s, 
        IN RvUint32        client,
        IN RvUint32        session,
        IN RvUint8*        mki,
        IN RvBool          type,
        IN RvBool          shareTrigger);

/************************************************************************************
 * rtptSessionSRTPSourceChangeKeyAt
 * description:  This function sets a point in time (based on the index) at
 *               which the specified master key should be used to receive
 *               from the specified source. The new key will be used until
 *               changed, either by another change specified by a call to
 *               this function or by a call rtptSessionSRTPSourceChangeKey once
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
 * input: s             - The pointer to session object (rtptSessionObj)
 *        client        - remote source client number (needed to retrive SSRC only)
 *        session       - remote source session ID (needed to retrive SSRC only)
 *        type          - RV_SRTP_STREAMTYPE_SRTP or RV_SRTP_STREAMTYPE_SRTCP
 *        mki           - Pointer to the MKI indentifier for the master key to use.
 *        indexoffset   - The addition to the current index at which the key should change.
 *        shareTrigger  - RV_TRUE if anything sharing keys should switch keys 
 *                        when this source switches, RV_FALSE otherwise.
 * output: none
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPSourceChangeKeyAt(
     IN rtptSessionObj* s,    
     IN RvUint32        client,   
     IN RvUint32        session,
     IN RvUint8*        mki, 
     IN RvUint64        indexoffset, 
     IN RvBool          type,
     IN RvBool          shareTrigger);


/************************************************************************************
 * rtptSessionSRTPGetLocalParams
 * description: 
-----------------------------------------------------------
 * input: s             - The pointer to session object (rtptSessionObj)
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPGetLocalParams(
        IN rtptSessionObj* s);

/************************************************************************************
 * rtptSessionSRTPGetLocalIndexes
 * description: Retrieves the current local SRTP/SRTCP session indexes
-----------------------------------------------------------
 * input: s             - The pointer to session object (rtptSessionObj)
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPGetLocalIndexes(
        IN rtptSessionObj* s,       
        IN const RvChar*    IPstring, 
        IN RvInt32          portRtp,
        IN RvInt32          portRtcp);

/************************************************************************************
 * rtptSessionSRTP_ReceiveLocalParams
 * description: 
-----------------------------------------------------------
 * input: s                 - The pointer to session object (rtptSessionObj)
 *        client            - the client number, obtained from the broadcaster
 *        remSession        - remote session number
 *        receivedParamsPtr - pointer to the received srtp parameters
 *        senderAddressPtr  - pointer to the sender address
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTP_ReceiveLocalParams(
        IN rtptSessionObj* s,
        IN RvUint32  client,
        IN RvUint32  remSession,
        IN rtptSrtpParams *receivedParamsPtr,
        IN RvAddress *senderAddressPtr);

/************************************************************************************
 * rtptSessionSRTP_ReceiveIndexes
 * description: Sets indexes, which are taken from "sendindexes" command
-----------------------------------------------------------
 * input: s                 - The pointer to session object (rtptSessionObj)
 *        client            - the client number, obtained from the broadcaster
 *        remSession        - remote session number
 *        receivedParamsPtr - pointer to the received srtp parameters
 *        senderAddressPtr  - pointer to the sender address
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTP_ReceiveIndexes(
        IN rtptSessionObj* s,
        IN RvUint32  client,
        IN RvUint32  remSession,
        IN rtptSrtpParams *receivedParamsPtr,
        IN RvAddress *senderAddressPtr);

/* SRTP configuration */

/************************************************************************************
 * rtptSessionSRTPSetMasterKeySizes
 * description: 
-----------------------------------------------------------
 * input: s             - The pointer to session object (rtptSessionObj)
 *        mkiSize       - size of MKI
 *        keySize       - size of master keys
 *        saltSize      - size of salt keys
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPSetMasterKeySizes(
    IN rtptSessionObj* s, 
    IN RvInt32     mkiSize, 
    IN RvInt32     keySize, 
    IN RvInt32     saltSize);

/************************************************************************************
 * rtptSessionSRTPKeyDerivation
 * description: 
-----------------------------------------------------------
 * input: s                 - The pointer to session object (rtptSessionObj)
 *        keyDerivationAlg  - key derivation algorithm
 *        keyDerivationRate - size of master keys
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RvStatus  rtptSessionSRTPKeyDerivation(
     IN rtptSessionObj* s, 
     IN const RvChar*   keyDerivationAlg, 
     IN RvInt32         keyDerivationRate);


/************************************************************************************
 * rtptSessionSRTPPrefixLength
 * description: This function sets keystream prefix length (as per the
 *              RFC). This is normally set to 0 for standard SRTP/SRTCP and
 *              that is what the value defaults to if not specifically set.
 *              Currently, only a value of 0 is supported.
 -------------------------------------------------------------------------------------
 * input: s                - The pointer to session object (rtptSessionObj)
 *        perfixLength     - The length of the keystream prefix.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPPrefixLength(
       IN rtptSessionObj* s, 
       IN RvInt32         prefixLength);

/************************************************************************************
 * rtptSessionSRTPSetEncryption
 * description: This function sets the type of encrytion to use for RTP or RTCP
 *              along with the encyrption block size and whether or not the MKI
 *              should be included in the packets. Currently supported
 *              encryption types are
 *              AES-CM (RvSrtpEncryptionAlg_AESCM),
 *              AES-F8 (RvSrtpEncryptionAlg_AESF8),
 *              no encyrption (RvSrtpEncryptionAlg_NULL).
 *              The algorithm defaults to AES-CM and with MKI enabled, if
 *              they are not specifically set.
 -------------------------------------------------------------------------------------
 * input: s                - The pointer to session object (rtptSessionObj)
 *        rtpRtcpType      - RV_TRUE for setting RTP encryption, RV_FALSE for RTCP encryption
 *        encryptType      - Type of encryption to use for RTP.
 *        useMki           - RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetEncryption(
       IN rtptSessionObj* s, 
       IN RvBool          rtpRtcpType,
       IN const RvChar*   encryptType,
       IN RvBool          useMki);

/************************************************************************************
 * rtptSessionSRTPSetAuthentication
 * description: This function sets the type of authentication to use for RTP/RTCP
 *              along with the size of the authentication tag that
 *              will be included in each packet. Currently supported
 *              authentication types are
 *              HMAC-SHA1 (RvSrtpAuthenticationAlg_HMACSHA1),
 *              no authentication (RvSrtpAuthenticationAlg_None).
 *              If the authentication tags size is 0 then an authentication
 *              algorithm of RvSrtpAuthenticationAlg_None is assumed.
 *              The algorithm defaults to HMAC-SHA1 with a tag size of 10
 *              bytes (80 bits), if they are not specifically set.
 *              Note that the SRTP/SRTCP RFC requires that authentication
 *              be used with RTP/RTCP.
 *              This function may only be called before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s                - The pointer to session object (rtptSessionObj)
 *        rtpRtcpType      - RV_TRUE for setting RTP encryption, RV_FALSE for RTCP encryption
 *        authType      - Type of authentication to use for RTP/RTCP.
 *        tagSize       - The size of the authentication tag to use for RTP/RTCP.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetAuthentication(
       IN rtptSessionObj* s, 
       IN RvBool          rtpRtcpType,
       IN const RvChar*   authType,
       IN RvInt32         tagSize);

/************************************************************************************
 * rtptSessionSRTPSetKeySizes
 * description: This function sets the RTP session key sizes to be used for
 *              the encryption keys, authentication keys, and salts.
 *              The session key size must have a value greater than 0.
 *              If authentication is enabled, then the authentication key size
 *              must also be greater than 0.
 *              The session key will default to 16 bytes (128 bits), the
 *              session salt will default to 14 bytes (112 bits), and the
 *              authentication key will default to 20 bytes (160 bits), if
 *              not specifically set.
 *              This function may only be called before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s                - The pointer to session object (rtptSessionObj)
 *        rtpRtcpType      - RV_TRUE for setting RTP encryption, RV_FALSE for RTCP encryption
 *        authType      - Type of authentication to use for RTP/RTCP.
 *        tagSize       - The size of the authentication tag to use for RTP/RTCP.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetKeySizes(
       IN rtptSessionObj* s, 
       IN RvBool          rtpRtcpType,
       IN RvSize_t        encryptKeySize, 
       IN RvSize_t        authKeySize, 
       IN RvSize_t        saltSize);

/************************************************************************************
 * rtptSessionSRTPSetReplayListSize
 * description: This function sets the RTP/RTCP session key sizes to be used for
 *              the encryption keys, authentication keys, and salts.
 *              The session key size must have a value greater than 0.
 *              If authentication is enabled, then the authentication key size
 *              must also be greater than 0.
 *              The session key will default to 16 bytes (128 bits), the
 *              session salt will default to 14 bytes (112 bits), and the
 *              authentication key will default to 20 bytes (160 bits), if
 *              not specifically set.
 *              This function may only be called before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s                 - The pointer to session object (rtptSessionObj)
  *        rtpRtcpType      - RV_TRUE for setting RTP encryption, RV_FALSE for RTCP encryption
 *        replayListSize    - The size of the RTP/RTCP replay lists (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetReplayListSize(
       IN rtptSessionObj* s, 
       IN RvBool          rtpRtcpType,
       IN RvSize_t        replayListSize);

/************************************************************************************
 * rtptSessionSRTPSetHistory
 * description: This function sets the size (according to index count) of
 *              the history list that should be kept for each RTP/RTCP source and
 *              destination. Since the indexes are unsigned values that can
 *              wrap, this creates the "window" of indexes that are considered
 *              to be old indexes in the history, with the rest being future
 *              index values not yet received.
 *              This value defaults to 65536. Refer to the RvSrtpDestinationChangeKeyAt,
 *              RvSrtpRemoteSourceChangeKeyAt, or RvSrtpForwardDestinationChangeKeyAt (@@@ Future Translators)
 *              functions for further information on the effect of this setting.
 *              If the value is lower than the replay list size (set with
 *              RvSrtpSetRtcpReplayListSize), then the value will
 *              be set to the same as the replay list size.
 *              This function may only be called before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s                 - The pointer to session object (rtptSessionObj)
 *        rtpRtcpType      - RV_TRUE for setting RTP encryption, RV_FALSE for RTCP encryption
 *        historySize    - The SRTP/SRTCP key map history size (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetHistory(
       IN rtptSessionObj* s, 
       IN RvBool          rtpRtcpType,
       IN RvSize_t        historySize);

/************************************************************************************
 * rtptSessionSRTPSetMasterKeyPool
 * description: This function sets the behavior of the master key pool. A
 *              master key object is required for every master key that
 *              SRTP/SRTCP will maintain.
 *              Fixed pools are just that, fixed, and will only allow the
 *              specified number of objects to exist. Expanding pools will
 *              expand as necessary, but will never shrink (until the
 *              plugin is unregistered). Dynamic pools will, in addition to
 *              expanding, shrink based on the freeLevel parameter.
 *              By default, this pool will be configured as an expanding
 *              pool with 10 master key objects per memory page and the memory
 *              being allocated from the default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession    - SRTP session handle
 *        poolType       - The type of pool.
 *        pageItems      - The number of objects per page (0 = calculate from pageSize).
 *        pageSize       - The size of a memory page (0 = calculate from pageItems).
 *        maxItems       - The number of objects will never exceed this value (0 = no max).
 *        minItems       - The number of objects will never go below this value.
 *        freeLevel      - The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetMasterKeyPool(
       IN rtptSessionObj* s,
       IN const RvChar*   poolTypeStr, 
       IN RvSize_t        pageItems, 
       IN RvSize_t        pageSize, 
       IN RvSize_t        maxItems, 
       IN RvSize_t        minItems, 
       IN RvSize_t        freeLevel);

/************************************************************************************
 * rtptSessionSRTPSetStreamPool
 * description: This function sets the behavior of the stream pool. A
 *              stream object is required for every remote source and every
 *              destination that SRTP/SRTCP will maintain.
 *              Fixed pools are just that, fixed, and will only allow the
 *              specified number of objects to exist. Expanding pools will
 *              expand as necessary, but will never shrink (until the
 *              plugin is unregistered). Dynamic pools will, in addition to
 *              expanding, shrink based on the freeLevel parameter.
 *              By default, this pool will be configured as an expanding
 *              pool with 20 stream objects per memory page and the memory
 *              being allocated from the default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession    - SRTP session handle
 *        poolType       - The type of pool.
 *        pageItems      - The number of objects per page (0 = calculate from pageSize).
 *        pageSize       - The size of a memory page (0 = calculate from pageItems).
 *        maxItems       - The number of objects will never exceed this value (0 = no max).
 *        minItems       - The number of objects will never go below this value.
 *        freeLevel      - The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetStreamPool(
       IN rtptSessionObj* s,
       IN const RvChar*   poolTypeStr, 
       IN RvSize_t        pageItems, 
       IN RvSize_t        pageSize, 
       IN RvSize_t        maxItems, 
       IN RvSize_t        minItems, 
       IN RvSize_t        freeLevel);

/************************************************************************************
 * rtptSessionSRTPSetContextPool
 * description: This function sets the behavior of the Context pool. A
 *              context object is required for every combination of master
 *              key and stream that will be used and maintained in the
 *              history.
 *              Fixed pools are just that, fixed, and will only allow the
 *              specified number of objects to exist. Expanding pools will
 *              expand as necessary, but will never shrink (until the
 *              plugin is unregistered). Dynamic pools will, in addition to
 *              expanding, shrink based on the freeLevel parameter.
 *              By default, this pool will be configured as an expanding
 *              pool with 40 context objects per memory page and the memory
 *              being allocated from the default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession    - SRTP session handle
 *        poolType       - The type of pool.
 *        pageItems      - The number of objects per page (0 = calculate from pageSize).
 *        pageSize       - The size of a memory page (0 = calculate from pageItems).
 *        maxItems       - The number of objects will never exceed this value (0 = no max).
 *        minItems       - The number of objects will never go below this value.
 *        freeLevel      - The minimum number of objects per 100 (0 to 100) - DYNAMIC pools only.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetContextPool(
       IN rtptSessionObj* s,
       IN const RvChar*   poolTypeStr, 
       IN RvSize_t        pageItems, 
       IN RvSize_t        pageSize, 
       IN RvSize_t        maxItems, 
       IN RvSize_t        minItems, 
       IN RvSize_t        freeLevel);


/************************************************************************************
 * rtptSessionSRTPSetKeyHash
 * description: This function sets the behavior of the master key hash. A
 *              master key object is required for every master key that
 *              SRTP/SRTCP will maintain.
 *              Fixed hashes are just that, fixed, and will only allow the
 *              specified number of objects to be put into the hash table.
 *              Expanding hashes will expand as necessary, but will never
 *              shrink (until the plugin is unregistered). Dynamic hashes
 *              will, in addition to expanding, shrink to maintain optimum
 *              size, but will never shrink below the startSize parameter.
 *              By default, this hash will be configured as an expanding
 *              hash of size 17 with the memory	being allocated from the
 *              default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s              - SRTP application session handle
 *        hashTypeStr    - The type of hash string.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetKeyHash(
       IN rtptSessionObj* s,
       IN const RvChar*   hashTypeStr,  
       IN RvSize_t        startSize);


/************************************************************************************
 * rtptSessionSRTPSetSourceHash
 * description: This function sets the behavior of the source hash. A
 *              source exists for every remote source that
 *              SRTP/SRTCP will maintain.
 *              Fixed hashes are just that, fixed, and will only allow the
 *              specified number of objects to be put into the hash table.
 *              Expanding hashes will expand as necessary, but will never
 *              shrink (until the plugin is unregistered). Dynamic hashes
 *              will, in addition to expanding, shrink to maintain optimum
 *              size, but will never shrink below the startSize parameter.
 *              By default, this hash will be configured as an expanding
 *              hash of size 17 with the memory	being allocated from the
 *              default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s              - SRTP application session handle
 *        hashTypeStr    - The type of hash string.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetSourceHash(
       IN rtptSessionObj*  s,
       IN const RvChar*    hashTypeStr,  
       IN RvSize_t         startSize);


/************************************************************************************
 * rtptSessionSRTPSetDestHash
 * description: This function sets the behavior of the destination hash. A
 *              destination exists for every destination that
 *              SRTP/SRTCP will maintain.}
 *              Fixed hashes are just that, fixed, and will only allow the
 *              specified number of objects to be put into the hash table.
 *              Expanding hashes will expand as necessary, but will never
 *              shrink (until the plugin is unregistered). Dynamic hashes
 *              will, in addition to expanding, shrink to maintain optimum
 *              size, but will never shrink below the startSize parameter.}
 *              By default, this hash will be configured as an expanding
 *              hash of size 17 with the memory	being allocated from the
 *              default region.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: s              - SRTP application session handle
 *        hashTypeStr    - The type of hash string.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus rtptSessionSRTPSetDestHash(
       IN rtptSessionObj* s,
       IN const RvChar*   hashTypeStr,  
       IN RvSize_t        startSize);


#else /* SRTP_ADD_ON */

#define rtptSessionSRTPConstruct(s)                                                                 0
#define rtptSessionSRTPInit(s)                                                                      0
#define rtptSessionSRTPDestruct(s)                                                                  0
#define rtptSessionSRTPMasterKeyAdd(s, mki, key, salt)                                              0
#define rtptSessionSRTPMasterKeyRemove(s, mki)                                                      0
#define rtptSessionSRTPMasterKeyRemoveAll(s)                                                        0
#define rtptSessionSRTPRemoteSourceAddRtp(s, client, session)                                       0
#define rtptSessionSRTPRemoteSourceAddRtcp(s, client, session)                                      0
#define rtptSessionSRTPRemoteSourceRemove(s, client, session, type)                                 0
#define rtptSessionSRTPRemoteSourceRemoveAll(s)                                                     0
#define rtptSessionSRTPAddDestinationContext(s, type, address, port, mki, shareTrigger)             0
#define rtptSessionSRTPAddDestinationContextAt(s, type, address, port, mki, index, shareTrigger)    0
#define rtptSessionSRTPSourceChangeKey(s, client, session, mki, type, shareTrigger)                 0
#define rtptSessionSRTPSourceChangeKeyAt(s, client, session, mki, index, type, shareTrigger)        0
#define rtptSessionSRTPGetLocalParams(s)                                                            0
#define rtptSessionSRTPGetLocalIndexes(s, IPstring, portRtp, portRtcp)                              0
#define rtptSessionSRTP_ReceiveLocalParams   (s, client, remSession, receivedParamsPtr, senderAddressPtr) 0                                                       0

#define rtptSessionSRTPSetMasterKeySizes(s, mkiSize, keySize, saltSize)                             0
#define rtptSessionSRTPKeyDerivation(s, keyDerivationAlg, keyDerivationRate)                        0
#define rtptSessionSRTPPrefixLength(s, prefixLength)                                                0
#define rtptSessionSRTPSetEncryption(s, rtpRtcpType, encryptType, useMki)                           0
#define rtptSessionSRTPSetAuthentication(s, rtpRtcpType, authType, tagSize)                         0
#define rtptSessionSRTPSetKeySizes(s, rtpRtcpType, encryptKeySize, authKeySize, saltSize)           0
#define rtptSessionSRTPSetReplayListSize(s, rtpRtcpType, replayListSize)                            0
#define rtptSessionSRTPSetHistory(s, rtpRtcpType, historySize)                                      0

#define rtptSessionSRTPSetMasterKeyPool(s, poolTypeStr, pageItems, pageSize, maxItems, minItems, freeLevel) 0
#define rtptSessionSRTPSetStreamPool(s, poolTypeStr, pageItems, pageSize, maxItems, minItems, freeLevel)    0
#define rtptSessionSRTPSetContextPool(s, poolTypeStr, pageItems, pageSize, maxItems, minItems, freeLevel)   0
#define rtptSessionSRTPSetKeyHash(s, hashTypeStr, startSize)    0
#define rtptSessionSRTPSetSourceHash(s, hashTypeStr, startSize) 0
#define rtptSessionSRTPSetDestHash(s, hashTypeStr, startSize)   0
#endif /* SRTP_ADD_ON */

#ifdef __cplusplus
}
#endif

#endif /* _RV_RTPTSESSION_SRTP_H_ */
