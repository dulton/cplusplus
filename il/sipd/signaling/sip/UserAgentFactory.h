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

#ifndef _USER_AGENT_FACTORY_H_
#define _USER_AGENT_FACTORY_H_

#include <set>
#include <vector>
#include <string>

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "UserAgent.h"

DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

class UserAgentManagerImpl;

class UserAgentFactory : public boost::noncopyable
{
 public:

  typedef std::set<unsigned int> InterfaceContainer;

  static const int MAX_NUMBER_OF_CHANNELS=8192;

  UserAgentFactory(ACE_Reactor& reactor, MEDIA_NS::VoIPMediaManager *voipMediaManager);
  virtual ~UserAgentFactory();

  void stopPortInterfaces(uint16_t port, const InterfaceContainer& ifIndexes);

  int setDNSServers(const std::vector<std::string> &dnsServers,uint16_t vdevblock);
  int setDNSTimeout(int timeout, int tries, uint16_t vdevblock);

  void activate();
  void deactivate();
  void ready_to_handle_cmds(bool bReady);

  MEDIA_NS::VoIPMediaManager *getVoipMediaManager() const;

  uaSharedPtr_t getNewUserAgent(uint16_t port, 
				uint16_t vdevblock,
				size_t index, size_t numUaObjects, size_t indexInDevice, size_t deviceIndex, 
				const std::string& name,const std::string& imsi,const std::string& ranid, const UserAgent::config_t& config);
  
  ACE_Reactor *Reactor(void);
 private:
  boost::scoped_ptr<UserAgentManagerImpl> mPimpl;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIP_NS

#endif //_USER_AGENT_FACTORY_H_
