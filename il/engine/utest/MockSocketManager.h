#include <list>

#include "AbstSocketManager.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class MockSocketMgr : public AbstSocketManager
{
  public:
    MockSocketMgr() {}

    virtual ~MockSocketMgr() {}

    int AsyncConnect(PlaylistInstance * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t connectDelegate, closeDelegate_t closeDelegate)
    {
        ConnectInfo ci;
        ci.playInst = pi;
        ci.serverPort = serverPort;
        ci.flowIdx = flowIdx;
        ci.connectDelegate = connectDelegate;
        ci.closeDelegate = closeDelegate;
        mConnects.push_back(ci);
        return 0;
    }

    void Listen(PlaylistInstance * pi, uint16_t serverPort, bool isTcp, int flowIdx, connectDelegate_t connectDelegate, closeDelegate_t closeDelegate)
    {
        ConnectInfo ci;
        ci.playInst = pi;
        ci.serverPort = serverPort;
        ci.flowIdx = flowIdx;
        ci.connectDelegate = connectDelegate;
        ci.closeDelegate = closeDelegate;
        mListens.push_back(ci);
    }

    void AsyncSend(int fd, const uint8_t * data, size_t dataLength, sendDelegate_t sendDelegate)
    {
        SendInfo si;
        si.fd = fd;
        si.data = data;
        si.dataLength = dataLength;
        si.sendDelegate = sendDelegate;
        mSends.push_back(si);
    }

    void AsyncRecv(int fd, size_t dataLength, recvDelegate_t recvDelegate)
    {
        RecvInfo ri;
        ri.fd = fd;
        ri.dataLength = dataLength;
        ri.recvDelegate = recvDelegate;
        mRecvs.push_back(ri);
    }

    void Close(int fd, bool reset)
    {
        mCloses.push_back(fd);
    }

    void Clean(int fd)
    {
        mCloses.push_back(fd);
    }

    // Mock activators

    
    // Mock accessors
    struct ConnectInfo
    {
        PlaylistInstance * playInst;
        uint16_t serverPort;
        int flowIdx;
        connectDelegate_t connectDelegate;
        closeDelegate_t closeDelegate;
    };
    
    struct SendInfo
    {
        int fd;
        const uint8_t * data;
        size_t dataLength; 
        sendDelegate_t sendDelegate;
    };

    struct RecvInfo
    {
        int fd;
        size_t dataLength; 
        recvDelegate_t recvDelegate;
    };

    typedef std::list<ConnectInfo> connectList_t;
    typedef std::list<ConnectInfo> listenList_t;
    typedef std::list<int> closeList_t;
    typedef std::list<SendInfo> sendList_t;
    typedef std::list<RecvInfo> recvList_t;
    
    connectList_t& GetConnects()
    {
        return mConnects;
    }

    listenList_t& GetListens()
    {
        return mListens;
    }

    closeList_t& GetCloses()
    {
        return mCloses;
    }

    sendList_t& GetSends()
    {
        return mSends;
    }

    recvList_t& GetRecvs()
    {
        return mRecvs;
    }

  private:
    connectList_t mConnects;
    listenList_t mListens;
    closeList_t mCloses;
    sendList_t mSends;
    recvList_t mRecvs;
};

