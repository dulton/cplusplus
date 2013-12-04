/// @file
/// @brief SIP User Agent (UA) object
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sys/sysinfo.h>

#include <utils/md5.h>

#include <locale>
#include <list>

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <Tr1Adapter.h>
#include <ace/Message_Queue.h>
#include <ace/Select_Reactor.h>
#include <ace/Task.h>
#include <ace/Thread_Semaphore.h>
#include <ildaemon/ilDaemonCommon.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>

#include "VoIPUtils.h"
#include "UserAgentFactory.h"
#include "UserAgentLean.h"
#include "RvUserAgent.h"
#include "SipEngine.h"
#include "AsyncCompletionToken.h"
#include "VoIPMediaManager.h"

USING_VOIP_NS;
USING_SIP_NS;
USING_SIPLEAN_NS;
USING_RVSIP_NS;
USING_RV_SIP_ENGINE_NAMESPACE;

///////////////////////////////////////////////////////////////////////////////

DECL_SIP_NS

//////////////////////////////// SIP Engine "callback" classes /////////////////////////////

class MD5ProcessorImpl : public MD5Processor {
public:
  MD5ProcessorImpl() {}
  virtual ~MD5ProcessorImpl() {}
  
  virtual void init(state_t *pms) const {
    md5_init((md5_state_t*)pms);
  }
  virtual void update(state_t *pms, const unsigned char *data, int nbytes) const {
    md5_append((md5_state_t*)pms,(const md5_byte_t*)data,nbytes);
  }
  
  virtual void finish(state_t *pms, unsigned char digest[16]) const {
    md5_finish((md5_state_t*)pms,(md5_byte_t*)digest);
  }
};

class DestAddrKey {
  
private:
  char name[129];
  int handle;
  unsigned int hv;

public:
  
  explicit DestAddrKey(const char* n, int h) {
    if(n) strncpy(name,n,sizeof(name)-1);
    else name[0]=0;
    handle=h;
    hv=hash();
  }

  DestAddrKey(const DestAddrKey& other) {
    strcpy(name,other.name);
    handle=other.handle;
    hv=other.hv;
  }

  DestAddrKey& operator=(const DestAddrKey& other) {
    strcpy(name,other.name);
    handle=other.handle;
    hv=other.hv;
    return *this;
  }
  
  unsigned int hash() const {
    unsigned int hashValue = 5381;
    int c = 0;
    unsigned char* str = (unsigned char*)&name;
    
    while ((c = *str++)) {
      hashValue = ((hashValue << 5) + hashValue) + c; /* hashValue * 33 + c */
    }
    return hashValue;
  }
  
  bool operator==(const DestAddrKey& other) const {
    if(this == &other) return true;
    if(other.hv != hv) return false;
    if(other.handle!=handle) return false;
    if(strcmp(name,other.name)) return false;
    return true;
  }
  
  /**
   * Returns @c true if @c this is less than @a rhs.  In this context,
   * "less than" is defined in terms of IP address and port
   * number.  This operator makes it possible to use @c ACE_INET_Addrs
   * in STL maps.
   */
  bool operator < (const DestAddrKey &rhs) const {
    if(this==&rhs) return false;
    if(hv < rhs.hv) return true;
    if(hv > rhs.hv) return false;
    if(handle<rhs.handle) return true;
    if(handle>rhs.handle) return false;
    int c = strcmp(name,rhs.name);
    if(c<0) return true;
    return false;
  }
};

class SipChannelFinderImpl : public SipChannelFinder {
public:

  SipChannelFinderImpl() {}

  virtual ~SipChannelFinderImpl() {}

  virtual SipChannel* findSipChannel(const char* strUserName, int handle) const {

    DestAddrKey key(strUserName,handle);

    VoIPGuard(mMutex);

    ChannelsMapType::const_iterator iter=channels.find(key);
    if(iter!=channels.end()) {
      if(!iter->second->isFree()) {
	return iter->second;
      }
    }

    return NULL;
  }

