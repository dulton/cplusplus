#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <string.h>

#include <ace/Event_Handler.h>
#include <ace/Thread_Mutex.h>
#include <ace/Reactor.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/INET_Addr.h>
#include <ace/Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>
#include <ace/Svc_Handler.h>

#include "abr_apple/src/abr_interfaces.h"
#include "abr_apple/src/abr_player.h"

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <errno.h>


#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67
#endif


#include <stdlib.h>
#include "AbrPlayer.h"


////////////////////////////////////////////////////////////////////////////////////
struct IoHandler: public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
};

////////////////////////////////////////////////////////////////////////////////////
typedef ACE_Connector<IoHandler, ACE_SOCK_CONNECTOR> IoConnector_t;

////////////////////////////////////////////////////////////////////////////////////
struct IoConnector: IoConnector_t
{
    IoConnector(ACE_Reactor *ioReactor, int flags) :
        ACE_Connector<IoHandler, ACE_SOCK_Connector>(ioReactor, flags)
        {}

};

////////////////////////////////////////////////////////////////////////////////////
struct MyPlayer: virtual GenericAbrPlayer, virtual Player, IoConnector,IoHandler
{

private:
    ACE_Reactor* mReactor;
    IoHandler* mMyHandler;
    IoConnector* mMyConnector;
    ACE_INET_Addr srvr;
public:
    MyPlayer(ACE_Reactor *ioReactor, int flags);


    ////////////////////////////////////////////////////////////////////////////////////
    void ReportToWithId(GenericAbrObserver* uGenericAbrObeserver, int uID)
    {
        mGenericAbrObeserver = uGenericAbrObeserver;
        mID = uID;
    }
    ////////////////////////////////////////////////////////////////////////////////////
    int Time();
    long StartTimeout(int, int);
    void CancelTimeout(long);

    ////////////////////////////////////////////////////////////////////////////////////
    int ConfigureConnector(string ServerIp, string ServerPort);
    ////////////////////////////////////////////////////////////////////////////////////

    int Connect();
    bool IsConnected();
    int Disconnect();
    int Write(void*, int);
    int Read(void*, int);
    void ClearToWrite();
    void ClearToRead();

private:
    int handle_timeout(const ACE_Time_Value &current_time, const void *act);
    ////////////////////////////////////////////////////////////////////////////////////
    int open(void* pPtr);
    int handle_input(ACE_HANDLE fd);
    int handle_output(ACE_HANDLE fd);
    int handle_close(ACE_HANDLE uHandle, ACE_Reactor_Mask uMask);
    int connect_svc_handler(IoHandler *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms);
    int activate_svc_handler(IoHandler* handler);
    ////////////////////////////////////////////////////////////////////////////////////
private:
    void NotifyStatusChanged(int uStatus);
    void NotifyStatusChanged(int uID, int uStatus);

public:
    virtual ~MyPlayer();

};


////////////////////////////////////////////////////////////////////////////////////
GenericAbrPlayer* CreateAppleAbrPlayer(ACE_Reactor *ioReactor, int flags)
{
    return new MyPlayer(ioReactor, flags);
}

////////////////////////////////////////////////////////////////////////////////////
void MyPlayer::NotifyStatusChanged(int uStatus)
{

    switch(uStatus)
    {
       case ACTION_GET_MANIFEST:
       case ACTION_GET_PLAYLIST:
       case ACTION_GET_FRAGMENT:
           mGenericAbrObeserver->NotifyStatusChanged(mID,GenericAbrObserver::STATUS_REPORT);
           break;

       case ACTION_ABORT:
           mGenericAbrObeserver->NotifyStatusChanged(mID,GenericAbrObserver::STATUS_ABORTED);
           break;
       case ACTION_STOP:
           mGenericAbrObeserver->NotifyStatusChanged(mID,GenericAbrObserver::STATUS_STOPPED);
          break;
       default:
          break;
    }

}

////////////////////////////////////////////////////////////////////////////////////
void MyPlayer::NotifyStatusChanged(int uID, int uStatus)
{
}

////////////////////////////////////////////////////////////////////////////////////

