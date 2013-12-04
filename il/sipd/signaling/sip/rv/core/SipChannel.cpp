/// @file
/// @brief SIP core channel functionality implementation.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "RvSipAka.h"

#include "SipChannel.h"
#include "VoIPUtils.h"

USING_RV_SIP_ENGINE_NAMESPACE;

const unsigned short SipChannel::SIP_DEFAULT_PORT = 5060;

const uint32_t SipChannel::DEFAULT_TIME_BEFORE_REREGISTRATION = 300; //Value in secs !
const uint32_t SipChannel::MIN_REREGISTRATION_SESSION_TIME = 60; //Value in secs !

const uint32_t SipChannel::MANY_UAS_PER_DEVICE_THRESHOLD = 256;
const uint32_t SipChannel::BULK_UAS_PER_DEVICE_THRESHOLD = 1024;
const uint32_t SipChannel::MULTIPLE_UAS_PER_DEVICE_THRESHOLD = 8;
const uint32_t SipChannel::LARGE_SIGNALING_SOCKET_SIZE = 1024000;
const uint32_t SipChannel::BULK_SIGNALING_SOCKET_SIZE = 10240000;
const uint32_t SipChannel::NORMAL_SIGNALING_SOCKET_SIZE = 128000;
const uint32_t SipChannel::MODEST_SIGNALING_SOCKET_SIZE = 8192;
const uint32_t SipChannel::SMALL_SIGNALING_SOCKET_SIZE = 4096;

const uint32_t SipChannel::POST_RING_PRACK_RCV_TIMEOUT = 32; //Value in secs !
const uint32_t SipChannel::POST_CALL_TERMINATE_TIMEOUT = 64; //Value in secs !
const uint32_t SipChannel::CALL_TIMER_MEDIA_POSTCALL_TIMEOUT = 20; //value in MSecs !
const std::string SipChannel::DEFAULT_NETWORKINFO_ID("00000000");
const std::string SipChannel::DEFAULT_IMSI("111222000000000");
const int SipChannel::DEFAULT_REG_SUBS_EXPIRES=3600;
const uint32_t SipChannel::CALL_STOP_GRACEFUL_TIME=1;//value in secs
const uint32_t SipChannel::DEFAULT_TIP_DEVICE_TYPE=481;//CTS500_37 for back compability

const char* SipChannel::ALLOWED_METHODS = "INVITE,PRACK,ACK,CANCEL,BYE,UPDATE,REFER,NOTIFY,MESSAGE,OPTIONS";

static void regTimerExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->regTimerExpirationEventCB();
    }
}

static void callTimerExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->callTimerExpirationEventCB();
    }
}

static void callTimerMediaExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->callTimerMediaExpirationEventCB();
    }
}

static void byeAcceptTimerMediaExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->byeAcceptTimerMediaExpirationEventCB();
    }
}

static void postCallTimerExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->postCallTimerExpirationEventCB();
    }
}

static void ringTimerExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->ringTimerExpirationEventCB();
    }
}

static void prackTimerExpires(void* context) {
    if(context) {
        SipChannel *sc=(SipChannel*)context;
        sc->prackTimerExpirationEventCB();
    }
}
TipStatus::TipStatus(){
    reset();
}
void TipStatus::reset(){
    for(int i=TIP_NS::TIPMediaAudio;i<TIP_NS::TIPMediaEnd;i++){
        mTipStatus[i] = TIP_NS::TIP_STATUS_IDLE;
        mLocalDone[i] = false;
        mRemoteDone[i] = false;
        mFail[i] = false;
        mToSendReInvite = false;
    }
}
void TipStatus::setStatus(TIP_NS::TIPMediaType type,TIP_NS::TIPNegoStatus status,void *arg){
    if(type != TIP_NS::TIPMediaEnd){
        mTipStatus[type] = status;
        switch(status){
            case TIP_NS::TIP_STATUS_LASTACKRCVD:
                mLocalDone[type] = true;
                break;
            case TIP_NS::TIP_STATUS_LASTACKXMIT:
                if(type == TIP_NS::TIPMediaVideo){
                    bool reinvite = arg?true:false;
                    mToSendReInvite = reinvite;
                }
                mRemoteDone[type] = true;
                break;
            case TIP_NS::TIP_STATUS_MISMATCH:
            case TIP_NS::TIP_STATUS_INCOMPATIBLE:
            case TIP_NS::TIP_STATUS_FAILED:
                mFail[type] = true;
                break;
            default:
                break;

        }
    }
}
    TIP_NS::TIPNegoStatus TipStatus::getStatus(TIP_NS::TIPMediaType type){
        if(type != TIP_NS::TIPMediaEnd)
            return mTipStatus[type];
        return TIP_NS::TIP_STATUS_IDLE;
    }
bool TipStatus::localNegoDone(TIP_NS::TIPMediaType type){
    if(type != TIP_NS::TIPMediaEnd){
        return mLocalDone[type];
    }
    return false;
}
bool TipStatus::remoteNegoDone(TIP_NS::TIPMediaType type){
    if(type != TIP_NS::TIPMediaEnd){
        return mRemoteDone[type];
    }
    return false;
}
bool TipStatus::NegoFail(){
    return mFail[TIP_NS::TIPMediaAudio] || mFail[TIP_NS::TIPMediaVideo];

}
bool TipStatus::NegoFail(TIP_NS::TIPMediaType type){
    if(type != TIP_NS::TIPMediaEnd){
        return mFail[type];
    }
    return false;
}
bool TipStatus::NegoDone(){
    return localNegoDone(TIP_NS::TIPMediaAudio) && localNegoDone(TIP_NS::TIPMediaVideo) && remoteNegoDone(TIPMediaAudio) && remoteNegoDone(TIP_NS::TIPMediaVideo);
}


SipChannel::SipChannel() : mSMType(CFT_BASIC),
    mTimers(mLocker),
    mIsFree(false),
    mRole(TERMINATE),
    mRel100(false),
    mSigOnly(false),
    mShortForm(false),
    mUasPerDevice(0),
    mSecureReqURI(false),
    mUseHostInCallID(false),
    mAnonymous(false),
    mPrivacyOptions(0),
    mCallTime(0),
    mRingTime(0),
    mPrackTime(0),
    mPostCallTime(0),
    mIsInviting(false),
    mIsDisconnecting(false),
    mIsConnected(false),
    mReadyToAccept(false),
    mInitialInviteSent(false),
    mInitialByeSent(false),
    mHandle(0),
    mIsEnabled(false),
    mTransport(RVSIP_TRANSPORT_UNDEFINED),
    mIpServiceLevel(0),
    mContactDomainPort(0),
    mLocalURIDomainPort(0),
    mRemoteDomainPort(0),
    mTelUriRemote(false),
    mProxyPort(0),
    mUseProxyDiscovery(false),
    mUsingProxy(false),
    mRegServerPort(0),
    mRegExpiresTime(3600),
    mRefreshRegistration(false),
    mReregSecsBeforeExpTime(300),
    mTelUriLocal(false),
    mIsDeregistering(false),
    mIsInRegTimer(false),
    mRegRetries(0),
    mIsRegistering(false),
    mUseRegistrarDiscovery(false),
    mUseAka(false),
    mUseMobile(false),
    mRefresher(RVSIP_SESSION_EXPIRES_REFRESHER_NONE),
    mRefresherPref(RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE),
    mRefresherStarted(false),
    mSessionExpiresTime(0),
    mRefreshed(false),
    mCallStateNotifier(0),
    mMediaHandler(0),
    mHCallLeg(0),
    mHReInvite(0),
    mHSubsCallLeg(0),
    mHByeTranscPending(0),
    mHMgr(0),
    mHMsgMgr(0),
    mHTrMgr(0),
    mHMidMgr(0),
    mHAuthMgr(0),
    mAppPool(0),
    mHRegMgr(0),
    mHRegClient(0),
    mHRegTimer(0),
    mHCallTimer(0),
    mHCallTimerMedia(0),
    mHByeAcceptTimerMedia(0),
    mHRingTimer(0),
    mHPrackTimer(0),
    mNetworkAccessType(RAN_TYPE_3GPP_GERAN),
    mNetworkAccessCellID(DEFAULT_NETWORKINFO_ID),
    mUseIMSI(false),
    mImsSubsExpires(DEFAULT_REG_SUBS_EXPIRES),
    mHImsSubs(0),
    mSubOnReg(false),
    mSubsToDeregister(false),
    mTipDone(false),
    mRemoteSDPOffered(false),
    mTipStarted(false),
    mTipDeviceType(DEFAULT_TIP_DEVICE_TYPE)
{
    memset(&mLocalAddress,0,sizeof(mLocalAddress));
    memset(&mAddressRemote,0,sizeof(mAddressRemote));
    memset(&mAuthVector,0,sizeof(mAuthVector));
    memset(&mOP,0,sizeof(mOP));
    mAuthTry=0;
}

void SipChannel::set(RvSipCallLegMgrHandle hMgr,
        RvSipMsgMgrHandle hMsgMgr,
        RvSipMidMgrHandle hMidMgr,
        RvSipRegClientMgrHandle hRegMgr,
        RvSipTransportMgrHandle hTrMgr,
        RvSipAuthenticatorHandle hAuthMgr,
        HRPOOL appPool,
        RvSipSubsMgrHandle hSubsMgr)
{
    ENTER();

    RVSIPLOCK(mLocker);

    mHMgr = hMgr;
    mHMsgMgr = hMsgMgr;
    mHMidMgr = hMidMgr;
    mHRegMgr = hRegMgr;
    mHTrMgr = hTrMgr;
    mHAuthMgr = hAuthMgr;
    mAppPool = appPool;
    mHSubsMgr = hSubsMgr;

    mTimers.set(hMidMgr);

    RET;
}

void SipChannel::setProcessingType(bool rel100,
        bool shortForm,
        bool sigOnly,
        bool secureReqURI,
        bool useHostInCallID,
        bool anonymous,
        bool useProxy,
        uint16_t privacyOptions)
{
    ENTER();

    RVSIPLOCK(mLocker);

    mRel100 = rel100;
    mShortForm = shortForm;
    if(mSecureReqURI != secureReqURI)
        mPreferredIdentity = "";
    mSecureReqURI = secureReqURI;
    mUseHostInCallID = useHostInCallID;
    mSigOnly=sigOnly;
    mAnonymous=anonymous;
    mUsingProxy = useProxy;
    mPrivacyOptions=privacyOptions;
    mTipDone = false;
    mRemoteSDPOffered = false;
    mTipStarted = false;

    RET;
}

void SipChannel::setSessionExpiration(int sessionExpires, RvSipSessionExpiresRefresherType rtype)
{
    ENTER();

    RVSIPLOCK(mLocker);

    mSessionExpiresTime = sessionExpires;
    mRefresher = rtype;

    RET;
}

void SipChannel::setIdentity(const std::string& userName,  
        const std::string& userNumber,
        const std::string& userNumberContext,
        const std::string& displayName,
        bool telUriLocal)
{
    ENTER();

    RVSIPLOCK(mLocker);

    mUserName = userName;
    mUserNumber = userNumber;
    mUserNumberContext = userNumberContext;
    mDisplayName=displayName;

    fixUserNamePrivate(mUserName);
    fixUserNamePrivate(mUserNumber);
    fixUserNamePrivate(mUserNumberContext);
    fixUserNamePrivate(mDisplayName);

    mTelUriLocal = telUriLocal;

    RET;
}

void SipChannel::setIMSIdentity(const std::string& impu,const std::string& impi){
    ENTER();

    RVSIPLOCK(mLocker);
    if(useIMSIPrivate()){
        mIMPU = impu;
        mIMPI = impi;
        mAuthName = impi;
    }
    RET;
}

void SipChannel::setContactDomain(const std::string& contactDomain,
        unsigned short contactDomainPort) {

    ENTER();

    RVSIPLOCK(mLocker);

    mContactDomain = contactDomain;
    fixIpv6Address(mContactDomain);
    fixUserNamePrivate(mContactDomain);

    mContactDomainPort = contactDomainPort;

    RET;
}

void SipChannel::setLocalURIDomain(const std::string& localURIDomain,
        unsigned short localURIDomainPort) {

    ENTER();

    RVSIPLOCK(mLocker);

    mLocalURIDomain = localURIDomain;
    fixIpv6Address(mLocalURIDomain);
    fixUserNamePrivate(mLocalURIDomain);

    mLocalURIDomainPort = localURIDomainPort;

    RET;
}

RvStatus SipChannel::setLocal(const RvSipTransportAddr& addr, uint8_t ipServiceLevel, uint32_t uasPerDevice) {

    RVSIPLOCK(mLocker);

    if(mLocalAddress.strIP[0] && 
            ((mLocalAddress.eTransportType != addr.eTransportType )  ||
             (mLocalAddress.port  != addr.port )) )
        removeLocalAddressPrivate();

    mLocalAddress = addr;

    fixIpv6Address(mLocalAddress.strIP,sizeof(mLocalAddress.strIP));

    if(mLocalAddress.eTransportType == RVSIP_TRANSPORT_UDP) {
        mTransport=RVSIP_TRANSPORT_UDP;
    } else if(mLocalAddress.eTransportType == RVSIP_TRANSPORT_TCP) {
        mTransport=RVSIP_TRANSPORT_TCP;
        mRel100=false;
    } else {
        mLocalAddress.eTransportType=RVSIP_TRANSPORT_UDP;
    }

    mIpServiceLevel=ipServiceLevel;
    mUasPerDevice=uasPerDevice;

    return addLocalAddressPrivate();
}

SipChannel::~SipChannel()
{
}


std::string SipChannel::getTransportStr() const {
    RVSIPLOCK(mLocker);
    if(mTransport==RVSIP_TRANSPORT_UDP) return "UDP";
    else if(mTransport == RVSIP_TRANSPORT_TCP) return "TCP";
    else return "";
}

void SipChannel::NotifyInterfaceDisabled(void) {

    RVSIPLOCK(mLocker);
    stopRegistration();
    stopCallSession();
    removeLocalAddressPrivate();
}

void SipChannel::NotifyInterfaceEnabled(void) {

    RVSIPLOCK(mLocker);
    addLocalAddressPrivate();
}

RvStatus SipChannel::startRegistration() {

    RVSIPLOCK(mLocker);

    stopRegistration();

    mIsDeregistering=false;
    return startRegistrationProcessPrivate(mRegExpiresTime);
}

