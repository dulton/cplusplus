/************************************************************************
 File Name	   : rvsrtpconfig.c
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
#include "rvsrtp.h"
#include "rvsrtpconfig.h"
#include "rtputil.h"
#include "srtpProfileRfc3711.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                            Configuration functions                               *
 ************************************************************************************/

/************************************************************************************
 * RvSrtpSetMasterKeySizes
 * description: This function sets the key sizes to be used for master
 *              keys, master salts, and the mki identifier.
 *              The master key and MKI size must have values greater than 0.
 *              Even if MKI values are not to be included in the packet, a
 *              size is required because the MKI value is used to identify master keys.
 *              The master key will default to 16 bytes (128 bits), the
 *              master salt will default to 14 bytes (112 bits), and the MKI
 *              will default to 4 bytes, if not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession - SRTP session handle
 *        mkiSize     - The size of the MKI.
 *        keySize     - The size of the master key.
 *        saltSize    - The size of the master salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetMasterKeySizes(
       IN RvRtpSession srtpsession, 
       IN RvSize_t     mkiSize, 
       IN RvSize_t     keySize, 
       IN RvSize_t     saltSize)
{
   RvRtpSessionInfo* s = (RvRtpSessionInfo*) srtpsession;
   RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
   RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
   
   if (srtpPtr->rtpSession == NULL)
       return rvSrtpSetMasterKeySizes(srtpPtr, mkiSize, keySize, saltSize);

   return RV_ERROR_UNINITIALIZED;
}       

/************************************************************************************
 * RvSrtpSetKeyDerivation
 * description: This function sets the algorithm which should be used to
 *              generate new session keys and the rate at which new keys
 *              should be generated (0 meaning that keys should only be
 *              generated once). The only algorithm currently
 *              supported is AES-CM (RvSrtpKeyDerivationAlgAESCM).
 *              The algorithm defaults to AES-CM and the derivation rate
 *              defaults to 0, if not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession          - SRTP session handle
 *        keyDerivationAlg     - The key derivation algorithm.
 *        keyDerivationRate    - The rate at which to regenerate session keys.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetKeyDerivation(
       IN RvRtpSession           srtpsession, 
       IN RvSrtpKeyDerivationAlg keyDerivationAlg,
#ifndef UPDATED_BY_SPIRENT
       IN RvUint32               keyDerivationRate
#else
       IN RvUint32               keyDerivationRateLocal,
       IN RvUint32               keyDerivationRateRemote
#endif // UPDATED_BY_SPIRENT
)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32           alg = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 

    switch (keyDerivationAlg)
    {
    case RvSrtpKeyDerivationAlgNULL:  alg = 0;                              break;
    case RvSrtpKeyDerivationAlgAESCM: alg = RV_SRTP_KEYDERIVATIONALG_AESCM; break;
    }
   if (srtpPtr->rtpSession == NULL)
#ifndef UPDATED_BY_SPIRENT
        return rvSrtpSetKeyDerivation(srtpPtr, alg, keyDerivationRate);
#else
   return rvSrtpSetKeyDerivation(srtpPtr, alg, keyDerivationRateLocal, keyDerivationRateRemote);
#endif // UPDATED_BY_SPIRENT
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetPrefixLength
 * description: This function sets keystream prefix length (as per the
 *              RFC). This is normally set to 0 for standard SRTP/SRTCP and
 *              that is what the value defaults to if not specifically set.
 *              Currently, only a value of 0 is supported.}
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession      - SRTP session handle
 *        perfixLength     - The length of the keystream prefix.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ***********************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetPrefixLength(
       IN RvRtpSession srtpsession, 
       IN RvSize_t     prefixLength)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
   if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetPrefixLength(srtpPtr, prefixLength);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtpEncryption
 * description: This function sets the type of encryption to use for RTP
 *              along with the encryption block size and whether or not the MKI
 *              should be included in the packets. Currently supported
 *              encryption types are
 *              AES-CM (RvSrtpEncryptionAlg_AESCM),
 *              AES-F8 (RvSrtpEncryptionAlg_AESF8),
 *              no encryption (RvSrtpEncryptionAlg_NULL).
 *              The algorithm defaults to AES-CM and with MKI enabled, if
 *              they are not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession      - SRTP session handle
 *        encryptType      - Type of encryption to use for RTP.
 *        useMki           - RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtpEncryption(
       IN RvRtpSession        srtpsession, 
       IN RvSrtpEncryptionAlg encryptType, 
       IN RvBool              useMki)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32           alg = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    switch (encryptType)
    {
    case RvSrtpEncryptionAlg_NULL:  alg = RV_SRTP_ENCYRPTIONALG_NULL;  break;
    case RvSrtpEncryptionAlg_AESCM: alg = RV_SRTP_ENCYRPTIONALG_AESCM; break;
    case RvSrtpEncryptionAlg_AESF8: alg = RV_SRTP_ENCYRPTIONALG_AESF8; break;
    }
   if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtpEncryption(srtpPtr, alg, useMki);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtcpEncryption
 * description: This function sets the type of encryption to use for RTCP
 *              along with the encryption block size and whether or not the MKI
 *              should be included in the packets. Currently supported
 *              encryption types are
 *              AES-CM (RvSrtpEncryptionAlg_AESCM),
 *              AES-F8 (RvSrtpEncryptionAlg_AESF8),
 *              no encryption (RvSrtpEncryptionAlg_NULL).
 *              The algorithm defaults to AES-CM and with MKI enabled, if
 *              they are not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession      - SRTP session handle
 *        encryptType      - Type of encryption to use for RTP.
 *        useMki           - RV_TRUE to use MKI in RTP packet, RV_FALSE to not include it.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtcpEncryption(
       IN RvRtpSession        srtpsession, 
       IN RvSrtpEncryptionAlg encryptType, 
       IN RvBool              useMki)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32           alg = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;

    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    switch (encryptType)
    {
    case RvSrtpEncryptionAlg_NULL:  alg = RV_SRTP_ENCYRPTIONALG_NULL;  break;
    case RvSrtpEncryptionAlg_AESCM: alg = RV_SRTP_ENCYRPTIONALG_AESCM; break;
    case RvSrtpEncryptionAlg_AESF8: alg = RV_SRTP_ENCYRPTIONALG_AESF8; break;
    }
   if (srtpPtr->rtpSession == NULL)
        return rvSrtpSetRtcpEncryption(srtpPtr, alg, useMki);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtpAuthentication
 * description:         {p: This function sets the type of authentication to use for RTP
 *              along with the size of the authentication tag that
 *              will be included in each packet. Currently supported
 *              authentication types are
 *              HMAC-SHA1 (RvSrtpAuthenticationAlg_HMACSHA1),
 *              no authentication (RvSrtpAuthenticationAlg_None).
 *              If the authentication tags size is 0 then an authentication
 *              algorithm of RvSrtpAuthenticationAlg_None is assumed.
 *              The algorithm defaults to HMAC-SHA1 with a tag size of 10
 *              bytes (80 bits), if they are not specifically set.
 *   Note:      1. This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 *              2. The tagSize must be less or equal than SHA_DIGESTSIZE, which is 20 for open SSL library. 
 -------------------------------------------------------------------------------------
 * input: srtpsession   - SRTP session handle
 *        authType      - Type of authentication to use for RTP.
 *        tagSize       - The size of the authentication tag to use for RTP.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtpAuthentication(
       IN RvRtpSession            srtpsession, 
       IN RvSrtpAuthenticationAlg authType, 
       IN RvSize_t                tagSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32           alg = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    switch (authType)
    {
    case RvSrtpAuthenticationAlg_NONE:     alg = RV_SRTP_AUTHENTICATIONALG_NONE;     break;
    case RvSrtpAuthenticationAlg_HMACSHA1: alg = RV_SRTP_AUTHENTICATIONALG_HMACSHA1; break;
    }
   if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtpAuthentication(srtpPtr, alg, tagSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtcpAuthentication
 * description:  This function sets the type of authentication to use for RTCP
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
 *              be used with RTCP.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession   - SRTP session handle
 *        authType      - Type of authentication to use for RTCP.
 *        tagSize       - The size of the authentication tag to use for RTCP.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtcpAuthentication(
       IN RvRtpSession             srtpsession,
       IN RvSrtpAuthenticationAlg  authType, 
       IN RvSize_t                 tagSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32           alg = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    switch (authType)
    {
    case RvSrtpAuthenticationAlg_NONE:     alg = RV_SRTP_AUTHENTICATIONALG_NONE;     break;
    case RvSrtpAuthenticationAlg_HMACSHA1: alg = RV_SRTP_AUTHENTICATIONALG_HMACSHA1; break;
    }
    if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtcpAuthentication(srtpPtr, alg, tagSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtpKeySizes
 * description: This function sets the RTP session key sizes to be used for
 *              the encryption keys, authentication keys, and salts.
 *              The session key size must have a value greater than 0.
 *              If authentication is enabled, then the authentication key size
 *              must also be greater than 0.
 *              The session key will default to 16 bytes (128 bits), the
 *              session salt will default to 14 bytes (112 bits), and the
 *              authentication key will default to 20 bytes (160 bits), if
 *              not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession       - SRTP session handle
 *        encryptKeySize    - The size of the RTP encryption key.
 *        authKeySize       - The size of the RTP authentication key.
 *        saltSize          - The size of the RTP salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtpKeySizes(
       IN RvRtpSession srtpsession, 
       IN RvSize_t     encryptKeySize, 
       IN RvSize_t     authKeySize, 
       IN RvSize_t     saltSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
   if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtpKeySizes(srtpPtr, encryptKeySize, authKeySize, saltSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtcpKeySizes
 * description: This function sets the RTCP session key sizes to be used for
 *              the encryption keys, authentication keys, and salts.
 *              The session key size must have a value greater than 0.
 *              If authentication is enabled, then the authentication key size
 *              must also be greater than 0.
 *              The session key will default to 16 bytes (128 bits), the
 *              session salt will default to 14 bytes (112 bits), and the
 *              authentication key will default to 20 bytes (160 bits), if
 *              not specifically set.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession       - SRTP session handle
 *        encryptKeySize    - The size of the RTCP encryption key.
 *        authKeySize       - The size of the RTCP authentication key.
 *        saltSize          - The size of the RTCP salt.
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtcpKeySizes(
       IN RvRtpSession srtpsession, 
       IN RvSize_t     encryptKeySize, 
       IN RvSize_t     authKeySize, 
       IN RvSize_t     saltSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
   if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtcpKeySizes(srtpPtr, encryptKeySize, authKeySize, saltSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtpReplayListSize
 * description: This function sets the size of the replay list to be used
 *              for insuring that RTP packets received from remote sources are not
 *              replicated (intentionally or accidentally). Refer to the
 *              SRTP/SRTCP RFC for further information.
 *              Setting the size to 0 disables the use of replay lists.
 *              The minimum (and default) size of the replay list is 64 (as per the RFC).
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession       - SRTP session handle
 *        replayListSize    - The size of the RTP replay lists (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtpReplayListSize(
       IN RvRtpSession srtpsession, 
       IN RvUint64     replayListSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtpReplayListSize(srtpPtr, replayListSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtcpReplayListSize
 * description: This function sets the size of the replay list to be used
 *              for insuring that RTCP packets received from remote sources are not
 *              replicated (intentionally or accidentally). Refer to the
 *              SRTP/SRTCP RFC for further information.
 *              Setting the size to 0 disables the use of replay lists.
 *              The minimum (and default) size of the replay list is 64 (as per the RFC).
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession       - SRTP session handle
 *        replayListSize    - The size of the RTCP replay lists (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtcpReplayListSize(
       IN RvRtpSession srtpsession, 
       IN RvUint64     replayListSize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
   if (srtpPtr->rtpSession == NULL)
        return rvSrtpSetRtcpReplayListSize(srtpPtr, replayListSize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetRtpHistory
 * description: This function sets the size (according to index count) of
 *              the history list that should be kept for each RTP source and
 *              destination. Since the indexes are unsigned values that can
 *              wrap, this creates the "window" of indexes that are considered
 *              to be old indexes in the history, with the rest being future
 *              index values not yet received.
 *              This value defaults to 65536. Refer to the RvSrtpDestinationChangeKeyAt,
 *              RvSrtpRemoteSourceChangeKeyAt, or RvSrtpForwardDestinationChangeKeyAt (@@@ Future Translators)
 *              functions for further information on the effect of this setting.
 *              If the value is lower than the replay list size (set with
 *              RvSrtpSetRtpReplayListSize), then the value will
 *              be set to the same as the replay list size.
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession    - SRTP session handle
 *        historySize    - The SRTP key map history size (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtpHistory(
       IN RvRtpSession srtpsession, 
       IN RvUint64     historySize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if (srtpPtr->rtpSession == NULL)
        return rvSrtpSetRtpHistory(srtpPtr, historySize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}


/************************************************************************************
 * RvSrtpSetRtcpHistory
 * description: This function sets the size (according to index count) of
 *              the history list that should be kept for each RTCP source and
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
 *   Note:      This function may be performed only after calling RvSrtpConstruct and
 *              before RvSrtpInit().
 -------------------------------------------------------------------------------------
 * input: srtpsession    - SRTP session handle
 *        historySize    - The SRTCP key map history size (in packets).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetRtcpHistory(
       IN RvRtpSession srtpsession, 
       IN RvUint64     historySize)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if ((srtpPtr)->rtpSession == NULL)
        return rvSrtpSetRtcpHistory(srtpPtr, historySize);
    
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 *                            Advanced Configuration                                *
 ************************************************************************************/

