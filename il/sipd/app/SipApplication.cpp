/// @file
/// @brief SIP application class
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>

#include <ildaemon/GenericTimer.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>
#include <Tr1Adapter.h>

#include "McoDriver.h"
#include "SipApplication.h"
#include "VoIPLog.h"
#include "UserAgentBlock.h"
#include "UserAgentFactory.h"

USING_VOIP_NS;
USING_MEDIA_NS;
USING_APP_NS;
using SIP_NS::UserAgentFactory;

///////////////////////////////////////////////////////////////////////////////

struct SipApplication::PortContext
{
    typedef std::tr1::shared_ptr<UserAgentBlock> uaBlockPtr_t;
    typedef boost::unordered_map<uint32_t, uaBlockPtr_t> uaBlockMap_t;
    typedef boost::unordered_map<uint32_t, uint16_t> uaBlockIdMap_t;
    typedef boost::unordered_multimap<uint32_t, uaBlockPtr_t> uaBlockMultiMap_t;
    typedef std::vector<uint32_t> handleVec_t;
    typedef std::set<uint16_t> vdeblockSet;
    
    std::tr1::shared_ptr<uaBlockMap_t> uaBlockBllHandleMap;
    std::tr1::shared_ptr<uaBlockIdMap_t> uaBlockIdBllHandleMap;
    std::tr1::shared_ptr<uaBlockMultiMap_t> uaBlockIfHandleMap;
    std::tr1::shared_ptr<vdeblockSet> avaUablockIdSet;

    std::tr1::shared_ptr<McoDriver> mcoDriver;

    std::tr1::shared_ptr<IL_DAEMON_LIB_NS::GenericTimer> statsClearTimer;
    std::tr1::shared_ptr<handleVec_t> statsClearHandleVec;
};

///////////////////////////////////////////////////////////////////////////////

SipApplication::SipApplication(uint16_t numPorts)
  : mReactor(0),mUserAgentBlockCounter(0)
{
    // Initialize database dictionary
    McoDriver::StaticInitialize();
    
    // Size per-port context vector, allocations are deferred until activation
    mPorts.resize(numPorts);
}

///////////////////////////////////////////////////////////////////////////////