RvStatus SipChannel::startDeregistration() {

    RVSIPLOCK(mLocker);

    if(mHImsSubs) {
        mSubsToDeregister=true;
        return UnsubscribeRegEventPrivate();
    }

    mIsDeregistering=true;
    return startRegistrationProcessPrivate(0);
}


RvStatus SipChannel::stopRegistration() {

    RVSIPLOCK(mLocker);

    regTimerResetPrivate();
    if(mHRegClient) {
        RvSipRegClientState eState = RVSIP_REG_CLIENT_STATE_UNDEFINED;
        RvSipRegClientGetCurrentState (mHRegClient,&eState);
        if(eState != RVSIP_REG_CLIENT_STATE_UNDEFINED && eState != RVSIP_REG_CLIENT_STATE_TERMINATED) {
            RvSipRegClientTerminate(mHRegClient);
        }
        mHRegClient=0;
    }

    stopSubsCallLegPrivate();

    mIsDeregistering=false;
    mIsInRegTimer=false;
    mIsRegistering=false;

    mServiceRoute.clear();
    mPath.clear();

    return RV_OK;
}

RvStatus SipChannel::connectSession()
{
    ENTER();

    RVSIPLOCK(mLocker);

    stopCallSession();

    RvStatus rv = RV_OK;

    mRole=ORIGINATE;

    rv = RvSipCallLegMgrCreateCallLeg(mHMgr, (RvSipAppCallLegHandle)this, &mHCallLeg, 0, 0xFF);
    if(rv < 0) {
        return rv;
    }

    if(mUseHostInCallID) {
        rv = RvSipCallLegGenerateCallId(mHCallLeg);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.333.000.111: rv=%d\n",__FUNCTION__,rv);
        } else {
            char callID[MAX_LENGTH_OF_URL]="\0";
            RvInt32 actualSize=0;
            std::string domain=getOurDomainStrPrivate();
            rv = RvSipCallLegGetCallId(mHCallLeg,sizeof(callID)-1,callID,&actualSize);
            if(rv<0) {
                RVSIPPRINTF("%s: error 111.333.000.222: rv=%d\n",__FUNCTION__,rv);
            } else {
                if(actualSize+domain.length()+1+1>sizeof(callID)) {
                    RVSIPPRINTF("%s: error 111.333.000.333: %s@%s\n",__FUNCTION__,callID,domain.c_str());
                } else {
                    size_t callID_len=strlen(callID);
                    snprintf(callID+callID_len,sizeof(callID)-callID_len,"@%s",domain.c_str());
                    callID[sizeof(callID)-1] = '\0';
                    rv = RvSipCallLegSetCallId (mHCallLeg,callID);
                    if(rv<0) {
                        RVSIPPRINTF("%s: error 111.333.000.444: rv=%d, callID=%s\n",__FUNCTION__,rv, callID);
                    }
                }
            }
        }
    }

    setRefreshTimer(true);

    std::string strFrom;
    std::string strTo;

    rv = sessionSetRemoteCallLegDataPrivate(strTo);
    if(rv<0) {
        RETCODE(rv);
    }

    rv = sessionSetLocalCallLegDataPrivate(strFrom);
    if(rv<0) {
        RETCODE(rv);
    }

    /*-----------------------------------------------------------------------
      Connecting ...
      -------------------------------------------------------------------------*/

    RvSipPartyHeaderHandle  hFrom          = NULL;
    RvSipPartyHeaderHandle  hTo            = NULL;

    rv = RvSipCallLegGetNewPartyHeaderHandle ( mHCallLeg, &hFrom );
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.333: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    rv = RvSipPartyHeaderParse ( hFrom, RV_FALSE, (RvChar*)(strFrom.c_str()) );
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.444: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    // now, set the previously corrected hFrom header to the callLeg.
    rv = RvSipCallLegSetFromHeader(mHCallLeg,hFrom);
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.555: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    rv = RvSipCallLegGetNewPartyHeaderHandle ( mHCallLeg, &hTo );
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.666: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    rv = RvSipPartyHeaderParse ( hTo, RV_TRUE, (RvChar*)(strTo.c_str()) );
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.777: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    // set callLeg's ToHeader to the constructed one.
    rv = RvSipCallLegSetToHeader(mHCallLeg,hTo);
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.888: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
    }

    if(rv>=0) {
        rv = addSdp();
    }

    setInviting(true);

    if(rv>=0) {
        rv = RvSipCallLegConnect(mHCallLeg);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.333.999: from %s to %s: rv=%d\n",__FUNCTION__,strFrom.c_str(),strTo.c_str(),rv);
        }
    }

    RETCODE(rv);
}

RvStatus SipChannel::disconnectSession()
{
    ENTER();

    RvStatus rv = RV_OK;
    RVSIPLOCK(mLocker);

    rv = disconnectPrivate();

    RETCODE(rv);	
}

void SipChannel::subsUnsubscribed(RvSipSubsHandle hSubs){
    RVSIPLOCK(mLocker);
    if(isRegSubs(hSubs)) {
        deregisterOnUnSubsPrivate();
    }
}

void SipChannel::subsTerminated(RvSipSubsHandle hSubs){
    RVSIPLOCK(mLocker);
    if(isRegSubs(hSubs)){
        mHImsSubs = (RvSipSubsHandle)0;
        deregisterOnUnSubsPrivate();
    }
}

void SipChannel::stopAllTimers() {

    RVSIPLOCK(mLocker);
    clearRefreshTimerPrivate();

    prackTimerStop();
    callTimerStopPrivate();
    callTimerMediaStopPrivate();
    byeAcceptTimerMediaStopPrivate();
    ringTimerStopPrivate();
}

void SipChannel::clearBasicData() {
    RVSIPLOCK(mLocker);
    mRole = TERMINATE;
    mIsInviting =false;
    mIsConnected = false;
    mReadyToAccept = false;
    mTipDone = false;
    mTipStatus.reset();
}

RvStatus SipChannel::stopCallSession(bool bGraceful) {

    RVSIPLOCK(mLocker);

    if(bGraceful){
        if(mHCallTimer){
            callTimerStopPrivate();
            callTimerExpirationEventCB();
        }else
            mCallTime = CALL_STOP_GRACEFUL_TIME;
        return RV_OK;
    }

    stopAllTimers();

    if(mMediaHandler) {
        mMediaHandler->media_stop();
    }

    if(mHByeTranscPending) {
        RvSipTransactionState eState=RVSIP_TRANSC_STATE_UNDEFINED;
        RvSipTransactionGetCurrentState(mHByeTranscPending,&eState);
        if(eState != RVSIP_TRANSC_STATE_UNDEFINED && eState != RVSIP_TRANSC_STATE_TERMINATED) {
            RvSipCallLegByeAccept(mHCallLeg,mHByeTranscPending);
            RvSipTransactionTerminate(mHByeTranscPending);
        }
        mHByeTranscPending=0;
    }
    
    if(mHCallLeg) {
        if(mHReInvite){
            RvSipCallLegReInviteTerminate(mHCallLeg,mHReInvite);
            mHReInvite = 0;
        }
        RvSipCallLegState    eState = RVSIP_CALL_LEG_STATE_UNDEFINED;
        RvSipCallLegGetCurrentState (mHCallLeg,&eState);
        if(eState != RVSIP_CALL_LEG_STATE_TERMINATED && 
                eState != RVSIP_CALL_LEG_STATE_DISCONNECTED && 
                eState != RVSIP_CALL_LEG_STATE_DISCONNECTING &&
                eState != RVSIP_CALL_LEG_STATE_UNDEFINED && 
                eState != RVSIP_CALL_LEG_STATE_CANCELLING &&
                eState!=RVSIP_CALL_LEG_STATE_IDLE) {
            disconnectPrivate();
        }
        mHCallLeg = 0;
    }

    clearBasicData();

    return RV_OK;
}

void SipChannel::setMediaInterface(MediaInterface *mediaHandler) {
    RVSIPLOCK(mLocker);
    mMediaHandler=mediaHandler;
}
void SipChannel::clearMediaInterface() {
    RVSIPLOCK(mLocker);
    mMediaHandler=NULL;
}

void SipChannel::setAuth(const std::string &authName, const std::string &authRealm, const std::string &authPasswd, const char* OP) {

    RVSIPLOCK(mLocker);

    if(!useIMSIPrivate()){
        mAuthName=authName;
        mAuthRealm=authRealm;
    }
    mAuthPasswd=authPasswd;
    if(OP)
        RvSipUtils::HexStringToKey(OP,mOP,sizeof(mOP));

}

void SipChannel::getAuth(std::string &authName, std::string &authRealm, std::string &authPasswd) const {

    RVSIPLOCK(mLocker);

    authName=getAuthNamePrivate();
    authRealm=getAuthRealmPrivate();
    authPasswd=mAuthPasswd;
}

void SipChannel::setRemote(const std::string& userName, 
        const std::string& userNumberContext,
        const std::string& displayName,
        const RvSipTransportAddr& addr,
        const std::string& hostName,
        const std::string& remoteDomain,
        unsigned short remoteDomainPort,
        bool telUriRemote)
{
    RVSIPLOCK(mLocker);

    if(remoteDomainPort) mRemoteDomainPort=remoteDomainPort;
    mUserNameRemote = userName;
    mUserNumberContextRemote = userNumberContext;
    mDisplayNameRemote = displayName;
    mHostNameRemote = hostName;
    fixIpv6Address(mHostNameRemote);
    mRemoteDomain = remoteDomain;
    fixIpv6Address(mRemoteDomain);
    mAddressRemote = addr;
    fixIpv6Address(mAddressRemote.strIP,sizeof(mAddressRemote.strIP));
    mAddressRemote.eTransportType = mLocalAddress.eTransportType;
    mTelUriRemote = telUriRemote;

    fixUserNamePrivate(mUserNameRemote);
    fixUserNamePrivate(mUserNumberContextRemote);
    fixUserNamePrivate(mDisplayNameRemote);
    fixUserNamePrivate(mRemoteDomain);
}

bool SipChannel::isTCP() const {
    RVSIPLOCK(mLocker);
    return mTransport==RVSIP_TRANSPORT_TCP;
}

RvStatus SipChannel::setRegistration(const std::string& regName, 
        uint32_t expires,
        uint32_t reregSecsBeforeExp,
        uint32_t regRetries,
        bool refreshRegistration) {

    RVSIPLOCK(mLocker);

    mRegName = regName;
    mRegRetries=regRetries;
    mRefreshRegistration = refreshRegistration;
    if(expires) {
        mRegExpiresTime=expires;
        if(mRefreshRegistration) {
            if(reregSecsBeforeExp) { 
                mReregSecsBeforeExpTime=reregSecsBeforeExp;
            } else if(expires>DEFAULT_TIME_BEFORE_REREGISTRATION * 2) {
                mReregSecsBeforeExpTime = DEFAULT_TIME_BEFORE_REREGISTRATION;
            } else {
                mReregSecsBeforeExpTime=expires/2;
            }
        }
    }

    return RV_OK;
}
RvStatus SipChannel::setRegistrar(const std::string& regServerName, 
        const std::string& regServerIP, 
        const std::string& regDomain,
        unsigned short regServerPort,
        bool useRegistrarDiscovery) {

    RVSIPLOCK(mLocker);

    mRegServerName = regServerName;
    fixIpv6Address(mRegServerName);
    mRegServerIP = regServerIP;
    fixIpv6Address(mRegServerIP);
    mRegDomain = regDomain;
    fixIpv6Address(mRegDomain);
    mRegServerPort=regServerPort;
    mUseRegistrarDiscovery=useRegistrarDiscovery;

    return RV_OK;
}
RvStatus SipChannel::setProxy(const std::string& ProxyName, 
        const std::string& ProxyIP, 
        const std::string& ProxyDomain,
        unsigned short ProxyPort, 
        bool useProxyDiscovery) {

    RVSIPLOCK(mLocker);

    mProxyName = ProxyName;
    fixIpv6Address(mProxyName);
    mProxyIP = ProxyIP;
    fixIpv6Address(mProxyIP);
    mProxyDomain = ProxyDomain;
    fixIpv6Address(mProxyDomain);
    mProxyPort = ProxyPort;
    mUseProxyDiscovery =useProxyDiscovery;

    return RV_OK;
}

void SipChannel::setExtraContacts(const std::vector<std::string> &ecs) {

    RVSIPLOCK(mLocker);

    mExtraContacts=ecs;
}

void SipChannel::clearExtraContacts() {

    RVSIPLOCK(mLocker);

    mExtraContacts.clear();
}

RvStatus SipChannel::setRefreshTimer(bool isOriginate) {

    ENTER();
    RVSIPLOCK(mLocker);

    RvStatus rv = RV_OK;
    mRefreshed = false;

    mRefresherPref=RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_DONT_CARE;


    if(mRefresher!=RVSIP_SESSION_EXPIRES_REFRESHER_NONE && !mRefresherStarted) {

        if(mHCallLeg) {
            if(isOriginate) {
                if(mRefresher==RVSIP_SESSION_EXPIRES_REFRESHER_UAC) {
                    mRefresherPref=RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
                } else if(mRefresher==RVSIP_SESSION_EXPIRES_REFRESHER_UAS) {
                    mRefresherPref=RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
                }
            } else {
                if(mRefresher==RVSIP_SESSION_EXPIRES_REFRESHER_UAS) {
                    mRefresherPref=RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
                } else if(mRefresher==RVSIP_SESSION_EXPIRES_REFRESHER_UAC) {
                    mRefresherPref=RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
                }
            }
            rv = RvSipCallLegSessionTimerSetPreferenceParams(mHCallLeg,
                    mSessionExpiresTime,
                    90,
                    mRefresherPref);

            if(rv>=0) mRefresherStarted=true;
            RVSIPPRINTF("%s: cl=0x%x, orig=%d, expiration=%d, type=%d, rv=%d\n",__FUNCTION__,
                    (int)mHCallLeg,(int)isOriginate,(int)mSessionExpiresTime,(int)mRefresherPref,(int)rv);
        }
    }

    RETCODE(rv);
}

void SipChannel::setCallLeg(RvSipCallLegHandle cl) {

    RVSIPLOCK(mLocker);

    mHCallLeg = cl;
}

void SipChannel::setReinviteHandle(RvSipCallLegInviteHandle rh) {

    RVSIPLOCK(mLocker);

    mHReInvite = rh;
}

