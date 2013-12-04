/// @file
/// @brief FTP Application implementation
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

#include "FtpApplication.h"
#include "FtpClientBlock.h"
#include "FtpdLog.h"
#include "FtpProtocol.h"
#include "FtpServerBlock.h"
#include "McoDriver.h"

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value FtpApplication::MAX_STATS_AGE(0, 500000);  // 0.5 seconds

///////////////////////////////////////////////////////////////////////////////

struct FtpApplication::PortContext
{
    typedef std::vector<uint32_t> handleVec_t;

    typedef std::tr1::shared_ptr<FtpClientBlock> clientBlockPtr_t;
    typedef boost::unordered_map<uint32_t, clientBlockPtr_t> clientBlockMap_t;
    typedef std::multimap<uint32_t, clientBlockPtr_t> clientBlockMultiMap_t;
    typedef boost::unordered_set<clientBlockPtr_t> clientBlockSet_t;

    typedef std::tr1::shared_ptr<FtpServerBlock> serverBlockPtr_t;
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

FtpApplication::FtpApplication(uint16_t ports)
    : mAppReactor(0), mNumPorts(ports)
{
    // Initialize database dictionary
    McoDriver::StaticInitialize();
}

///////////////////////////////////////////////////////////////////////////////

FtpApplication::~FtpApplication()
{
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::Activate(ACE_Reactor *appReactor, const std::vector<ACE_Reactor *>& ioReactorVec, const std::vector<ACE_Lock *>& ioBarrierVec)
{
    mAppReactor = appReactor;

    // Allocate per-port items for the physical ports at start-up time
    for (size_t port = 0; port < mNumPorts; port++)
    {
        AddPort(port, ioReactorVec.at(port), ioBarrierVec.at(port));
    }

    // Initialize FTP protocol
    FtpProtocol::Init();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::Deactivate(void)
{
    mPorts.clear();
    mAppReactor = 0;
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    const size_t numClients = clients.size();

    // Construct FtpClientBlock objects first
    std::vector<PortContext::clientBlockPtr_t> clientBlockVec;
    clientBlockVec.reserve(numClients);

    BOOST_FOREACH(const clientConfig_t& config, clients)
    {
        PortContext::clientBlockPtr_t clientBlock(new FtpClientBlock(port, config, mAppReactor, thisPort.ioReactor, thisPort.ioBarrier));
        clientBlockVec.push_back(clientBlock);
    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numClients);

    BOOST_FOREACH(const PortContext::clientBlockPtr_t& clientBlock, clientBlockVec)
    {
        const uint32_t bllHandle = clientBlock->BllHandle();
        const uint32_t ifHandle = clientBlock->IfHandle();

        // Echo FtpClientBlock BLL handle back as the handle
        handles.push_back(bllHandle);

        // Insert new FtpClientBlock object into maps
        thisPort.clientBllHandleMap->insert(std::make_pair(bllHandle, clientBlock));
        thisPort.clientIfHandleMap->insert(std::make_pair(ifHandle, clientBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertFtpClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    AddPort(port);

    // Delete existing FtpClientBlock objects on these handles
    DeleteClients(port, handles);

    // Configure new FtpClientBlock objects
    handleList_t newHandles;
    ConfigClients(port, clients, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::DeleteClients(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Delete FtpClientBlock objects
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
        thisPort.mcoDriver->DeleteFtpClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Register FtpClientBlock load profile notifier
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

void FtpApplication::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Unregister FtpClientBlock load profile notifier
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterLoadProfileNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::ConfigServers(uint16_t port, const serverConfigList_t& servers, handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    const size_t numServers = servers.size();

    // Construct FtpServerBlock objects first
    std::vector<PortContext::serverBlockPtr_t> serverBlockVec;
    serverBlockVec.reserve(numServers);

    BOOST_FOREACH(const serverConfig_t& config, servers)
    {
        PortContext::serverBlockPtr_t serverBlock(new FtpServerBlock(port, config, mAppReactor, thisPort.ioReactor, thisPort.ioBarrier));
        serverBlockVec.push_back(serverBlock);
    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numServers);

    BOOST_FOREACH(const PortContext::serverBlockPtr_t& serverBlock, serverBlockVec)
    {
        const uint32_t bllHandle = serverBlock->BllHandle();
        const uint32_t ifHandle = serverBlock->IfHandle();

        // Echo FtpServerBlock BLL handle back as the handle
        handles.push_back(bllHandle);

        // Insert new FtpServerBlock object into maps
        thisPort.serverBllHandleMap->insert(std::make_pair(bllHandle, serverBlock));
        thisPort.serverIfHandleMap->insert(std::make_pair(ifHandle, serverBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertFtpServerResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::UpdateServers(uint16_t port, const handleList_t& handles, const serverConfigList_t& servers)
{
    AddPort(port);

    // Delete existing FtpServerBlock objects on these handles
    DeleteServers(port, handles);

    // Configure new FtpServerBlock objects
    handleList_t newHandles;
    ConfigServers(port, servers, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::DeleteServers(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Delete FtpServerBlock objects
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
        thisPort.mcoDriver->DeleteFtpServerResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::StartProtocol(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Start FtpClientBlock or FtpServerBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try FtpClientBlock lookup
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

        // Try FtpServerBlock lookup
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

void FtpApplication::StopProtocol(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Stop FtpClientBlock or FtpServerBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try FtpClientBlock lookup
        {
            PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
            if (iter != thisPort.clientBllHandleMap->end())
            {
                PortContext::clientBlockPtr_t clientBlock(iter->second);

                // Stop the block
                clientBlock->Stop();
                thisPort.runningClientSet->erase(clientBlock);

                // Sync the block's stats and update in the database
                FtpClientBlock::stats_t& stats = clientBlock->Stats();
                stats.Sync(true);
                thisPort.mcoDriver->UpdateFtpClientResult(stats);

                continue;
            }
        }

        // Try FtpServerBlock lookup
        {
            PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
            if (iter != thisPort.serverBllHandleMap->end())
            {
                PortContext::serverBlockPtr_t serverBlock(iter->second);

                // Stop the block
                serverBlock->Stop();
                thisPort.runningServerSet->erase(serverBlock);

                // Sync the block's stats and update in the database
                FtpServerBlock::stats_t& stats = serverBlock->Stats();
                stats.Sync(true);
                thisPort.mcoDriver->UpdateFtpServerResult(stats);

                continue;
            }
        }

        TC_LOG_ERR(0, "Failed to StopProtocol on invalid handle " << bllHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
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

void FtpApplication::SyncResults(uint16_t port)
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
        FtpClientBlock::stats_t& stats = clientBlock->Stats();
        ACE_GUARD(FtpClientBlock::stats_t::lock_t, guard, stats.Lock());
        //stats.Sync();
        thisPort.mcoDriver->UpdateFtpClientResult(stats);
    }

    // For each running server block...
    BOOST_FOREACH(const PortContext::serverBlockPtr_t& serverBlock, *thisPort.runningServerSet)
    {
        // Sync the block's stats and update in the database
        FtpServerBlock::stats_t& stats = serverBlock->Stats();
        ACE_GUARD(FtpServerBlock::stats_t::lock_t, guard, stats.Lock());
        //stats.Sync();
        thisPort.mcoDriver->UpdateFtpServerResult(stats);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    // Set loads on FtpClientBlock objects
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
void FtpApplication::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    PortContext::clientBlockMultiMap_t::iterator endClient = thisPort.clientIfHandleMap->end();;
    PortContext::serverBlockMultiMap_t::iterator endServer = thisPort.serverIfHandleMap->end();;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        {
            // Notify all FtpClientBlock objects on this interface handle that their interfaces are disabled
            PortContext::clientBlockMultiMap_t::iterator iter = thisPort.clientIfHandleMap->find(ifHandle);
            while (iter != endClient && iter->first == ifHandle)
            {
                iter->second->NotifyInterfaceDisabled();
                ++iter;
            }
        }

        {
            // Notify all FtpServerBlock objects on this interface handle that their interfaces are disabled
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

void FtpApplication::NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    handleList_t clientHandles, serverHandles;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        // Delete all FtpClientBlock objects on this interface handle
        BOOST_FOREACH(const PortContext::clientBlockMultiMap_t::value_type& pair, thisPort.clientIfHandleMap->equal_range(ifHandle))
            clientHandles.push_back(pair.second->BllHandle());

        // Delete all FtpServerBlock objects on this interface handle
        BOOST_FOREACH(const PortContext::serverBlockMultiMap_t::value_type& pair, thisPort.serverIfHandleMap->equal_range(ifHandle))
            serverHandles.push_back(pair.second->BllHandle());
    }

    if (!clientHandles.empty())
        DeleteClients(port, clientHandles);

    if (!serverHandles.empty())
        DeleteServers(port, serverHandles);
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::DoStatsClear(uint16_t port, const handleList_t& handles)
{
    AddPort(port);

    PortContext& thisPort(mPorts.at(port));

    thisPort.statsSyncTime = ACE_OS::gettimeofday();

    if (handles.empty())
    {
        // For each FtpClientBlock object
        BOOST_FOREACH(PortContext::clientBlockMap_t::value_type& pair, *thisPort.clientBllHandleMap)
        {
            FtpClientBlock::stats_t& stats = pair.second->Stats();
            ACE_GUARD(FtpClientBlock::stats_t::lock_t, guard, stats.Lock());

            /// Reset the block's stats and update in the database
            stats.Reset();
            thisPort.mcoDriver->UpdateFtpClientResult(stats);
        }

        // For each FtpServerBlock object
        BOOST_FOREACH(PortContext::serverBlockMap_t::value_type& pair, *thisPort.serverBllHandleMap)
        {
            FtpServerBlock::stats_t& stats = pair.second->Stats();
            ACE_GUARD(FtpServerBlock::stats_t::lock_t, guard, stats.Lock());

            /// Reset the block's stats and update in the database
            stats.Reset();
            thisPort.mcoDriver->UpdateFtpServerResult(stats);
        }
    }
    else
    {
        // For selected objects
        BOOST_FOREACH(uint32_t bllHandle, handles)
        {
            // Try FtpClientBlock lookup
            {
                PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
                if (iter != thisPort.clientBllHandleMap->end())
                {
                    FtpClientBlock::stats_t& stats = iter->second->Stats();
                    ACE_GUARD(FtpClientBlock::stats_t::lock_t, guard, stats.Lock());

                    /// Reset the block's stats and update in the database
                    stats.Reset();
                    thisPort.mcoDriver->UpdateFtpClientResult(stats);
                    continue;
                }
            }

            // Try FtpServerBlock lookup
            {
                PortContext::serverBlockMap_t::iterator iter = thisPort.serverBllHandleMap->find(bllHandle);
                if (iter != thisPort.serverBllHandleMap->end())
                {
                    FtpServerBlock::stats_t& stats = iter->second->Stats();
                    ACE_GUARD(FtpServerBlock::stats_t::lock_t, guard, stats.Lock());

                    /// Reset the block's stats and update in the database
                    stats.Reset();
                    thisPort.mcoDriver->UpdateFtpServerResult(stats);
                    continue;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act)
{
    const size_t port = reinterpret_cast<const size_t>(act);
    AddPort(port);
    PortContext& thisPort(mPorts.at(port));

    DoStatsClear(port, *thisPort.statsClearHandleVec);
    thisPort.statsClearHandleVec->clear();
}

///////////////////////////////////////////////////////////////////////////////

void FtpApplication::AddPort(uint16_t port, ACE_Reactor *ioReactor, ACE_Lock *ioBarrier)
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
        mPorts[port].statsClearTimer.reset(new IL_DAEMON_LIB_NS::GenericTimer(fastdelegate::MakeDelegate(this, &FtpApplication::StatsClearTimeoutHandler), mAppReactor));
    }
}

///////////////////////////////////////////////////////////////////////////////