SipApplication::~SipApplication()
{
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::Activate(ACE_Reactor& appReactor, 
			      VOIP_NS::AsyncCompletionHandler *asyncCompletionHandler,
			      MEDIA_NS::VoIPMediaManager *voipMediaManager)
{
    mAsyncCompletionHandler = asyncCompletionHandler;

    mUserAgentFactory.reset(new UserAgentFactory(appReactor, voipMediaManager));
    mUserAgentFactory->activate();
    mReactor = &appReactor;

    // Allocate per-port items
    const size_t numPorts = mPorts.size();
    for (size_t port = 0; port < numPorts; port++)
    {
        mPorts[port].uaBlockBllHandleMap.reset(new PortContext::uaBlockMap_t);
        mPorts[port].uaBlockIdBllHandleMap.reset(new PortContext::uaBlockIdMap_t);
        mPorts[port].avaUablockIdSet.reset(new PortContext::vdeblockSet);
        mPorts[port].avaUablockIdSet->clear();
        mPorts[port].uaBlockIfHandleMap.reset(new PortContext::uaBlockMultiMap_t);

        mPorts[port].mcoDriver.reset(new McoDriver(port));
        mPorts[port].statsClearHandleVec.reset(new PortContext::handleVec_t);
        mPorts[port].statsClearTimer.reset(new IL_DAEMON_LIB_NS::GenericTimer(boost::bind(&SipApplication::StatsClearTimeoutHandler, this, _1, _2), mReactor));
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::Deactivate(void)
{
    mPorts.clear();
    mReactor = 0;
    mAsyncCompletionHandler = 0;
    mUserAgentFactory.reset();
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::ConfigUserAgents(uint16_t port, const uaConfigVec_t& uaConfigs, handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    mUserAgentFactory->ready_to_handle_cmds(false);

    const size_t numConfigs = uaConfigs.size();
    
    // Construct UserAgentBlock objects first
    std::vector<PortContext::uaBlockPtr_t> uaBlockVec;
    uaBlockVec.reserve(numConfigs);
    PortContext::uaBlockIdMap_t::iterator end = thisPort.uaBlockIdBllHandleMap->end();

    BOOST_FOREACH(const UserAgentConfig& uaConfig, uaConfigs)
    {
        uint32_t bllHandle = uaConfig.Config().ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle;
        uint16_t vdevblock = 0;

        PortContext::uaBlockIdMap_t::iterator iter = thisPort.uaBlockIdBllHandleMap->find(bllHandle);
        if (iter == end){
            if(!thisPort.avaUablockIdSet->empty()){
                PortContext::vdeblockSet::iterator it = thisPort.avaUablockIdSet->begin();
                vdevblock = *it;
                thisPort.avaUablockIdSet->erase(it);
            }else
                vdevblock = mUserAgentBlockCounter++;
        }else{
            vdevblock = iter->second ;
        }

        PortContext::uaBlockPtr_t uaBlock(new UserAgentBlock(port, vdevblock, 
                    uaConfig, *mReactor, 
                    NULL, 
                    *(mUserAgentFactory.get())));

        uaBlockVec.push_back(uaBlock);

    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numConfigs);

    BOOST_FOREACH(const PortContext::uaBlockPtr_t& uaBlock, uaBlockVec)
    {
        const uint32_t bllHandle = uaBlock->BllHandle();
        const uint32_t ifHandle = uaBlock->IfHandle();
        const uint16_t vdevblockId = uaBlock->VDevBlock();

        // Echo UserAgentBlock BLL handle back as the handle
        handles.push_back(bllHandle);
        
        // Insert UserAgentBlock object into handle maps
        thisPort.uaBlockBllHandleMap->insert(std::make_pair(bllHandle, uaBlock));
        thisPort.uaBlockIdBllHandleMap->insert(std::make_pair(bllHandle, vdevblockId));
        thisPort.uaBlockIfHandleMap->insert(std::make_pair(ifHandle, uaBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertSipUaResult(bllHandle, uaBlock->TotalCount());
    }
    mUserAgentFactory->ready_to_handle_cmds(true);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::UpdateUserAgents(uint16_t port, const handleList_t& handles, const uaConfigVec_t& uaConfigs)
{
    // Delete existing UserAgentBlock objects on these handles
    DeleteUserAgents(port, handles);

    // Configure new UserAgentBlock objects
    handleList_t newHandles;
    ConfigUserAgents(port, uaConfigs, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::DeleteUserAgents(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    mUserAgentFactory->ready_to_handle_cmds(false);

    McoDriver::Transaction t = thisPort.mcoDriver->StartTransaction();
    size_t deletes = 0;
    
    // Delete UserAgentBlock objects
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
            continue;

        PortContext::uaBlockPtr_t uaBlock(iter->second);

        // Remove object from maps
        thisPort.avaUablockIdSet->insert(uaBlock->VDevBlock());
        thisPort.uaBlockBllHandleMap->erase(iter);

        {
            const uint32_t ifHandle = uaBlock->IfHandle();
            PortContext::uaBlockMultiMap_t::iterator end = thisPort.uaBlockIfHandleMap->end();
            PortContext::uaBlockMultiMap_t::iterator iter = thisPort.uaBlockIfHandleMap->find(ifHandle);

            while (iter != end && iter->first == ifHandle)
            {
                if (iter->second == uaBlock)
                {
                    thisPort.uaBlockIfHandleMap->erase(iter);
                    break;
                }

                ++iter;
            }
        }

        // Remove object from the stats database
        thisPort.mcoDriver->DeleteSipUaResult(bllHandle, t);
        deletes++;
    }

    if (deletes)
        thisPort.mcoDriver->CommitTransaction(t);
    mUserAgentFactory->ready_to_handle_cmds(true);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::RegisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Register UserAgentBlock load profile notifier
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::RegisterUserAgentLoadProfileNotifier] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }
        
        iter->second->RegisterLoadProfileNotifier(notifier);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::UnregisterUserAgentLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Unregister UserAgentBlock load profile notifier
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterLoadProfileNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::RegisterUserAgents(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    UserAgentBlock::contactAddressResolver_t contactAddressResolver = boost::bind(&SipApplication::EnumerateContactAddresses, this, port, _1, _2);
    mUserAgentFactory->ready_to_handle_cmds(true);
    
    // Start UserAgentBlock registrations
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::RegisterUserAgents] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }
        
        iter->second->Register(contactAddressResolver);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::UnregisterUserAgents(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Start UserAgentBlock unregistrations
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::UnregisterUserAgents] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }
        
        iter->second->Unregister();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::CancelUserAgentsRegistrations(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Cancel UserAgentBlock registrations
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::CancelUserAgentsRegistrations] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }
        
        iter->second->CancelRegistrations();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::RegisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Unregister state notifiers
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::RegisterUserAgentRegStateNotifier] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }

        iter->second->RegisterRegStateNotifier(notifier);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::UnregisterUserAgentRegStateNotifier(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Unregister state notifiers
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterRegStateNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::ResetProtocol(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    
    // Reset UserAgentBlock registrations and calls
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::ResetProtocol] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }

        iter->second->Reset();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::StartProtocol(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    mUserAgentFactory->ready_to_handle_cmds(true);
    
    // Start UserAgentBlock calls
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::StartProtocol] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }

        iter->second->Start();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::StopProtocol(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    mUserAgentFactory->ready_to_handle_cmds(false);

    // Stop UserAgentBlock calls
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::StopProtocol] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }

        iter->second->Stop();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    uint64_t relExecTime = 0;
    
    if (absExecTime)
    {
        const uint64_t now = Hal::TimeStamp::getTimeStamp();
        relExecTime = (absExecTime < now) ? 0 : (absExecTime - now);
    }
    
    if (relExecTime)
    {
        PortContext& thisPort(mPorts.at(port));
        const size_t act(port);

        // Save the handle list
        *thisPort.statsClearHandleVec = handles;

        // Schedule a timer to execute the operation at a later time (note: relExecTime is specified in 2.5ns backplane ticks)
        thisPort.statsClearTimer->Start(ACE_Time_Value(0, relExecTime / 400), reinterpret_cast<const void *>(act));
    }
    else
        DoStatsClear(port, handles);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::SyncResults(uint16_t port)
{
    PortContext& thisPort(mPorts.at(port));

    McoDriver::Transaction t = thisPort.mcoDriver->StartTransaction();
    size_t updates = 0;

    // For each UserAgentBlock...
    BOOST_FOREACH(const PortContext::uaBlockMap_t::value_type& pair, *thisPort.uaBlockBllHandleMap)
    {
       UserAgentBlock::stats_t& stats = pair.second->Stats();
       if (stats.isDirty())
       {
          // Sync the block's stats and update in the database
         stats.Sync();
         thisPort.mcoDriver->UpdateSipUaResult(stats, t);
         stats.setDirty(false);
         updates++;
       }
    }

    if (updates)
        thisPort.mcoDriver->CommitTransaction(t);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    PortContext& thisPort(mPorts.at(port));
    
    PortContext::uaBlockMultiMap_t::iterator end = thisPort.uaBlockIfHandleMap->end();
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        PortContext::uaBlockMultiMap_t::iterator iter = thisPort.uaBlockIfHandleMap->find(ifHandle);
        
        // Notify all UserAgentBlock objects on this interface handle that their interfaces are disabled
        while (iter != end && iter->first == ifHandle)
        {
            iter->second->NotifyInterfaceDisabled();
            ++iter;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::NotifyInterfaceUpdateCompleted(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    PortContext& thisPort(mPorts.at(port));

    // Copy the config structures of all UserAgentBlock objects on the affected interfaces
    handleList_t handles;
    uaConfigVec_t uaConfigs;

    handles.reserve(ifHandleVec.size());  // assumes one UA per ifHandle to get us in the ballpark
    uaConfigs.reserve(ifHandleVec.size());      
    
    PortContext::uaBlockMultiMap_t::iterator end = thisPort.uaBlockIfHandleMap->end();
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        PortContext::uaBlockMultiMap_t::iterator iter = thisPort.uaBlockIfHandleMap->find(ifHandle);
        
        while (iter != end && iter->first == ifHandle)
        {
            handles.push_back(iter->second->BllHandle());
            uaConfigs.push_back(iter->second->Config());
            ++iter;
        }
    }

    // Delete the existing UserAgentBlock objects
    DeleteUserAgents(port, handles);

    // Create new UserAgentBlock objects using the updated interface configurations
    ConfigUserAgents(port, uaConfigs, handles);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    PortContext& thisPort(mPorts.at(port));
    
    handleList_t handles;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        // Delete all UserAgentBlock objects on this interface handle
      BOOST_FOREACH(const PortContext::uaBlockMultiMap_t::value_type& pair, thisPort.uaBlockIfHandleMap->equal_range(ifHandle)) {
	handles.push_back(pair.second->BllHandle());
	pair.second->NotifyInterfaceDisabled();
      }
    }

    if (!handles.empty())
        DeleteUserAgents(port, handles);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::DispatchAsyncCompletion(WeakAsyncCompletionToken weakAct, int data)
{
    if (AsyncCompletionToken act = weakAct.lock())
        act->Call(data);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::EnumerateContactAddresses(uint16_t port, uint32_t bllHandle, const contactAddressConsumer_t& consumer) const
{
    const PortContext& thisPort(mPorts.at(port));
    
    PortContext::uaBlockMap_t::const_iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
    if (iter != thisPort.uaBlockBllHandleMap->end())
        iter->second->EnumerateContactAddresses(consumer);
    else
        TC_LOG_WARN_LOCAL(port, LOG_SIP, "[SipApplication::EnumerateContactAddresses] invalid UA block handle (" << bllHandle << ")");
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::DoStatsClear(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    McoDriver::Transaction t = thisPort.mcoDriver->StartTransaction();
    size_t updates = 0;
    
    if (handles.empty())
    {
        // For each UserAgentBlock object
        BOOST_FOREACH(PortContext::uaBlockMap_t::value_type& pair, *thisPort.uaBlockBllHandleMap)
        {
            UserAgentBlock::stats_t& stats = pair.second->Stats();
                    
            /// Reset the block's stats and update in the database
            stats.Reset();
            thisPort.mcoDriver->UpdateSipUaResult(stats, t);
            updates++;
        }
    }
    else
    {
        // For selected objects
        PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
        BOOST_FOREACH(uint32_t bllHandle, handles)
        {
            // Try UserAgentBlock lookup
            PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
            if (iter == end)
                continue;
            
            UserAgentBlock::stats_t& stats = iter->second->Stats();
                
            /// Reset the block's stats and update in the database
            stats.Reset();
            thisPort.mcoDriver->UpdateSipUaResult(stats, t);
            updates++;
        }
    }

    if (updates)
        thisPort.mcoDriver->CommitTransaction(t);
}

///////////////////////////////////////////////////////////////////////////////

void SipApplication::StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act)
{
    const size_t port = reinterpret_cast<const size_t>(act);
    PortContext& thisPort(mPorts.at(port));

    DoStatsClear(port, *thisPort.statsClearHandleVec);
    thisPort.statsClearHandleVec->clear();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST

void SipApplication::GetUserAgentStats(uint16_t port, const handleList_t& handles, std::vector<UserAgentRawStats>& stats) const
{
    const PortContext& thisPort(mPorts.at(port));

    stats.clear();
    stats.reserve(handles.size());
    
    PortContext::uaBlockMap_t::iterator end = thisPort.uaBlockBllHandleMap->end();
    BOOST_FOREACH(uint32_t bllHandle, handles)
    {
        // Try UserAgentBlock lookup
        PortContext::uaBlockMap_t::iterator iter = thisPort.uaBlockBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[SipApplication::GetUserAgentStats] invalid UA block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_SIP, err);
            throw EInval(err);
        }

        stats.push_back(iter->second->Stats());
    }
}

#endif    

///////////////////////////////////////////////////////////////////////////////