RvSipCallLegState SipChannel::getImmediateCallLegState() const {

    RVSIPLOCK(mLocker);

    RvSipCallLegState    cleState = RVSIP_CALL_LEG_STATE_UNDEFINED;
    if(mHCallLeg) {
        RvSipCallLegGetCurrentState (mHCallLeg,&cleState);
    }
    return cleState;
}

/*===================================================================================*/

RvStatus SipChannel::callLegSessionTimerRefreshAlertEv(RvSipCallLegHandle hCallLeg)
{
    RVSIPLOCK(mLocker);

    RvStatus status = RV_OK;

    if(mHCallLeg && mHCallLeg==hCallLeg) {

        RvSipTranscHandle                              hTransc=(RvSipTranscHandle)0;
        RvInt32 sessionExpires = 90;
        RvInt32 minSE = 90;

        status = RvSipCallLegSessionTimerGetSessionExpiresValue(hCallLeg,&sessionExpires);
        if(status<0) {
            return status;
        }

        status = RvSipCallLegSessionTimerGetMinSEValue(hCallLeg,&minSE);
        if(status<0) {
            return status;
        }
        if(!mRefreshed){
            if(getSMType() == CFT_TIP){
                if(isOriginate())
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
                else
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
            }else{
                RvSipSessionExpiresRefresherType eRefresher = RVSIP_SESSION_EXPIRES_REFRESHER_UAC;
                status = RvSipCallLegSessionTimerGetRefresherType(hCallLeg, &eRefresher);

                if(RVSIP_SESSION_EXPIRES_REFRESHER_UAC == eRefresher){
                    if(isOriginate())
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;
                    else
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;

                }else if(RVSIP_SESSION_EXPIRES_REFRESHER_UAS == eRefresher){
                    if(isOriginate())
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_REMOTE;
                    else
                        mRefresherPref = RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL;

                }
                //TODO
                RVSIPPRINTF("%s: cl=0x%x: se=%d, min=%d, mrt=%d, rt=%d, pref=%d\n",__FUNCTION__,
                        (int)hCallLeg,(int)sessionExpires,(int)minSE,(int)mRefresher,(int)eRefresher,(int)mRefresherPref);
            }
            mRefreshed = true;
        }



        if(mRefresherPref == RVSIP_CALL_LEG_SESSION_TIMER_REFRESHER_LOCAL) {
            /*Sends a Refresh using UPDATE.*/
            RvChar method[33];
            strcpy(method,"UPDATE");
            status = RvSipCallLegTranscSessionTimerGeneralRefresh (mHCallLeg,
                    method,
                    sessionExpires,
                    minSE,
                    mRefresherPref,
                    &hTransc);
            RVSIPPRINTF("%s: status0=%d\n",__FUNCTION__,status);
            if(status<0) {
                return status;
            } else {
                CallStateNotifier* notifier = mCallStateNotifier;
                if(notifier) {
                    notifier->refreshCompleted();
                }
            }
        }
    }
    return status;
}

RvStatus SipChannel::callLegSessionTimerNotificationEv(RvSipCallLegHandle                         hCallLeg,
        RvSipCallLegSessionTimerNotificationReason eReason) {

    RVSIPLOCK(mLocker);

    RvStatus rv = RV_OK;

    RVSIPPRINTF("%s: cl=0x%x, reason=%d\n",__FUNCTION__,(int)hCallLeg,(int)eReason);

    if(hCallLeg == mHCallLeg) {

        if(eReason == RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_SESSION_EXPIRES) {
            rv = this->disconnectSession();
        } else  if(eReason == RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_422_RECEIVED) {
            //TODO
            //Not supported. See RVSIP_CALL_LEG_SESSION_TIMER_NOTIFY_REASON_422_RECEIVED description in the Radvision docs
            //for implementation guideline.
        }
    }

    return rv;
}

void SipChannel::regSuccessNotify() {

    RVSIPLOCK(mLocker);

    if(mIsDeregistering) {
        mServiceRoute.clear();
        mPath.clear();
    } else {
        if(mHRegClient) {
            mServiceRoute.clear();
            mPath.clear();
            RvSipMsgHandle hMsg = (RvSipMsgHandle)0;
            if(RvSipRegClientGetReceivedMsg(mHRegClient,&hMsg)>=0 && hMsg) {

                { //Path handling:
                    RvSipRouteHopHeaderHandle    hRouteHop;
                    RvSipHeaderListElemHandle    listElem;
                    RvSipRouteHopHeaderType      routeHopType;
                    RvStatus                     rv;
                    std::string                  str;

                    // Get value of Path header.
                    // get the first path header
                    hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                            RVSIP_HEADERTYPE_ROUTE_HOP, 
                            RVSIP_FIRST_HEADER, 
                            RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
                    while (hRouteHop != NULL) {

                        routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);

                        // if the route hop is a PATH, save the path
                        if(routeHopType == RVSIP_ROUTE_HOP_PATH_HEADER) {

                            RvSipAddressHandle hSipAddr = RvSipRouteHopHeaderGetAddrSpec(hRouteHop);

                            if(hSipAddr) {

                                HRPOOL   hPool        = mAppPool;
                                HPAGE    tempPage     = NULL;
                                RvUint32 encodedLen   = 0;

                                rv = RvSipAddrEncode(hSipAddr,
                                        hPool,
                                        &tempPage,
                                        &encodedLen);

                                if(rv>=0 && encodedLen > 0) {

                                    char *pStr = new char[encodedLen+1];
                                    // Copy the encoded value to our buffer
                                    RPOOL_CopyToExternal(hPool, tempPage, 0, pStr, encodedLen);
                                    pStr[encodedLen]=0;

                                    std::string route=std::string(pStr);
                                    if(!route.empty()) {
                                        if(route[0]!='<') {
                                            route=std::string("<")+route+std::string(">");
                                        }
                                        if( !str.empty() ) str += std::string(",");
                                        str += route;
                                    }

                                    delete [] pStr;
                                }

                                if(tempPage) RPOOL_FreePage(hPool, tempPage);
                            }
                        }
                        // get the next route hop
                        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                                RVSIP_HEADERTYPE_ROUTE_HOP, 
                                RVSIP_NEXT_HEADER, 
                                RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
                    }
                    //str=std::string("<sip:1.2.3.4:5067;lr;transport=TCP>, <sip:11.12.13.14:15067;lr;transport=SCTP>");
                    mPath=str;
                }

                { //Service-Route handling:
                    RvSipRouteHopHeaderHandle    hRouteHop;
                    RvSipHeaderListElemHandle    listElem;
                    RvSipRouteHopHeaderType      routeHopType;
                    RvStatus                     rv;
                    std::string                  str;

                    // Get value of Service-Route header.
                    // get the first record route header
                    hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                            RVSIP_HEADERTYPE_ROUTE_HOP, 
                            RVSIP_FIRST_HEADER, 
                            RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
                    while (hRouteHop != NULL) {

                        routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);

                        // if the route hop is a service-route, save the route for sending out future requests
                        if(routeHopType == RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER) {

                            RvSipAddressHandle hSipAddr = RvSipRouteHopHeaderGetAddrSpec(hRouteHop);

                            if(hSipAddr) {

                                HRPOOL   hPool        = mAppPool;
                                HPAGE    tempPage     = NULL;
                                RvUint32 encodedLen   = 0;

                                rv = RvSipAddrEncode(hSipAddr,
                                        hPool,
                                        &tempPage,
                                        &encodedLen);

                                if(rv>=0 && encodedLen > 0) {

                                    char *pStr = new char[encodedLen+1];
                                    // Copy the encoded value to our buffer
                                    RPOOL_CopyToExternal(hPool, tempPage, 0, pStr, encodedLen);
                                    pStr[encodedLen]=0;

                                    std::string route=std::string(pStr);
                                    if(!route.empty()) {
                                        if(route[0]!='<') {
                                            route=std::string("<")+route+std::string(">");
                                        }
                                        if( !str.empty() ) str += std::string(",");
                                        str += route;
                                    }

                                    delete [] pStr;
                                }

                                if(tempPage) RPOOL_FreePage(hPool, tempPage);
                            }
                        }
                        // get the next route hop
                        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                                RVSIP_HEADERTYPE_ROUTE_HOP, 
                                RVSIP_NEXT_HEADER, 
                                RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
                    }
                    //str=std::string("<sip:1.2.3.4:5067;lr;transport=TCP>, <sip:11.12.13.14:15067;lr;transport=SCTP>");
                    mServiceRoute=str;
                }

                if(!mPath.empty()) mPath = std::string("Route: ")+mPath;
                if(!mServiceRoute.empty()) mServiceRoute = std::string("Route: ")+mServiceRoute;

                {//Expires handling
                    uint32_t expiresFromMsg = 0;
                    RvSipHeaderListElemHandle   hListElem = (RvSipHeaderListElemHandle)0;
                    RvSipExpiresHeaderHandle hExpiresHeader = (RvSipExpiresHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                            RVSIP_HEADERTYPE_EXPIRES,
                            RVSIP_FIRST_HEADER,
                            &hListElem);
                    while(hExpiresHeader) {
                        RvUint32 efm=0;
                        RvSipExpiresHeaderGetDeltaSeconds(hExpiresHeader,&efm);
                        if((efm>0) && ((expiresFromMsg==0) || (efm<expiresFromMsg))) expiresFromMsg=efm;
                        hExpiresHeader = (RvSipExpiresHeaderHandle)RvSipMsgGetHeaderByType( hMsg,
                                RVSIP_HEADERTYPE_EXPIRES, 
                                RVSIP_NEXT_HEADER, 
                                &hListElem );
                    }

                    hListElem = (RvSipHeaderListElemHandle)0;
                    RvSipContactHeaderHandle hContactHeader = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                            RVSIP_HEADERTYPE_CONTACT,
                            RVSIP_FIRST_HEADER,
                            &hListElem);
                    while(hContactHeader) {
                        hExpiresHeader = RvSipContactHeaderGetExpires(hContactHeader);
                        if(hExpiresHeader) {
                            RvUint32 efm=0;
                            RvSipExpiresHeaderGetDeltaSeconds(hExpiresHeader,&efm);
                            if((efm>0) && ((expiresFromMsg==0) || (efm<expiresFromMsg))) expiresFromMsg=efm;
                        }
                        hContactHeader = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType( hMsg,
                                RVSIP_HEADERTYPE_CONTACT, 
                                RVSIP_NEXT_HEADER, 
                                &hListElem );
                    }

                    if(expiresFromMsg>0) {
                        mRegExpiresTime=expiresFromMsg;
                    }
                }

                {//Reregistration handling
                    if(mRefreshRegistration && mRegExpiresTime>0 && mHMidMgr) {
                        uint32_t interval=0;
                        if(mRegExpiresTime>mReregSecsBeforeExpTime) {
                            interval=mRegExpiresTime-mReregSecsBeforeExpTime;
                        } else if(mRegExpiresTime>=MIN_REREGISTRATION_SESSION_TIME) {
                            interval=mRegExpiresTime/2;
                        }
                        if(interval>0) {
                            interval = interval*1000 - SipChannel::getTimeVariationPrivate();
                            ExpirationCB       cb = regTimerExpires;

                            mHRegTimer = mTimers.getTimer();
                            mHRegTimer->load(this,cb,interval,(void**)&mHRegClient,(void**)&mHRegTimer);
                        }
                    }
                }
            }
        }
        if(useSubOnRegPrivate()) subscribeRegEventPrivate();
    }

    if(mCallStateNotifier) mCallStateNotifier->regSuccess();

    mIsDeregistering=false;
}

void SipChannel::regFailureNotify() {

    RVSIPLOCK(mLocker);

    if(mCallStateNotifier) mCallStateNotifier->regFailure(mRegExpiresTime,mRegRetries);

    if(mRegRetries>0 && !mIsDeregistering) {
        mRegRetries--;
        RvStatus rv = RV_OK;
        if(mHRegClient) {
            rv = RvSipRegClientRegister(mHRegClient);
        } else {
            rv = startRegistrationProcessPrivate(mRegExpiresTime);
        }
        RVSIPPRINTF("%s: reregister: %d\n",__FUNCTION__,rv);
    } else {
        mServiceRoute.clear();
        mPath.clear();
        mIsDeregistering=false;
    }
}

void SipChannel::MediaMethodCompletionAsyncNotification(bool generate, bool initiatorParty, bool success) {

    RVSIPLOCK(mLocker);

    if(!generate) {
        if(!initiatorParty) {
            setByeAcceptTimerMediaPrivate();
        } else {
            setCallTimerMediaPrivate();
        }
    }
}

int32_t SipChannel::setNonceCount(const std::string &realm, const std::string &nonce, int32_t nonceCount) {

    RVSIPLOCK(mLocker);

    int32_t ret = nonceCount;

    NonceMapKeyType key(realm,nonce);

    NonceMapType::iterator iter=mNonceMap.find(key);
    if(iter != mNonceMap.end()) {
        iter->second+=1;
        ret=iter->second;
    } else {
        NonceMapType::value_type val(key,nonceCount);
        mNonceMap.insert(val);
    }

    return ret;
}

void SipChannel::setCallTimer() {

    ENTER();

    RVSIPLOCK(mLocker);

    if(mHCallTimer) {
        mHCallTimer->unload();
        mHCallTimer=0;
    }

    uint32_t interval=mCallTime;

    if(interval>0) {
        if(isTerminatePrivate()) interval += POST_CALL_TERMINATE_TIMEOUT;
        interval = interval*1000 - SipChannel::getTimeVariationPrivate();
        ExpirationCB       cb = callTimerExpires;

        mHCallTimer = mTimers.getTimer();
        mHCallTimer->load(this,cb,interval,(void**)&mHCallLeg,(void**)&mHCallTimer);
    }

    RET;
}

void SipChannel::setRingTimer() {

    ENTER();

    RVSIPLOCK(mLocker);

    if(mHRingTimer) {
        mHRingTimer->unload();
        mHRingTimer=0;
    }

    uint32_t interval=mRingTime;

    if(interval<1) {

        if(mReadyToAccept) {
            acceptPrivate();
        }

    } else {

        ExpirationCB       cb = ringTimerExpires;
        interval = interval*1000 - SipChannel::getTimeVariationPrivate();

        mHRingTimer = mTimers.getTimer();
        mHRingTimer->load(this,cb,interval,(void**)&mHCallLeg,(void**)&mHRingTimer);
    }

    RET;
}

