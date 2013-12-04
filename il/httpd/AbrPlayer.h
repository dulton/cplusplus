#ifndef __ABR_PLAYER_H
#define __ABR_PLAYER_H

#include <string>
#include <ace/Reactor.h>


#include "abr_apple/src/abr_interfaces.h"

using namespace std;

struct GenericAbrObserver
{
    enum
    {
        STATUS_REPORT,
        STATUS_ABORTED,
        STATUS_STOPPED

    };

    virtual void NotifyStatusChanged(int uID, int uStatus) = 0;
    virtual ~GenericAbrObserver(){};
};

struct GenericAbrPlayer: virtual PlayerInterface, virtual ConfigurationInterface, virtual StatusInterface, virtual GenericAbrObserver
{
    ////////////////////////////////////////////////////////////////////////////////
protected:
    GenericAbrObserver* mGenericAbrObeserver;
    uint8_t mIpv4Tos;
    uint8_t mIpv6TrafficClass;
    uint32_t mTcpWindowSizeLimit;
    bool mTcpDelayedAck;
    string mIfName;
    ACE_INET_Addr mSrcAddr;

public:
    GenericAbrPlayer()
    {
        mGenericAbrObeserver = this;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void  SetSourceAddr(ACE_INET_Addr uSrcAddr)
    {
        mSrcAddr = uSrcAddr;
    }

    ////////////////////////////////////////////////////////////////////////////////
    virtual void ReportToWithId(GenericAbrObserver* uGenericAbrObeserver, int uID)=0;

    ////////////////////////////////////////////////////////////////////////////////
    void SetIpv4Tos(uint8_t tos)
    {
        mIpv4Tos = tos;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void SetIpv6TrafficClass(uint8_t trafficClass)
    {
        mIpv6TrafficClass = trafficClass;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void SetTcpWindowSizeLimit(uint32_t tcpWindowSizeLimit)
    {
        mTcpWindowSizeLimit = tcpWindowSizeLimit;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void SetTcpDelayedAck(bool delayedAck)
    {
        mTcpDelayedAck = delayedAck;
    }

    ////////////////////////////////////////////////////////////////////////////////
    void BindToIfName(string& ifName)
    {
        mIfName = ifName;
    }

    ////////////////////////////////////////////////////////////////////////////////
    virtual ~GenericAbrPlayer()
    {
    }
};

GenericAbrPlayer* CreateAppleAbrPlayer(ACE_Reactor *ioReactor, int flags);


#endif

