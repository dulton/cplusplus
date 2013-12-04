/// @file
/// @brief Mock engine - tracks delegate calls 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MOCK_ENGINE_H_
#define _MOCK_ENGINE_H_

#include <boost/bind.hpp>

#include "SocketManager.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

class MockEngine
{
public:
    MockEngine()
    {
    }

    SocketManager::connectDelegate_t GetConnectDelegate()
    {
        return SocketManager::connectDelegate_t(boost::bind(&MockEngine::OnConnect, this, _1));
    }


    SocketManager::connectDelegate_t GetAcceptDelegate()
    {
        return SocketManager::connectDelegate_t(boost::bind(&MockEngine::OnAccept, this, _1));
    }

    SocketManager::closeDelegate_t GetCloseDelegate()
    {
        return SocketManager::closeDelegate_t(boost::bind(&MockEngine::OnClose, this, _1));
    }

    SocketManager::sendDelegate_t GetSendDelegate()
    {
        return SocketManager::sendDelegate_t(boost::bind(&MockEngine::OnSend, this));
    }

    SocketManager::recvDelegate_t GetRecvDelegate()
    {
        return SocketManager::recvDelegate_t(boost::bind(&MockEngine::OnRecv, this, _1, _2));
    }

    SocketManager::spawnAcceptDelegate_t GetSpawnAcceptDelegate()
    {
        return SocketManager::spawnAcceptDelegate_t(boost::bind(&MockEngine::OnSpawnAccept, this, _1, _2, _3, _4));
    }

    void ResetMock()
    {
        mAccepts.clear();  
        mConnects.clear();  
        mCloses.clear(); 
        mSends.clear(); 
        mRecvs.clear(); 
    }

    struct AcceptInfo { int fd; };
    const std::vector<AcceptInfo>& GetAccepts() { return mAccepts; }

    struct ConnectInfo { int fd; };
    const std::vector<ConnectInfo>& GetConnects() { return mConnects; }

    struct CloseInfo { SocketManager::CloseType closeType; };
    const std::vector<CloseInfo>& GetCloses() { return mCloses; }

    struct SendInfo { /* nothing? */ };
    const std::vector<SendInfo>& GetSends() { return mSends; }

    struct RecvInfo { std::vector<uint8_t> data; };
    const std::vector<RecvInfo>& GetRecvs() { return mRecvs; }

    struct SpawnAcceptInfo { SocketManager::playlistInstance_t * pi; int fd; ACE_INET_Addr localAddr; ACE_INET_Addr remoteAddr; };
    const std::vector<SpawnAcceptInfo>& GetSpawnAccepts() { return mSpawnAccepts; }

private:
    bool OnAccept(int fd)
    {
        AcceptInfo info;
        info.fd = fd;
        mAccepts.push_back(info);
        return true;
    }

    bool OnConnect(int fd)
    {
        ConnectInfo info;
        info.fd = fd;
        mConnects.push_back(info);
        return true;
    }

    bool OnClose(SocketManager::CloseType closeType)
    {
        CloseInfo info;
        info.closeType = closeType;
        mCloses.push_back(info);
        return true;
    }

    void OnSend()
    {
        SendInfo info;
        mSends.push_back(info);
    }

    void OnRecv(const uint8_t * data, size_t dataLength)
    {
        RecvInfo info;
        info.data.assign(data, data + dataLength);
        mRecvs.push_back(info);
    }

    bool OnSpawnAccept(SocketManager::playlistInstance_t * pi, int fd, const ACE_INET_Addr& localAddr, const ACE_INET_Addr& remoteAddr)
    {
        SpawnAcceptInfo info;
        info.pi = pi;
        info.fd = fd;
        info.localAddr = localAddr;
        info.remoteAddr = remoteAddr;
        mSpawnAccepts.push_back(info);
        return true;
    }

    std::vector<AcceptInfo> mAccepts;
    std::vector<ConnectInfo> mConnects;
    std::vector<CloseInfo> mCloses;
    std::vector<SendInfo> mSends;
    std::vector<RecvInfo> mRecvs;
    std::vector<SpawnAcceptInfo> mSpawnAccepts;
};


///////////////////////////////////////////////////////////////////////////////

#endif