  virtual void put(SipChannel* channel) {

    if(channel) {
      
      int handle=channel->getHandle();
      DestAddrKey key(channel->getLocalUserName().c_str(),handle);
      
      VoIPGuard(mMutex);

      ChannelsMapType::iterator iter=channels.find(key);
      if(iter!=channels.end()) {
	if(channel != iter->second) {
	  channels.erase(iter);
	} else {
	  return;
	}
      }

      channels.insert(std::make_pair(key,channel));
    }
  }

  virtual void remove(SipChannel *channel) {

    if(channel) {

      int handle=channel->getHandle();
      DestAddrKey key(channel->getLocalUserName().c_str(),handle);
      
      VoIPGuard(mMutex);
      
      ChannelsMapType::iterator iter=channels.find(key);
      if(iter!=channels.end()) {
	if(channel == iter->second) {
	  channels.erase(iter);
	}
      }
    }
  }
private:
  mutable VoIPUtils::MutexType mMutex;
  typedef std::map<DestAddrKey,SipChannel*> ChannelsMapType;
  ChannelsMapType channels;
};

////////////////////////////////////////////////////////////////////////

class UserAgentManagerImpl : public boost::noncopyable, public ACE_Event_Handler
{
 public:

  typedef RvUserAgent::notification_t CommandType;
  typedef std::list<CommandType> CommandContainerType;
  typedef RvUserAgent::lpcommand_t LPCommandType;
  typedef std::list<LPCommandType> LPCommandContainerType;
  typedef ACE_Message_Queue_Ex<IL_DAEMON_LIB_NS::BoundMethodRequest, ACE_MT_SYNCH> methodQueue_t;

  UserAgentManagerImpl(ACE_Reactor& reactor, MEDIA_NS::VoIPMediaManager *voipMediaManager) : 
    mIsActive(false), mVoipMediaManager(voipMediaManager),mQueue(MESSAGE_QUEUE_LWM,MESSAGE_QUEUE_HWM) {
    mReadyToHandleCmds = false;
    mTimerId=-1;
    VoIPUtils::calculateCPUNumber(mNumberOfCpus);
    mCommandsRunner = boost::bind(&UserAgentManagerImpl::save_command,this,_1);
    mLPCommandsRunner = boost::bind(&UserAgentManagerImpl::save_lpcommand,this,_1);
    mReactor = &reactor;

  }
    
  virtual ~UserAgentManagerImpl() {
    deactivate();
  }

  uaSharedPtr_t getNewUserAgent(uint16_t port, 
				uint16_t vdevblock,
				size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
				const std::string& name, 
				const std::string& imsi, 
				const std::string& ranid, 
				const UserAgent::config_t& config) {

    uaSharedPtr_t ua;
    if(mIsActive) {
      ua.reset(new RvUserAgent(port,vdevblock,index,numUaObjects,indexInDevice,deviceIndex,name,imsi,ranid,config,*mReactor,mVoipMediaManager,
			       mCommandsRunner,mLPCommandsRunner));
    }
    return ua;
  }


  void stopPortInterfaces(uint16_t port, const UserAgentFactory::InterfaceContainer& ifIndexes) {
    size_t size = mLPCommandQueue.size();
    while(size>0) { 
      LPCommandType command=mLPCommandQueue.front();
      mLPCommandQueue.pop_front();
      if(port != boost::tuples::get<0>(command) || ifIndexes.count(boost::tuples::get<1>(command))<1 ) {
	mLPCommandQueue.push_back(command);
      }
      --size;
    }
  }

  void activate() {
      deactivate();
      mScFinder.reset(new SipChannelFinderImpl());
      mMD5Processor.reset(new MD5ProcessorImpl());
      int numOfThreads = mNumberOfCpus*4;
      if(numOfThreads < 4) numOfThreads=4;
      this->reactor(mReactor);
      SipEngine::init(numOfThreads,VoIPUtils::SipCallCapacity()+100,*(mScFinder.get()),*(mMD5Processor.get()));
      methodCompletionDelegate_t mediaCompletionDelegation = boost::bind(&UserAgentManagerImpl::DispatchMediaAsyncCompletion,this,_1,_2);
      mVoipMediaManager->setMediaCompletionDelegation(mediaCompletionDelegation);
      ACE_Time_Value initialDelay(0,100);
      ACE_Time_Value interval(0,500);
      mTimerId = this->reactor()->schedule_timer(this,this->reactor(),initialDelay,interval);
      mIsActive=true;
  }

