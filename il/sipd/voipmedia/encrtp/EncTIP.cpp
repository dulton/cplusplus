#include <sys/types.h>
#include <sys/socket.h>
#include <ace/Recursive_Thread_Mutex.h>
#include "EncTIP.h"
#include "VoIPCommon.h"


BEGIN_DECL_ENC_TIP_NAMESPACE


#define ENCTIP_LOOP_INTERVAL_MAX 100//ms;
#define ENCTIP_LOOP_INTERVAL_MIN 1//ms;
#define TIP_V6 LibTip::SUPPORTED_VERSION_MIN
#define TIP_V7 LibTip::SUPPORTED_VERSION_MAX

typedef LibTip::Status      EncTIPStatus;
typedef LibTip::MediaType   EncTIPMediaType;
typedef enum CTIPMediaType{
    CTIPMediaAudio= LibTip::AUDIO,
    CTIPMediaVideo = LibTip::VIDEO
};
typedef enum CTIPStatus{
    CTIPStatusOK= LibTip::TIP_OK,
    CTIPStatusERROR= LibTip::TIP_ERROR
};

typedef enum ENCTIP_Profile_Type{
    SINGLE_SCREEN=1,
    SINGLE_SCREEN_EXT,
    TRIPLE_SCREEN
};
EncTIPMediaType _mediaTypeMap(TIPMediaType type){
    switch(type){
        case TIPMediaAudio:
            return static_cast<EncTIPMediaType>(CTIPMediaAudio);
        case TIPMediaVideo:
            return static_cast<EncTIPMediaType>(CTIPMediaVideo);
        default:
            return static_cast<EncTIPMediaType>(CTIPMediaAudio);
    }
}
TIPMediaType _mediaTypeMap(EncTIPMediaType type){
    if(static_cast<CTIPMediaType>(type) == CTIPMediaVideo)
        return TIPMediaVideo;
    else
        return TIPMediaAudio;
}

class EncTIPPacketXmit : public LibTip::CTipPacketTransmit{
    public:
        EncTIPPacketXmit(){
            mRtcpSock = -1;
            mIsIpv6 = false;
            memset(&mPeer,0,sizeof(mPeer));
        }
        void setSocket(int sock){mRtcpSock = sock;}
        int getSocket(void){return mRtcpSock;}
        void setPeerInfo(ACE_INET_Addr *peer){
            mIsIpv6 = false;
            if(peer){
                char ipstr[128];
                mPeer = *peer;
                mPeer.get_host_addr(ipstr,sizeof(ipstr)-1);
                if(strstr(ipstr,":")){
                    mIsIpv6 = true;
                    memset((char *)&mRemoteAdddrIpv6, 0, sizeof(mRemoteAdddrIpv6));
                    mRemoteAdddrIpv6.sin6_family = AF_INET6;
                    mRemoteAdddrIpv6.sin6_port = htons(mPeer.get_port_number()+1);
                    inet_pton(AF_INET6,ipstr,&mRemoteAdddrIpv6.sin6_addr);
                }else{
                    memset((char *)&mRemoteAdddr, 0, sizeof(mRemoteAdddr));
                    mRemoteAdddr.sin_family = AF_INET;
                    mRemoteAdddr.sin_port = htons(mPeer.get_port_number()+1);
                    inet_pton(AF_INET,ipstr,&mRemoteAdddr.sin_addr);
                }
            }
            //TODO valid address
        }
        ~EncTIPPacketXmit(){}
    EncTIPStatus Transmit(const uint8_t* pktBuffer, uint32_t pktSize,
            EncTIPMediaType type){
        int ret = -1;
        if(mRtcpSock != -1){
            if(mIsIpv6)
                ret = sendto(mRtcpSock,(char *)pktBuffer,pktSize,0,(struct sockaddr *)&mRemoteAdddrIpv6,sizeof(mRemoteAdddrIpv6));
            else
                ret = sendto(mRtcpSock,(char *)pktBuffer,pktSize,0,(struct sockaddr *)&mRemoteAdddr,sizeof(mRemoteAdddr));
            if(ret != (int) pktSize){
                return static_cast<EncTIPStatus>(CTIPStatusERROR);
            }else 
                return static_cast<EncTIPStatus>(CTIPStatusOK);

        }else{
            return static_cast<EncTIPStatus>(CTIPStatusERROR);
        }
        return static_cast<EncTIPStatus>(CTIPStatusOK);
    }
    private:
    int mRtcpSock;
    ACE_INET_Addr mPeer;
    struct sockaddr_in mRemoteAdddr;
    struct sockaddr_in6 mRemoteAdddrIpv6;
    bool mIsIpv6;

};