RvStatus SipChannel::setReadyToAccept(bool value,bool bAccept) {
    RVSIPLOCK(mLocker);
    mReadyToAccept=value;
    if(mReadyToAccept && bAccept) {
        if(!mHRingTimer) {
            return acceptPrivate();
        }
    }
    return RV_OK;
}

void SipChannel::setPrackTimer() {

    ENTER();

    RVSIPLOCK(mLocker);

    if(mHPrackTimer) {
        mHPrackTimer->unload();
        mHPrackTimer=0;
    }

    uint32_t interval=mPrackTime;

    if(interval>0) {

        ExpirationCB       cb = prackTimerExpires;
        interval = interval*1000 - SipChannel::getTimeVariationPrivate();

        mHPrackTimer = mTimers.getTimer();
        mHPrackTimer->load(this,cb,interval,(void**)mHCallLeg,(void**)&mHPrackTimer);
    }

    RET;
}

void SipChannel::prackTimerStop() {

    ENTER();

    RVSIPLOCK(mLocker);

    if(mHPrackTimer) {
        mHPrackTimer->unload();
        mHPrackTimer=0;
    }

    RET;
}

void SipChannel::callCompletionNotification() {
    RVSIPLOCK(mLocker);
    setPostCallTimerPrivate();
}

RvStatus SipChannel::setOutboundCallLegMsg() {
    RVSIPLOCK(mLocker);
    RvStatus rv = addPreferredIdentity();
    rv |= sessionSetCallLegPrivacy();
    rv |= sessionSetCallLegAllowPrivate();
    return rv;
}

RvStatus SipChannel::addDynamicRoutes(RvSipCallLegHandle hCallLeg) {

    // Add dynamic routes
    RvStatus rv = RV_OK;
    RVSIPLOCK(mLocker);

    if(!hCallLeg) hCallLeg = mHCallLeg;

    std::string route;
    std::string serviceRoute;

    rv = getDynamicRoutesPrivate(route,serviceRoute);

    if(rv == RV_OK) {

        const char* r = route.c_str();

        const char* sr = serviceRoute.c_str();

        if(*r || *sr) {
            rv = RvSipCallLegRouteListAdd(hCallLeg, (RvChar*)(r), (RvChar*)sr );
            if(rv<0) {
                RVSIPPRINTF("%s: error 111.333.666: rv=%d: route=%s, port=%d\n",__FUNCTION__,rv,route.c_str(), getNextHopPortPrivate());
                RETCODE(rv);
            }
        }
    }

    return rv;
}

RvStatus SipChannel::addDynamicRoutes(RvSipMsgHandle hMsg) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;

    if(hMsg) {

        if(!containsRoutesPrivate(hMsg)) {

            std::string route;
            std::string serviceRoute;

            rv = getDynamicRoutesPrivate(route,serviceRoute);

            if(rv == RV_OK) {

                const char* r = route.c_str();

                if(*r) rv=addDynamicRoutes(hMsg,r);
                if(rv == RV_OK) {
                    const char* sr = serviceRoute.c_str();
                    if(*sr) rv=addDynamicRoutes(hMsg,sr);
                }
            }
        }
    }

    return rv;
}

RvStatus SipChannel::addDynamicRoutes(RvSipMsgHandle hMsg, const char* s) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;

    if(hMsg && s && *s) {

        RvChar* ss=const_cast<RvChar*>(s);

        RvSipRouteHopHeaderHandle hRouteHeader = (RvSipRouteHopHeaderHandle)0;

        rv = RvSipRouteHopHeaderConstructInMsg(hMsg, RVSIP_FIRST_HEADER, &hRouteHeader);
        if(rv != RV_OK) {
            RVSIPPRINTF("%s:%d failed rv =%d\n", __FUNCTION__, __LINE__, rv);
        } else {
            rv = RvSipRouteHopHeaderSetHeaderType(hRouteHeader, RVSIP_ROUTE_HOP_ROUTE_HEADER);
            if(rv != RV_OK) {
                RVSIPPRINTF("%s:%d failed rv =%d\n", __FUNCTION__, __LINE__, rv);
            } else {
                if(strstr(ss,"Route:")||strstr(ss,"route:")||strstr(ss,"ROUTE:")) {
                    rv = RvSipRouteHopHeaderParse( hRouteHeader, ss );
                } else {
                    rv = RvSipRouteHopHeaderParseValue( hRouteHeader, ss );
                }
            }
        }
    }

    return rv;
}

RvStatus SipChannel::addPreferredIdentity(void *msg) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;
    if(!useMobilePrivate() || useIMSIPrivate())
        return rv;

    if(mPreferredIdentity.empty()){
        getPreferredIdentityPrivate();
    }
    if(!mPreferredIdentity.empty()) {

        RvSipMsgHandle            hMsg=(RvSipMsgHandle)msg;
        if(!hMsg)
            rv = RvSipCallLegGetOutboundMsg(mHCallLeg,&hMsg);

        if(rv>=0 && hMsg) {
            RvSipPUriHeaderHandle hPUriHeader = NULL;
            rv = RvSipPUriHeaderConstructInMsg(hMsg, RV_TRUE, &hPUriHeader);
            if(rv>=0 && hPUriHeader) {
                rv = RvSipPUriHeaderSetPHeaderType(hPUriHeader, RVSIP_P_URI_PREFERRED_IDENTITY_HEADER);
                if(rv >= 0) {
                    RvChar value[MAX_LENGTH_OF_USER_NAME];
                    strncpy(value,mPreferredIdentity.c_str(),sizeof(value)-1);
                    rv = RvSipPUriHeaderParseValue(hPUriHeader, value);
                }
            }
        }
    }

    return rv;
}

RvStatus SipChannel::addNetworkAccessInfoToMsg(void *msg) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;
    if(!useMobilePrivate())
        return rv;
    RvSipMsgHandle            hMsg=(RvSipMsgHandle)(msg);

    if(hMsg){
        char buf[1024];
        switch(mNetworkAccessType){
            case RAN_TYPE_3GPP_GERAN:
                snprintf(buf,1024,"3GPP-GERAN; cgi-3gpp=%s",mNetworkAccessCellID.c_str());
                break;
            case RAN_TYPE_3GPP_UTRAN_TDD:
                snprintf(buf,1024,"3GPP-UTRAN-TDD; utran-cell-id-3gpp=%s",mNetworkAccessCellID.c_str());
                break;
            case RAN_TYPE_3GPP_UTRAN_FDD:
                snprintf(buf,1024,"3GPP-UTRAN-FDD; utran-cell-id-3gpp=%s",mNetworkAccessCellID.c_str());
                break;
        }
        RvSipPAccessNetworkInfoHeaderHandle hPAccessNetworkInfoHeader = NULL;
        rv = RvSipPAccessNetworkInfoHeaderConstructInMsg(hMsg,RV_TRUE,&hPAccessNetworkInfoHeader);
        if(rv>=0 && hPAccessNetworkInfoHeader) {
            rv = RvSipPAccessNetworkInfoHeaderParseValue(hPAccessNetworkInfoHeader,buf);
        }
    }

    return rv;
}
RvStatus SipChannel::addUserAgentInfo(void *msg) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;
    if(!isTipDevice())//only set for TIP
        return rv;
    RvSipMsgHandle            hMsg=(RvSipMsgHandle)(msg);

    if(hMsg){
        RvSipOtherHeaderHandle hUserAgentHeader = NULL;
        rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_TRUE, &hUserAgentHeader);
        if(rv>=0 && hUserAgentHeader) {
            RvChar name[16];
            strcpy(name,"User-Agent");
            rv = RvSipOtherHeaderSetName(hUserAgentHeader, name);
            RvChar value[128];
            snprintf(value,sizeof(value),"Cisco-Telepresence-#%d/1.0",mTipDeviceType);
            rv = RvSipOtherHeaderSetValue(hUserAgentHeader, value);
        }
    }

    return rv;
}

RvStatus SipChannel::sessionSetCallLegPrivacy(void *msg) {

    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;
    std::string value="";

    if(mPrivacyOptions) {

        RvSipMsgHandle            hMsg=(RvSipMsgHandle)msg;
        if(!msg)
            rv = RvSipCallLegGetOutboundMsg(mHCallLeg,&hMsg);
        if(rv>=0 && hMsg) {
            RvSipOtherHeaderHandle hOtherHeader = NULL;
            rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_TRUE, &hOtherHeader);
            if(rv>=0 && hOtherHeader) {
                RvChar name[11];
                strcpy(name,"Privacy");
                rv = RvSipOtherHeaderSetName(hOtherHeader, name);
                if(rv >= 0) {
                    if(mPrivacyOptions & RVSIP_PRIVACY_HEADER_OPTION_NONE) value="none";
                    else {
                        if(mPrivacyOptions & RVSIP_PRIVACY_HEADER_OPTION_HEADER) value="header";
                        if(mPrivacyOptions & RVSIP_PRIVACY_HEADER_OPTION_SESSION) { 
                            if(!value.empty())
                                value+=";";
                            value+="session";
                        }
                        if(mPrivacyOptions & RVSIP_PRIVACY_HEADER_OPTION_USER) { 
                            if(!value.empty())
                                value+=";";
                            value+="user";
                        }
                        if(!value.empty()) {
                            if(mPrivacyOptions & RVSIP_PRIVACY_HEADER_OPTION_CRITICAL) { 
                                value+=";critical";
                            }
                        }
                    }
                    rv = RvSipOtherHeaderSetValue(hOtherHeader, (RvChar *)value.c_str());
                }
            }
        }
    }

    return rv;
}

RvStatus SipChannel::sessionSetRegClientAllow() {
    RVSIPLOCK(mLocker);

    RvStatus rv = RV_OK;

    RvSipMsgHandle            hMsg=(RvSipMsgHandle)0;
    rv = RvSipRegClientGetOutboundMsg(mHRegClient,&hMsg);
    if(rv>=0 && hMsg) {
        rv = sessionSetAllowPrivate(hMsg);
    }

    return rv;
}

RvStatus SipChannel::addSdp() {
    RVSIPLOCK(mLocker);
    RvStatus rv = RV_OK;
    if( mMediaHandler && mHCallLeg) {
        rv = mMediaHandler->media_beforeConnect(mHCallLeg);
    }
    return rv;
}

void SipChannel::TipStateUpdate(TIP_NS::TIPMediaType type,TIP_NS::TIPNegoStatus status,EncTipHandle TipHandler,void *arg){
    RVSIPLOCK(mLocker);

    mTipStatus.setStatus(type,status,arg);
    if(TipDone())
        return;
    if(mTipStatus.NegoDone()){
        TipDone(true);
        if(!isTerminatePrivate()){
            RvSipCallLegState eState = RVSIP_CALL_LEG_STATE_UNDEFINED;
            RvSipCallLegGetCurrentState(mHCallLeg,&eState);
            if(eState == RVSIP_CALL_LEG_STATE_CONNECTED || eState == RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED)
                RvSipCallLegAck(mHCallLeg);
        }else{
            if(TipStarted()){
                setConnected(true);
                CallStateNotifier* notifier = mCallStateNotifier;
                if(notifier) 
                    notifier->callAnswered();
                setCallTimer();
            }
        }
        if(TipHandler && mTipStatus.toSendReInvite()){
            RvStatus rv = RV_OK;
            if(!mHReInvite){
                rv = RvSipCallLegReInviteCreate(mHCallLeg,NULL,&mHReInvite);
                if(RV_OK != rv){
                    RVSIPPRINTF("%s: create reinvite for TIP sigaling failed.\n",__FUNCTION__);
                    RvSipCallLegDisconnect(mHCallLeg);
                    return;
                }
            }
            if(mMediaHandler)
                mMediaHandler->media_beforeAccept(mHCallLeg);
            rv = RvSipCallLegSessionTimerInviteRefresh(mHCallLeg,mHReInvite);
            if(RV_OK != rv){
                RVSIPPRINTF("%s: send reinvite for TIP sigaling failed.\n",__FUNCTION__);
                RvSipCallLegDisconnect(mHCallLeg);
                return;
            }
        }
    }else if(mTipStatus.NegoFail()){
        TipDone(true);
        if(isTerminatePrivate()){
            RvSipCallLegState eState = RVSIP_CALL_LEG_STATE_UNDEFINED;
            RvSipCallLegGetCurrentState(mHCallLeg,&eState);
            if(eState == RVSIP_CALL_LEG_STATE_CONNECTED){
                RvSipCallLegDisconnect(mHCallLeg);
            }
            else{
                RvUint16 code=415;//FIXME 1:is this the right code? 2:how about video fail but audio OK?
                RvSipCallLegReject ( mHCallLeg, code );
            }
        }else{
            RvSipCallLegDisconnect(mHCallLeg);
        }
    }

    return;
}

unsigned int SipChannel::generate_mux_csrc(void) const{
    RVSIPLOCK(mLocker);
    unsigned int sci;//sampling clock id
    unsigned char out_pos;
    unsigned char transmit_pos;
    unsigned char recv_pos;
    unsigned int ret;

    sci = VoIPUtils::MakeRandomInteger() & 0xFFFFF000;
    out_pos = 0;
    out_pos = (out_pos <<8)&0xF00;
    transmit_pos = 0x1;//center
    transmit_pos = (transmit_pos << 4)&0xF0;
    recv_pos = 0x1;//center
    ret = (sci|out_pos|transmit_pos|recv_pos);
    return ret;
}

bool SipChannel::setImsi(const std::string &imsi,eMNCLen mnc_len){
    RVSIPLOCK(mLocker); 

    if(imsi.empty()){
        mUseMobile = true;
        mUseIMSI = false; 
    }else{
        size_t mnclen = (mnc_len==MNC_LEN_2)?2:3;
        if(imsi.length() <= 3+mnclen) return false;
        mUseMobile = true;
        mUseIMSI = true; 
        mImsi=imsi;
        mMncLen = mnc_len;

        std::string mcc=imsi.substr(0,3);
        std::string mnc=imsi.substr(3,mnclen);

        mAuthRealm = "ims.mnc";
        mAuthRealm += mnc;
        mAuthRealm += ".mcc";
        mAuthRealm += mcc;
        mAuthRealm += ".3gppnetwork.org";

        mAuthName = imsi;
        mAuthName += "@";
        mAuthName += mAuthRealm;

        mIMPI = mAuthName;
        mIMPU = "sip:"+mIMPI;
    }

    return true;
}