MyPlayer::MyPlayer(ACE_Reactor *ioReactor, int flags) :
    IoConnector(ioReactor, flags)
{
    mMyHandler = this;
    mMyConnector = this;
    mReactor = ioReactor;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::ConfigureConnector(string ServerIp, string ServerPort)
{
    srvr.set(atoi(ServerPort.c_str()), ServerIp.c_str());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////

int MyPlayer::Connect()
{
 //   cout<<"Connect ["<< mID<< "]"<< endl;
    int result = (int) connect(mMyHandler, srvr, ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR), mSrcAddr, 1);
//    if (result < 0 && errno != EWOULDBLOCK)
//    {
//        cout << result << " " << strerror(errno) << endl;
//        NotifyConnectError();
//    }
    return result;
}

bool MyPlayer::IsConnected()
{

    ACE_INET_Addr addr;
    return (this->peer().get_remote_addr(addr) != -1);

}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::Disconnect()
{
 //   cout<<"Disconnect ["<< mID<< "]"<< endl;
    return (int) peer().close();
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::Read(void* pBuffer, int szBuffer)
{
    int result = (int) peer().recv(pBuffer, szBuffer);

    if (result <= 0)
    {
        if (result == 0) Disconnect();
        else if (errno == EAGAIN || errno == EINTR) result = 0;
        else
        {
//            cout <<"------------------------------------------- "<<errno<<endl;
            NotifyReadError();
        }
    }
    mReactor->cancel_wakeup(mMyHandler, ACE_Event_Handler::READ_MASK);
    return result;
}


////////////////////////////////////////////////////////////////////////////////////
void MyPlayer::ClearToWrite()
{
    //mReactor->schedule_wakeup(mMyHandler, ACE_Event_Handler::WRITE_MASK);
    mReactor->register_handler(mMyHandler, ACE_Event_Handler::WRITE_MASK);
}

////////////////////////////////////////////////////////////////////////////////////
void MyPlayer::ClearToRead()
{
//    mReactor->schedule_wakeup(mMyHandler, ACE_Event_Handler::READ_MASK);
    mReactor->register_handler(mMyHandler, ACE_Event_Handler::READ_MASK);
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::Write(void* pBuffer, int szBuffer)
{
    int result = (int) peer().send(pBuffer, szBuffer);

    if (result <= 0)
    {
        if (result == 0) Disconnect();
        else if (errno == EAGAIN || errno == EINTR) result = 0;
        else NotifyWriteError();

    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::Time()
{
    ACE_UINT64 result = 0;
    const ACE_Time_Value now = ACE_OS::gettimeofday();
    now.msec(result);
    return (int) result;
}

////////////////////////////////////////////////////////////////////////////////////
void MyPlayer::CancelTimeout(long id)
{
    mReactor->cancel_timer(id);
}

////////////////////////////////////////////////////////////////////////////////////
long MyPlayer::StartTimeout(int id, int time)
{
    ACE_Time_Value delay;
    delay.msec((long) time);
    return (long) mReactor->schedule_timer(mMyHandler, (const void*) id, delay, ACE_Time_Value::zero);
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::handle_timeout(const ACE_Time_Value &current_time, const void *act)
{
    NotifyTimeoutExpired((int) act);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::open(void* pPtr)
{
 //   cout<<"open ["<< mID<< "]"<< endl;
    peer().enable(ACE_NONBLOCK);
//    mReactor->register_handler(mMyHandler, ACE_Event_Handler::DONT_CALL);
//    mReactor->register_handler(mMyHandler, ACE_Event_Handler::ALL_EVENTS_MASK);
    mReactor->register_handler(mMyHandler, ACE_Event_Handler::EXCEPT_MASK);
    NotifyConnected();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::handle_input(ACE_HANDLE fd)
{
  //  cout<<"NotifyClearToRead ["<< mID<< "]"<< endl;
    NotifyClearToRead();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::handle_output(ACE_HANDLE fd)
{
  //  cout<<"NotifyClearToWrite ["<< mID<< "]"<< endl;
    //mReactor->remove_handler(mMyHandler, ACE_Event_Handler::WRITE_MASK);
    mReactor->cancel_wakeup(mMyHandler, ACE_Event_Handler::WRITE_MASK);
    NotifyClearToWrite();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask uMask)
{
    // cout<<"close ["<< mID<< "]"<< endl;
    //mReactor->remove_handler(mMyHandler, ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL);
    mReactor->remove_handler(mMyHandler, ACE_Event_Handler::ALL_EVENTS_MASK );
    NotifyDisconnected();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::connect_svc_handler(IoHandler *&handler, const ACE_INET_Addr& remote_addr, ACE_Time_Value *timeout, const ACE_INET_Addr& local_addr, int reuse_addr, int flags, int perms)
{
    ACE_SOCK_Stream &peer = handler->peer();

    // Peer socket shouldn't be open yet, so we have to open it ourselves (note: ACE will use the pre-opened socket without opening a new one)
    if (peer.get_handle() == ACE_INVALID_HANDLE)
    {
        if (peer.open(SOCK_STREAM, remote_addr.get_type(), 0, reuse_addr) == -1)
            return -1;
    }
    // Set socket options before connection process is started
    if (local_addr.get_type() == AF_INET)
    {
        const int tos = mIpv4Tos;
        peer.set_option(IPPROTO_IP, IP_TOS, (char *) &tos, sizeof(tos));
    }

    if (local_addr.get_type() == AF_INET6)
    {
        const int tclass = mIpv6TrafficClass;
        peer.set_option(IPPROTO_IPV6, IPV6_TCLASS, (char *) &tclass, sizeof(tclass));
    }

    if (mTcpWindowSizeLimit)
    {
        const int windowClamp = static_cast<const int>(mTcpWindowSizeLimit);
        peer.set_option(IPPROTO_TCP, TCP_WINDOW_CLAMP, (char *) &windowClamp, sizeof(windowClamp));
    }


    const int quickAck = mTcpDelayedAck ? 0 : 1;
    peer.set_option(IPPROTO_TCP, TCP_QUICKACK, (char *) &quickAck, sizeof(quickAck));

    // If the user is requesting a specific interface, we need to bind to that device before the connection process is started
    if (!mIfName.empty())
    {
        peer.set_option(SOL_SOCKET, SO_BINDTODEVICE, const_cast<char *>(mIfName.c_str()), mIfName.size() + 1);
    }

    const int err = ACE_Connector<IoHandler, ACE_SOCK_Connector>::connect_svc_handler(handler, remote_addr, timeout, local_addr, reuse_addr, flags, perms);
    if (err == -1 && errno == EWOULDBLOCK)
    {
    }

    return err;
}

////////////////////////////////////////////////////////////////////////////////////
int MyPlayer::activate_svc_handler(IoHandler* handler)
{
 //   handler->add_reference();
    const int err = ACE_Connector<IoHandler, ACE_SOCK_Connector>::activate_svc_handler(handler);

    return err;
}

////////////////////////////////////////////////////////////////////////////////////
MyPlayer::~MyPlayer()
{
}
