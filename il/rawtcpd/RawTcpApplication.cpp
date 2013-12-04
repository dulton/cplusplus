/// @file
/// @brief Raw tcp Application implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <map>
#include <utility>

#include <ildaemon/GenericTimer.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <FastDelegate.h>
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>
#include <Tr1Adapter.h>

#include "RawTcpApplication.h"
#include "RawTcpClientBlock.h"
#include "RawtcpdLog.h"
#include "RawTcpProtocol.h"
#include "RawTcpServerBlock.h"
#include "McoDriver.h"

USING_RAWTCP_NS;

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value RawTcpApplication::MAX_STATS_AGE(0, 500000);  // 0.5 seconds

///////////////////////////////////////////////////////////////////////////////

struct RawTcpApplication::PortContext
{
    typedef std::vector<uint32_t> handleVec_t;

    typedef std::tr1::shared_ptr<RawTcpClientBlock> clientBlockPtr_t;
    typedef boost::unordered_map<uint32_t, clientBlockPtr_t> clientBlockMap_t;
    typedef std::multimap<uint32_t, clientBlockPtr_t> clientBlockMultiMap_t;
    typedef boost::unordered_set<clientBlockPtr_t> clientBlockSet_t;

    typedef std::tr1::shared_ptr<RawTcpServerBlock> serverBlockPtr_t;
    typedef boost::unordered_map<uint32_t, serverBlockPtr_t> serverBlockMap_t;
    typedef std::multimap<uint32_t, serverBlockPtr_t> serverBlockMultiMap_t;
    typedef boost::unordered_set<serverBlockPtr_t> serverBlockSet_t;

    ACE_Reactor *ioReactor;
    ACE_Lock *ioBarrier;

    std::tr1::shared_ptr<clientBlockMap_t> clientBllHandleMap;
    std::tr1::shared_ptr<clientBlockMultiMap_t> clientIfHandleMap;
    std::tr1::shared_ptr<clientBlockSet_t> runningClientSet;

    std::tr1::shared_ptr<serverBlockMap_t> serverBllHandleMap;
    std::tr1::shared_ptr<serverBlockMultiMap_t> serverIfHandleMap;
    std::tr1::shared_ptr<serverBlockSet_t> runningServerSet;

    std::tr1::shared_ptr<McoDriver> mcoDriver;

    ACE_Time_Value statsSyncTime;
    std::tr1::shared_ptr<IL_DAEMON_LIB_NS::GenericTimer> statsClearTimer;
    std::tr1::shared_ptr<handleVec_t> statsClearHandleVec;
};

///////////////////////////////////////////////////////////////////////////////

RawTcpApplication::RawTcpApplication(uint16_t ports)
    : mAppReactor(0), mNumPorts(ports)
{
    // Initialize database dictionary
    McoDriver::StaticInitialize();
}

///////////////////////////////////////////////////////////////////////////////

