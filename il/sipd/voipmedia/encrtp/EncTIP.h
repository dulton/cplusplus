#ifndef __ENCTIP_H__
#define __ENCTIP_H__

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <ace/Reactor.h>

#include "EncRTPRvInc.h"
#include "EncRTPCommon.h"
#include "EncTIPCommon.h"

#include "tip_debug_print.h"
#include "tip.h"
#include "tip_profile.h"


BEGIN_DECL_ENC_TIP_NAMESPACE

using namespace LibTip;
class EncTIPImpl;
class EncTIP;
typedef EncTIP* EncTipHandle;
typedef boost::function4<int,TIPMediaType,TIPNegoStatus,EncTipHandle,void *> TipStatusDelegate_t;



class EncTIP : public boost::noncopyable {
    public:
        EncTIP();
        ~EncTIP();
        void doPeriodicActivity(void);
        int startNegotiate(ACE_Reactor *reactor);
        int stopNegotiate(void);
        int receivePacket(uint8_t* userData,uint32_t userDataLen);
        void setTipCB(TipStatusDelegate_t cb);
        void setupTip(TIPMediaType type,int rtcpSock,ACE_INET_Addr *remoteAddress);
        uint64_t getDelay(void);
        bool isActive(void);
        ACE_Reactor* reactor(void){return mReactor;}

    private:
        boost::scoped_ptr<EncTIPImpl> mPimpl;
        ACE_Reactor *mReactor;

};


typedef boost::shared_ptr<EncTIP> TIPHandler;


END_DECL_ENC_TIP_NAMESPACE

USING_ENC_TIP_NAMESPACE;



#endif
