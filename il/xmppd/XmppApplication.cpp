/// @file
/// @brief XMPP Application implementation
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
#include <sstream>

#include <ildaemon/GenericTimer.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <FastDelegate.h>
#include <hal/TimeStamp.h>
#include <phxexception/PHXException.h>

#include "XmppApplication.h"
#include "XmppdLog.h"
#include "McoDriver.h"

USING_XMPP_NS;

///////////////////////////////////////////////////////////////////////////////

const ACE_Time_Value XmppApplication::MAX_STATS_AGE(0, 500000);  // 0.5 seconds

///////////////////////////////////////////////////////////////////////////////

struct XmppApplication::PortContext
{
    typedef std::vector<uint32_t> handleVec_t;

    typedef std::tr1::shared_ptr<XmppClientBlock> clientBlockPtr_t;
    typedef boost::unordered_map<uint32_t, clientBlockPtr_t> clientBlockMap_t;
    typedef std::multimap<uint32_t, clientBlockPtr_t> clientBlockMultiMap_t;
    typedef boost::unordered_set<clientBlockPtr_t> clientBlockSet_t;

    ACE_Reactor *ioReactor;
    ACE_Lock *ioBarrier;

    std::tr1::shared_ptr<clientBlockMap_t> clientBllHandleMap;
    std::tr1::shared_ptr<clientBlockMultiMap_t> clientIfHandleMap;
    std::tr1::shared_ptr<clientBlockSet_t> runningClientSet;

    std::tr1::shared_ptr<McoDriver> mcoDriver;

    ACE_Time_Value statsSyncTime;
    std::tr1::shared_ptr<IL_DAEMON_LIB_NS::GenericTimer> statsClearTimer;
    std::tr1::shared_ptr<handleVec_t> statsClearHandleVec;
};

///////////////////////////////////////////////////////////////////////////////

XmppApplication::XmppApplication(uint16_t ports)
    : mAppReactor(0)
{
    // Initialize database dictionary
    McoDriver::StaticInitialize();

    // Size per-port context vector, allocations are deferred until activation
    mPorts.resize(ports);
}

///////////////////////////////////////////////////////////////////////////////

