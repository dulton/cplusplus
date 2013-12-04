/// @file
/// @brief SIP transaction layer header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_TRANSACTION_LAYER_H_
#define _SIP_TRANSACTION_LAYER_H_

#include <memory>
#include <string>

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>
#include <ildaemon/GenericTimer.h>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <Tr1Adapter.h>

#include "SipCommon.h"
#include "SipMessage.h"
#include "SipUri.h"

// Forward declarations (global)
class ACE_Reactor;
class Packet;

#ifdef UNIT_TEST
class TestSipTransactionLayer;
#endif

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

class SipTransactionLayer : boost::noncopyable
{
  public:
    typedef boost::function1<bool, const Packet&> sendDelegate_t;
    typedef boost::function2<void, const SipMessage *, const std::string&> messageDelegate_t;

    SipTransactionLayer(uint16_t port, unsigned int ifIndex, const ACE_INET_Addr& localAddr, ACE_Reactor *reactor);
    ~SipTransactionLayer();

    // Config accessors
    const ACE_INET_Addr& Address(void) const { return mLocalAddr; }
    unsigned int IfIndex() const { return mIfIndex; }
    int AddressFamily(void) const { return mLocalAddr.get_type(); }

    // IPv4 ToS / IPv6 traffic class mutator
    void SetIpServiceLevel(uint8_t ipServiceLevel) { mIpServiceLevel = ipServiceLevel; }
    
    // Delegate mutators
    void SetSendDelegate(const sendDelegate_t& sendDelegate) { mSendDelegate = sendDelegate; }
    void SetClientMessageDelegate(const messageDelegate_t& messageDelegate) { mClientMessageDelegate = messageDelegate; }
    void SetServerMessageDelegate(const messageDelegate_t& messageDelegate) { mServerMessageDelegate = messageDelegate; }
    
    // Enable methods
    bool Enabled(void) const { return mEnabled; }
    void Enable(bool enable);
    
    // Client transaction methods
    bool SendClientRequest(SipMessage& msg);
    
    // Server-oriented methods
    bool SendServerStatus(const SipMessage& msg);

    /// Packet receive method
    void ReceivePacket(const Packet& packet);

  private:
    /// Private inner state class forward declarations (see SipTransactionLayerState.h)
    struct State;
    struct ClientInviteCallingState;
    struct ClientInviteProceedingState;
    struct ClientInviteCompletedState;
    struct ClientInviteTerminatedState;
    struct ClientNonInviteTryingState;
    struct ClientNonInviteProceedingState;
    struct ClientNonInviteCompletedState;
    struct ClientNonInviteTerminatedState;
    struct ServerInviteProceedingState;
    struct ServerInviteCompletedState;
    struct ServerInviteConfirmedState;
    struct ServerInviteTerminatedState;
    struct ServerNonInviteTryingState;
    struct ServerNonInviteProceedingState;
    struct ServerNonInviteCompletedState;
    struct ServerNonInviteTerminatedState;

    /// Inner transaction class
    struct Transaction
    {
        struct ClientData
        {
            explicit ClientData(const SipMessage& msg);
            
            ACE_INET_Addr peer;                         ///< server's address
            SipUri uri;                                 ///< request URI
            std::string callId;                         ///< Call-ID value
            uint32_t cseq;                              ///< CSeq value
            SipMessage::MethodType method;              ///< request method
        };
        
        struct ServerData
        {
            explicit ServerData(std::auto_ptr<SipMessage> msg);
            
            std::tr1::shared_ptr<SipMessage> msg;       ///< client's original request
            SipUri uri;                                 ///< request URI
            std::string branch;                         ///< transaction branch parameter
        };

        Transaction(uint32_t id, SipTransactionLayer& owner, const ClientData& clientData, ACE_Reactor *reactor);
        Transaction(uint32_t id, SipTransactionLayer& owner, const ServerData& serverData, ACE_Reactor *reactor);
        ~Transaction();

        uint32_t Id(void) const { return mId; }
        bool Matches(const SipMessage& msg) const;
        const std::string& LocalTag(void) const { return mLocalTag; }

        bool SendMessage(const SipMessage& msg);
        bool RetransmitMessage(void) const;

        bool SendInternalAckRequest(const SipMessage& msg) const;
        bool SendInternalTryingResponse(const SipMessage& msg);

        void ChangeState(const State *toState);
    
        void RetransmitTimerExpired(void);
        void TransactionTimerExpired(void);
        void InvokeTerminationHook(void) { if (mTerminationHook) mTerminationHook(); }