bool SipChannel::isRegSubs(RvSipSubsHandle hSubs){
    RVSIPLOCK(mLocker);
    return mHImsSubs && (hSubs == mHImsSubs);
}

void SipChannel::fixIpv6AddressStrPrivate(std::string &str){
    int pos =0;
    int processed = 0;
    int len=str.length();
    std::string tmp;
    if( len > 3 && (str.find(":") != std::string::npos)) {
        if(str[1] == '0'){
            pos = 1;
            while(str[pos+1] == '0' ) pos++;
            if(str[pos+1] && str[pos+1] != ']' && str[pos+1] != ':') pos++;
            str = "[" + str.substr(pos);
        }
        pos = 0;
        while((pos=str.find(":0",processed)) != (int)std::string::npos ){
            int pos_part2 = ++pos;
            while(str[pos_part2+1] == '0' ) pos_part2++;
            if(str[pos_part2+1] && str[pos_part2+1] != ']' && str[pos_part2+1] != ':') pos_part2++ ;

            tmp = str.substr(0,pos) + str.substr(pos_part2);
            str = tmp;
            processed = pos;
        }
    }
}

void SipChannel::fixIpv6Address(std::string& s) {
    fixUserNamePrivate(s);
    const char* ss=s.c_str();
    if(strstr(ss,":") && ss[0]!='[') {
        char sd[MAX_LENGTH_OF_URL];
        size_t len=s.length();
        if(len+2<sizeof(sd)) {
            sd[0]='[';
            memcpy(sd+1,ss,len);
            sd[len+1]=']';
            sd[len+2]=0;
            s = std::string(sd);
        }
    }
    fixIpv6AddressStrPrivate(s);
}

void SipChannel::fixIpv6Address(char* s, int size) {

    //empty symbols:

    int esn=0;
    while(s[esn] && s[esn]==' ') esn++;
    if(esn>0) {
        int len=strlen(s);
        if(esn>len) {
            ///???
        } else if(esn==len) {
            s[0]=0;
        } else {
            int i=0;
            for(i=0;i<=len-esn;i++) {
                s[i]=s[i+esn];
            }
        }
    }

    // []
    const char* ss=s;
    if(strstr(ss,":") && ss[0]!='[') {
        char sd[MAX_LENGTH_OF_URL];
        size_t len=strlen(ss);
        if(len+2<sizeof(sd)) {
            sd[0]='[';
            memcpy(sd+1,ss,len);
            sd[len+1]=']';
            sd[len+2]=0;
            strncpy(s,sd,size);
            s[size-1]=0;
        }
    }
    std::string tmp(s);
    fixIpv6AddressStrPrivate(tmp);
    if(tmp.length() < (size_t)size)
        strcpy(s,tmp.c_str());
}
void SipChannel::setLocalCallLegData(){
    RVSIPLOCK(mLocker);

    std::string tmp;
    sessionSetLocalCallLegDataPrivate(tmp);
}


/******************* Private methods ***************************************/

std::string SipChannel::getOurDomainStrPrivate() const{

    std::string rd;
    char str[MAX_LENGTH_OF_URL];

    if(!mLocalURIDomain.empty()){
        if(!isSecureReqURIPrivate()){
            if(mLocalURIDomainPort>0)
                sprintf(str,"%s:%u",mLocalURIDomain.c_str(),mLocalURIDomainPort);
            else
                sprintf(str,"%s:%u",mLocalURIDomain.c_str(),SIP_DEFAULT_PORT);
        }else
            strcpy(str,mLocalURIDomain.c_str());

    }else if(usePeer2PeerPrivate()){
        if(!isSecureReqURIPrivate()){
            if(mLocalAddress.port>0)
                sprintf(str,"%s:%u",mLocalAddress.strIP,mLocalAddress.port);
            else
                sprintf(str,"%s:%u",mLocalAddress.strIP,SIP_DEFAULT_PORT);
        }else
            strcpy(str,mLocalAddress.strIP);

    }else{ 
        if(!mRegDomain.empty() && !useRegistrarDiscoveryPrivate()){
            if(!isSecureReqURIPrivate()){
                if(mRegServerPort>0)
                    sprintf(str,"%s:%u",mRegDomain.c_str(),mRegServerPort);
                else
                    sprintf(str,"%s:%u",mRegDomain.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mRegDomain.c_str());
        }else if(useRegistrarDiscoveryPrivate()){
            strcpy(str,mRegDomain.c_str());
        }else if(mRegDomain.empty() && !mRegServerName.empty()){
            if(!isSecureReqURIPrivate()){
                if(mRegServerPort>0)
                    sprintf(str,"%s:%u",mRegServerName.c_str(),mRegServerPort);
                else
                    sprintf(str,"%s:%u",mRegServerName.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mRegServerName.c_str());
        }else if(mRegDomain.empty() && !mRegServerIP.empty()){
            if(!isSecureReqURIPrivate()){
                if(mRegServerPort>0)
                    sprintf(str,"%s:%u",mRegServerIP.c_str(),mRegServerPort);
                else
                    sprintf(str,"%s:%u",mRegServerIP.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mRegServerIP.c_str());
        }
    }


    if(!isSecureReqURIPrivate() && !useRegistrarDiscoveryPrivate()) {
        if(getTransportPrivate() == RVSIP_TRANSPORT_TCP) {
            strcat(str,";transport=TCP");
        } else if(getTransportPrivate() == RVSIP_TRANSPORT_UDP) {
            strcat(str,";transport=UDP");
        }
    }
    rd = str;
    return rd;
}

std::string SipChannel::getRemoteDomainStrPrivate() const{

    std::string rd;
    char str[MAX_LENGTH_OF_URL];

    if(!mRemoteDomain.empty() && !useProxyDiscoveryPrivate()){
        if(!isSecureReqURIPrivate()){
            if(mRemoteDomainPort>0)
                sprintf(str,"%s:%u",mRemoteDomain.c_str(),mRemoteDomainPort);
            else
                sprintf(str,"%s:%u",mRemoteDomain.c_str(),SIP_DEFAULT_PORT);
        }else
            strcpy(str,mRemoteDomain.c_str());

    }else if(usePeer2PeerPrivate()){
        if(!isSecureReqURIPrivate()){
            if(mAddressRemote.port > 0)
                sprintf(str,"%s:%u",mAddressRemote.strIP,mAddressRemote.port);
            else
                sprintf(str,"%s:%u",mAddressRemote.strIP,SIP_DEFAULT_PORT);
        }
        else
            strcpy(str,mAddressRemote.strIP);
    }else{
        if(!mProxyDomain.empty() && !useProxyDiscoveryPrivate()){
            if(!isSecureReqURIPrivate()){
                if(mProxyPort>0)
                    sprintf(str,"%s:%u",mProxyDomain.c_str(),mProxyPort);
                else
                    sprintf(str,"%s:%u",mProxyDomain.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mProxyDomain.c_str());
        }else if(useProxyDiscoveryPrivate()){
            strcpy(str,mProxyDomain.c_str());
        }else if(mProxyDomain.empty() && !mProxyName.empty()){
            if(!isSecureReqURIPrivate()){
                if(mProxyPort>0)
                    sprintf(str,"%s:%u",mProxyName.c_str(),mProxyPort);
                else
                    sprintf(str,"%s:%u",mProxyName.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mProxyName.c_str());
        }else if(mProxyDomain.empty() && !mProxyIP.empty()){
            if(!isSecureReqURIPrivate()){
                if(mProxyPort>0)
                    sprintf(str,"%s:%u",mProxyIP.c_str(),mProxyPort);
                else
                    sprintf(str,"%s:%u",mProxyIP.c_str(),SIP_DEFAULT_PORT);
            }else
                strcpy(str,mProxyIP.c_str());
        }
    }

    if(!isSecureReqURIPrivate() && !useProxyDiscoveryPrivate()) {
        if(getTransportPrivate() == RVSIP_TRANSPORT_TCP) {
            strcat(str,";transport=TCP");
        } else if(getTransportPrivate() == RVSIP_TRANSPORT_UDP) {
            strcat(str,";transport=UDP");
        }
    }
    rd = str;
    return rd;
}

std::string SipChannel::getContactDomainPrivate() const {
    if(!mContactDomain.empty()) return mContactDomain;
    return mLocalAddress.strIP;
}

unsigned short SipChannel::getContactDomainPortPrivate() const {
    if(!mContactDomain.empty()) {
        if(mContactDomainPort>0) return mContactDomainPort;
        else return SIP_DEFAULT_PORT;
    }
    return mLocalAddress.port;
}

std::string SipChannel::getLocalDisplayNamePrivate() const {
    if(useIMSIPrivate() && !mImsi.empty()) return mImsi;
    if(!mDisplayName.empty()) return mDisplayName;
    return getRegNamePrivate();
}

std::string SipChannel::getDisplayNameRemotePrivate() const {
    if(!mDisplayNameRemote.empty()) return mDisplayNameRemote;
    return mUserNameRemote;
}

std::string SipChannel::getRegNamePrivate() const { 
    if(!mRegName.empty()) return mRegName; 
    return getLocalUserNamePrivate(true); 
}

std::string SipChannel::getLocalUserNamePrivate(bool useTel) const { 
    if(mTelUriLocal && useTel) {
        if(!mUserNumber.empty()) return mUserNumber; 
    }
    if(useIMSIPrivate() && !mImsi.empty()) return mImsi;
    if(!mUserName.empty()) return mUserName; 
    if(!mUserNumber.empty()) return mUserNumber; 
    return mRegName;
}

std::string SipChannel::getLocalUserNumberPrivate() const { 
    if(!mUserNumber.empty()) return mUserNumber; 
    if(!mUserName.empty()) return mUserName; 
    return mRegName;
}

void SipChannel::callTimerStopPrivate() {

    ENTER();

    if(mHCallTimer) {
        mHCallTimer->unload();
        mHCallTimer=0;
    }

    RET;
}

void SipChannel::setCallTimerMediaPrivate() {

    ENTER();

    if(mHCallTimerMedia) {
        mHCallTimerMedia->unload();
        mHCallTimerMedia=0;
    }

    uint32_t interval=CALL_TIMER_MEDIA_POSTCALL_TIMEOUT;

    if(interval>0) {
        ExpirationCB       cb = callTimerMediaExpires;

        mHCallTimerMedia = mTimers.getTimer();
        mHCallTimerMedia->load(this,cb,interval,(void**)&mHCallLeg,(void**)&mHCallTimerMedia);
    }

    RET;
}

void SipChannel::callTimerMediaStopPrivate() {

    ENTER();

    if(mHCallTimerMedia) {
        mHCallTimerMedia->unload();
        mHCallTimerMedia=0;
    }

    RET;
}

void SipChannel::setByeAcceptTimerMediaPrivate() {

    ENTER();

    if(mHByeAcceptTimerMedia) {
        mHByeAcceptTimerMedia->unload();
        mHByeAcceptTimerMedia=0;
    }

    uint32_t interval=CALL_TIMER_MEDIA_POSTCALL_TIMEOUT;

    if(interval>0) {
        ExpirationCB       cb = byeAcceptTimerMediaExpires;

        mHByeAcceptTimerMedia = mTimers.getTimer();
        mHByeAcceptTimerMedia->load(this,cb,interval,(void**)&mHCallLeg,(void**)&mHByeAcceptTimerMedia);
    }

    RET;
}

void SipChannel::byeAcceptTimerMediaStopPrivate() {

    ENTER();

    if(mHByeAcceptTimerMedia) {
        mHByeAcceptTimerMedia->unload();
        mHByeAcceptTimerMedia=0;
    }

    RET;
}

void SipChannel::setPostCallTimerPrivate() {

    ENTER();

    uint32_t interval=mPostCallTime;

    if(interval>10) {
        interval = interval - SipChannel::getTimeVariationPrivate();
        ExpirationCB       cb = postCallTimerExpires;

        TimerHandle postCallTimer = mTimers.getTimer();
        postCallTimer->load(this,cb,interval,NULL,NULL);

    } else {
        postCallTimerExpires(this);
    }

    RET;
}

void SipChannel::ringTimerStopPrivate() {

    ENTER();

    if(mHRingTimer) {
        mHRingTimer->unload();
        mHRingTimer=0;
    }

    RET;
}

RvStatus SipChannel::addLocalAddressPrivate() {

    RvStatus rv = RV_OK;
    RvSipTransportLocalAddrHandle   hLocalAddr=(RvSipTransportLocalAddrHandle)0;

    RvSipTransportAddr addr = mLocalAddress;

    if(!mHTrMgr) {
        mIsEnabled=false;
        return RV_ERROR_UNKNOWN;
    }

    if(RvSipTransportMgrLocalAddressFind(mHTrMgr,&addr,sizeof(addr),&hLocalAddr) < 0 || !hLocalAddr) {
        rv = RvSipTransportMgrLocalAddressAdd(mHTrMgr,
                &addr, sizeof(addr),
                RVSIP_FIRST_ELEMENT,NULL/*Base Address*/,
                &hLocalAddr);
        if(rv>=0) {
            RVSIPPRINTF("%s: successfully added local addr %s:%d / %s [%d]\n",__FUNCTION__,addr.strIP,(int)addr.port,addr.if_name,(int)addr.vdevblock);
            if(mUasPerDevice<=2) {
                RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,SMALL_SIGNALING_SOCKET_SIZE);
            } else if(mUasPerDevice<=MULTIPLE_UAS_PER_DEVICE_THRESHOLD) {
                RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,MODEST_SIGNALING_SOCKET_SIZE);
            } else if(mUasPerDevice>=BULK_UAS_PER_DEVICE_THRESHOLD) {
                RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,BULK_SIGNALING_SOCKET_SIZE);
            } else if(mUasPerDevice>=MANY_UAS_PER_DEVICE_THRESHOLD) {
                RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,LARGE_SIGNALING_SOCKET_SIZE);
            } else {
                RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,NORMAL_SIGNALING_SOCKET_SIZE);
            }
        } else {
            RVSIPPRINTF("%s: ERROR %d while adding local addr %s:%d / %s [%d]\n",__FUNCTION__,(int)rv,addr.strIP,(int)addr.port,addr.if_name,(int)addr.vdevblock);
        }
    } else if(hLocalAddr) {
        if(mUasPerDevice>=MANY_UAS_PER_DEVICE_THRESHOLD) {
            RvSipTransportMgrLocalAddressSetSockSize(hLocalAddr,LARGE_SIGNALING_SOCKET_SIZE);
        }
    }

    if(rv>=0 && hLocalAddr && mIpServiceLevel) {
        RvSipTransportMgrLocalAddressSetIpTosSockOption(hLocalAddr,mIpServiceLevel);
        VoIPUtils::getInfMac(mLocalAddress.if_name,mInfMacAddress);
    }

    mIsEnabled=(rv>=0);
    mHandle=(int)hLocalAddr;

    return rv;
}