RawTcpApplication::~RawTcpApplication()
{
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::Activate(ACE_Reactor *appReactor, const std::vector<ACE_Reactor *>& ioReactorVec, const std::vector<ACE_Lock *>& ioBarrierVec)
{
    mAppReactor = appReactor;

    // Allocate per-port items for the physical ports at start-up time
    for (size_t port = 0; port < mNumPorts; port++)
    {
        AddPort(port, ioReactorVec.at(port), ioBarrierVec.at(port));
    }

    // Initialize RAWTCP protocol
    RawTcpProtocol::Init();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::Deactivate(void)
{
    mPorts.clear();
    mAppReactor = 0;
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    const size_t numClients = clients.size();

    // Construct RawTcpClientBlock objects first
    std::vector<PortContext::clientBlockPtr_t> clientBlockVec;
    clientBlockVec.reserve(numClients);

    BOOST_FOREACH(const clientConfig_t& config, clients)
    {
        PortContext::clientBlockPtr_t clientBlock(new RawTcpClientBlock(port, config, mAppReactor, thisPort.ioReactor, thisPort.ioBarrier));
        clientBlockVec.push_back(clientBlock);
    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numClients);

    BOOST_FOREACH(const PortContext::clientBlockPtr_t& clientBlock, clientBlockVec)
    {
        const uint32_t bllHandle = clientBlock->BllHandle();
        const uint32_t ifHandle = clientBlock->IfHandle();

        // Echo RawTcpClientBlock BLL handle back as the handle
        handles.push_back(bllHandle);

        // Insert new RawTcpClientBlock object into maps
        thisPort.clientBllHandleMap->insert(std::make_pair(bllHandle, clientBlock));
        thisPort.clientIfHandleMap->insert(std::make_pair(ifHandle, clientBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertRawTcpClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    AddPort(port);

    // Delete existing RawTcpClientBlock objects on these handles
    DeleteClients(port, handles);

    // Configure new RawTcpClientBlock objects
    handleList_t newHandles;
    ConfigClients(port, clients, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::DeleteClients(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Delete RawTcpClientBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == thisPort.clientBllHandleMap->end())
            continue;

        PortContext::clientBlockPtr_t clientBlock(iter->second);

        // Remove object from maps
        thisPort.clientBllHandleMap->erase(iter);
        thisPort.runningClientSet->erase(clientBlock);

        {
            const uint32_t ifHandle = clientBlock->IfHandle();
            PortContext::clientBlockMultiMap_t::iterator end = thisPort.clientIfHandleMap->end();
            PortContext::clientBlockMultiMap_t::iterator iter = thisPort.clientIfHandleMap->find(ifHandle);

            while (iter != end && iter->first == ifHandle)
            {
                if (iter->second == clientBlock)
                {
                    thisPort.clientIfHandleMap->erase(iter);
                    break;
                }

                ++iter;
            }
        }

        // Remove object from the stats database
        thisPort.mcoDriver->DeleteRawTcpClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Register RawTcpClientBlock load profile notifier
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            TC_LOG_ERR(0, "Failed to register load profile on invalid handle " << bllHandle);
            throw EInval();
        }

        iter->second->RegisterLoadProfileNotifier(notifier);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Unregister RawTcpClientBlock load profile notifier
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterLoadProfileNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    const size_t numServers = servers.size();

    // Construct RawTcpServerBlock objects first
    std::vector<PortContext::serverBlockPtr_t> serverBlockVec;
    serverBlockVec.reserve(numServers);

    BOOST_FOREACH(const serverConfig_t& config, servers)
    {
        PortContext::serverBlockPtr_t serverBlock(new RawTcpServerBlock(port, config, mAppReactor, thisPort.ioReactor, thisPort.ioBarrier));
        serverBlockVec.push_back(serverBlock);
    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numServers);

    BOOST_FOREACH(const PortContext::serverBlockPtr_t& serverBlock, serverBlockVec)
    {
        const uint32_t bllHandle = serverBlock->BllHandle();
        const uint32_t ifHandle = serverBlock->IfHandle();

        // Echo RawTcpServerBlock BLL handle back as the handle
        handles.push_back(bllHandle);

        // Insert new RawTcpServerBlock object into maps
        thisPort.serverBllHandleMap->insert(std::make_pair(bllHandle, serverBlock));
        thisPort.serverIfHandleMap->insert(std::make_pair(ifHandle, serverBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertRawTcpServerResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    AddPort(port);

    // Delete existing RawTcpServerBlock objects on these handles
    DeleteServers(port, handles);

    // Configure new RawTcpServerBlock objects
    handleList_t newHandles;
    ConfigServers(port, servers, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::DeleteServers(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Delete RawTcpServerBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
        if (iter == thisPort.serverBllHandleMap->end())
            continue;

        PortContext::serverBlockPtr_t serverBlock(iter->second);

        // Remove object from maps
        thisPort.serverBllHandleMap->erase(iter);
        thisPort.runningServerSet->erase(serverBlock);

        {
            const uint32_t ifHandle = serverBlock->IfHandle();
            PortContext::serverBlockMultiMap_t::iterator end = thisPort.serverIfHandleMap->end();
            PortContext::serverBlockMultiMap_t::iterator iter = thisPort.serverIfHandleMap->find(ifHandle);

            while (iter != end && iter->first == ifHandle)
            {
                if (iter->second == serverBlock)
                {
                    thisPort.serverIfHandleMap->erase(iter);
                    break;
                }

                ++iter;
            }
        }

        // Remove object from the stats database
        thisPort.mcoDriver->DeleteRawTcpServerResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::StartProtocol(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Start RawTcpClientBlock or RawTcpServerBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try RawTcpClientBlock lookup
        {
            PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
            if (iter != thisPort.clientBllHandleMap->end())
            {
                PortContext::clientBlockPtr_t clientBlock(iter->second);

                clientBlock->Start();
                clientBlock->Stats().Start();
                thisPort.runningClientSet->insert(clientBlock);
                continue;
            }
        }

        // Try RawTcpServerBlock lookup
        {
            PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
            if (iter != thisPort.serverBllHandleMap->end())
            {
                PortContext::serverBlockPtr_t serverBlock(iter->second);

                serverBlock->Start();
                serverBlock->Stats().Start();
                thisPort.runningServerSet->insert(serverBlock);
                continue;
            }
        }

        TC_LOG_ERR(0, "Failed to StartProtocol on invalid handle " << bllHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::StopProtocol(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Stop RawTcpClientBlock or RawTcpServerBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try RawTcpClientBlock lookup
        {
            PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
            if (iter != thisPort.clientBllHandleMap->end())
            {
                PortContext::clientBlockPtr_t clientBlock(iter->second);

                // Stop the block
                clientBlock->Stop();
                thisPort.runningClientSet->erase(clientBlock);

                // Sync the block's stats and update in the database
                RawTcpClientBlock::stats_t& stats = clientBlock->Stats();
                stats.Sync(true);
                thisPort.mcoDriver->UpdateRawTcpClientResult(stats);
                continue;
            }
        }

        // Try RawTcpServerBlock lookup
        {
            PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
            if (iter != thisPort.serverBllHandleMap->end())
            {
                PortContext::serverBlockPtr_t serverBlock(iter->second);

                // Stop the block
                serverBlock->Stop();
                thisPort.runningServerSet->erase(serverBlock);

                // Sync the block's stats and update in the database
                RawTcpServerBlock::stats_t& stats = serverBlock->Stats();
                stats.Sync(true);
                thisPort.mcoDriver->UpdateRawTcpServerResult(stats);
                continue;
            }
        }

        TC_LOG_ERR(0, "Failed to StopProtocol on invalid handle " << bllHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
{
    AddPort(port);

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

void RawTcpApplication::SyncResults(uint16_t port)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Only sync stats when they are older than MAX_STATS_AGE
    const ACE_Time_Value now = ACE_OS::gettimeofday();
    if ((now - thisPort.statsSyncTime) < MAX_STATS_AGE)
        return;

    thisPort.statsSyncTime = now;

    // For each running client block...
    BOOST_FOREACH(const PortContext::clientBlockPtr_t& clientBlock, *thisPort.runningClientSet)
    {
        // Sync the block's stats and update in the database
        RawTcpClientBlock::stats_t& stats = clientBlock->Stats();
        //stats.Sync();
        thisPort.mcoDriver->UpdateRawTcpClientResult(stats);
    }

    // For each running server block...
    BOOST_FOREACH(const PortContext::serverBlockPtr_t& serverBlock, *thisPort.runningServerSet)
    {
        // Sync the block's stats and update in the database
        RawTcpServerBlock::stats_t& stats = serverBlock->Stats();
        //stats.Sync();
        thisPort.mcoDriver->UpdateRawTcpServerResult(stats);
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Set loads on RawTcpClientBlock objects
    BOOST_FOREACH(const loadPair_t& loadPair, loadList)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(loadPair.ProtocolHandle);
        if (iter != thisPort.clientBllHandleMap->end())
        {
            PortContext::clientBlockPtr_t clientBlock(iter->second);

            // Set the dynamic load
            clientBlock->SetDynamicLoad(loadPair.LoadValue);
            continue;
        }

        TC_LOG_ERR(0, "Failed to SetDynamicLoad on invalid handle " << loadPair.ProtocolHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    PortContext::clientBlockMultiMap_t::iterator endClient = thisPort.clientIfHandleMap->end();;
    PortContext::serverBlockMultiMap_t::iterator endServer = thisPort.serverIfHandleMap->end();;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        {
            // Notify all RawTcpClientBlock objects on this interface handle that their interfaces are disabled
            PortContext::clientBlockMultiMap_t::iterator iter = thisPort.clientIfHandleMap->find(ifHandle);
            while (iter != endClient && iter->first == ifHandle)
            {
                iter->second->NotifyInterfaceDisabled();
                ++iter;
            }
        }

        {
            // Notify all RawTcpServerBlock objects on this interface handle that their interfaces are disabled
            PortContext::serverBlockMultiMap_t::iterator iter = thisPort.serverIfHandleMap->find(ifHandle);
            while (iter != endServer && iter->first == ifHandle)
            {
                iter->second->NotifyInterfaceDisabled();
                ++iter;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    handleList_t clientHandles, serverHandles;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        // Delete all RawTcpClientBlock objects on this interface handle
        BOOST_FOREACH(const PortContext::clientBlockMultiMap_t::value_type& pair, thisPort.clientIfHandleMap->equal_range(ifHandle))
            clientHandles.push_back(pair.second->BllHandle());

        // Delete all RawTcpServerBlock objects on this interface handle
        BOOST_FOREACH(const PortContext::serverBlockMultiMap_t::value_type& pair, thisPort.serverIfHandleMap->equal_range(ifHandle))
            serverHandles.push_back(pair.second->BllHandle());
    }

    if (!clientHandles.empty())
        DeleteClients(port, clientHandles);

    if (!serverHandles.empty())
        DeleteServers(port, serverHandles);
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::DoStatsClear(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    thisPort.statsSyncTime = ACE_OS::gettimeofday();

    if (handles.empty())
    {
        // For each RawTcpClientBlock object
        BOOST_FOREACH(PortContext::clientBlockMap_t::value_type& pair, *thisPort.clientBllHandleMap)
        {
            /// Reset the block's stats and update in the database
            RawTcpClientBlock::stats_t& stats = pair.second->Stats();
            stats.Reset();
            thisPort.mcoDriver->UpdateRawTcpClientResult(stats);
        }

        // For each RawTcpServerBlock object
        BOOST_FOREACH(PortContext::serverBlockMap_t::value_type& pair, *thisPort.serverBllHandleMap)
        {
            /// Reset the block's stats and update in the database
            RawTcpServerBlock::stats_t& stats = pair.second->Stats();
            stats.Reset();
            thisPort.mcoDriver->UpdateRawTcpServerResult(stats);
        }
    }
    else
    {
        // For selected objects
        BOOST_FOREACH(uint32_t bllHandle, handles)
        {
            // Try RawTcpClientBlock lookup
            {
                PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
                if (iter != thisPort.clientBllHandleMap->end())
                {
                    /// Reset the block's stats and update in the database
                    RawTcpClientBlock::stats_t& stats = iter->second->Stats();
                    stats.Reset();
                    thisPort.mcoDriver->UpdateRawTcpClientResult(stats);
                    continue;
                }
            }

            // Try RawTcpServerBlock lookup
            {
                PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
                if (iter != thisPort.serverBllHandleMap->end())
                {
                    /// Reset the block's stats and update in the database
                    RawTcpServerBlock::stats_t& stats = iter->second->Stats();
                    stats.Reset();
                    thisPort.mcoDriver->UpdateRawTcpServerResult(stats);
                    continue;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act)
{
    const size_t port = reinterpret_cast<const size_t>(act);
    AddPort(port);
    PortContext& thisPort(mPorts.at(port));

    DoStatsClear(port, *thisPort.statsClearHandleVec);
    thisPort.statsClearHandleVec->clear();
}

///////////////////////////////////////////////////////////////////////////////

void RawTcpApplication::AddPort(uint16_t port, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
{
    // Allocate per-port items. for port groups, they'll follow the pattern from the
    // framework's Application class and use port zero's reactor & barrier
    //
    // there IS an assumption that the physical ports are initialized at Application
    // start-up time before any port group handling is processed so mPorts[0]'s data
    // is available
    //
    if (mPorts.find(port) == mPorts.end())
    {
        if (ioReactor == NULL)
        {
            ioReactor = mPorts[0].ioReactor;
        }
        if (ioBarrier == NULL)
        {
            ioBarrier = mPorts[0].ioBarrier;
        }
        (void)mPorts[port];
        mPorts[port].ioReactor = ioReactor;
        mPorts[port].ioBarrier = ioBarrier;

        mPorts[port].clientBllHandleMap.reset(new PortContext::clientBlockMap_t);
        mPorts[port].clientIfHandleMap.reset(new PortContext::clientBlockMultiMap_t);
        mPorts[port].runningClientSet.reset(new PortContext::clientBlockSet_t);

        mPorts[port].serverBllHandleMap.reset(new PortContext::serverBlockMap_t);
        mPorts[port].serverIfHandleMap.reset(new PortContext::serverBlockMultiMap_t);
        mPorts[port].runningServerSet.reset(new PortContext::serverBlockSet_t);

        mPorts[port].mcoDriver.reset(new McoDriver(port));
        mPorts[port].statsClearHandleVec.reset(new PortContext::handleVec_t);
        mPorts[port].statsClearTimer.reset(new IL_DAEMON_LIB_NS::GenericTimer(fastdelegate::MakeDelegate(this, &RawTcpApplication::StatsClearTimeoutHandler), mAppReactor));
    }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST

void RawTcpApplication::GetRawTcpClientStats(uint16_t port, const handleList_t& handles, std::vector<RawTcpClientRawStats>& stats) const
{
    const PortContext& thisPort(mPorts.at(port));

    stats.clear();
    stats.reserve(handles.size());

    PortContext::clientBlockMap_t::const_iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(uint32_t bllHandle, handles)
    {
        // Try RawTcpClientBlock lookup
        PortContext::clientBlockMap_t::const_iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[RawTcpApplication::GetRawTcpClientStats] invalid handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR(0, err);
            throw EInval(err);
        }

        stats.push_back(iter->second->Stats());
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST

void RawTcpApplication::GetRawTcpServerStats(uint16_t port, const handleList_t& handles, std::vector<RawTcpServerRawStats>& stats) const
{
    const PortContext& thisPort(mPorts.at(port));

    stats.clear();
    stats.reserve(handles.size());

    PortContext::serverBlockMap_t::const_iterator end = thisPort.serverBllHandleMap->end();
    BOOST_FOREACH(uint32_t bllHandle, handles)
    {
        // Try RawTcpServerBlock lookup
        PortContext::serverBlockMap_t::const_iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[RawTcpApplication::GetRawTcpServerStats] invalid handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR(0, err);
            throw EInval(err);
        }

        stats.push_back(iter->second->Stats());
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////