/************************************************************************************
 * RvSrtpSetMasterKeyPool
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
RvStatus RVCALLCONV RvSrtpSetMasterKeyPool(
       IN RvRtpSession   srtpsession, 
       IN RvRtpPoolType poolType, 
       IN RvSize_t       pageItems, 
       IN RvSize_t       pageSize, 
       IN RvSize_t       maxItems, 
       IN RvSize_t       minItems, 
       IN RvSize_t       freeLevel/*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 pltype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;

    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if (srtpPtr->rtpSession == NULL)
    {
        switch (poolType)
        {
        case RvRtpPoolTypeFixed:     pltype = RV_SRTP_POOLTYPE_FIXED;     break;
        case RvRtpPoolTypeExpanding: pltype = RV_SRTP_POOLTYPE_EXPANDING; break;
        case RvRtpPoolTypeDynamic:   pltype = RV_SRTP_POOLTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetMasterKeyPool(
            srtpPtr, pltype, pageItems, pageSize, maxItems, minItems, freeLevel, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
    
}

/************************************************************************************
 * RvSrtpSetStreamPool
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
RvStatus RVCALLCONV RvSrtpSetStreamPool(
       IN RvRtpSession   srtpsession, 
       IN RvRtpPoolType poolType, 
       IN RvSize_t       pageItems, 
       IN RvSize_t       pageSize, 
       IN RvSize_t       maxItems, 
       IN RvSize_t       minItems, 
       IN RvSize_t       freeLevel /*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 pltype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if (srtpPtr->rtpSession == NULL)
    {
        switch (poolType)
        {
        case RvRtpPoolTypeFixed:     pltype = RV_SRTP_POOLTYPE_FIXED;     break;
        case RvRtpPoolTypeExpanding: pltype = RV_SRTP_POOLTYPE_EXPANDING; break;
        case RvRtpPoolTypeDynamic:   pltype = RV_SRTP_POOLTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetStreamPool(
            srtpPtr, pltype, pageItems, pageSize, maxItems, minItems, freeLevel, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
    
}

/************************************************************************************
 * RvSrtpSetContextPool
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
RvStatus RVCALLCONV RvSrtpSetContextPool(
       IN RvRtpSession   srtpsession, 
       IN RvRtpPoolType  poolType, 
       IN RvSize_t       pageItems, 
       IN RvSize_t       pageSize, 
       IN RvSize_t       maxItems, 
       IN RvSize_t       minItems, 
       IN RvSize_t       freeLevel/*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 pltype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if ((srtpPtr)->rtpSession == NULL)
    {
        switch (poolType)
        {
        case RvRtpPoolTypeFixed:     pltype = RV_SRTP_POOLTYPE_FIXED;     break;
        case RvRtpPoolTypeExpanding: pltype = RV_SRTP_POOLTYPE_EXPANDING; break;
        case RvRtpPoolTypeDynamic:   pltype = RV_SRTP_POOLTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetContextPool(
            srtpPtr, pltype, pageItems, pageSize, maxItems, minItems, freeLevel, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
    
}

/************************************************************************************
 * RvSrtpSetMasterKeyHash
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
 * input: srtpsession    - SRTP session handle
 *        hashType       - The type of hash.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetMasterKeyHash(
       IN RvRtpSession   srtpsession, 
       IN RvSrtpHashType hashType, 
       IN RvSize_t       startSize/*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 htype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if ((srtpPtr)->rtpSession == NULL)
    {
        switch (hashType)
        {
        case RvSrtpHashTypeFixed:     htype = RV_SRTP_HASHTYPE_FIXED;     break;
        case RvSrtpHashTypeExpanding: htype = RV_SRTP_HASHTYPE_EXPANDING; break;
        case RvSrtpHashTypeDynamic:   htype = RV_SRTP_HASHTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetMasterKeyHash(srtpPtr, htype, startSize, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
    
}


/************************************************************************************
 * RvSrtpSetSourceHash
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
 * input: srtpsession    - SRTP session handle
 *        hashType       - The type of hash.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetSourceHash(
       IN RvRtpSession   srtpsession, 
       IN RvSrtpHashType hashType, 
       IN RvSize_t       startSize/*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 htype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if ((srtpPtr)->rtpSession == NULL)
    {
        switch (hashType)
        {
        case RvSrtpHashTypeFixed:     htype = RV_SRTP_HASHTYPE_FIXED;     break;
        case RvSrtpHashTypeExpanding: htype = RV_SRTP_HASHTYPE_EXPANDING; break;
        case RvSrtpHashTypeDynamic:   htype = RV_SRTP_HASHTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetSourceHash(srtpPtr, htype, startSize, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

/************************************************************************************
 * RvSrtpSetDestHash
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
 * input: srtpsession    - SRTP session handle
 *        hashType       - The type of hash.
 *        startSize      - The starting number of buckets in the hash table (and minimum size).
 * output: none.
 * return value: Non-negative value on success
 *               Negative value on failure
 ************************************************************************************/