RvStatus SipChannel::removeLocalAddressPrivate() {

    mIsEnabled=false;

    if(!mHTrMgr) return RV_ERROR_UNKNOWN;
    RvSipTransportLocalAddrHandle   hLocalAddr=(RvSipTransportLocalAddrHandle)0;

    RvSipTransportAddr addr = mLocalAddress;
    memset(&mLocalAddress,0,sizeof(mLocalAddress));

    RvStatus rv = RvSipTransportMgrLocalAddressFind(mHTrMgr,&addr,sizeof(addr),&hLocalAddr);
    if(!hLocalAddr) return RV_ERROR_UNKNOWN;
    if(rv<0) return rv;
    return RvSipTransportMgrLocalAddressRemove(mHTrMgr,hLocalAddr);
}

RvStatus SipChannel::startReregistrationPrivate() {

    mIsDeregistering=false;
    return startRegistrationProcessPrivate(mRegExpiresTime);
}

void SipChannel::getFromStrPrivate(std::string domain,std::string &from){
    char _from[MAX_LENGTH_OF_URL];

    if(useMobilePrivate()){
        if(useIMSIPrivate() && !mIMPU.empty()){
            snprintf(_from,sizeof(_from),"\"%s\" <%s>",getLocalDisplayNamePrivate().c_str(),mIMPU.c_str());
            from = _from;
            return;
        }
    }
    if(mTelUriLocal) {
        if(mUserNumberContext.empty()) {
            snprintf(_from, sizeof(_from), "\"%s\" <tel:+%s>", getLocalDisplayNamePrivate().c_str(),getLocalUserNumberPrivate().c_str());
        } else {
            snprintf(_from,sizeof(_from), "\"%s\" <tel:%s;phone-context=%s>", getLocalDisplayNamePrivate().c_str(),getLocalUserNumberPrivate().c_str(),mUserNumberContext.c_str());
        }
    } else {
        snprintf(_from,sizeof(_from),"\"%s\" <sip:%s@%s>",getLocalDisplayNamePrivate().c_str(),getRegNamePrivate().c_str(),domain.c_str());
    }
    from = _from;
}

RvStatus SipChannel::startRegistrationProcessPrivate(uint32_t expires) 
{
    ENTER();
    RvStatus rv = RV_OK;

    regTimerResetPrivate();

    if (!usePeer2PeerPrivate() && useRegistrarPrivate()){

        if(!mHRegClient) {

            RvChar strFrom[MAX_LENGTH_OF_URL]; // "From:sip:172.20.3.38";
            RvChar strTo[MAX_LENGTH_OF_URL]; // "To:sip:me@172.20.2.151";
            RvChar strLocalContactAddress[MAX_LENGTH_OF_URL] = "\0"; // "Contact:sip:172.20.3.38";
            RvChar strRequestURI[MAX_LENGTH_OF_URL]; // "sip:172.20.2.151";
            std::string strRegistrar=getOurDomainStrPrivate();
            std::string strFromStr;

            getFromStrPrivate(strRegistrar,strFromStr);

            strcpy(strTo,"To: ");
            strcpy(strFrom,"From: ");

            strcpy(strTo+4,strFromStr.c_str());
            strcpy(strFrom+6,strFromStr.c_str());

            strcpy(strRequestURI,"sip:");
            strcpy(strRequestURI+4,strRegistrar.c_str());

            getLocalContactAddressForRegisterPrivate(strLocalContactAddress, sizeof(strLocalContactAddress)-1);

            RVSIPPRINTF("%s: [%s]:[%s]:[%s]:[%s]\n",__FUNCTION__,strFrom, strTo, strRequestURI, strLocalContactAddress);

            rv = RvSipRegClientMgrCreateRegClient(mHRegMgr, (RvSipAppRegClientHandle)this, &mHRegClient);
            if (rv<0) {
                RVSIPPRINTF("Failed to create new register-client\n");
                return rv;
            }

            rv = RvSipRegClientDetachFromMgr(mHRegClient);
            if (rv<0) {
                RVSIPPRINTF("Failed to detach new register-client\n");
                return rv;
            }

            rv = RvSipRegClientSetLocalAddress(mHRegClient,
                    mLocalAddress.eTransportType,
                    mLocalAddress.eAddrType,
                    mLocalAddress.strIP,
                    mLocalAddress.port,
                    mLocalAddress.if_name,
                    mLocalAddress.vdevblock);
            if (rv<0) {
                RVSIPPRINTF("Failed to set local address to reg-client\n");
                return rv;
            }

            if(mExtraContacts.size()>0) {

                //First, the main contact:
                {
                    RvSipContactHeaderHandle   hContact = (RvSipContactHeaderHandle)0;
                    void* phElement=0;

                    rv = RvSipRegClientGetNewMsgElementHandle (mHRegClient,RVSIP_HEADERTYPE_CONTACT,RVSIP_ADDRTYPE_UNDEFINED,&phElement);
                    if (rv != RV_Success) {
                        RVSIPPRINTF("%s: 111.222.111\n",__FUNCTION__);
                        return rv;
                    }

                    hContact=(RvSipContactHeaderHandle)phElement;

                    {
                        rv = RvSipContactHeaderParse(hContact, strLocalContactAddress);
                        if (rv != RV_Success) {
                            RVSIPPRINTF("%s: 111.222.222: %s: %d\n",__FUNCTION__,strLocalContactAddress,(int)rv);
                            return rv;
                        }
                    }
                    rv = RvSipRegClientSetContactHeader(mHRegClient, hContact);
                    if (rv != RV_Success) {
                        RVSIPPRINTF("%s: 111.222.333\n",__FUNCTION__);
                        return rv;
                    }
                }

                //Then, N-1 extra contacts:
                unsigned int i=0;
                for(i=0;i<mExtraContacts.size()-1;i++) {

                    RvSipContactHeaderHandle   hContact = (RvSipContactHeaderHandle)0;
                    void* phElement=0;

                    rv = RvSipRegClientGetNewMsgElementHandle (mHRegClient,RVSIP_HEADERTYPE_CONTACT,RVSIP_ADDRTYPE_UNDEFINED,&phElement);
                    if (rv != RV_Success) {
                        RVSIPPRINTF("%s: 111.222.111\n",__FUNCTION__);
                        return rv;
                    }

                    hContact=(RvSipContactHeaderHandle)phElement;

                    {
                        char                     temp[MAX_LENGTH_OF_URL];
                        snprintf(temp, sizeof(temp)-1, "Contact: %s", mExtraContacts[i].c_str());
                        RVSIPPRINTF("%s: 111.222.111.111: extra contact: %s\n",__FUNCTION__,temp);
                        rv = RvSipContactHeaderParse(hContact, temp);
                        if (rv != RV_Success) {
                            RVSIPPRINTF("%s: 111.222.222: %s: %d\n",__FUNCTION__,temp,(int)rv);
                            return rv;
                        }
                    }
                    rv = RvSipRegClientSetContactHeader(mHRegClient, hContact);
                    if (rv != RV_Success) {
                        RVSIPPRINTF("%s: 111.222.333\n",__FUNCTION__);
                        return rv;
                    }
                }

                //Then, the last extra contact:
                snprintf(strLocalContactAddress,sizeof(strLocalContactAddress)-1,"Contact: %s",mExtraContacts[i].c_str());
            }

            {
                RvSipExpiresHeaderHandle   hExpires = (RvSipExpiresHeaderHandle)0;
                void* phElement=0;

                rv = RvSipRegClientGetNewMsgElementHandle (mHRegClient,RVSIP_HEADERTYPE_EXPIRES,RVSIP_ADDRTYPE_UNDEFINED,&phElement);
                if (rv != RV_Success) {
                    return rv;
                }

                hExpires=(RvSipExpiresHeaderHandle)phElement;

                {
                    char                     temp[100];
                    snprintf(temp, sizeof(temp)-1, "Expires:%lu", (unsigned long)expires);
                    rv = RvSipExpiresHeaderParse(hExpires, temp);
                    if (rv != RV_Success) {
                        return rv;
                    }
                }
                rv = RvSipRegClientSetExpiresHeader(mHRegClient, hExpires);
                if (rv != RV_Success) {
                    return rv;
                }
            }

            if(!useRegistrarDiscoveryPrivate()) {

                RvSipTransportOutboundProxyCfg outboundCfg;
                memset(&outboundCfg,0,sizeof(outboundCfg));

                outboundCfg.eTransport=getTransportPrivate();
                outboundCfg.eCompression = RVSIP_COMP_UNDEFINED;
                setOutboundCfgPrivate(true,outboundCfg);

                if(outboundCfg.eTransport == RVSIP_TRANSPORT_UNDEFINED) outboundCfg.eTransport=RVSIP_TRANSPORT_UDP;

                RVSIPPRINTF("Reg outbound address: %s:%s:%d:%d\n",outboundCfg.strIpAddress, outboundCfg.strHostName,outboundCfg.port,outboundCfg.eTransport);

                rv = RvSipRegClientSetOutboundDetails(mHRegClient,&outboundCfg,sizeof(outboundCfg));
                if (rv != RV_Success) {
                    return rv;
                }
            }

            rv = RvSipRegClientPrepare(mHRegClient, strFrom, strTo, strRequestURI, strLocalContactAddress);
            if (rv<0) {
                RVSIPPRINTF("Register request failed, 111.222: %d: [%s]:[%s]:[%s]:[%s]\n",rv, strFrom, strTo, strRequestURI, strLocalContactAddress);
                return rv;
            }

            if(mUseHostInCallID) { //Call ID
                char callID[MAX_LENGTH_OF_URL]="\0";
                RvInt32 actualSize=0;
                std::string domain=getOurDomainStrPrivate();
                rv = RvSipRegClientGetCallId(mHRegClient,sizeof(callID)-1,callID,&actualSize);
                if(rv<0) {
                    RVSIPPRINTF("%s: error 111.333.000.222: rv=%d\n",__FUNCTION__,rv);
                    return rv;
                } else {
                    if(actualSize+domain.length()+1+1>sizeof(callID)) {
                        RVSIPPRINTF("%s: error 111.333.000.333: %s@%s\n",__FUNCTION__,callID,domain.c_str());
                    } else {
                        size_t callID_len=strlen(callID);
                        snprintf(callID+callID_len,sizeof(callID)-callID_len,"@%s",domain.c_str());
                        callID[sizeof(callID)-1] = '\0';
                        rv = RvSipRegClientSetCallId (mHRegClient,callID);
                        if(rv<0) {
                            RVSIPPRINTF("%s: error 111.333.000.444: rv=%d, callID=%s\n",__FUNCTION__,rv, callID);
                            return rv;
                        }
                    }
                }
            }

            rv = sessionSetRegClientAllow();
            if(rv<0) {
                RVSIPPRINTF("%s: error 111.333.000.444.111: rv=%d\n",__FUNCTION__,rv);
                return rv;
            }
            if(useAkaPrivate()) {
                rv = RvSipUtils::RvSipAka::SIP_AkaInsertInitialAuthorizationToMessage(mHRegClient, getAuthNamePrivate(), getAuthRealmPrivate());
            }

            rv = RvSipRegClientSendRequest(mHRegClient);
            if (rv<0) {
                RVSIPPRINTF("Register request failed, 111.333: %d\n",rv);
                return rv;
            }

        } else {

            RvSipExpiresHeaderHandle   hExpires = (RvSipExpiresHeaderHandle)0;

            rv = RvSipRegClientGetExpiresHeader(mHRegClient, &hExpires);
            if (rv != RV_Success) {
                return rv;
            }
            {
                char                     temp[100];
                snprintf(temp, sizeof(temp)-1, "Expires:%lu", (unsigned long)expires);
                rv = RvSipExpiresHeaderParse(hExpires, temp);
                if (rv != RV_Success) {
                    return rv;
                }
            }
            rv = RvSipRegClientSetExpiresHeader(mHRegClient, hExpires);
            if (rv != RV_Success) {
                return rv;
            }

            rv = sessionSetRegClientAllow();
            if(rv<0) {
                RVSIPPRINTF("%s: error 111.333.000.444.111: rv=%d\n",__FUNCTION__,rv);
                return rv;
            }
            rv = RvSipRegClientRegister(mHRegClient);
            if (rv<0) {
                RVSIPPRINTF("Re-register request failed: %d\n",rv);
                return rv;
            }
        }
    }

    RETCODE(rv);	
}

void SipChannel::regTimerResetPrivate() {

    if(!mIsInRegTimer && mHRegTimer) {
        mHRegTimer->unload();
        mHRegTimer=0;
    }
}

RvStatus SipChannel::acceptPrivate()
{
    ENTER();

    RvStatus rv=RV_ERROR_UNKNOWN;

    if(mHCallLeg) {
        if( mMediaHandler) {
            rv = mMediaHandler->media_beforeAccept(mHCallLeg);
        }
        rv = sessionSetCallLegAllowPrivate();
        rv |= RvSipCallLegAccept(mHCallLeg);
    }

    RETCODE(rv);
}

RvStatus SipChannel::disconnectPrivate()
{
    ENTER();

    RvStatus rv = RV_OK;

    if( mMediaHandler) {
        mMediaHandler->media_beforeDisconnect(mHCallLeg);
    }

    if(mHCallLeg) {
        if(mHReInvite){
            RvSipCallLegReInviteTerminate(mHCallLeg,mHReInvite);
            mHReInvite = 0;
        }
        rv = RvSipCallLegDisconnect(mHCallLeg);
    }

    RETCODE(rv);
}