class EncTIPCallback : public LibTip::CTipCallback{
    //TODO change delegate interface --> int (int,bool,void*).
    public:
        EncTIPCallback(): mTipHandle(0), mUpdate(0){}
        virtual void TipNegotiationEarly(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_EARLY,mTipHandle,NULL);
        }
        virtual void TipNegotiationFailed(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_FAILED,mTipHandle,NULL);
        }
        virtual void TipNegotiationMismatch(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_MISMATCH,mTipHandle,NULL);
        }
        virtual void TipNegotiationIncompatible(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_INCOMPATIBLE,mTipHandle,NULL);
        }
        virtual void TipNegotiationLastAckReceived(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_LASTACKRCVD,mTipHandle,NULL);
        }
        virtual bool TipNegotiationLastAckTransmit(EncTIPMediaType type,bool doReinvite,void* id){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_LASTACKXMIT,mTipHandle,(void *)(doReinvite?1:0));
            return true;//FIXME return val
        }
        virtual void TipNegotiationUpdate(EncTIPMediaType type){
            if(mUpdate)
                mUpdate(_mediaTypeMap(type),TIP_STATUS_UPDATE,mTipHandle,NULL);
        }
        virtual void LocalPresentationStart(uint8_t position,LibTip::CTipSystem::PresentationStreamFrameRate fps){
            if(mUpdate)
                mUpdate(TIPMediaAudio,TIP_STATUS_LOCAL_PRESENTATION_START,mTipHandle,NULL);
        }
        virtual void LocalPresentationStop(){
            if(mUpdate)
                mUpdate(TIPMediaAudio,TIP_STATUS_LOCAL_PRESENTATION_STOP,mTipHandle,NULL);
        }
        virtual void RemotePresentationStart(uint8_t position,LibTip::CTipSystem::PresentationStreamFrameRate fps){
            if(mUpdate)
                mUpdate(TIPMediaAudio,TIP_STATUS_REMOTE_PRESENTATION_START,mTipHandle,NULL);
        }
        virtual void RemotePresentationStop(){
            if(mUpdate)
                mUpdate(TIPMediaAudio,TIP_STATUS_REMOTE_PRESENTATION_STOP,mTipHandle,NULL);
        }
        virtual bool TipSecurityKeyUpdate(EncTIPMediaType type, uint16_t spi, 
                const uint8_t* salt, const uint8_t* kek, 
                void* id)
        {
            //TODO
            return true;
        }
        virtual void TipSecurityStateUpdate(LibTip::MediaType mType, bool secure)
        {
            //TODO
        }
    void setCB(TipStatusDelegate_t update,EncTipHandle handle){
        mUpdate = update;
        mTipHandle = handle;
    }
    private:
        EncTipHandle mTipHandle;
        TipStatusDelegate_t mUpdate;

};


class EncTIPImpl : public boost::noncopyable{

    typedef boost::shared_ptr<LibTip::CTip> CTipHandler;
    typedef EncTIPCallback* CTipCBHandler;
    typedef boost::shared_ptr<EncTIPPacketXmit> CTipXmitHandler;
    typedef ACE_Recursive_Thread_Mutex MUTEX;

#define TipGuard(A) ACE_Guard<ACE_Recursive_Thread_Mutex> __lock__##__LINE__(A)