  void deactivate() {
      mIsActive=false;
      if(mTimerId!=-1 && this->reactor()) {
          this->reactor()->cancel_timer(mTimerId);
          mTimerId = -1;
          this->reactor()->remove_handler(this,ACE_Event_Handler::TIMER_MASK | ACE_Event_Handler::DONT_CALL);
          SipEngine::destroy();
          this->reactor(0);
          mScFinder.reset();
          mMD5Processor.reset();
      }
  }

  void ready_to_handle_cmds(bool bReady){

      VoIPGuard(mReadyMutex);
      bool notchanged= (mReadyToHandleCmds == bReady);
      mReadyToHandleCmds = bReady;
      if(!notchanged){
          mCommandQueue.clear();
          mLPCommandQueue.clear();
          mQueue.flush();
      }
  }
  void run_completion(void){

      if(mIsActive){
          IL_DAEMON_LIB_NS::BoundMethodRequest *rawReq;
          while(1){
              rawReq = 0;
              ACE_Time_Value noBlock(ACE_Time_Value::zero);
              mQueue.dequeue(rawReq,&noBlock);
              if(!rawReq)
                  break;
              boost::intrusive_ptr<IL_DAEMON_LIB_NS::BoundMethodRequest> req(rawReq);
              req->Call();
          }
      }
  }

  int handle_timeout(const ACE_Time_Value &current_time, const void *arg) {
      if(mIsActive) {
          SipEngine::getInstance()->run_immediate_events(0);
          VoIPGuard(mReadyMutex);
          if(mReadyToHandleCmds){
              size_t cnt=mLPCommandQueue.size();
              run_commands();
              run_completion();
              run_lpcommands(cnt);
          }else
              run_completion();
      }
      return 0;
  }
  ACE_Reactor *Reactor(void){
      return mReactor;
  }
  void run_mediaCompletion(WeakAsyncCompletionToken weakAct,int data)
  {
      if(AsyncCompletionToken act = weakAct.lock())
          act->Call(data);
  }
  void DispatchMediaAsyncCompletion(WeakAsyncCompletionToken weakAct,int data)
  {
      if(weakAct.lock()  ){
          IL_DAEMON_LIB_MAKE_ASYNC_REQUEST(req,void,&UserAgentManagerImpl::run_mediaCompletion,this,weakAct,data);
          mQueue.enqueue(req);
      }
  }

  int handle_close (ACE_HANDLE handle, ACE_Reactor_Mask close_mask) {
    return 0;
  }

  MEDIA_NS::VoIPMediaManager *getVoipMediaManager() const {
    return mVoipMediaManager;
  }

  int setDNSServers(const std::vector<std::string> &dnsServers, uint16_t vdevblock) {
    if(mIsActive) {
      return SipEngine::getInstance()->setDNSServers(dnsServers,vdevblock,false);
    }
    return -1;
  }

  int setDNSTimeout(int timeout, int tries, uint16_t vdevblock) {
    if(mIsActive) {
      if(SipEngine::getInstance()->setDNSTimeout(timeout,tries,vdevblock)<0) return -1;
      return 0;
    }
    return -1;
  }
  static const size_t MESSAGE_QUEUE_HWM;              ///< method queue high-water mark
  static const size_t MESSAGE_QUEUE_LWM;              ///< method queue low-water mark
private:
  
  void save_command(const CommandType &command) {
    VoIPGuard(mCommandMutex);
    if(mReadyToHandleCmds)
        mCommandQueue.push_back(command);
  }

  void save_lpcommand(const LPCommandType &command) {
    if(mReadyToHandleCmds)
        mLPCommandQueue.push_back(command);
  }