std::string SipChannel::SIP_GetProxyRouteHeaderStringPrivate() {

    char buf[MAX_LENGTH_OF_URL];
    int len=sizeof(buf)-1;

    buf[0]=0;

    if(useProxyPrivate()) {

        if(useProxyDiscoveryPrivate()) {
            snprintf(buf,len,"Route: <sip:%s;lr>",mProxyDomain.c_str());

        } else {
            // Format the route header as followed:  "Route: <uri-scheme:address:port;[transport-param];lr"

            bool btmp=false;
            std::string sremoteAddr;
            uint16_t port;
            getNextHopPrivate(false,sremoteAddr,port,btmp);
            switch(getTransportPrivate()) {
                case RVSIP_TRANSPORT_TCP:
                    // TCP is not the default transport for SIP; so, we have to explicitly list it in the transport parameter.
                    snprintf(buf,len,"Route: <sip:%s:%u;transport=TCP;lr>",sremoteAddr.c_str(),port);
                    break;
                case RVSIP_TRANSPORT_UDP:
                    // UDP is not the default transport for SIP; so, we have to explicitly list it in the transport parameter.
                    snprintf(buf,len,"Route: <sip:%s:%u;transport=UDP;lr>",sremoteAddr.c_str(),port);
                    break;
                default:
                    // for default, we don't have to specify a type of the transport implying it's UDP.
                    snprintf(buf,len,"Route: <sip:%s:%u;lr>",sremoteAddr.c_str(),port);
                    break;
            }//end-switch
        }
    }
    return buf;
}

uint32_t SipChannel::getTimeVariationPrivate() {
    return 8;
}

void SipChannel::fixUserNamePrivate(std::string& s) {

    const char* ss = s.c_str();

    if(ss) {

        size_t len=(size_t)(strlen(ss));

        size_t pos = 0;
        size_t n = len;
        bool found=false;

        while(n>0 && ss[pos]==' ') { --n; ++pos; found=true; }

        while(n>0 && ss[pos+n-1]==' ') { --n; found=true; }

        if(found) {
            std::string sfixed(s,pos,n);
            char* sf=strdup(sfixed.c_str());
            if(sf){
                s = sf;
                free(sf);
            }else
                s = "";
        }
    }
}

void SipChannel::callCompletionNotifyPrivate() {
    if(mCallStateNotifier) {
        mCallStateNotifier->callCompleted();
    }
}

void SipChannel::getLocalContactAddressForGeneralRequestPrivate(char* strLocalContactAddress, size_t size) {

    std::string cd = getContactDomainPrivate();

    unsigned short port = getContactDomainPortPrivate();
    if(!useProxyDiscoveryPrivate()){

        if(getTransportPrivate() == RVSIP_TRANSPORT_TCP) {
            snprintf(strLocalContactAddress, size, 
                    "sip:%s@%s:%u;transport=TCP",
                    getLocalUserNamePrivate(false).c_str(), 
                    cd.c_str(), 
                    (unsigned int)port);
        } else if(getTransportPrivate() == RVSIP_TRANSPORT_UDP) {
            snprintf(strLocalContactAddress, size, 
                    "sip:%s@%s:%u;transport=UDP",
                    getLocalUserNamePrivate(false).c_str(), 
                    cd.c_str(), 
                    (unsigned int)port);
        } else {
            snprintf(strLocalContactAddress, size, 
                    "sip:%s@%s:%u",
                    getLocalUserNamePrivate(false).c_str(), 
                    cd.c_str(), 
                    (unsigned int)port);
        }
    }else
        snprintf(strLocalContactAddress, size, 
                "sip:%s@%s:%u",
                getLocalUserNamePrivate(false).c_str(), 
                cd.c_str(), 
                (unsigned int)port);
}

void SipChannel::getLocalContactAddressForRegisterPrivate(char* strLocalContactAddress, size_t size) {

    if(getTransportPrivate() == RVSIP_TRANSPORT_TCP) {
        snprintf(strLocalContactAddress, size, 
                "Contact: <sip:%s@%s:%u;transport=TCP>",
                getLocalUserNamePrivate(false).c_str(), 
                mLocalAddress.strIP,
                (unsigned int)mLocalAddress.port);
    } else if(getTransportPrivate() == RVSIP_TRANSPORT_UDP) {
        snprintf(strLocalContactAddress, size, 
                "Contact: <sip:%s@%s:%u;transport=UDP>",
                getLocalUserNamePrivate(false).c_str(), 
                mLocalAddress.strIP, 
                (unsigned int)mLocalAddress.port);
    } else {
        snprintf(strLocalContactAddress, size, 
                "Contact: <sip:%s@%s:%u>",
                getLocalUserNamePrivate(false).c_str(), 
                mLocalAddress.strIP, 
                (unsigned int)mLocalAddress.port);
    }
    if(isTipDevice() && !mInfMacAddress.empty()){
		size_t len = strlen(strLocalContactAddress);
		snprintf(strLocalContactAddress+len,sizeof(strLocalContactAddress)-len,";+sip.instance=\"<urn:uuid:00000000-0000-0000-0000-%s>\";+u.sip!model.ccm.cisco.com=\"%d\"",mInfMacAddress.c_str(),mTipDeviceType);

    }
}

RvStatus SipChannel::getDynamicRoutesPrivate(std::string &route, std::string &serviceRoute) {

    // Add dynamic routes
    RvStatus rv = RV_OK;

    route=std::string("");
    serviceRoute=std::string("");

    route = SIP_GetProxyRouteHeaderStringPrivate();

    if(!mServiceRoute.empty()) {
        serviceRoute=mServiceRoute;
    }

    return rv;
}

RvStatus SipChannel::addDynamicRoutesToSubscribePrivate(RvSipCallLegHandle hCallLeg) {

    // Add dynamic routes
    RvStatus rv = RV_OK;
    RVSIPLOCK(mLocker);
    std::string route;;

    if(hCallLeg) {

        std::string serviceRoute=mServiceRoute;

        route = SIP_GetProxyRouteHeaderStringPrivate();

        if(rv == RV_OK) {

            const char* r = route.c_str();

            const char* sr = serviceRoute.c_str();

            if(*r || *sr) {
                rv = RvSipCallLegRouteListAdd(hCallLeg, (RvChar*)(r), (RvChar*)sr );
                if(rv<0) {
                    RVSIPPRINTF("%s: error 111.333.666: rv=%d: route=%s, port=%d\n",__FUNCTION__,rv,route.c_str(), getNextHopPortPrivate());
                    RETCODE(rv);
                }
            }
        }
    }

    return rv;
}

bool SipChannel::containsRoutesPrivate(RvSipMsgHandle hMsg) {

    bool ret=false;

    if(hMsg) {

        RvSipRouteHopHeaderHandle    hRouteHop;
        RvSipHeaderListElemHandle    listElem;
        RvSipRouteHopHeaderType      routeHopType;

        hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                RVSIP_HEADERTYPE_ROUTE_HOP, 
                RVSIP_FIRST_HEADER, 
                RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
        while (hRouteHop != NULL) {

            routeHopType = RvSipRouteHopHeaderGetHeaderType(hRouteHop);

            if(routeHopType == RVSIP_ROUTE_HOP_ROUTE_HEADER) {
                ret=true;
                break;
            }

            // get the next route hop
            hRouteHop = (RvSipRouteHopHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
                    RVSIP_HEADERTYPE_ROUTE_HOP, 
                    RVSIP_NEXT_HEADER, 
                    RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
        }
    }

    return ret;
}

RvStatus SipChannel::sessionSetCallLegAllowPrivate() {

    RvStatus rv = RV_OK;

    RvSipMsgHandle            hMsg=(RvSipMsgHandle)0;
    rv = RvSipCallLegGetOutboundMsg(mHCallLeg,&hMsg);
    if(rv>=0 && hMsg) {
        rv = sessionSetAllowPrivate(hMsg);
    }

    return rv;
}

RvStatus SipChannel::sessionSetAllowPrivate(RvSipMsgHandle            hMsg) {

    RvStatus rv = RV_OK;

    if(hMsg) {
        RvSipOtherHeaderHandle hAllowHeader = NULL;
        rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_TRUE, &hAllowHeader);
        if(rv>=0 && hAllowHeader) {
            RvChar name[11];
            strcpy(name,"Allow");
            rv = RvSipOtherHeaderSetName(hAllowHeader, name);
            RvChar value[129];
            strcpy(value,ALLOWED_METHODS);
            rv = RvSipOtherHeaderSetValue(hAllowHeader, value);
        }
    }

    return rv;
}

/******************************* Auth *******************************************/

std::string SipChannel::getAuthNamePrivate() const {


    if(useIMSIPrivate()) return mIMPI;
    std::string ret = mUserName;

    if(!mAuthName.empty()) ret = mAuthName;
    else if(!mRegName.empty()) ret = mRegName;

    return ret;
}

std::string SipChannel::getAuthRealmPrivate() const {

    std::string ret = mAuthRealm;
    if(useAkaPrivate()) {
        if(ret.empty()) ret=mRegDomain;
        if(ret.empty()) ret=mLocalURIDomain;
        if(ret.empty()) ret=mContactDomain;
    }
    return ret;
}

void SipChannel::getNextHopPrivate(bool isReg,std::string &next_hop,uint16_t &port,bool &isAddr){

    isAddr = false;
    if(usePeer2PeerPrivate()){
        if(!RvSipUtils::isEmptyIPAddress(mAddressRemote.strIP)) { 
            isAddr=true; 
            next_hop = mAddressRemote.strIP; 
        }else 
            next_hop = RvSipUtils::getFQDN(mHostNameRemote,mRemoteDomain);
        port = mAddressRemote.port;
    }else{
        if(isReg){
            if(!mRegServerIP.empty()){
                next_hop = mRegServerIP;
                isAddr = true;
            }else if(!mRegServerName.empty() || !mRegDomain.empty()){
                next_hop = RvSipUtils::getFQDN(mRegServerName,mRegDomain);
            }
            if(mRegServerPort > 0)
                port = mRegServerPort;
            else
                port = SIP_DEFAULT_PORT;
        }else{
            if(!mProxyIP.empty()){
                next_hop = mProxyIP;
                isAddr = true;
            }else if(!mProxyName.empty() || !mProxyDomain.empty()){
                next_hop = RvSipUtils::getFQDN(mProxyName,mProxyDomain);
            }
            if(mProxyPort> 0)
                port = mProxyPort;
            else
                port = SIP_DEFAULT_PORT;
        }
    }
}

void SipChannel::setOutboundCfgPrivate(bool isReg,RvSipTransportOutboundProxyCfg &cfg){
    //not hanle case of Discovery.
    std::string next_hop;
    bool isAddr = false;
    uint16_t port = 0;

    if(isReg && useRegistrarDiscoveryPrivate())
        return;
    if(!isReg && useProxyDiscoveryPrivate())
        return;

    getNextHopPrivate(isReg,next_hop,port,isAddr);

    if(isAddr)
        cfg.strIpAddress=(RvChar*)(next_hop.c_str());
    else
        cfg.strHostName=(RvChar*)(next_hop.c_str());
    cfg.port=port;

    return;
}

RvSipTransport SipChannel::getTransportPrivate() const {
    return mTransport;
}
RvStatus SipChannel::stopSubsCallLegPrivate() {

    if(mHSubsCallLeg) {
        RvSipCallLegState    eState = RVSIP_CALL_LEG_STATE_UNDEFINED;
        RvSipCallLegGetCurrentState (mHSubsCallLeg,&eState);
        if(eState != RVSIP_CALL_LEG_STATE_TERMINATED && 
                eState != RVSIP_CALL_LEG_STATE_DISCONNECTED && 
                eState != RVSIP_CALL_LEG_STATE_DISCONNECTING &&
                eState != RVSIP_CALL_LEG_STATE_UNDEFINED && 
                eState != RVSIP_CALL_LEG_STATE_CANCELLING &&
                eState!=RVSIP_CALL_LEG_STATE_IDLE) {
            RvSipCallLegTerminate(mHSubsCallLeg);
        }
        mHSubsCallLeg = 0;
    }

    return RV_OK;
}
RvStatus SipChannel::subscribeRegEventPrivate(void)
{
    ENTER();

    if(mHImsSubs) {
        UnsubscribeRegEventPrivate();
        mHImsSubs = (RvSipSubsHandle)0;
    }

    mSubsToDeregister=false;

    stopSubsCallLegPrivate();

    RvStatus rv = RV_OK;

    try {

        RvChar strFrom[MAX_LENGTH_OF_URL];
        RvChar strTo[MAX_LENGTH_OF_URL];
        RvChar strLocalContactAddress[MAX_LENGTH_OF_URL] = "\0";
        RvChar strRequestURI[MAX_LENGTH_OF_URL];


        std::string strRegistrar=getOurDomainStrPrivate();
        std::string strFromStr;

        getFromStrPrivate(strRegistrar,strFromStr);

        strcpy(strTo,"To: ");
        strcpy(strFrom,"From: ");

        strcpy(strTo+4,strFromStr.c_str());
        strcpy(strFrom+6,strFromStr.c_str());

        strcpy(strRequestURI,"sip:");
        strcpy(strRequestURI+4,strRegistrar.c_str());


        getLocalContactAddressForRegisterPrivate(strLocalContactAddress, sizeof(strLocalContactAddress)-1);

        RVSIPPRINTF("%s: [%s]:[%s]:[%s]:[%s]\n",__FUNCTION__,strFrom, strTo, strRequestURI, strLocalContactAddress);

        rv = RvSipSubsMgrCreateSubscription(mHSubsMgr,NULL,(RvSipAppSubsHandle)this, &mHImsSubs, 0, 0xFF);
        if(rv < 0) {
            throw std::string("Cannot create subscription");
        }

        rv = RvSipSubsSetNoNotifyTimer(mHImsSubs,mImsSubsExpires*1000);
        if(rv < 0) {
            throw std::string("Cannot set no notify timer");
        }

        RvSipCallLegHandle mHSubsCallLeg = (RvSipCallLegHandle)0;

        rv = RvSipSubsGetDialogObj(mHImsSubs,&mHSubsCallLeg);
        if(rv < 0 || !mHSubsCallLeg) {
            throw std::string("Cannot get dialog");
        }

        RvChar subsStr[33];
        strcpy(subsStr,"reg");
        rv = RvSipSubsInitStr(mHImsSubs,strFrom+6,strTo+4,mImsSubsExpires,subsStr);
        if(rv < 0 ) {
            throw std::string("Cannot init subscription");
        }

        rv = RvSipCallLegSetLocalAddress(mHSubsCallLeg,
                mLocalAddress.eTransportType,
                mLocalAddress.eAddrType,
                mLocalAddress.strIP,
                mLocalAddress.port,
                mLocalAddress.if_name,
                mLocalAddress.vdevblock);
        if (rv<0) {
            throw std::string("Cannot set local address");
        }

        RvSipAddressHandle hLocalContactAddress = NULL;

        rv = RvSipCallLegGetNewMsgElementHandle( mHSubsCallLeg, 
                RVSIP_HEADERTYPE_UNDEFINED, RVSIP_ADDRTYPE_URL, 
                (void **)&hLocalContactAddress);

        if(rv<0) {
            throw std::string("Cannot get new msg element");
        }
        char *pContact = strLocalContactAddress;
        size_t pContact_len = strlen(pContact);

        if(pContact_len > 11){//"Contact: <" + ">"
            pContact[pContact_len-1] = '\0';
        }else
            throw std::string("Cannot get invalid local contact address.");

        pContact +=10;//"Contact: <"

        rv = RvSipAddrParse (hLocalContactAddress,  pContact);
        if(rv<0) {
            throw std::string("Cannot parse address");
        }

        rv  = RvSipCallLegSetLocalContactAddress (mHSubsCallLeg, hLocalContactAddress);
        if (rv < 0) {
            throw std::string("Cannot set contact address");
        }

        if(!useRegistrarDiscoveryPrivate()) {

            RvSipTransportOutboundProxyCfg outboundCfg;
            memset(&outboundCfg,0,sizeof(outboundCfg));

            outboundCfg.eTransport=getTransportPrivate();
            outboundCfg.eCompression = RVSIP_COMP_UNDEFINED;
            if(outboundCfg.eTransport == RVSIP_TRANSPORT_UNDEFINED) outboundCfg.eTransport=RVSIP_TRANSPORT_UDP;
            setOutboundCfgPrivate(true,outboundCfg);

            RVSIPPRINTF("%s: Reg outbound address: %s:%s:%d:%d\n",__FUNCTION__,
                    outboundCfg.strIpAddress, outboundCfg.strHostName,outboundCfg.port,outboundCfg.eTransport);

            rv = RvSipCallLegSetOutboundDetails(mHSubsCallLeg,&outboundCfg,sizeof(outboundCfg));
            if (rv != RV_Success) {
                throw std::string("Cannot set outbound details");
            }
        }

        rv = addDynamicRoutesToSubscribePrivate(mHSubsCallLeg);
        if(rv < 0 ){
            throw std::string("Cannot add dynamic routes");
        }

        //TODO whether to add authentication header

        rv = RvSipSubsSubscribe(mHImsSubs);
        if(rv < 0 ) {
            throw std::string("Cannot subscribe");
        }

    } catch(std::string ex) {
        RVSIPPRINTF("%s: subscription problem: %s, rv=%d\n",__FUNCTION__,ex.c_str(),(int)rv);
        if(mHImsSubs) {
            RvSipSubsHandle hSubs = mHImsSubs;
            //set to zero so will will have no callback to subsTerminated:
            mHImsSubs = (RvSipSubsHandle)0;
            RvSipSubsTerminate(hSubs);
        }
    }

    RETCODE(rv);
}

