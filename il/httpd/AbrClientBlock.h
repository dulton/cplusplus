/// @file
/// @brief Abr Client Block header file
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ABR_CLIENT_BLOCK_H_
#define _ABR_CLIENT_BLOCK_H_


#include <base/RateBasedLoadStrategy.h>
#include <base/StaticLoadStrategy.h>

#include "HttpClientBlock.h"
//#include "abr_apple/src/my_player.h"
#include "AbrPlayer.h"

namespace L4L7_APP_NS
{
    template<class, class> class AsyncMessageInterface;

}

namespace L4L7_BASE_NS
{
    class EndpointPairEnumerator;
    class LoadProfile;
    class LoadScheduler;
    class LoadStrategy;
}

DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

class AbrClientBlock : public GenericAbrObserver, public ClientBlock, L4L7_APP_NS::BandwidthLoadManager, boost::noncopyable
{
  public:


    AbrClientBlock(uint16_t port, const config_t& config, ACE_Reactor *appReactor, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier);
    ~AbrClientBlock();

    /// Handle accessors
     uint32_t BllHandle(void) const { return mConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle; }
     uint32_t IfHandle(void) const { return mConfig.Common.Endpoint.SrcIfHandle; }

  	void NotifyInterfaceDisabled(void);
  	void NotifyInterfaceEnabled(void);
      // Load profile notifier methods
    void RegisterLoadProfileNotifier(loadProfileNotifier_t notifier) { mLoadProfileNotifier = notifier; }
    void UnregisterLoadProfileNotifier(void) { mLoadProfileNotifier.clear(); }

    void SetDynamicLoad(int32_t load){};

  	stats_t& Stats(void) { return mStats; }
    videoStats_t& VideoStats(void) { return mVideoStats; };
    void Start(void);
    void PreStop(void);
    void Stop(void);
    bool VideoEnabled(void){ return mConfig.ProtocolProfile.EnableVideoClient; };

    void NotifyStatusChanged(int uID, int uStatus);

  private:
   class LoadStrategy;
   class SessionsLoadStrategy;
   class SessionsOverTimeLoadStrategy;

   struct IOLogicMessage;
   struct AppLogicMessage;


   typedef L4L7_BASE_NS::LoadProfile loadProfile_t;
   typedef L4L7_BASE_NS::LoadScheduler loadScheduler_t;
   typedef L4L7_BASE_NS::EndpointPairEnumerator endpointEnum_t;
   typedef L4L7_UTILS_NS::TimingPredicate<5> MsgLimit_t; // 5 msec
   typedef L4L7_APP_NS::AsyncMessageInterface<IOLogicMessage, MsgLimit_t> ioMessageInterface_t;
   typedef L4L7_APP_NS::AsyncMessageInterface<AppLogicMessage, MsgLimit_t> appMessageInterface_t;
   typedef ACE_Recursive_Thread_Mutex sessionLock_t;
   /// Factory methods
    LoadStrategy *MakeLoadStrategy(void);
    /// Load strategy convenience methods
    void SetIntendedLoad(uint32_t intendedLoad);
    void SetAvailableLoad(uint32_t availableLoad, uint32_t intendedLoad);

    /// Private message handlers
    void HandleIOLogicMessage(const IOLogicMessage& msg);
    void SpawnSessions(size_t count);
    void ReapSessions(size_t count);
    void HandleAppLogicMessage(const AppLogicMessage& msg);

    // Load profile notification handler
    void LoadProfileHandler(bool running);
    void ForwardStopNotification();

    const uint16_t mPort;                                       ///< physical port number
  	stats_t mStats;                                             ///< client stats
  	videoStats_t mVideoStats;                                             ///< client stats
    loadProfileNotifier_t mLoadProfileNotifier;                 ///< app logic: owner's load profile notifier delegate

    ACE_Reactor * const mAppReactor;                            ///< application reactor instance
    ACE_Reactor * const mIOReactor;                             ///< I/O reactor instance
    ACE_Lock * const mIOBarrier;                                ///< I/O barrier instance


    bool mEnabled;
    bool mRunning;                                              ///< app logic: true when running

    boost::scoped_ptr<loadProfile_t> mLoadProfile;              ///< app logic: load profile
    std::tr1::shared_ptr<LoadStrategy> mLoadStrategy;           ///< app logic: load strategy
    std::tr1::shared_ptr<loadScheduler_t> mLoadScheduler;       ///< app logic: load scheduler
  //  loadProfileNotifier_t mLoadProfileNotifier;                 ///< app logic: owner's load profile notifier delegate


    size_t mAttemptedSessionCount;                                 ///< I/O logic: number of attempted sessions
    size_t mActiveSessionCount;                                    ///< I/O logic: number of active session
    boost::scoped_ptr<endpointEnum_t> mEndpointEnum;               ///< I/O logic: endpoint enumerator
    sessionLock_t mSessionLock;                                       ///< I/O logic: protects session against concurrent access
    boost::scoped_ptr<ioMessageInterface_t> mIOLogicInterface;  ///< app logic -> I/O logic
    boost::scoped_ptr<appMessageInterface_t> mAppLogicInterface;///< I/O logic -> app logic

    uint32_t mAvailableLoadMsgsOut;
    bool mStoringStopNotification;
    int mPlayerSerial;
    map<int, GenericAbrPlayer*> mAbrPlayerMap;

};

class AbrClientBlock::LoadStrategy : virtual public L4L7_BASE_NS::LoadStrategy
{
  public:
    LoadStrategy(AbrClientBlock& clientBlock)
        : mClientBlock(clientBlock)
    {
    }

    // Connection events
    virtual void SessionClosed(void) { }

  protected:
    AbrClientBlock& mClientBlock;
};

///////////////////////////////////////////////////////////////////////////////

class AbrClientBlock::SessionsLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::StaticLoadStrategy
{
  public:
    SessionsLoadStrategy(AbrClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void SessionClosed(void);

  private:
    void LoadChangeHook(int32_t load);
};

///////////////////////////////////////////////////////////////////////////////

class AbrClientBlock::SessionsOverTimeLoadStrategy : public LoadStrategy, public L4L7_BASE_NS::RateBasedLoadStrategy
{
  public:
    SessionsOverTimeLoadStrategy(AbrClientBlock& clientBlock)
        : LoadStrategy(clientBlock)
    {
    }

    void SessionClosed(void);

  private:
    void LoadChangeHook(double load);
};


END_DECL_HTTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif
