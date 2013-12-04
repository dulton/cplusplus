/// @file
/// @brief Enc RTP Media Process object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#if defined YOCTO_I686
#include <time.h>
#else
#include <posix_time.h>
#endif

#include <algorithm>
#include <string>
#include <cstring>
#include <iterator>
#include <sstream>
#include <list>
#include <set>

using std::string;

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <ace/INET_Addr.h>
#include <ace/Reactor.h>
#include <ace/Semaphore.h>
#include <ace/SOCK_Dgram.h>

#include <pthreadwrapper.h>

#include "VoIPUtils.h"
#include "EncRTPRvInc.h"
#include "EncRTPUtils.h"
#include "EncRTPMediaEndpoint.h"
#include "EncRTPProcess.h"
#include "VQStream.h"
#include "MediaPacket.h"

///////////////////////////////////////////////////////////////////////////////

#if !defined(spt_pthread_setcancelstate)
#define spt_pthread_setcancelstate pthread_setcancelstate
#endif

///////////////////////////////////////////////////////////////////////////////

USING_MEDIAFILES_NS;
USING_VQSTREAM_NS;

DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

class EncRTPRadvisionServiceProcess;

class EncRTPRadvisionService {
 public:
  typedef boost::function0<int> CommandType;
  EncRTPRadvisionService();
  virtual ~EncRTPRadvisionService();
  int call_command(CommandType command);
 private:
  static EncRTPRadvisionServiceProcess mProcess;
};

///////////////////////////////////////////////////////////////////////////////

class EncRTPRadvisionServiceProcess : public pthreadwrapper, public boost::noncopyable {

protected:

  typedef EncRTPRadvisionService::CommandType CommandType;
  
  class TaskToken : public boost::noncopyable {
  public:
    explicit TaskToken(CommandType command) : mCommand(command), mNotifier(0), mResult(0) {}
    void call() {
      if(mCommand) {
	mResult=mCommand();
      }
      mNotifier.release();
    }
    int waitForResult() {
      mNotifier.acquire();
      return mResult;
    }
  private:
    CommandType mCommand;
    ACE_Semaphore mNotifier;
    int mResult;
  };

  typedef boost::shared_ptr<TaskToken> TaskTokenHandler;
  typedef std::list<TaskTokenHandler> CommandContainerType;

public:
 
  EncRTPRadvisionServiceProcess() : 
    mProcess_active(false), mProcess_done(true)
  {
    Start(0);
  }
  
  virtual ~EncRTPRadvisionServiceProcess() {
    this->EncRTPRadvisionServiceProcess::Stop();
  }
  
  void Stop(void) {
    while(true) {
      {
	EncRTPGuard(mMutex);
	if(mProcess_done) break;
	mProcess_active=false;
      }
      VoIPUtils::mksSleep(1000);
    }
  }

  int call_command(CommandType command) {
    TaskTokenHandler token(new TaskToken(command));
    {
      EncRTPGuard(mMutex);
      mCommandQueue.push_back(token);
    }
    return token->waitForResult();
  }
  
protected:

  int run_commands() {
    int ret=0;
    while(true) {
      TaskTokenHandler token;
      {
	EncRTPGuard(mMutex);
	if(mCommandQueue.empty()) {
	  break;
	} else {
	  token=mCommandQueue.front();
	  mCommandQueue.pop_front();
	}
      }
      token->call();
      ret++;
    }
    return ret;
  }

  virtual void * Execute(void* arg) {

    {
      EncRTPGuard(mMutex);
      mProcess_done=false;
    }

    spt_pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
    Detach();
    VoIPUtils::ExtendsFdLimitation(0);

    {
      int ret=0;
      if((ret=RvRtpInit())<0) {
	TC_LOG_ERR_LOCAL(0, LOG_ENCRTP, "RvRtpInit failed");
      }
      if((ret=RvRtcpInit())<0) {
	TC_LOG_ERR_LOCAL(0, LOG_ENCRTP, "RvRtcpInit failed");
      }
    }

    while (mProcess_active) {

        run_commands();

        if(RvRtcpSeliSelectUntilPrivate(0)<0) {
            TC_LOG_ERR_LOCAL(0, LOG_ENCRTP, "SeliSelect failed");
        }

        VoIPUtils::mksSleep(EncRTPOptimizationParams::ProcessIdleDurationInMks);
    }

    {
      run_commands();
      
      if(RvRtcpSeliSelectUntilPrivate(0)<0) {
	TC_LOG_ERR_LOCAL(0, LOG_ENCRTP, "SeliSelect failed");
      }

      EncRTPGuard(mMutex);

      mProcess_active=false;
      mProcess_done=true; 
    }

    run_commands();

    RvRtcpEnd();
    RvRtpEnd();

    return arg;
  }
  
  virtual void Setup()
  {
    EncRTPGuard(mMutex);
    mProcess_active=true;    
    this->pthreadwrapper::Setup();
  }
  
private:
  mutable EncRTPUtils::MutexType mMutex;

  bool mProcess_active;
  bool mProcess_done;

  CommandContainerType mCommandQueue;
};

EncRTPRadvisionServiceProcess EncRTPRadvisionService::mProcess;

EncRTPRadvisionService::EncRTPRadvisionService() {
  ;
}

EncRTPRadvisionService::~EncRTPRadvisionService() {
}

int EncRTPRadvisionService::call_command(CommandType command) {
  return mProcess.call_command(command);
}

//////////////////////////////////////////////////////////////////////////

class PacketSchedulerContainerType : public boost::noncopyable {

public:

  static const uint64_t BUCKETS=0xFFF;

  PacketSchedulerContainerType() {
    ;
  }

  void put(uint64_t ctime, MediaPacketDescHandler pi) {
    int bucketNumber=static_cast<int>(ctime & BUCKETS);
    mBuckets[bucketNumber].put(ctime,pi);
  }

  void get(uint64_t ctime,PacketContainerType &packs) {
    int bucketNumber=static_cast<int>(ctime & BUCKETS);
    mBuckets[bucketNumber].get(ctime,packs);
  }

  void clear() {
    for(unsigned int i=0;i<=BUCKETS;i++) {
      mBuckets[i].clear();
    }
  }

private:

  class PacketSchedulerBucketType : public boost::noncopyable {

  public:

    PacketSchedulerBucketType() : mCtime(0) {
      ;
    }

    void put(uint64_t ctime, MediaPacketDescHandler pi) {
        EncRTPGuard(mMutex);
        if(mCtime!=ctime) {
            mCtime=ctime;
            mPackets.clear();
        }
        mPackets.push_back(pi);
        if(mPackets.size()>EncRTPProcess::MAX_PACKETS_IN_BUCKET) {
            mPackets.pop_front();
        }
    }
    
    void get(uint64_t ctime,PacketContainerType &packs) {
        EncRTPGuard(mMutex);
        if(mCtime!=ctime) return;
        if(packs.empty()) {
            packs.swap(mPackets);
        } else {
            packs.insert(packs.end(),mPackets.begin(),mPackets.end());
            mPackets.clear();
        }
    }
    
    void clear() {
      EncRTPGuard(mMutex);
      mCtime=0;
      mPackets.clear();
    }
    
  private:
    EncRTPUtils::MutexType mMutex;
    uint64_t mCtime;
    PacketContainerType mPackets;
  };

  PacketSchedulerBucketType mBuckets[BUCKETS+1];
};

//////////////////////////////////////////////////////////////////////////

class EncRTPSenderService : public pthreadwrapper, public boost::noncopyable {
  
public:
  
  EncRTPSenderService(boost::shared_ptr<EncRTPUtils::MutexType> mutex,
		      int size, 
		      boost::shared_ptr<EncRTPUtils::AtomicBool> process_active, 
		      boost::shared_ptr<bool> process_done,
              int cpu,
              int cpuType) : 
    mMutexPtr(mutex), 
    mSize(size), 
    mProcess_activePtr(process_active), 
    mProcess_donePtr(process_done),
    mCpu(cpu),
    mCpuType(cpuType)
  {
    ;
  }
  
  virtual ~EncRTPSenderService() {
    this->EncRTPSenderService::Stop();
  }
  
  void Stop(void) {
    while(true) {
      {
        EncRTPGuard(*mMutexPtr);
	if(*mProcess_donePtr) break;
	*mProcess_activePtr=false;
      }
      VoIPUtils::mksSleep(1000);
    }
  }

  int run_packets(uint64_t &jiffie, uint64_t ctime) {
    
    int ret=0;
    
    PacketContainerType packs;
    get_packets(jiffie,ctime,packs);

        while(true) {

            if(!*mProcess_activePtr) {
                ret=-1;
                break;
            }

            if(packs.empty()) break;

            MediaPacketDescHandler pi=packs.front();
            if(pi) {
                EncRTPMediaEndpoint::sendPacket(pi,mCpu);
                ret++;
            }
            packs.pop_front();
        }
    
    return ret;
  }

  virtual void * Execute(void* arg) {

      {
          EncRTPGuard(*mMutexPtr);
          *mProcess_donePtr=false;
      }

      spt_pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
      Detach();

      uint64_t ctime=VoIPUtils::getMilliSeconds();
      uint64_t jiffie=ctime;
      uint64_t silent_cycles_time=ctime;

      while (true) {

          ctime=VoIPUtils::getMilliSeconds();

          int res = run_packets(jiffie,ctime);
          if(res==-1) break;


          if(res>0) {
              ctime=VoIPUtils::getMilliSeconds();
              silent_cycles_time=ctime;
          } else {
              if((ctime-silent_cycles_time)>EncRTPProcessingParams::MAX_TIME_OF_SILENT_REACTOR_CYCLES) {
                  TC_LOG_INFO_LOCAL(0, LOG_ENCRTP, "EncRTPProcess Sender stopped due to the silence");
                  break;
              }
          }
          //Wait till the next jiffie time
		  VoIPUtils::mksSleep(EncRTPOptimizationParams::SenderProcessIdleDurationInMks);
      }

      {
          EncRTPGuard(*mMutexPtr);

          mPacketQueue.clear();

          *mProcess_activePtr=false;
          *mProcess_donePtr=true;
      }

      return arg;
  }
  
  virtual void Setup()
  {
    *mProcess_activePtr=true;    
    this->pthreadwrapper::Setup();
  }

  int schedulePacket(uint64_t ctime,MediaPacketDescHandler packet) {
    mPacketQueue.put(ctime,packet);
    return 0;
  }
  
private:
  
  void get_packets(uint64_t &jiffie, uint64_t ctime, PacketContainerType &packs) {
    while(!VoIPUtils::time_after64(jiffie,ctime)) {
    mPacketQueue.get(jiffie,packs);
      ++jiffie;
    }
  }

  mutable boost::shared_ptr<EncRTPUtils::MutexType> mMutexPtr;

  int mSize;
  boost::shared_ptr<EncRTPUtils::AtomicBool> mProcess_activePtr;
  boost::shared_ptr<bool> mProcess_donePtr;
  PacketSchedulerContainerType mPacketQueue;
  int mCpu;
  int mCpuType;
};

class EncRTPReceiverService : public pthreadwrapper, public boost::noncopyable {

public:

  typedef boost::function1<int,ACE_Reactor*> CommandType;
  typedef std::list<CommandType> CommandContainerType;
  typedef std::set<EndpointHandler>  EPContainer_t;
 
  EncRTPReceiverService(boost::shared_ptr<EncRTPUtils::MutexType> mutex,
			int size, 
			boost::shared_ptr<bool> process_active, 
			boost::shared_ptr<bool> process_done,
			boost::shared_ptr<CommandContainerType> commands,
			boost::shared_ptr<EncRTPSenderService> sender,
            int cpu,
            int cpuType) : 
    mMutexPtr(mutex), 
    mSize(size), 
    mProcess_activePtr(process_active), mProcess_donePtr(process_done), 
    mCommandQueuePtr(commands),
    mSenderPtr(sender),
    mCpu(cpu),
    mCpuType(cpuType)
  {
    mEplist.reset(new EPContainer_t);
  }
  
  virtual ~EncRTPReceiverService() {
    this->EncRTPReceiverService::Stop();
  }
  
  void Stop(void) {
    while(true) {
      {
	EncRTPGuard(*mMutexPtr);
	if(*mProcess_donePtr) break;
	*mProcess_activePtr=false;
      }
      VoIPUtils::mksSleep(1000);
    }
  }
  
  int open_command(EndpointHandler handler, ACE_Reactor *reactor) {
    return handler->openMEP(reactor);
  }
  