    public:
    EncTIPImpl():mActivated(false){
        mXmitHandler.reset(new EncTIPPacketXmit());
        mHandler.reset(new LibTip::CTip(*(mXmitHandler.get())));
        //FIXME profile should be configurable
        LibTip::CSingleScreenProfile::Configure(mHandler->GetTipSystem(),false,false);
        mTipCallback = new EncTIPCallback();
        mHandler->SetCallback(mTipCallback);
        mHandler->GetTipSystem().SetTipVersion((LibTip::ProtocolVersion)TIP_V7);
        // CR#318653526
        CTipAudioMediaOption opt(LibTip::CTipAudioMediaOption::CAPABLE_G722_LEGACY, LibTip::CTipMediaOption::OPTION_NOT_SUPPORTED);
        mHandler->GetTipSystem().AddMediaOption(opt);
    }
    virtual ~EncTIPImpl(){
        //TODO
    }
    int startNegotiate(void){
        int  ret = 0;
        TipGuard(mLock);
        ret = mHandler->StartTipNegotiate(_mediaTypeMap(mType));
        mActivated = true;
        return ret;
    }
    int stopNegotiate(void){
        int ret = 0;
        TipGuard(mLock);
        ret = mHandler->StopTipNegotiate(_mediaTypeMap(mType));
        mActivated = false;
        return ret;
    }
    void doPeriodicActivity(void){
        TipGuard(mLock);
        mHandler->DoPeriodicActivity();
    }
    int receivePacket(uint8_t* data,uint32_t len){
        int ret = 0;
        TipGuard(mLock);

        if(mActivated)
            ret = mHandler->ReceivePacket(data,len,_mediaTypeMap(mType));
        return ret;
    }
    void setTipCB(TipStatusDelegate_t cb,EncTipHandle handle){
        TipGuard(mLock);
        if(mTipCallback)
            mTipCallback->setCB(cb,handle);
    }
    void setupTip(TIPMediaType type,int rtcpSock,ACE_INET_Addr *remoteAddress){
        TipGuard(mLock);
        mType = type;
        mXmitHandler->setSocket(rtcpSock);
        mXmitHandler->setPeerInfo(remoteAddress);
    }
    uint64_t getDelay(void){
        TipGuard(mLock);
        uint64_t delay = mHandler->GetIdleTime();
        if(delay == (uint64_t)-1)
            delay = ENCTIP_LOOP_INTERVAL_MAX;
        if(delay == 0)
            delay = ENCTIP_LOOP_INTERVAL_MIN;
        return delay;
    }
    bool isActive(void){
        TipGuard(mLock);
        return mActivated;
    }

    private:
    CTipHandler mHandler;
    CTipCBHandler mTipCallback;
    CTipXmitHandler mXmitHandler;

    TIPMediaType mType;
    MUTEX mLock;
    bool mActivated;
};

EncTIP::EncTIP(){
    mPimpl.reset(new EncTIPImpl());
}
EncTIP::~EncTIP(){
    //TODO
}
void EncTIP::doPeriodicActivity(void){
    mPimpl->doPeriodicActivity();
}
int EncTIP::startNegotiate(ACE_Reactor *reactor){
    mReactor = reactor;
    return mPimpl->startNegotiate();
}
int EncTIP::stopNegotiate(void){
    return mPimpl->stopNegotiate();
}
int EncTIP::receivePacket(uint8_t* userData,uint32_t userDataLen){
    return mPimpl->receivePacket(userData,userDataLen);
}

void EncTIP::setTipCB(TipStatusDelegate_t cb){
    mPimpl->setTipCB(cb,this);
}
void EncTIP::setupTip(TIPMediaType type,int rtcpSock,ACE_INET_Addr *remoteAddress){
    mPimpl->setupTip(type,rtcpSock,remoteAddress);
}
uint64_t EncTIP::getDelay(void){
    return mPimpl->getDelay();
}
bool EncTIP::isActive(void){
    return mPimpl->isActive();
}
END_DECL_ENC_TIP_NAMESPACE


