/// @brief Generator client handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <queue>
#include <sstream>
#include <string>

#include <ildaemon/BoundMethodRequestImpl.tcc>

#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <DelegatedMessageHandler.h>

#include <mps/mps.h>
#include <mps/message.h>

#include "GeneratorClientHandler.h"

USING_FPGARTP_NS;

///////////////////////////////////////////////////////////////////////////////

#define GcGuard(A) ACE_Guard<ACE_Recursive_Thread_Mutex> __lock__##__LINE__(A)

///////////////////////////////////////////////////////////////////////////////

class GeneratorClientHandler::FpgaCommandSeq{
    public:
        FpgaCommandSeq(uint16_t port);
        ~FpgaCommandSeq(){};
        uint16_t port(void){return mPort;}
        void queue(const std::vector<uint32_t> &si);
        void queue(const std::vector<uint32_t> &si,asyncCompletionToken_t act);
        void queue(const std::vector<streamModifierArray_t> &mods);
        void swap(std::vector<uint32_t> &so);
        void swap(std::vector<uint32_t> &so,std::vector<asyncCompletionToken_t> &acts);
        void swap(std::vector<streamModifierArray_t> &mods);
    private:
        uint16_t mPort;
        std::vector<uint32_t> _si0;
        std::vector<uint32_t> _si1;
        std::vector<asyncCompletionToken_t> _acts;
        std::vector<streamModifierArray_t> _mods;
};
GeneratorClientHandler::FpgaCommandSeq::FpgaCommandSeq(uint16_t port) : mPort(port){
    _si0.clear();
    _si1.clear();
    _acts.clear();
    _mods.clear();

}
void GeneratorClientHandler::FpgaCommandSeq::queue(const std::vector<uint32_t> &si){
    std::vector<uint32_t>::const_iterator it;
    for(it=si.begin();it != si.end(); it++)
        _si1.push_back(*it);
}
void GeneratorClientHandler::FpgaCommandSeq::queue(const std::vector<uint32_t> &si,asyncCompletionToken_t act){
    std::vector<uint32_t>::const_iterator it;
    for(it = si.begin();it != si.end(); it++)
        _si0.push_back(*it);
    _acts.push_back(act);
}
void GeneratorClientHandler::FpgaCommandSeq::queue(const std::vector<streamModifierArray_t> &mods){
    std::vector<streamModifierArray_t>::const_iterator it;
    for(it=mods.begin();it != mods.end(); it++)
        _mods.push_back(*it);
}

void GeneratorClientHandler::FpgaCommandSeq::swap(std::vector<uint32_t> &so){
    _si1.swap(so);
}
void GeneratorClientHandler::FpgaCommandSeq::swap(std::vector<uint32_t> &so,std::vector<asyncCompletionToken_t> &acts){
    _si0.swap(so);
    _acts.swap(acts);
}
void GeneratorClientHandler::FpgaCommandSeq::swap(std::vector<streamModifierArray_t> &mods){
    _mods.swap(mods);
}

class GeneratorClientHandler::MessageSetClient : public MessageSet
{
public:
    MessageSetClient(void);
    ~MessageSetClient();

    void SetMethodCompletionDelegate(methodCompletionDelegate_t delegate) { mMethodCompletionDelegate = delegate; }
    void ClearMethodCompletionDelegate(void) { mMethodCompletionDelegate.clear(); }
    
    bool GetStreamIndexes(uint16_t port, uint32_t streamBlock, std::vector<uint32_t>& streamIndexes);
    void ModifyStreams(uint16_t port, const std::vector<streamModifierArray_t>& mods);
    void ControlStreams(uint16_t port,int type,const std::vector<uint32_t>& streamIndexes,std::vector<VOIP_NS::asyncCompletionToken_t>& acts);
    
private:
    void ResponseHandler(MPS_Handle *handle, uint16_t port, Message *response);
    void MethodComplete(VOIP_NS::asyncCompletionToken_t act, bool success);

    static const std::string MESSAGE_SET_NAME;
    methodCompletionDelegate_t mMethodCompletionDelegate;
};

const std::string GeneratorClientHandler::MessageSetClient::MESSAGE_SET_NAME = "Generator_3";

///////////////////////////////////////////////////////////////////////////////
    
GeneratorClientHandler::MessageSetClient::MessageSetClient(void)
{
    setMessageSetName(MESSAGE_SET_NAME);

    // Get the MPS handle for the Generator
    MPS *mps = MPS::instance();
    MPS_Handle *handle = mps->findMPSHandle(getMessageSetName());
    
    if (handle)
    {
        setHandle(handle);
        mps->localRegisterMessageSet(this);
    }
    else
    {
        std::ostringstream oss;
        oss << "[GeneratorClientHandler::MessageSetClient ctor] Could not find handle for " << MESSAGE_SET_NAME;
        const std::string err(oss.str());
        TC_LOG_ERR_LOCAL(0, LOG_GEN, err);
        throw EPHXUnknown(err);
    }
}