XmppApplication::~XmppApplication()
{
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::Activate(ACE_Reactor *appReactor, const std::vector<ACE_Reactor *>& ioReactorVec, const std::vector<ACE_Lock *>& ioBarrierVec)
{
    mAppReactor = appReactor;
    // Allocate per-port items
    const size_t numPorts = mPorts.size();
    for (size_t port = 0; port < numPorts; port++)
    {
        mPorts[port].ioReactor = ioReactorVec.at(port);
        mPorts[port].ioBarrier = ioBarrierVec.at(port);

        mPorts[port].clientBllHandleMap.reset(new PortContext::clientBlockMap_t);
        mPorts[port].clientIfHandleMap.reset(new PortContext::clientBlockMultiMap_t);
        mPorts[port].runningClientSet.reset(new PortContext::clientBlockSet_t);

        mPorts[port].mcoDriver.reset(new McoDriver(port));
        mPorts[port].statsClearHandleVec.reset(new PortContext::handleVec_t);
        mPorts[port].statsClearTimer.reset(new IL_DAEMON_LIB_NS::GenericTimer(fastdelegate::MakeDelegate(this, &XmppApplication::StatsClearTimeoutHandler), mAppReactor));
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::Deactivate(void)
{
    mPorts.clear();
    mAppReactor = 0;
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::ConfigClients(uint16_t port, const clientConfigList_t& clients, handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    const size_t numClients = clients.size();

    // Construct XmppClientBlock objects first
    std::vector<PortContext::clientBlockPtr_t> clientBlockVec;
    clientBlockVec.reserve(numClients);

    BOOST_FOREACH(const clientConfig_t& config, clients)
    {
        PortContext::clientBlockPtr_t clientBlock(new XmppClientBlock(port, config, mAppReactor, thisPort.ioReactor, thisPort.ioBarrier));
        clientBlockVec.push_back(clientBlock);
    }

    // All objects were created successfully (i.e. without throwing exceptions) -- now commit them
    handles.clear();
    handles.reserve(numClients);

    BOOST_FOREACH(const PortContext::clientBlockPtr_t& clientBlock, clientBlockVec)
    {
        const uint32_t bllHandle = clientBlock->BllHandle();
        const uint32_t ifHandle = clientBlock->IfHandle();

        // Echo XmppClientBlock BLL handle back as the handle
        handles.push_back(bllHandle);

        // Insert new XmppClientBlock object into maps
        thisPort.clientBllHandleMap->insert(std::make_pair(bllHandle, clientBlock));
        TC_LOG_ERR(0,"[XmppApplication::ConfigClients] ekim bllhandle inserted is "<<bllHandle);
        thisPort.clientIfHandleMap->insert(std::make_pair(ifHandle, clientBlock));

        // Insert new results row into the stats database
        thisPort.mcoDriver->InsertXmppClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::UpdateClients(uint16_t port, const handleList_t& handles, const clientConfigList_t& clients)
{
    // Delete existing XmppClientBlock objects on these handles
    DeleteClients(port, handles);

    // Configure new XmppClientBlock objects
    handleList_t newHandles;
    ConfigClients(port, clients, newHandles);
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::DeleteClients(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    // Delete XmppClientBlock objects
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
        thisPort.mcoDriver->DeleteXmppClientResult(bllHandle);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::RegisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles, loadProfileNotifier_t notifier)
{
    PortContext& thisPort(mPorts.at(port));

    // Register XmppClientBlock load profile notifier
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

void XmppApplication::UnregisterClientLoadProfileNotifier(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    // Unregister XmppClientBlock load profile notifier
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterLoadProfileNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::RegisterXmppClients(uint16_t port, const handleList_t& handles)
{
    TC_LOG_ERR(0,"shig [XmppApplication::RegisterXmppClients] ekim function called");
    PortContext& thisPort(mPorts.at(port));

    // Start XmppClientBlock registrations
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
     
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[XmppApplication::RegisterXmppClients] invalid Client block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_CLIENT, err);
            throw EInval(err);
        }
        PortContext::clientBlockPtr_t clientBlock(iter->second);
        clientBlock->RegisterRegCompleteNotifier(boost::bind(&XmppApplication::RegStateCompletionHandler, this, clientBlock, thisPort));
        TC_LOG_ERR(0,"shig [XmppApplication::RegisterXmppClients] runningClientSet->insert("<<clientBlock<<")");
        clientBlock->Register();
        thisPort.runningClientSet->insert(clientBlock);
    }
}

///////////////////////////////////////////////////////////////////////////////
void XmppApplication::RegStateCompletionHandler(PortContext::clientBlockPtr_t clientBlock, PortContext& thisPort)
{
    TC_LOG_ERR(0,"shig [XmppApplication::RegStateCompletionHandler] deleting clientBlock "<<clientBlock<<" from running set");
    thisPort.runningClientSet->erase(clientBlock);
    clientBlock->UnregisterRegCompleteNotifier();

    // Sync the block's stats and update in the database one last time
    XmppClientBlock::stats_t& stats = clientBlock->Stats();
    thisPort.mcoDriver->UpdateXmppClientResult(stats);
}

void XmppApplication::UnregisterXmppClients(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    TC_LOG_ERR(0,"shig [XmppApplication::UnregisterXmppClients] called");
    // Start XmppClientBlock unregistrations
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[XmppApplication::UnregisterXmppClients] invalid Client block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_CLIENT, err);
            throw EInval(err);
        }
        PortContext::clientBlockPtr_t clientBlock(iter->second);
        clientBlock->RegisterRegCompleteNotifier(boost::bind(&XmppApplication::RegStateCompletionHandler, this, clientBlock, thisPort));
        clientBlock->Unregister();
        thisPort.runningClientSet->insert(clientBlock);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::CancelXmppClientsRegistrations(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));
    TC_LOG_ERR(0,"cncl [XmppApplication::CancelXmppClientsRegistrations] called");
    // Cancel XmppClientBlock registrations
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "cncl [XmppApplication::CancelXmppClientsRegistrations] invalid Client block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_CLIENT, err);
            throw EInval(err);
        }
        TC_LOG_ERR(0,"cncl [XmppApplication::CancelXmppClientsRegistrations] canceling bllhandle "<<bllHandle);
        iter->second->CancelRegistrations();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::RegisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles, regStateNotifier_t notifier)
{
    PortContext& thisPort(mPorts.at(port));

    // register state notifiers
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        TC_LOG_ERR(0,"[XmppApplication::RegisterXmppClientRegStateNotifier] ekim checking bll handle "<<bllHandle);
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[XmppApplication::RegisterXmppClientRegStateNotifier] invalid Client block handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR_LOCAL(port, LOG_CLIENT, err);
            throw EInval(err);
        }

        iter->second->RegisterRegStateNotifier(notifier);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::UnregisterXmppClientRegStateNotifier(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    // Unregister state notifiers
    PortContext::clientBlockMap_t::iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter != end)
            iter->second->UnregisterRegStateNotifier();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::StartProtocol(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    // Start XmppClientBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try XmppClientBlock lookup
        {
            PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
            if (iter != thisPort.clientBllHandleMap->end())
            {
                PortContext::clientBlockPtr_t clientBlock(iter->second);

                clientBlock->Start();
                thisPort.runningClientSet->insert(clientBlock);
                continue;
            }
        }

        TC_LOG_ERR(0, "Failed to StartProtocol on invalid handle " << bllHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::StopProtocol(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    // Stop XmppClientBlock objects
    BOOST_FOREACH(const uint32_t bllHandle, handles)
    {
        // Try XmppClientBlock lookup
        {
            PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
            if (iter != thisPort.clientBllHandleMap->end())
            {
                PortContext::clientBlockPtr_t clientBlock(iter->second);

                // Stop the block
                clientBlock->Stop();
                thisPort.runningClientSet->erase(clientBlock);

                // Sync the block's stats and update in the database
                XmppClientBlock::stats_t& stats = clientBlock->Stats();
                thisPort.mcoDriver->UpdateXmppClientResult(stats);
                continue;
            }
        }

        TC_LOG_ERR(0, "Failed to StopProtocol on invalid handle " << bllHandle);
        throw EPHXBadConfig();
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::ClearResults(uint16_t port, const handleList_t& handles, uint64_t absExecTime)
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

void XmppApplication::SyncResults(uint16_t port)
{
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
        XmppClientBlock::stats_t& stats = clientBlock->Stats();
        stats.Sync();
        thisPort.mcoDriver->UpdateXmppClientResult(stats);
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::SetDynamicLoad(uint16_t port, const loadPairList_t& loadList)
{
    PortContext& thisPort(mPorts.at(port));

    // Set loads on XmppClientBlock objects
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

void XmppApplication::NotifyInterfaceEnablePending(uint16_t port, const std::vector<IFMGR_CLIENT_NS::InterfaceEnable>& ifEnableVec)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::NotifyInterfaceUpdatePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    PortContext& thisPort(mPorts.at(port));

    PortContext::clientBlockMultiMap_t::iterator endClient = thisPort.clientIfHandleMap->end();;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        {
            // Notify all XmppClientBlock objects on this interface handle that their interfaces are disabled
            PortContext::clientBlockMultiMap_t::iterator iter = thisPort.clientIfHandleMap->find(ifHandle);
            while (iter != endClient && iter->first == ifHandle)
            {
                iter->second->NotifyInterfaceDisabled();
                ++iter;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::NotifyInterfaceDeletePending(uint16_t port, const std::vector<uint32_t>& ifHandleVec)
{
    PortContext& thisPort(mPorts.at(port));

    handleList_t clientHandles;
    BOOST_FOREACH(const uint32_t ifHandle, ifHandleVec)
    {
        // Delete all XmppClientBlock objects on this interface handle
        BOOST_FOREACH(const PortContext::clientBlockMultiMap_t::value_type& pair, thisPort.clientIfHandleMap->equal_range(ifHandle))
            clientHandles.push_back(pair.second->BllHandle());
    }

    if (!clientHandles.empty())
        DeleteClients(port, clientHandles);
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::DoStatsClear(uint16_t port, const handleList_t& handles)
{
    PortContext& thisPort(mPorts.at(port));

    thisPort.statsSyncTime = ACE_OS::gettimeofday();

    if (handles.empty())
    {
        // For each XmppClientBlock object
        BOOST_FOREACH(PortContext::clientBlockMap_t::value_type& pair, *thisPort.clientBllHandleMap)
        {
            /// Reset the block's stats and update in the database
            XmppClientBlock::stats_t& stats = pair.second->Stats();
            stats.Reset();
            thisPort.mcoDriver->UpdateXmppClientResult(stats);
        }
    }
    else
    {
        // For selected objects
        BOOST_FOREACH(uint32_t bllHandle, handles)
        {
            // Try XmppClientBlock lookup
            {
                PortContext::clientBlockMap_t::iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
                if (iter != thisPort.clientBllHandleMap->end())
                {
                    /// Reset the block's stats and update in the database
                    XmppClientBlock::stats_t& stats = iter->second->Stats();
                    stats.Reset();
                    thisPort.mcoDriver->UpdateXmppClientResult(stats);
                    continue;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void XmppApplication::StatsClearTimeoutHandler(const ACE_Time_Value& tv, const void *act)
{
    const size_t port = reinterpret_cast<const size_t>(act);
    PortContext& thisPort(mPorts.at(port));

    DoStatsClear(port, *thisPort.statsClearHandleVec);
    thisPort.statsClearHandleVec->clear();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST

void XmppApplication::GetXmppClientStats(uint16_t port, const handleList_t& handles, std::vector<XmppClientRawStats>& stats) const
{
    const PortContext& thisPort(mPorts.at(port));

    stats.clear();
    stats.reserve(handles.size());

    PortContext::clientBlockMap_t::const_iterator end = thisPort.clientBllHandleMap->end();
    BOOST_FOREACH(uint32_t bllHandle, handles)
    {
        // Try XmppClientBlock lookup
        PortContext::clientBlockMap_t::const_iterator iter = thisPort.clientBllHandleMap->find(bllHandle);
        if (iter == end)
        {
            std::ostringstream oss;
            oss << "[XmppApplication::GetXmppClientStats] invalid handle (" << bllHandle << ")";
            const std::string err(oss.str());
            TC_LOG_ERR(0, err);
            throw EInval(err);
        }

        stats.push_back(iter->second->Stats());
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////