        const uint32_t mId;                                     ///< unique transaction id
        SipTransactionLayer &mOwner;                            ///< transaction layer
        boost::variant<ClientData, ServerData> mData;           ///< transaction-specific data
        const State *mState;                                    ///< current state
        ACE_Time_Value mRTT;                                    ///< current round-trip-time estimate
        std::auto_ptr<Packet> mPacket;                          ///< packet held for retransmission
        std::string mLocalTag;                                  ///< local tag of held packet (for UA timeout notifications)
        IL_DAEMON_LIB_NS::GenericTimer mRetransmitTimer;             ///< retransmit timer
        IL_DAEMON_LIB_NS::GenericTimer mTransactionTimer;            ///< transaction timer
        IL_DAEMON_LIB_NS::GenericTimer mTerminationTimer;            ///< termination timer
        boost::function0<void> mTerminationHook;                ///< terimnation hook
    };

    typedef std::tr1::shared_ptr<Transaction> transSharedPtr_t;
    typedef boost::unordered_map<uint32_t, transSharedPtr_t> transMap_t;

    /// Transaction id generator
    uint32_t NextTransactionId(void)
    {
        while (++mLastTransId == 0);
        return mLastTransId;
    }

    /// Transaction lookup by id
    transSharedPtr_t LookupTransaction(uint32_t transId) const
    {
        transMap_t::const_iterator iter = mTransMap.find(transId);
        return (iter != mTransMap.end()) ? iter->second : transSharedPtr_t();
    }
    
    /// Packet factory method
    std::auto_ptr<Packet> MakePacket(const SipMessage& msg);

    /// Send packet method
    bool SendPacket(const Packet& packet) { return mSendDelegate ? mSendDelegate(packet) : false; }
    
    /// Send message method
    std::auto_ptr<Packet> SendMessage(const SipMessage& msg);

    /// Receive message method
    void ReceiveMessage(std::auto_ptr<SipMessage> msg);

    /// Termination hook
    void TerminateTransaction(uint32_t id);
    
    /// Class data
    static const ClientInviteCallingState CLIENT_INVITE_CALLING_STATE;
    static const ClientInviteProceedingState CLIENT_INVITE_PROCEEDING_STATE;
    static const ClientInviteCompletedState CLIENT_INVITE_COMPLETED_STATE;
    static const ClientInviteTerminatedState CLIENT_INVITE_TERMINATED_STATE;
    static const ClientNonInviteTryingState CLIENT_NON_INVITE_TRYING_STATE;
    static const ClientNonInviteProceedingState CLIENT_NON_INVITE_PROCEEDING_STATE;
    static const ClientNonInviteCompletedState CLIENT_NON_INVITE_COMPLETED_STATE;
    static const ClientNonInviteTerminatedState CLIENT_NON_INVITE_TERMINATED_STATE;
    static const ServerInviteProceedingState SERVER_INVITE_PROCEEDING_STATE;
    static const ServerInviteCompletedState SERVER_INVITE_COMPLETED_STATE;
    static const ServerInviteConfirmedState SERVER_INVITE_CONFIRMED_STATE;
    static const ServerInviteTerminatedState SERVER_INVITE_TERMINATED_STATE;
    static const ServerNonInviteTryingState SERVER_NON_INVITE_TRYING_STATE;
    static const ServerNonInviteProceedingState SERVER_NON_INVITE_PROCEEDING_STATE;
    static const ServerNonInviteCompletedState SERVER_NON_INVITE_COMPLETED_STATE;
    static const ServerNonInviteTerminatedState SERVER_NON_INVITE_TERMINATED_STATE;

    static const ACE_Time_Value TIMER_T1;
    static const ACE_Time_Value TIMER_T2;
    static const ACE_Time_Value TIMER_T4;

    const uint16_t mPort;                               ///< physical port
    const unsigned int mIfIndex;                        ///< local interface index
    const ACE_INET_Addr mLocalAddr;                     ///< local IP address and port number
    uint8_t mIpServiceLevel;                            ///< IPv4 ToS or IPv6 traffic class
    ACE_Reactor *mReactor;                              ///< our reactor instance

    sendDelegate_t mSendDelegate;                       ///< send packet delegate
    messageDelegate_t mClientMessageDelegate;           ///< client message notification delegate
    messageDelegate_t mServerMessageDelegate;           ///< server message notification delegate

    uint32_t mLastTransId;                              ///< last transaction id counter
    bool mEnabled;                                      ///< enable flag
    transMap_t mTransMap;                               ///< active transactions, indexed by id

#ifdef UNIT_TEST
    friend class ::TestSipTransactionLayer;
#endif    
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