RvStatus SipChannel::UnsubscribeRegEventPrivate() {

    if(mHImsSubs) {

        RvSipSubsState             eState = RVSIP_SUBS_STATE_IDLE;

        RvSipSubsGetCurrentState(mHImsSubs,&eState);

        if(eState != RVSIP_SUBS_STATE_IDLE && 
                eState != RVSIP_SUBS_STATE_UNDEFINED && 
                eState !=RVSIP_SUBS_STATE_TERMINATED) {
            RvStatus rv = RvSipSubsUnsubscribe(mHImsSubs);
            if(rv == RV_OK) return rv;
        }
    }

    return deregisterOnUnSubsPrivate();
}

RvStatus SipChannel::deregisterOnUnSubsPrivate() {

    if(mSubsToDeregister) {
        mSubsToDeregister=false;
        mIsDeregistering=true;
        return startRegistrationProcessPrivate(0);  
    }
    return RV_OK;
}

void SipChannel::getPreferredIdentityPrivate(){

    if(mPreferredIdentity.empty()){
        std::string strExternalContactAddress;
        char strLocalContactAddress[MAX_LENGTH_OF_URL]="\0";

        strLocalContactAddress[sizeof(strLocalContactAddress)-1] = 0;

        getFromStrPrivate(getOurDomainStrPrivate(),strExternalContactAddress);
        getLocalContactAddressForGeneralRequestPrivate(strLocalContactAddress, sizeof(strLocalContactAddress)-1);

        if(mTelUriLocal) {
            if(!strExternalContactAddress.empty()) mPreferredIdentity = strExternalContactAddress;
            else if(strLocalContactAddress[0]) mPreferredIdentity = strLocalContactAddress;
        } else if(!strExternalContactAddress.empty()) {
            mPreferredIdentity = strExternalContactAddress;
        }
    }
}

RvStatus SipChannel::sessionSetLocalCallLegDataPrivate(std::string& strFrom)
{
    ENTER();

    RvStatus rv;

    rv = RvSipCallLegSetLocalAddress(mHCallLeg,
            mLocalAddress.eTransportType,
            mLocalAddress.eAddrType,
            mLocalAddress.strIP,
            mLocalAddress.port,
            mLocalAddress.if_name,
            mLocalAddress.vdevblock);
    if(rv<0) {
        RVSIPPRINTF("%s: error 111.333.000\n",__FUNCTION__);
        RETCODE(rv);
    }

    /*---------------------------------------
     * Form fields ...
     *---------------------------------------*/
    strFrom.clear();
    char strFromStr[MAX_LENGTH_OF_URL]="\0";
    std::string strExternalContactAddress;
    char strLocalContactAddress[MAX_LENGTH_OF_URL]="\0";

    strFromStr[sizeof(strFromStr) -1] = 0;
    strLocalContactAddress[sizeof(strLocalContactAddress)-1] = 0;
    getFromStrPrivate(getOurDomainStrPrivate(),strExternalContactAddress);

    getLocalContactAddressForGeneralRequestPrivate(strLocalContactAddress, sizeof(strLocalContactAddress)-1);

    strcpy(strFromStr,"From: ");
    if(mAnonymous) {
        strcpy(strFromStr+6,"\"Anonymous\" <sip:c8oqz84zk7z@privacy.org>");
    } else {
        strcpy(strFromStr+6,strExternalContactAddress.c_str());
    }

    strFrom=strFromStr;

    RVSIPPRINTF("%s: %s\n",__FUNCTION__,strFromStr);

    if(!mTelUriLocal || mAnonymous) {

        /*Handles to the contact addresses.*/
        RvSipAddressHandle hLocalContactAddress = NULL;

        //---------------------------------------
        //  Gets address handles from the call-leg.
        //  ---------------------------------------
        rv = RvSipCallLegGetNewMsgElementHandle(
                mHCallLeg, RVSIP_HEADERTYPE_UNDEFINED,
                RVSIP_ADDRTYPE_URL, (void **)&hLocalContactAddress);

        if(rv<0) {
            RVSIPPRINTF("%s: error 111.111\n",__FUNCTION__);
            RETCODE(rv);
        }

        //-----------------------------------------------------
        //  Fills the address handles with the contact information.
        //  -----------------------------------------------------
        rv = RvSipAddrParse (hLocalContactAddress,  strLocalContactAddress);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.222: %s\n",__FUNCTION__,strLocalContactAddress);
            RETCODE(rv);
        }

        //--------------------------------------------
        //  Sets the contact address to the call-leg.
        //  --------------------------------------------
        rv  = RvSipCallLegSetLocalContactAddress (mHCallLeg, hLocalContactAddress);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.333\n",__FUNCTION__);
            RETCODE(rv);
        }
    }

    if(mRole == ORIGINATE) {
        if(mTelUriLocal) {
            if(!strExternalContactAddress.empty()) mPreferredIdentity = strExternalContactAddress;
            else if(strLocalContactAddress[0]) mPreferredIdentity = strLocalContactAddress;
        } else if(!strExternalContactAddress.empty()) {
            mPreferredIdentity = strExternalContactAddress;
        }
    } else {
        mPreferredIdentity = "";
    }

    rv = setOutboundCallLegMsg();

    if(useAkaPrivate() && isOriginatePrivate()) {
        rv = RvSipUtils::RvSipAka::SIP_AkaInsertInitialAuthorizationToMessage(mHCallLeg, getAuthNamePrivate(), getAuthRealmPrivate());
    }

    RETCODE(rv);
}

RvStatus SipChannel::sessionSetRemoteCallLegDataPrivate(std::string& strTo) {

    ENTER();

    RvStatus rv=RV_OK;

    /*---------------------------------------
     * Form fields ...
     *---------------------------------------*/

    char strToStr[MAX_LENGTH_OF_URL];

    if(mTelUriRemote) {
        if(mUserNumberContextRemote.empty()) {
            snprintf(strToStr, sizeof(strToStr)-1, "To: \"%s\" <tel:+%s>",getDisplayNameRemotePrivate().c_str(),mUserNameRemote.c_str());
        } else {
            snprintf(strToStr, sizeof(strToStr)-1, "To: \"%s\" <tel:%s;phone-context=%s>",
                    getDisplayNameRemotePrivate().c_str(),mUserNameRemote.c_str(),mUserNumberContextRemote.c_str());
        }
    } else {
        snprintf(strToStr, sizeof(strToStr)-1, "To: \"%s\" <sip:%s@%s>",
                getDisplayNameRemotePrivate().c_str(),
                mUserNameRemote.c_str(),
                getRemoteDomainStrPrivate().c_str());
    }

    strTo = strToStr;

    if(!useProxyPrivate() && !isSecureReqURIPrivate()) {

        /*
           DO NOT REMOVE THIS SECTION !!!
           Strictly speaking, we do not need this section. Radvision stack would figure and handle the remote contact
           automatically. But this addition makes a huge performance difference in the back-to-back tests. 
           */

        /*Handles to the contact addresses.*/
        RvSipAddressHandle hRemoteContactAddress = NULL;
        RvChar strRemoteContactAddress[128];

        snprintf(strRemoteContactAddress, sizeof(strRemoteContactAddress)-1,"sip:%s@%s",mUserNameRemote.c_str(),getRemoteDomainStrPrivate().c_str());

        RVSIPPRINTF("%s: to <%s addr %s>\n",__FUNCTION__,strToStr,strRemoteContactAddress);

        /*---------------------------------------
          Gets address handles from the call-leg.
          ---------------------------------------*/
        rv = RvSipCallLegGetNewMsgElementHandle(
                mHCallLeg, RVSIP_HEADERTYPE_UNDEFINED,
                RVSIP_ADDRTYPE_URL, (void **)&hRemoteContactAddress);

        if(rv<0) {
            RVSIPPRINTF("%s: error 111.111\n",__FUNCTION__);
            RETCODE(rv);
        }

        /*-----------------------------------------------------
          Fills the address handles with the contact information.
          -----------------------------------------------------*/
        rv = RvSipAddrParse(hRemoteContactAddress, strRemoteContactAddress);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.222\n",__FUNCTION__);
            RETCODE(rv);
        }

        /*--------------------------------------------
          Sets the contact address to the call-leg.
          --------------------------------------------*/
        rv = RvSipCallLegSetRemoteContactAddress(mHCallLeg, hRemoteContactAddress);
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.333.111\n",__FUNCTION__);
            RETCODE(rv);
        }

    } else if(!useProxyDiscoveryPrivate()) {

        RvSipTransportOutboundProxyCfg outboundCfg;
        memset(&outboundCfg,0,sizeof(outboundCfg));

        outboundCfg.eTransport=getTransportPrivate();
        outboundCfg.eCompression = RVSIP_COMP_UNDEFINED;
        if(outboundCfg.eTransport == RVSIP_TRANSPORT_UNDEFINED) outboundCfg.eTransport=RVSIP_TRANSPORT_UDP;

        setOutboundCfgPrivate(false,outboundCfg);


        rv = RvSipCallLegSetOutboundDetails(mHCallLeg,&outboundCfg,sizeof(outboundCfg));
        if(rv<0) {
            RVSIPPRINTF("%s: error 111.333.111.111: rv=%d: ip=%s, port=%d\n",__FUNCTION__,rv,remoteAddr, remotePort);
            RETCODE(rv);
        } else {
            RVSIPPRINTF("%s: success 111.333.111.111: rv=%d: ip=%s, port=%d\n",__FUNCTION__,rv,remoteAddr, remotePort);
        }
    }

    if(rv>=0) {
        rv = addDynamicRoutes();
    }

    RETCODE(rv);
}

void SipChannel::clearRefreshTimerPrivate() {

    ENTER();

    RvStatus rv = RV_OK;

    if(mRefresherStarted) {

        if(mHCallLeg) {
            rv = RvSipCallLegSessionTimerStopTimer(mHCallLeg);
        }
        mRefresherStarted=false;
    }

    RET;
}
//////////////////////Timer Callback Mutex Lock Needed///////////////////////////

void SipChannel::regTimerExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHRegClient);

    mHRegTimer=0;

    mIsInRegTimer=true;
    startReregistrationPrivate();
    mIsInRegTimer=false;
}

void SipChannel::callTimerExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    mHCallTimer=0;

    if(mMediaHandler) {
        bool hold=false;
        mMediaHandler->media_onCallExpired(mHCallLeg, hold);
        if(!hold) {
            disconnectSession();
        }
    } else {
        disconnectSession();
    }
}
void SipChannel::callTimerMediaExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    mHCallTimerMedia=0;

    disconnectSession();
}

void SipChannel::byeAcceptTimerMediaExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    mHByeAcceptTimerMedia=0;

    if(mHCallLeg && mHByeTranscPending) {
        RvSipCallLegByeAccept(mHCallLeg,mHByeTranscPending);
    }

    mHByeTranscPending=0;
}

void SipChannel::postCallTimerExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    callCompletionNotifyPrivate();
}

void SipChannel::ringTimerExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    mHRingTimer=0;

    if(mReadyToAccept) {
        acceptPrivate();
    }
}

void SipChannel::prackTimerExpirationEventCB() {

    RVSIPLOCK(mLocker);

    RVSIPPRINTF("%s: cl=0x%x\n",__FUNCTION__,(unsigned int)mHCallLeg);

    mHPrackTimer=0;

    disconnectSession();
}



/********************************************************************************/

/*===================================================================================*/