  int startListening_command(EndpointHandler h,
			     EncodedMediaStreamHandler file,
			     VQStreamManager *vqStreamManager,
			     VQInterfaceHandler interface,
			     ACE_Reactor *reactor) {
      int ret = 0;
      if(h && !h->isListening()){
          ret=h->startListening(reactor,file,vqStreamManager,interface);
          if(!ret && h->isListening()){
              mEplist->insert(h);
          }
      }
    return ret;
  }
  int startTip_command(EndpointHandler h,TipStatusDelegate_t cb,ACE_Reactor *reactor){
      int ret = -1;
      if(h){
          ret = h->startTIPNegotiate(reactor,cb);
          mEplist->insert(h);//attach endpoint to run tip sm.
      }
      return ret;
  }
  
  int stopListening_command(EndpointHandler h,ACE_Reactor *reactor) {
    int ret = 0;
    ret = h->stopListening(reactor);
    mEplist->erase(h);
    return ret;
  }

  int startTalking_command(EndpointHandler h,ACE_Reactor *reactor) {
    int ret = h->startTalking(reactor,boost::bind(&EncRTPSenderService::schedulePacket,mSenderPtr.get(),_1,_2));
    return ret;
  }

  int stopTalking_command(EndpointHandler h,ACE_Reactor *reactor) {
    int ret = h->stopTalking(reactor);
    return ret;
  }

  int close_command(EndpointHandler h,ACE_Reactor *reactor) {
    int ret=0;
    ret = h->closeMEP(reactor);
    return ret;
  }

  int release_command(EndpointHandler h,TrivialFunctionType f,ACE_Reactor *reactor) {
    int ret=0;
    ret = h->release(reactor,f);
    return ret;
  }

  int run_commands(ACE_Reactor *reactor,bool check_active) {

    int ret=0;

    CommandContainerType cmds;
    {
      EncRTPGuard(*mMutexPtr);
      if(check_active && !*mProcess_activePtr) return -1;
      cmds.swap(*mCommandQueuePtr);
    }
    
    while(!cmds.empty()) {
      CommandType c=cmds.front();
      c(reactor);
      cmds.pop_front();
      ++ret;
    }
    
    return ret;
  }

protected:

  virtual void * Execute(void* arg) {

      {
          EncRTPGuard(*mMutexPtr);
          *mProcess_donePtr=false;
      }

      spt_pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
      Detach();

      boost::scoped_ptr<ACE_Reactor> reactor(new ACE_Reactor(new ReactorType));

      reactor->open(mSize);

      uint64_t silent_cycles_time=VoIPUtils::getMilliSeconds();


      while (*mProcess_activePtr) {
          std::set<EndpointHandler>::iterator it;

          int events=0;
          EndpointHandler ep;

          for(it = mEplist->begin();it != mEplist->end();++it){
              ep = *it;
              if(ep){
                  if(ep->recvPacket(ep))
                      events++;
              }
          }


          if(run_commands(reactor.get(),true)==-1) {
              break;
          }

#ifndef UNIT_TEST
          //this reactor is used by TIP logic.
          //if close reactor when silence (in unitest it's always silence) TIP negotiate fails sometime randomly. 
          //-dfb
          if(events>0) {
              silent_cycles_time=VoIPUtils::getMilliSeconds();
          } else {
              if((VoIPUtils::getMilliSeconds()-silent_cycles_time)>
                      EncRTPProcessingParams::MAX_TIME_OF_SILENT_REACTOR_CYCLES) {
					TC_LOG_INFO_LOCAL(0, LOG_ENCRTP, "EncRtpProcess Receiver stopped due to the silence");
                  break;
              }
          }
#endif

           VoIPUtils::mksSleep(EncRTPOptimizationParams::ReceiverProcessIdleDurationInMks);
      }

      {
          EncRTPGuard(*mMutexPtr);

          reactor->close();

          run_commands(0,false); //clean the queue

          *mProcess_activePtr=false;
          *mProcess_donePtr=true; 

      }

      return arg;
  }
  
  virtual void Setup()
  {
    EncRTPGuard(*mMutexPtr);
    *mProcess_activePtr=true;    
    this->pthreadwrapper::Setup();
  }
  
private:
  mutable boost::shared_ptr<EncRTPUtils::MutexType> mMutexPtr;

  int mSize;
  boost::shared_ptr<bool> mProcess_activePtr;
  boost::shared_ptr<bool> mProcess_donePtr;
  boost::shared_ptr<CommandContainerType> mCommandQueuePtr;
  boost::shared_ptr<EncRTPSenderService> mSenderPtr;
  boost::shared_ptr<EPContainer_t>  mEplist;
  int mCpu;
  int mCpuType;
};