RVAPI
RvStatus RVCALLCONV RvSrtpSetDestHash(
       IN RvRtpSession   srtpsession, 
       IN RvSrtpHashType hashType, 
       IN RvSize_t       startSize/*,       IN RvMemory *region get from default region*/)
{
    RvRtpSessionInfo* s   = (RvRtpSessionInfo*) srtpsession;
    RvInt32 htype = 0;
    RtpProfileRfc3711* rfc3711ProfilePlugin = NULL;
    RvSrtp*           srtpPtr              = NULL;
    
    if (s == NULL || s->profilePlugin == NULL)
        return RV_ERROR_NULLPTR; /* perhaps it already has been destructed */
    
    rfc3711ProfilePlugin = (RtpProfileRfc3711*) s->profilePlugin->userData;
    srtpPtr              =  &rfc3711ProfilePlugin->srtp; 
    
    if (srtpPtr->rtpSession == NULL)
    {
        switch (hashType)
        {
        case RvSrtpHashTypeFixed:     htype = RV_SRTP_HASHTYPE_FIXED;     break;
        case RvSrtpHashTypeExpanding: htype = RV_SRTP_HASHTYPE_EXPANDING; break;
        case RvSrtpHashTypeDynamic:   htype = RV_SRTP_HASHTYPE_DYNAMIC;   break;
        }
        return rvSrtpSetDestHash(srtpPtr, htype, startSize, NULL);
    }
    return RV_ERROR_UNINITIALIZED; /* SRTP uninitialised */
}

#ifdef __cplusplus
}
#endif

