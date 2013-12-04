/// @file
/// @brief SIP transaction layer implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <algorithm>
#include <ext/functional>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <phxexception/PHXException.h>
#include <vif/Packet.h>

#include "SipProtocol.h"
#include "SipTransactionLayer.h"
#include "VoIPLog.h"
#include "VoIPUtils.h"

USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

#define _DEFINE_SIP_TRANSACTION_LAYER_STATES_
#include "SipTransactionLayerState.h"

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::Transaction::ClientData::ClientData(const SipMessage& msg)
    : peer(msg.peer),
      uri(msg.request.uri),
      cseq(msg.seq),
      method(msg.method)
{
    const SipMessage::HeaderField *callIdHdr = msg.FindHeader(SipMessage::SIP_HEADER_CALL_ID);
    if (!callIdHdr)
    {
        const std::string err("[SipTransactionLayer::Transaction::ClientData::ClientData] Message is missing Call-ID header");
        TC_LOG_ERR_LOCAL(0, LOG_SIP, err);
        throw EPHXInternal(err);
    }

    this->callId = callIdHdr->value;
}

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::Transaction::ServerData::ServerData(std::auto_ptr<SipMessage> theMsg)
    : msg(theMsg)
{
    const SipMessage::HeaderField *viaHdr = msg->FindHeader(SipMessage::SIP_HEADER_VIA);
    if (!viaHdr || !SipProtocol::ParseViaString(viaHdr->value, this->uri, &this->branch) || this->branch.empty())
    {
        const std::string err("[SipTransactionLayer::Transaction::ServerData::ServerData] Message has missing or invalid Via header");
        TC_LOG_ERR_LOCAL(0, LOG_SIP, err);
        throw EPHXInternal(err);
    }
}

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::Transaction::Transaction(uint32_t id, SipTransactionLayer& owner, const ClientData& clientData, ACE_Reactor *reactor)
    : mId(id),
      mOwner(owner),
      mData(clientData),
      mState((clientData.method == SipMessage::SIP_METHOD_INVITE) ? static_cast<const State *>(&CLIENT_INVITE_CALLING_STATE) : static_cast<const State *>(&CLIENT_NON_INVITE_TRYING_STATE)),
      mRetransmitTimer(boost::bind(&SipTransactionLayer::Transaction::RetransmitTimerExpired, this), reactor),
      mTransactionTimer(boost::bind(&SipTransactionLayer::Transaction::TransactionTimerExpired, this), reactor),
      mTerminationTimer(boost::bind(&SipTransactionLayer::Transaction::InvokeTerminationHook, this), reactor)
{
}

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::Transaction::Transaction(uint32_t id, SipTransactionLayer& owner, const ServerData& serverData, ACE_Reactor *reactor)
    : mId(id),
      mOwner(owner),
      mData(serverData),
      mState((serverData.msg->method == SipMessage::SIP_METHOD_INVITE) ? static_cast<const State *>(&SERVER_INVITE_PROCEEDING_STATE) : static_cast<const State *>(&SERVER_NON_INVITE_TRYING_STATE)),
      mRetransmitTimer(boost::bind(&SipTransactionLayer::Transaction::RetransmitTimerExpired, this), reactor),
      mTransactionTimer(boost::bind(&SipTransactionLayer::Transaction::TransactionTimerExpired, this), reactor),
      mTerminationTimer(boost::bind(&SipTransactionLayer::Transaction::InvokeTerminationHook, this), reactor)
{
}

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::Transaction::~Transaction()
{
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::Transaction::Matches(const SipMessage& msg) const
{
    bool match = false;
    
    if (msg.type == SipMessage::SIP_MSG_REQUEST)
    {
        const Transaction::ServerData *serverData(boost::get<Transaction::ServerData>(&mData));
        if (serverData)
        {
            const SipMessage::HeaderField *viaHdr = msg.FindHeader(SipMessage::SIP_HEADER_VIA);
            SipUri uri;
            std::string branch;
            
            if (viaHdr && SipProtocol::ParseViaString(viaHdr->value, uri, &branch))
            {
                const bool ackToInvite = (msg.method == SipMessage::SIP_METHOD_ACK && serverData->msg->method == SipMessage::SIP_METHOD_INVITE);
                match = (branch == serverData->branch) && (uri.GetSipHost() == serverData->uri.GetSipHost()) && (uri.GetSipPort() == serverData->uri.GetSipPort()) && (msg.method == serverData->msg->method || ackToInvite);
            }
        }
        
    }
    else  // msg.type == SipMessage::SIP_MSG_STATUS
    {
        const Transaction::ClientData *clientData(boost::get<Transaction::ClientData>(&mData));
        if (clientData)
            match = VOIP_NS::VoIPUtils::AddrKey::equal(msg.peer,clientData->peer) && (msg.callId == clientData->callId) && (msg.seq == clientData->cseq);
    }

    return match;
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::Transaction::SendMessage(const SipMessage& msg) 
{
    mPacket = mOwner.SendMessage(msg);
    if (mPacket.get())
    {
        mLocalTag = (msg.type == SipMessage::SIP_MSG_REQUEST) ? msg.fromTag : msg.toTag;
        return true;
    }
    else
    {
        mLocalTag.clear();
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::Transaction::RetransmitMessage(void) const
{
    return mPacket.get() ? mOwner.SendPacket(*mPacket) : false;
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::Transaction::SendInternalAckRequest(const SipMessage& msg) const
{
    const ClientData& clientData(boost::get<ClientData>(mData));
    
    SipMessage req;
    req.peer = clientData.peer;
    req.type = SipMessage::SIP_MSG_REQUEST;
    req.method = SipMessage::SIP_METHOD_ACK;
    req.request.uri = clientData.uri;
    req.request.version = SipMessage::SIP_VER_2_0;

    // Internally generated ACK request only contains the top-most VIA header from the response
    const SipMessage::HeaderField *viaHdr = msg.FindHeader(SipMessage::SIP_HEADER_VIA);
    if (viaHdr)
        req.headers.push_back(*viaHdr);
    else
        return false;
    
    msg.ForEachHeader(SipMessage::SIP_HEADER_TO, SipMessage::PushHeader(req));
    msg.ForEachHeader(SipMessage::SIP_HEADER_FROM, SipMessage::PushHeader(req));
    msg.ForEachHeader(SipMessage::SIP_HEADER_CALL_ID, SipMessage::PushHeader(req));
    req.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(clientData.cseq, req.method)));
    req.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    boost::scoped_ptr<Packet> packet(mOwner.MakePacket(req));
    return mOwner.SendPacket(*packet);
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::Transaction::SendInternalTryingResponse(const SipMessage& msg)
{
    if (!mPacket.get())
    {
        SipMessage resp;
    
        resp.peer = msg.peer;
        resp.type = SipMessage::SIP_MSG_STATUS;
        resp.status.code = 100;
        resp.status.version = SipMessage::SIP_VER_2_0;
        resp.status.message = "Trying";

        msg.ForEachHeader(SipMessage::SIP_HEADER_VIA, SipMessage::PushHeader(resp));
        msg.ForEachHeader(SipMessage::SIP_HEADER_TO, SipMessage::PushHeader(resp));
        msg.ForEachHeader(SipMessage::SIP_HEADER_FROM, SipMessage::PushHeader(resp));
        msg.ForEachHeader(SipMessage::SIP_HEADER_CALL_ID, SipMessage::PushHeader(resp));
        msg.ForEachHeader(SipMessage::SIP_HEADER_CSEQ, SipMessage::PushHeader(resp));
        resp.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

        mPacket = mOwner.MakePacket(resp);
        mLocalTag.clear();
    }
    
    return mOwner.SendPacket(*mPacket);
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::Transaction::ChangeState(const State *toState)
{
    TC_LOG_INFO_LOCAL(mOwner.mPort, LOG_SIP, "[SipTransactionLayer::Transaction::ChangeState] changing state from " << mState->Name() << " to " << toState->Name());
    mState = toState;
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::Transaction::RetransmitTimerExpired(void)
{
    mState->RetransmitTimerExpired(*this);
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::Transaction::TransactionTimerExpired(void)
{
    mState->TransactionTimerExpired(*this);
}

///////////////////////////////////////////////////////////////////////////////

// Timer values from RFC 3261, Appendix A
const ACE_Time_Value SipTransactionLayer::TIMER_T1(0, 500000); 
const ACE_Time_Value SipTransactionLayer::TIMER_T2(4, 0);
const ACE_Time_Value SipTransactionLayer::TIMER_T4(5, 0);

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::SipTransactionLayer(uint16_t port, unsigned int ifIndex, const ACE_INET_Addr& localAddr, ACE_Reactor *reactor)
    : mPort(port),
      mIfIndex(ifIndex),
      mLocalAddr(localAddr),
      mIpServiceLevel(0),
      mReactor(reactor),
      mLastTransId(0),
      mEnabled(true)
{
}

///////////////////////////////////////////////////////////////////////////////

SipTransactionLayer::~SipTransactionLayer()
{
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::Enable(bool enable)
{
    if (mEnabled && !enable)
    {
        // Terminate transactions and disable layer
        mLastTransId = 0;
        mTransMap.clear();
        mEnabled = false;
    }
    else if (!mEnabled && enable)
    {
        // Re-enable layer
        mEnabled = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::SendClientRequest(SipMessage& msg)
{
    if (!mEnabled)
    {
        const std::string err("[SipTransactionLayer::StartClientTransaction] Transaction layer is disabled");
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXInternal(err);
    }

    if (msg.method != SipMessage::SIP_METHOD_ACK)
    {
        const uint32_t transId = NextTransactionId();
        
        // Create a new client transaction and send request through state machine
        transSharedPtr_t trans(new Transaction(transId, *this, Transaction::ClientData(msg), mReactor));
        trans->mTerminationHook = boost::bind(&SipTransactionLayer::TerminateTransaction, this, transId);
        mTransMap.insert(std::make_pair(transId, trans));
        msg.transactionId = transId;
        return trans->mState->SendMessage(*trans, msg);
    }
    else
    {
        // ACK requests are passed directly to the transport (RFC 3261, Section 17.1)
        std::auto_ptr<Packet> packet = SendMessage(msg);
        return packet.get() ? true : false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::SendServerStatus(const SipMessage& msg)
{
    if (!mEnabled)
    {
        const std::string err("[SipTransactionLayer::SendServerStatus] Transaction layer is disabled");
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXInternal(err);
    }

    // Lookup active server transaction and 
    transMap_t::const_iterator iter = mTransMap.find(msg.transactionId);
    if (iter != mTransMap.end())
    {
        // Send status through transaction state machine
        return iter->second->mState->SendMessage(*iter->second, msg);
    }
    else
    {
        // No transaction specified, send directly to transport
        std::auto_ptr<Packet> packet = SendMessage(msg);
        return packet.get() ? true : false;
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::ReceivePacket(const Packet& packet)
{
    if (!mEnabled)
    {
        TC_LOG_DEBUG_LOCAL(mPort, LOG_SIP, "[SipTransactionLayer::ReceivePacket] Transaction layer is disabled, dropping packet");
        return;
    }
    
    std::auto_ptr<SipMessage> msg(new SipMessage);

    // Save source IP address as message peer address
    const int sinLen = (packet.SrcAddr.storage.ss_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    msg->peer.set(&packet.SrcAddr.sin, sinLen);

    // Parse SIP protocol headers
    if (!SipProtocol::ParsePacket(packet, *msg))
    {
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, "[SipTransactionLayer::ReceivePacket] Couldn't parse incoming SIP packet");
        return;
    }

    ReceiveMessage(msg);
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<Packet> SipTransactionLayer::MakePacket(const SipMessage& msg)
{
    // The message most specify a peer address
    if (msg.peer.is_any())
    {
        const std::string err("[SipTransactionLayer::MakePacket] Internal error, no peer address specified");
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXInternal(err);
    }
    
    // Build a new packet
    std::auto_ptr<Packet> packet(new Packet);
    packet->Port = mPort;
    packet->IfIndex = mIfIndex;
    packet->Flags = mIpServiceLevel;
    
    if (mLocalAddr.get_type() == AF_INET)
    {
        const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in *>(mLocalAddr.get_addr());
        packet->SrcAddr.sin = *sin;
    }
    else
    {
        const struct sockaddr_in6 *sin6 = reinterpret_cast<const struct sockaddr_in6 *>(mLocalAddr.get_addr());
        packet->SrcAddr.sin6 = *sin6;
    }

    if (msg.peer.get_type() == AF_INET)
    {
        const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in *>(msg.peer.get_addr());
        packet->DestAddr.sin = *sin;
    }
    else
    {
        const struct sockaddr_in6 *sin6 = reinterpret_cast<const struct sockaddr_in6 *>(msg.peer.get_addr());
        packet->DestAddr.sin6 = *sin6;
    }

    if (!SipProtocol::BuildPacket(msg, *packet))
    {
        const std::string err("[SipTransactionLayer::MakePacket] Internal error, failed to build SIP packet");
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXInternal(err);
    }

    return packet;
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<Packet> SipTransactionLayer::SendMessage(const SipMessage& msg)
{
    // Build request packet and send
    std::auto_ptr<Packet> packet = MakePacket(msg);
    if (!SendPacket(*packet))
        packet.reset();

    return packet;
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::ReceiveMessage(std::auto_ptr<SipMessage> msg)
{
    // Locate mandatory header fields (RFC 3261, Section 8.1.1)
    const SipMessage::HeaderField *fromHdr = msg->FindHeader(SipMessage::SIP_HEADER_FROM);
    if (!fromHdr || !SipProtocol::ParseAddressString(fromHdr->value, &msg->fromUri, &msg->fromTag))
        return;

    const SipMessage::HeaderField *toHdr = msg->FindHeader(SipMessage::SIP_HEADER_TO);
    if (!toHdr || !SipProtocol::ParseAddressString(toHdr->value, &msg->toUri, &msg->toTag))
        return;

    const SipMessage::HeaderField *callIdHdr = msg->FindHeader(SipMessage::SIP_HEADER_CALL_ID);
    if (callIdHdr)
        msg->callId = callIdHdr->value;
    else
        return;
        
    const SipMessage::HeaderField *cseqHdr = msg->FindHeader(SipMessage::SIP_HEADER_CSEQ);
    SipMessage::MethodType method;
    if (!cseqHdr || !SipProtocol::ParseCSeqString(cseqHdr->value, msg->seq, method))
        return;

    if (msg->type != SipMessage::SIP_MSG_REQUEST)
        msg->method = method;

    // Find matching transaction
    transMap_t::const_iterator iter = std::find_if(mTransMap.begin(), mTransMap.end(), boost::bind(&Transaction::Matches, boost::bind(__gnu_cxx::select2nd<transMap_t::value_type>(), _1), boost::cref(*msg)));
    if (iter != mTransMap.end())
    {
        // Dispatch message to matching transaction
        const transSharedPtr_t& trans(iter->second);
        msg->transactionId = trans->Id();
        trans->mState->ReceiveMessage(*trans, *msg);
    }
    else if (msg->type == SipMessage::SIP_MSG_REQUEST)
    {
        if (msg->method != SipMessage::SIP_METHOD_ACK)
        {
            const uint32_t transId = NextTransactionId();
            
            // Create a new server transaction
            msg->transactionId = transId;
            transSharedPtr_t trans(new Transaction(transId, *this, Transaction::ServerData(msg), mReactor));
            trans->mTerminationHook = boost::bind(&SipTransactionLayer::TerminateTransaction, this, transId);
            mTransMap.insert(std::make_pair(transId, trans));
            
            // Pass request to state machine and then to UA core to start transaction processing
            // NB: we double check the server transaction in case the terminate hook is invoked
            const Transaction::ServerData& serverData(boost::get<Transaction::ServerData>(trans->mData));
            trans->mState->ReceiveMessage(*trans, *serverData.msg);
            if (!trans.unique() && serverData.msg.get())
                mServerMessageDelegate(serverData.msg.get(), serverData.msg->toTag);
        }
        else
        {
            // Pass non-matching ACK request to UA core outside of transaction
            mServerMessageDelegate(msg.get(), msg->toTag);
        }
    }
    else  // msg->type == SipMessage::SIP_MSG_STATUS
    {
        // Pass non-matching responses to UA core outside of transaction
        mClientMessageDelegate(msg.get(), msg->fromTag);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SipTransactionLayer::TerminateTransaction(uint32_t transId)
{
    // Terminate transaction
    transMap_t::iterator iter = mTransMap.find(transId);
    if (iter == mTransMap.end())
    {
        std::ostringstream oss;
        oss << "[SipTransactionLayer::TerminateTransaction] Termination hook invoked with unknown transaction id " << transId;
        const std::string err(oss.str());
        TC_LOG_ERR_LOCAL(mPort, LOG_SIP, err);
        throw EPHXInternal(err);
    }
    else
        mTransMap.erase(iter);
}

///////////////////////////////////////////////////////////////////////////////
