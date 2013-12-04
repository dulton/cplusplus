#include "AbstTimerManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class MockTimerMgr : public AbstTimerManager
{
  public:
    MockTimerMgr() :mNextHandle(1) {}

    virtual ~MockTimerMgr() {}

    struct TimerConfig
    {
        timerDelegate_t delegate;
        uint32_t msDelay;
    };

    handle_t CreateTimer(timerDelegate_t delegate, uint32_t msDelay)
    {
        TimerConfig config;
        config.delegate = delegate;
        config.msDelay = msDelay;

        mTimers[mNextHandle] = config;

        return mNextHandle++;
    }

    void CancelTimer(handle_t handle)
    {
        mCancelledTimers.insert(handle);
    }

    uint64_t GetTimeOfDayMsec() const
    {
        return 0;
    }

    // Mock accessors
    
    typedef std::map<handle_t, TimerConfig> timerMap_t;
    typedef std::set<handle_t> timerSet_t;

    timerMap_t& GetTimers() 
    { 
        return mTimers; 
    } 

    timerSet_t& GetCancelledTimers()
    {
        return mCancelledTimers;
    }
     
  private:
    timerMap_t mTimers;
    timerSet_t mCancelledTimers;
    handle_t mNextHandle;
};