  int run_commands() {
    int ret=0;
    CommandContainerType current_cmds;
    {
      VoIPGuard(mCommandMutex);
      current_cmds.swap(mCommandQueue);
    }
    while(mIsActive && !current_cmds.empty()) {
      CommandType command=current_cmds.front();
      current_cmds.pop_front();
      command();
      ret++;
    }
    return ret;
  }

  int run_lpcommands(size_t cnt) {
    int ret=0;
    while(mIsActive && cnt-- && !mLPCommandQueue.empty() && (RvUserAgent::getPendingChannels()<SipEngine::getMaxPendingChannels())) {
      LPCommandType command=mLPCommandQueue.front();
      mLPCommandQueue.pop_front();
      bool cont = boost::tuples::get<2>(command)();
      ret++;
      if(!cont) break;
    }
    return ret;
  }

  //data:
  mutable VoIPUtils::MutexType mCommandMutex;
  mutable VoIPUtils::MutexType mReadyMutex;
  bool mIsActive;
  bool mReadyToHandleCmds;
  RvUserAgent::run_notification_t mCommandsRunner;
  RvUserAgent::run_lpcommand_t mLPCommandsRunner;
  unsigned int mNumberOfCpus;
  long mTimerId;

  //references:
  ACE_Reactor *mReactor;
  MEDIA_NS::VoIPMediaManager *mVoipMediaManager;

  //owned members:
  boost::scoped_ptr<SipChannelFinder> mScFinder;
  boost::scoped_ptr<MD5Processor> mMD5Processor;
  CommandContainerType mCommandQueue;
  LPCommandContainerType mLPCommandQueue;
  methodQueue_t mQueue;
};
const size_t UserAgentManagerImpl::MESSAGE_QUEUE_HWM = sizeof(IL_DAEMON_LIB_NS::BoundMethodRequest) * 16 * 1024;
const size_t UserAgentManagerImpl::MESSAGE_QUEUE_LWM = sizeof(IL_DAEMON_LIB_NS::BoundMethodRequest) * 16 * 1024;

END_DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

UserAgentFactory::UserAgentFactory(ACE_Reactor& reactor, MEDIA_NS::VoIPMediaManager *voipMediaManager) {
  mPimpl.reset(new UserAgentManagerImpl(reactor,voipMediaManager));
}
    
UserAgentFactory::~UserAgentFactory() {}

MEDIA_NS::VoIPMediaManager* UserAgentFactory::getVoipMediaManager() const {
  return mPimpl->getVoipMediaManager();
}

uaSharedPtr_t UserAgentFactory::getNewUserAgent(uint16_t port, 
						uint16_t vdevblock,
						size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
						const std::string& name, 
						const std::string& imsi, 
						const std::string& ranid, 
						const UserAgent::config_t& config) {
  return mPimpl->getNewUserAgent(port, 
				 vdevblock,
				 index, numUaObjects, indexInDevice, deviceIndex, 
				 name, 
				 imsi, 
				 ranid, 
				 config);
}

void UserAgentFactory::activate() {
  mPimpl->activate();
}

void UserAgentFactory::deactivate() {
  mPimpl->deactivate();
}

void UserAgentFactory::stopPortInterfaces(uint16_t port, const InterfaceContainer& ifIndexes) {
  mPimpl->stopPortInterfaces(port,ifIndexes);
}
void UserAgentFactory::ready_to_handle_cmds(bool bReady){
    mPimpl->ready_to_handle_cmds(bReady);
}

int UserAgentFactory::setDNSServers(const std::vector<std::string> &dnsServers, uint16_t vdevblock) {
  return mPimpl->setDNSServers(dnsServers,vdevblock);
}

int UserAgentFactory::setDNSTimeout(int timeout, int tries, uint16_t vdevblock) {
  return mPimpl->setDNSTimeout(timeout,tries,vdevblock);
}
ACE_Reactor *UserAgentFactory::Reactor(void){
    return mPimpl->Reactor();
}