////////////////////////////////////////////////////////////////////

class EncRTPProcessImpl : public boost::noncopyable {

public:
 
  EncRTPProcessImpl(int size,int cpu) : 
    mRadService(new EncRTPRadvisionService()),
    mSize(size),
    mSenderMutexPtr(new EncRTPUtils::MutexType()),
    mReceiverMutexPtr(new EncRTPUtils::MutexType()),
    mSenderProcess_activePtr(new EncRTPUtils::AtomicBool(false)),
    mSenderProcess_donePtr(new bool(true)),
    mReceiverProcess_activePtr(new bool(false)),
    mReceiverProcess_donePtr(new bool(true)),
    mReceiverCommandQueuePtr(new EncRTPReceiverService::CommandContainerType())
  {
      mCpuType = VoIPUtils::isIntelCPU() ? EncRTPOptimizationParams::CPU_ARCH_INTEL:EncRTPOptimizationParams::CPU_ARCH_OTHER;
    mSenderPtr.reset(new EncRTPSenderService(mSenderMutexPtr,mSize,
					     mSenderProcess_activePtr,mSenderProcess_donePtr,cpu,mCpuType));
    
    mReceiverPtr.reset(new EncRTPReceiverService(mReceiverMutexPtr,mSize,
						 mReceiverProcess_activePtr,mReceiverProcess_donePtr,
						 mReceiverCommandQueuePtr,
						 mSenderPtr,mCpuType,cpu));
  }
  
  virtual ~EncRTPProcessImpl() {}

  int open(EndpointHandler ep,STREAM_TYPE type, 
	   string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr, uint8_t tos,uint32_t csrc) {
    if(ep) {
      ep->lock();
      EncRTPRadvisionService::CommandType cmd=
	boost::bind(&EncRTPMediaEndpoint::openTransport,ep.get(),type,ifName,vdevblock,thisAddr,peerAddr,tos,csrc,mCpuType);
      ep->unlock();
      int ret=mRadService->call_command(cmd);
      if(ret!=-1) {
	return open_handler(ep);
      } else {
	return -1;
      }
    } else {
      return -1;
    }
  }
  int startTip(EndpointHandler ep,TipStatusDelegate_t cb){
      int ret = -1;
      if(ep){
          ret = schedule_command(boost::bind(&EncRTPReceiverService::startTip_command,mReceiverPtr.get(),ep,cb,_1));
      }
      return ret;



  }
  
  int startListening(EndpointHandler ep,EncodedMediaStreamHandler file,VQStreamManager *vqStreamManager,VQInterfaceHandler interface) {
    int ret=-1;
    if(ep) {
      ret = startListening_handler(ep,file,vqStreamManager,interface);
    }
    return ret;
  }
  
  int stopListening(EndpointHandler ep) {
    int ret=0;
    if(ep) {
      ret = stopListening_handler(ep);
    }
    return ret;
  }
  
  int startTalking(EndpointHandler ep) {
    int ret=-1;
    if(ep) {
      ret = startTalking_handler(ep);
    }
    return ret;
  }
  
  int stopTalking(EndpointHandler ep) {
    int ret=0;
    if(ep) {
      ret = stopTalking_handler(ep);
    }
    return ret;
  }
  
  int close(EndpointHandler ep) {
    if(ep) {
      ep->lock();
      ep->setReady(false);
      EncRTPRadvisionService::CommandType cmd=boost::bind(&EncRTPMediaEndpoint::closeTransport,ep.get());
      ep->unlock();
      mRadService->call_command(cmd);
      return close_handler(ep);
    }
    return 0;
  }
  
  int release(EndpointHandler ep,TrivialFunctionType f) {
    if(ep) {
      return release_handler(ep,f);
    }
    return 0;
  }
  
protected:
  
