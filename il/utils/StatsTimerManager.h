#ifndef _L4L7_UTILS_STATS_TIMER_MANAGER_H_
#define _L4L7_UTILS_STATS_TIMER_MANAGER_H_

#include <boost/utility.hpp>
#include <ace/Reactor.h>
#include <utils/UtilsCommon.h>

DECL_L4L7_UTILS_NS

///////////////////////////////////////////////////////////////////////////////
class StatsTimerManager : public ACE_Event_Handler
{
public:
	StatsTimerManager(): mAppReactor(0), mTimerId(INVALID_TIMER_ID)
	{ 
	} 

	virtual ~StatsTimerManager()
	{
		CancelTimer();
	}

	/// @brief Sync stats
	virtual void Sync(bool zeroRates) = 0;

	/// @brief Start stats timer    
	virtual void CreateTimer() 
	{
		if (mAppReactor && mTimerId == INVALID_TIMER_ID) {
			// Start stats timer
			ACE_Time_Value initialDelay(0);
			ACE_Time_Value interval(1);
			mTimerId = mAppReactor->schedule_timer(this,NULL,initialDelay,interval);
		}
	}

	/// @brief Start stats timer    
    void SetReactorInstance(ACE_Reactor * appReactor) 
	{ 		
		mAppReactor = appReactor;
	}

	/// @brief Start stats timer    
    virtual void CancelTimer()
	{
		if (mTimerId != INVALID_TIMER_ID && mAppReactor)
		{
			int cancelled = mAppReactor->cancel_timer(mTimerId);
			if (cancelled != -1)
				mTimerId = INVALID_TIMER_ID;

			mAppReactor->remove_handler(this, ACE_Event_Handler::TIMER_MASK | ACE_Event_Handler::DONT_CALL);
		}
	}

	/// @brief Implement ACE_Event_Handler handle_timeout hook
	virtual int handle_timeout(const ACE_Time_Value &current_time, const void *arg) 
	{	
		Sync(false);  
		return 0;
	}

private:
	ACE_Reactor * mAppReactor;            ///< application reactor instance
	long mTimerId;
	static const long INVALID_TIMER_ID = -1;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_UTILS_NS

#endif