///////////////////////////////////////////////////////////////////////////////
    
GeneratorClientHandler::MessageSetClient::~MessageSetClient()
{
    MPS::instance()->localUnregisterMessageSet(this);
}

///////////////////////////////////////////////////////////////////////////////

void GeneratorClientHandler::SetMethodCompletionDelegate(methodCompletionDelegate_t delegate)
{
    mClient->SetMethodCompletionDelegate(delegate);
}

///////////////////////////////////////////////////////////////////////////////

void GeneratorClientHandler::ClearMethodCompletionDelegate(void)
{
    mClient->ClearMethodCompletionDelegate();
}

///////////////////////////////////////////////////////////////////////////////

bool GeneratorClientHandler::MessageSetClient::GetStreamIndexes(uint16_t port, uint32_t streamBlock, std::vector<uint32_t>& streamIndexes)
{
    MPS_Handle *handle = getHandle();
    if (!handle)
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GetStreamIndexes] Generator message server handle does not exist");
        return false;
    }

    // Build request
    std::vector<uint32_t> blockIds(1, streamBlock);
    Message req;
    create_req_GetStreamID(&req, blockIds);

    // Send request
    Message resp;
    if (handle->sendRequest(&req, &resp, port))
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GetStreamIndexes] Failed to send/receive " << req.get_request_name());
        return false;
    }

    if (resp.get_msg_type() != Message::TMT_RESPONSE)
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GetStreamIndexes] Got fault (" << resp.get_fault_code() << "): " << resp.get_fault_string());
        return false;
    }

    // Parse the response
    std::vector<streamIndexes_t> indexes;
    parse_resp_GetStreamID(&resp, &indexes);
    if (indexes.empty() || indexes[0].blockId != streamBlock)
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GetStreamIndexes] Got empty response or invalid response for stream block " << streamBlock);
        return false;
    }

    streamIndexes = indexes[0].streamIndexes;

    const size_t numIndexes = streamIndexes.size();
    if (numIndexes)
    {
        TC_LOG_DEBUG_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GenStreamIndexes] Stream block " << streamBlock << " has " << numIndexes << " stream indexes");
    }
    else
    {
        TC_LOG_WARN_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::GetStreamIndexes] Got empty stream index list for stream block " << streamBlock);
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
    
void GeneratorClientHandler::MessageSetClient::ModifyStreams(uint16_t port, const std::vector<streamModifierArray_t>& mods)
{
    MPS_Handle *handle = getHandle();
    if (!handle)
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::ModifyStreams] Generator message server handle does not exist");
        return;
    }
        
    // Build request
    Message req;
    create_req_ModifyStream(&req, mods);
    
    // Send request
    Message resp;
    if (handle->sendRequest(&req, &resp, port))
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::ModifyStreams] Failed to send/receive " << req.get_request_name());
    }
}