  int open_handler(EndpointHandler handler) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::open_command,mReceiverPtr.get(),
				       handler,
				       _1));
    }
    return ret;
  }
  
  int startListening_handler(EndpointHandler handler,EncodedMediaStreamHandler file,
			     VQStreamManager *vqStreamManager,VQInterfaceHandler interface) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::startListening_command,mReceiverPtr.get(),
				       handler,file,vqStreamManager,interface,_1));
    }
    return ret;
  }
  
  int stopListening_handler(EndpointHandler handler) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::stopListening_command,mReceiverPtr.get(),handler,_1));
    }
    return ret;
  }

  int startTalking_handler(EndpointHandler handler) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::startTalking_command,mReceiverPtr.get(),handler,_1));
    }
    return ret;
  }

  int stopTalking_handler(EndpointHandler handler) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::stopTalking_command,mReceiverPtr.get(),handler,_1));
    }
    return ret;
  }

  int close_handler(EndpointHandler handler) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::close_command,mReceiverPtr.get(),handler,_1));
    }
    return ret;
  }

  int release_handler(EndpointHandler handler,TrivialFunctionType f) {
    int ret=0;
    if(handler) {
      ret=schedule_command(boost::bind(&EncRTPReceiverService::release_command,mReceiverPtr.get(),handler,f,_1));
    }
    return ret;
  }
  
  int schedule_command(const EncRTPReceiverService::CommandType c) {
    bool recStarted=false;
    bool senStarted=false;
    while(true) {
      EncRTPGuard(*mReceiverMutexPtr);
      if(!*mReceiverProcess_activePtr && *mReceiverProcess_donePtr) {
	if(!recStarted) {
	  mReceiverPtr->Start(0);
	  recStarted=true;
	}
	continue;
      }
      if(!*mSenderProcess_activePtr && *mSenderProcess_donePtr) {
	if(!senStarted) {
	  mSenderPtr->Start(0);
	  senStarted=true;
	}
	continue;
      }
      mReceiverCommandQueuePtr->push_back(c);
      break;
    }
    return 0;
  }
  
private:

  boost::scoped_ptr<EncRTPRadvisionService> mRadService;

  int mSize;

  mutable boost::shared_ptr<EncRTPUtils::MutexType> mSenderMutexPtr;
  mutable boost::shared_ptr<EncRTPUtils::MutexType> mReceiverMutexPtr;

  boost::shared_ptr<EncRTPUtils::AtomicBool> mSenderProcess_activePtr;
  boost::shared_ptr<bool> mSenderProcess_donePtr;
  boost::shared_ptr<EncRTPSenderService> mSenderPtr;

  boost::shared_ptr<bool> mReceiverProcess_activePtr;
  boost::shared_ptr<bool> mReceiverProcess_donePtr;
  boost::shared_ptr<EncRTPReceiverService::CommandContainerType> mReceiverCommandQueuePtr;
  boost::scoped_ptr<EncRTPReceiverService> mReceiverPtr;
  int mCpuType;
};

//////////////////////////////////////////////////////////////////////////

EncRTPProcess::EncRTPProcess(int size,int cpu) {
  mPimpl.reset(new EncRTPProcessImpl(size,cpu));
}

EncRTPProcess::~EncRTPProcess() {
  ;
}

int EncRTPProcess::open(EndpointHandler ep,STREAM_TYPE type, 
			string ifName, uint16_t vdevblock, ACE_INET_Addr *thisAddr, ACE_INET_Addr *peerAddr,uint8_t tos,uint32_t csrc) {
  return mPimpl->open(ep,type,ifName,vdevblock,thisAddr,peerAddr,tos,csrc);
}

int EncRTPProcess::startListening(EndpointHandler ep,
				  EncodedMediaStreamHandler file,
				  VQStreamManager *vqStreamManager,
				  VQInterfaceHandler interface) {
  return mPimpl->startListening(ep,file,vqStreamManager,interface);
}

int EncRTPProcess::stopListening(EndpointHandler ep) {
  return mPimpl->stopListening(ep);
}

int EncRTPProcess::startTalking(EndpointHandler ep) {
  return mPimpl->startTalking(ep);
}

int EncRTPProcess::stopTalking(EndpointHandler ep) {
  return mPimpl->stopTalking(ep);
}

int EncRTPProcess::close(EndpointHandler ep) {
  return mPimpl->close(ep);
}

int EncRTPProcess::release(EndpointHandler ep,TrivialFunctionType f) {
  return mPimpl->release(ep,f);
}
int EncRTPProcess::startTip(EndpointHandler ep,TipStatusDelegate_t cb){
    return mPimpl->startTip(ep,cb);
}

END_DECL_ENCRTP_NS

////////////////////////////////////////////////////////////////////////////////


