/// @file
/// @brief SIP channel timer header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPCHANNELTIMER_H_
#define SIPCHANNELTIMER_H_

#include <list>
#include <set>

#include "RvSipHeaders.h"
#include "RvSipUtils.h"

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

class SipChannelTimer;

class SipChannelTimerContainerInterface {
 public: 
  SipChannelTimerContainerInterface() {}
  virtual ~SipChannelTimerContainerInterface() {}
  virtual void returnTimer(SipChannelTimer* timer) = 0;
};

typedef void (*ExpirationCB)(void* context);

static void _commodSCT_CB(void *context);
    
class SipChannelTimer {

 private:

  SipChannelTimer(const SipChannelTimer& other);
  const SipChannelTimer& operator=(const SipChannelTimer& other) {
    return *this;
  }

 public:

  SipChannelTimer(const RvSipUtils::Locker &locker, const RvSipMidMgrHandle& hMid, SipChannelTimerContainerInterface *c) : 
    mLocker(locker),hMidMgr(hMid),container(c) {
    ENTER();
    active=false;
    context=NULL;
    testContext1=NULL;
    testContextAddr1=NULL;
    testContext2=NULL;
    testContextAddr2=NULL;
    cb=NULL;
    handle=NULL;
    RET;
  }

  void load(void* context, ExpirationCB cb, uint32_t interval, void** testContextAddr1, void** testContextAddr2) {
    ENTER();
    if(cb && !active) {
      RVSIPLOCK(mLocker);
      RvSipMidTimerHandle handle=0;
      RvStatus rv = RvSipMidTimerSet(hMidMgr,
				     interval,
				     _commodSCT_CB,
				     this,
				     &handle);
      if(rv>=0 && handle) {
	this->cb=cb;
	this->context=context;
	this->testContextAddr1=testContextAddr1;
	if(this->testContextAddr1) {
	  this->testContext1=*(this->testContextAddr1);
	} else {
	  this->testContext1=NULL;
	}
	this->testContextAddr2=testContextAddr2;
	if(this->testContextAddr2) {
	  this->testContext2=*(this->testContextAddr2);
	} else {
	  this->testContext2=NULL;
	}
	this->handle=handle;
	this->active=true;
      } else {
	handle=0;
	RVSIPPRINTF("%s: cannot set timer: %d: hMidMgr=0x%x, _commodSCT_CB=0x%x, h=0x%x\n",__FUNCTION__,rv,(unsigned int)hMidMgr,(unsigned int)_commodSCT_CB,(unsigned int)&handle);
      }
    }
    RET;
  }

  void fire() {
    ENTER();
    RVSIPLOCK(mLocker);
    if(active && cb) {
      handle=NULL;
      active=false;
      bool valid=true;
      if(testContextAddr1) {
	if(*testContextAddr1 != testContext1) {
	  valid=false;
	}
      }
      if(testContextAddr2) {
	if(*testContextAddr2 != testContext2) {
	  valid=false;
	}
      }
      if(valid) {
	cb(context);
      }
      cb=NULL;
      context=NULL;
      testContextAddr1=NULL;
      testContextAddr2=NULL;
      testContext1=NULL;
      testContext2=NULL;
    }
    container->returnTimer(this);
    RET;
  }

  void unload() {
    ENTER();
    RVSIPLOCK(mLocker);
    active=false;
    handle=NULL;
    context=NULL;
    testContextAddr1=NULL;
    testContextAddr2=NULL;
    testContext1=NULL;
    testContext2=NULL;
    cb=NULL;
    RET;
  }

  void setActive(bool value) {
    active=value;
  }

 private:
  const RvSipUtils::Locker &mLocker;
  RvSipMidMgrHandle hMidMgr;
  SipChannelTimerContainerInterface *container;
  RvSipMidTimerHandle handle;
  ExpirationCB cb;
  void* context;
  void** testContextAddr1;
  void** testContextAddr2;
  void* testContext1;
  void* testContext2;
  bool active;
};

void _commodSCT_CB(void *context) {
  if(context) {
    SipChannelTimer* timer=(SipChannelTimer*)context;
    timer->fire();
  }
}

class SipChannelTimerContainer : public SipChannelTimerContainerInterface {

 private:

  static const size_t BUFFER_SIZE=10;

  typedef std::list<SipChannelTimer*> acontainer;
  typedef std::set<SipChannelTimer*> bcontainer;

 public:

  SipChannelTimerContainer(const RvSipUtils::Locker &locker) : mLocker(locker) {}

  void set(const RvSipMidMgrHandle& hMidMgr) {
    ENTER();
    RVSIPLOCK(mLocker);
    this->hMidMgr=hMidMgr;
    while(available.size()<BUFFER_SIZE) {
      available.push_back(new SipChannelTimer(mLocker,hMidMgr,this));
    }
    RET;
  }

  virtual ~SipChannelTimerContainer() {

    ENTER();

    RVSIPLOCK(mLocker);

    {
      acontainer::iterator iter=available.begin();
      while(iter!=available.end()) {
	SipChannelTimer* timer=*iter;
	if(timer) delete timer;
	++iter;
      }
      available.clear();
    }
    {
      bcontainer::iterator iter=busy.begin();
      while(iter!=busy.end()) {
	SipChannelTimer* timer=*iter;
	if(timer) delete timer;
	++iter;
      }
      busy.clear();
    }

    RET;
  }

  SipChannelTimer* getTimer() {

    ENTER();

    RVSIPLOCK(mLocker);

    SipChannelTimer* timer=NULL;

    if(available.empty()) {
      while(available.size()<BUFFER_SIZE) {
	available.push_back(new SipChannelTimer(mLocker,hMidMgr,this));
      }
    }

    timer=available.front();
    available.pop_front();

    busy.insert(timer);

    RETCODE(timer);
  }
  
  virtual void returnTimer(SipChannelTimer* timer) {

    ENTER();

    if(timer) {

      RVSIPLOCK(mLocker);

      if(busy.count(timer)>0) {
	busy.erase(timer);
	available.push_back(timer);
      }

      timer=NULL;
    }

    RET;
  }

 private:
  const RvSipUtils::Locker &mLocker;
  RvSipMidMgrHandle hMidMgr;
  acontainer available;
  bcontainer busy;
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*SIPCHANNELTIMER_H_*/