void GeneratorClientHandler::MessageSetClient::ControlStreams(uint16_t port,int type,const std::vector<uint32_t>& streamIndexes,std::vector<VOIP_NS::asyncCompletionToken_t>& acts){
    MPS_Handle *handle = getHandle();
    if (!handle)
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::ControlStreams] Generator message server handle does not exist");
        return;
    }
    Message req;
    switch(type){
        case 0:
            create_req_ControlStream(&req,GEN_CONTROL_STOP,streamIndexes);
            break;
        case  1:
            create_req_ControlStream(&req,GEN_CONTROL_START,streamIndexes);
            break;
        default:
            TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::ControlStreams] unknown fpga command");
            return;
    }
    Message resp;
    if (handle->sendRequest(&req, &resp, port))
    {
        TC_LOG_ERR_LOCAL(port, LOG_GEN, "[GeneratorClientHandler::MessageSetClient::ControlStreams] Failed to send/receive " << req.get_request_name());
        if(type == 0){
            for(size_t i=0;i< acts.size();i++)
                MethodComplete(acts[i],false);
        }
    }else{
        if(type == 0){
            for(size_t i=0;i< acts.size();i++)
                MethodComplete(acts[i],true);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
    
void GeneratorClientHandler::MessageSetClient::MethodComplete(asyncCompletionToken_t act, bool success)
{
    if (act.lock() && !mMethodCompletionDelegate.empty())
        mMethodCompletionDelegate(act, static_cast<int>(success));
}

///////////////////////////////////////////////////////////////////////////////

GeneratorClientHandler::GeneratorClientHandler(ACE_Reactor& reactor)
    : mOwner(ACE_OS::thr_self())
{
#ifndef UNIT_TEST      
    mClient.reset(new MessageSetClient());
#endif      
    methodSeqQueue.clear();
    fpga_streamIndexes.reserve(2048*2);
    fpga_acts.reserve(2048);
    fpga_modifiers.reserve(2048*2);
    ACE_Time_Value initialDelay(0,100);
    ACE_Time_Value interval(0,500);
    this->reactor(&reactor);
    this->reactor()->schedule_timer(this,this->reactor(),initialDelay,interval);
}

///////////////////////////////////////////////////////////////////////////////

GeneratorClientHandler::~GeneratorClientHandler()
{
    fpga_streamIndexes.clear();
    fpga_acts.clear();
    fpga_modifiers.clear();
    methodSeqQueue.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool GeneratorClientHandler::GetStreamIndexes(uint16_t port, uint32_t streamBlock, std::vector<uint32_t>& streamIndexes)
{
    if (ACE_OS::thr_equal(ACE_OS::thr_self(), mOwner))
        return mClient->GetStreamIndexes(port, streamBlock, streamIndexes);
    else
        throw EPHXInternal("GeneratorClientHandler::GetStreamIndexes");
}

///////////////////////////////////////////////////////////////////////////////

void GeneratorClientHandler::ModifyStreamsDestination(uint16_t port, std::vector<uint32_t>& streamIndexes,const Ipv4Address_t& addr, std::vector<uint16_t>&  udpPorts,uint8_t ipServiceLevel){
    size_t nbrofStreams = streamIndexes.size();
    std::vector<uint32_t>::iterator it;
    std::vector<streamModifierArray_t> modifiers;

    for(size_t i = 0; i< nbrofStreams; i++){
        streamModifierArray_t modA;
        modA.streams.assign(1,streamIndexes[i]);
        streamModifier_t destIpAddrModifier;
        destIpAddrModifier.refCount = -1;
        destIpAddrModifier.modifier.utype = STREAM_MOD_DATA_TYPE_DSTIPV4ADDR;
        destIpAddrModifier.modifier.uval.dstIpv4Addr = addr;

        streamModifier_t destUdpPortModifier;
        destUdpPortModifier.refCount = -1;
        destUdpPortModifier.modifier.utype = STREAM_MOD_DATA_TYPE_DSTUDPPORT;
        destUdpPortModifier.modifier.uval.dstUdpPort = udpPorts[i];

        streamModifier_t ipTosModifier;
        ipTosModifier.refCount = -1;
        ipTosModifier.modifier.utype = STREAM_MOD_DATA_TYPE_GENERIC;
        ipTosModifier.modifier.uval.generic.offsetReference = OFFSET_REFERENCE_IPV4;
        ipTosModifier.modifier.uval.generic.offset = 8;
        ipTosModifier.modifier.uval.generic.length= 8;
        ipTosModifier.modifier.uval.generic.data[0] = ipServiceLevel;

        std::vector<streamModifier_t> mods;
        mods.reserve(3);
        mods.push_back(destIpAddrModifier);
        mods.push_back(destUdpPortModifier);
        mods.push_back(ipTosModifier);
        modA.modifiers = mods;
        modifiers.push_back(modA);
    }
    if (ACE_OS::thr_equal(ACE_OS::thr_self(), mOwner))
        return mClient->ModifyStreams(port, modifiers);
    else
    {
        std::vector<fpga_method_seq_t>::iterator it;
        for(it=methodSeqQueue.begin(); it != methodSeqQueue.end(); it++){
            if((*it)->port() == port){
                GcGuard(mLock);
                (*it)->queue(modifiers);
                return;
            }
        }
        fpga_method_seq_t fms;
        fms.reset(new FpgaCommandSeq(port));
        fms->queue(modifiers);
        {
            GcGuard(mLockFpgaCommandSeq);
            methodSeqQueue.push_back(fms);
        }
    }

}
void GeneratorClientHandler::ModifyStreamsDestination(uint16_t port ,std::vector<uint32_t>& streamIndexes,const Ipv6Address_t& addr , std::vector<uint16_t>& udpPorts,uint8_t ipServiceLevel){
    size_t nbrofStreams = streamIndexes.size();
    std::vector<uint32_t>::iterator it;
    std::vector<streamModifierArray_t> modifiers;

    for(size_t i = 0; i< nbrofStreams; i++){
        streamModifierArray_t modA;
        modA.streams.assign(1,streamIndexes[i]);
        streamModifier_t destIpAddrModifier;
        destIpAddrModifier.refCount = -1;
        destIpAddrModifier.modifier.utype = STREAM_MOD_DATA_TYPE_DSTIPV6ADDR;
        destIpAddrModifier.modifier.uval.dstIpv6Addr = addr;

        streamModifier_t destUdpPortModifier;
        destUdpPortModifier.refCount = -1;
        destUdpPortModifier.modifier.utype = STREAM_MOD_DATA_TYPE_DSTUDPPORT;
        destUdpPortModifier.modifier.uval.dstUdpPort = udpPorts[i];

        streamModifier_t trafficClassModifer;
        trafficClassModifer.refCount = -1;
        trafficClassModifer.modifier.utype = STREAM_MOD_DATA_TYPE_GENERIC;
        trafficClassModifer.modifier.uval.generic.offsetReference = OFFSET_REFERENCE_IPV6;
        trafficClassModifer.modifier.uval.generic.offset = 4;
        trafficClassModifer.modifier.uval.generic.length= 8;
        trafficClassModifer.modifier.uval.generic.data[0] = ipServiceLevel;

        std::vector<streamModifier_t> mods;
        mods.reserve(3);
        mods.push_back(destIpAddrModifier);
        mods.push_back(destUdpPortModifier);
        mods.push_back(trafficClassModifer);
        modA.modifiers = mods;
        modifiers.push_back(modA);
    }
    if (ACE_OS::thr_equal(ACE_OS::thr_self(), mOwner))
        return mClient->ModifyStreams(port, modifiers);
    else
    {
        std::vector<fpga_method_seq_t>::iterator it;
        for(it=methodSeqQueue.begin(); it != methodSeqQueue.end(); it++){
            if((*it)->port() == port){
                GcGuard(mLock);
                (*it)->queue(modifiers);
                return;
            }
        }
        fpga_method_seq_t fms;
        fms.reset(new FpgaCommandSeq(port));
        fms->queue(modifiers);
        {
            GcGuard(mLockFpgaCommandSeq);
            methodSeqQueue.push_back(fms);
        }
    }

}

///////////////////////////////////////////////////////////////////////////////

void GeneratorClientHandler::StartStreams(uint16_t port, const std::vector<uint32_t>& streamIndexes, asyncCompletionToken_t act)
{
    std::vector<fpga_method_seq_t>::iterator it;
    for(it=methodSeqQueue.begin(); it != methodSeqQueue.end(); it++){
        if((*it)->port() == port){
            GcGuard(mLock);
            (*it)->queue(streamIndexes);
            return;
        }
    }
    fpga_method_seq_t fms;
    fms.reset(new FpgaCommandSeq(port));
    fms->queue(streamIndexes);
    {
        GcGuard(mLockFpgaCommandSeq);
        methodSeqQueue.push_back(fms);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GeneratorClientHandler::StopStreams(uint16_t port, const std::vector<uint32_t>& streamIndexes, asyncCompletionToken_t act)
{
    std::vector<fpga_method_seq_t>::iterator it;
    for(it=methodSeqQueue.begin(); it != methodSeqQueue.end(); it++){
        if((*it)->port() == port){
            GcGuard(mLock);
            (*it)->queue(streamIndexes,act);
            return;
        }
    }
    fpga_method_seq_t fms;
    fms.reset(new FpgaCommandSeq(port));
    fms->queue(streamIndexes,act);
    {
        GcGuard(mLockFpgaCommandSeq);
        methodSeqQueue.push_back(fms);
    }
}

///////////////////////////////////////////////////////////////////////////////

int GeneratorClientHandler::handle_timeout(const ACE_Time_Value &current_time, const void *arg){
    std::vector<fpga_method_seq_t>::iterator it;

    uint16_t port;
    size_t sz = 0;
    {
        GcGuard(mLockFpgaCommandSeq);
        sz = methodSeqQueue.size();
    }

    for(size_t i = 0;i < sz; i++){
        fpga_streamIndexes.clear();
        fpga_modifiers.clear();
        fpga_acts.clear();
        fpga_method_seq_t seq=methodSeqQueue[i];
        port = seq->port();
        if(true){
            GcGuard(mLock);
            seq->swap(fpga_modifiers);
        }
        if(!fpga_modifiers.empty())
            mClient->ModifyStreams(port,fpga_modifiers);

        if(true) {
            GcGuard(mLock);
            seq->swap(fpga_streamIndexes);
        }
        if(!fpga_streamIndexes.empty())
            mClient->ControlStreams(port,1,fpga_streamIndexes,fpga_acts);
        fpga_streamIndexes.clear();
        if(true) {
            GcGuard(mLock);
            seq->swap(fpga_streamIndexes,fpga_acts);
        }
        if(!fpga_streamIndexes.empty())
            mClient->ControlStreams(port,0,fpga_streamIndexes,fpga_acts);
        
    }
    return 0;

}

///////////////////////////////////////////////////////////////////////////////
