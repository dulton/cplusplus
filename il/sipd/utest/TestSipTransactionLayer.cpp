#include <memory>
#include <string>

#include <ace/Reactor.h>
#include <boost/lambda/if.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/scoped_ptr.hpp>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipTransactionLayer.h"
#include "SipUri.h"
#include "SipUtils.h"
#include "VoIPUtils.h"

#include "TestUtilities.h"

using namespace boost::lambda;

USING_VOIP_NS;
USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSipTransactionLayer : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestSipTransactionLayer);
    CPPUNIT_TEST(testInviteClientRequestUaAck);
#ifndef QUICK_TEST
    CPPUNIT_TEST(testInviteClientRequestInternalAck);
#endif
    CPPUNIT_TEST(testNonInviteClientRequestProvisional);
    CPPUNIT_TEST(testNonInviteClientRequestNonProvisional);
    CPPUNIT_TEST(testInviteServerRequestSuccess);
    CPPUNIT_TEST(testInviteServerRequestNonSuccess);
#ifndef QUICK_TEST
    CPPUNIT_TEST(testNonInviteServerRequestProvisional);
    CPPUNIT_TEST(testNonInviteServerRequestNonProvisional);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    static const ACE_INET_Addr LOOPBACK_ADDR;
    static const SipUri URI;
    static const ACE_Time_Value EPSILON;

    std::auto_ptr<SipMessage> MakeRequest(SipMessage::MethodType method) const;
    std::auto_ptr<SipMessage> MakeStatus(uint16_t code, SipMessage *req = 0) const;
    
    void testInviteClientRequestUaAck(void);
    void testInviteClientRequestInternalAck(void);
    void testNonInviteClientRequestProvisional(void);
    void testNonInviteClientRequestNonProvisional(void);
    void testInviteServerRequestSuccess(void);
    void testInviteServerRequestNonSuccess(void);
    void testNonInviteServerRequestProvisional(void);
    void testNonInviteServerRequestNonProvisional(void);
};

///////////////////////////////////////////////////////////////////////////////

const ACE_INET_Addr TestSipTransactionLayer::LOOPBACK_ADDR(5060, INADDR_LOOPBACK);
const SipUri TestSipTransactionLayer::URI("foo.bar.com", 0);
const ACE_Time_Value TestSipTransactionLayer::EPSILON(0, 10000);

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<SipMessage> TestSipTransactionLayer::MakeRequest(SipMessage::MethodType method) const
{
    std::auto_ptr<SipMessage> msg(new SipMessage);
    msg->peer = LOOPBACK_ADDR;
    msg->type = SipMessage::SIP_MSG_REQUEST;
    msg->method = method;
    msg->seq = 1;
    msg->request.uri = URI;
    msg->request.version = SipMessage::SIP_VER_2_0;

    SipUri contact("123456789", "baz.bar.com", 5060);
    std::string branch("z9hG4bK" + VoIPUtils::MakeRandomHexString(32));

    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(contact, &branch)));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(contact)));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(contact)));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, SipProtocol::BuildCallIdString(URI.GetSipHost(), 32)));
    msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(1, method)));

    return msg;
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<SipMessage> TestSipTransactionLayer::MakeStatus(uint16_t code, SipMessage *req) const
{
    std::auto_ptr<SipMessage> msg(new SipMessage);
    msg->peer = LOOPBACK_ADDR;
    msg->type = SipMessage::SIP_MSG_STATUS;
    msg->status.version = SipMessage::SIP_VER_2_0;
    msg->status.code = code;
    msg->status.message = "FOO";

    if (req)
    {
        msg->transactionId = req->transactionId;
        req->ForEachHeader(SipMessage::SIP_HEADER_VIA, SipMessage::PushHeader(*msg));
        req->ForEachHeader(SipMessage::SIP_HEADER_FROM, SipMessage::PushHeader(*msg));
        req->ForEachHeader(SipMessage::SIP_HEADER_TO, SipMessage::PushHeader(*msg));
        req->ForEachHeader(SipMessage::SIP_HEADER_CALL_ID, SipMessage::PushHeader(*msg));
        req->ForEachHeader(SipMessage::SIP_HEADER_CSEQ, SipMessage::PushHeader(*msg));
    }
    else
    {
        SipUri contact("123456789", "foo.bar.com", 0);
        std::string branch("z9hG4bK" + VoIPUtils::MakeRandomHexString(32));
    
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, SipProtocol::BuildViaString(contact, &branch)));
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, SipProtocol::BuildAddressString(contact)));
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, SipProtocol::BuildAddressString(contact)));
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, SipProtocol::BuildCallIdString(URI.GetSipHost(), 32)));
        msg->headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, SipProtocol::BuildCSeqString(100, SipMessage::SIP_METHOD_BYE)));
    }
    
    return msg;
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testInviteClientRequestUaAck(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedStatuses = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetClientMessageDelegate(if_then_else(boost::lambda::_1, var(receivedStatuses) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Start INVITE request
    std::auto_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_INVITE));
    const ACE_Time_Value startTime = ACE_OS::gettimeofday();
    transLayer.SendClientRequest(*req);

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_CALLING_STATE));

    // Run reactor until three retries are sent
    ACE_Time_Value retry[3];
    while (sentPackets < 4)
    {
        reactor.handle_events();
        
        if (sentPackets == 2 && !retry[0].msec())
            retry[0] = ACE_OS::gettimeofday();

        if (sentPackets == 3 && !retry[1].msec())
            retry[1] = ACE_OS::gettimeofday();

        if (sentPackets == 4 && !retry[2].msec())
            retry[2] = ACE_OS::gettimeofday();
    }

    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_CALLING_STATE));
    _CPPUNIT_ASSERT((retry[0] - startTime + EPSILON) >= SipTransactionLayer::TIMER_T1);
    _CPPUNIT_ASSERT((retry[1] - retry[0] + EPSILON) >= (2 * SipTransactionLayer::TIMER_T1));
    _CPPUNIT_ASSERT((retry[2] - retry[1] + EPSILON) >= (4 * SipTransactionLayer::TIMER_T1));
    
    // Receive provisional 1xx status message without matching Call-ID and CSeq value, should be passed to UA core outside transaction
    std::auto_ptr<SipMessage> status(MakeStatus(100));
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_CALLING_STATE));

    // Receive provisional 1xx status message, should be passed to UA core
    status = MakeStatus(100, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 2 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_PROCEEDING_STATE));

    // Receive additional provisional 1xx status message, should be passed to UA core
    status = MakeStatus(180, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 3 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_PROCEEDING_STATE));

    // Receive success 2xx status message, should be passed to UA core terminating transaction
    status = MakeStatus(200, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 4 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_TERMINATED_STATE));

    // Send ACK request outside transaction
    req = MakeRequest(SipMessage::SIP_METHOD_ACK);
    transLayer.SendClientRequest(*req);

    _CPPUNIT_ASSERT(sentPackets == 5);
    _CPPUNIT_ASSERT(receivedStatuses == 4 && timeout == false && timeoutLocalTag.empty());

    // Receive additional success 2xx status message, should be passed to UA core outside transaction
    status = MakeStatus(200, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 5);
    _CPPUNIT_ASSERT(receivedStatuses == 5 && timeout == false && timeoutLocalTag.empty());
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testInviteClientRequestInternalAck(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedStatuses = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetClientMessageDelegate(if_then_else(boost::lambda::_1, var(receivedStatuses) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Start INVITE request
    std::auto_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_INVITE));
    transLayer.SendClientRequest(*req);

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_CALLING_STATE));

    // Receive provisional 1xx status message, should be passed to UA core
    std::auto_ptr<SipMessage> status(MakeStatus(100, req.get()));
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_PROCEEDING_STATE));

    // Receive error 4xx status message, internal ACK should be sent and status should be passed to UA core 
    status = MakeStatus(480, req.get());
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedStatuses == 2 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_COMPLETED_STATE));

    // Receive duplicate error 4xx status message, internal ACK should be resent and status should be filtered out
    status = MakeStatus(480, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedStatuses == 2 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_INVITE_COMPLETED_STATE));

    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedStatuses == 2 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT((termTime - complTime + EPSILON) >= SipTransactionLayer::TIMER_T4);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testNonInviteClientRequestProvisional(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedStatuses = 0;
    bool timeout = false;
    std::string timeoutLocalTag;

    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetClientMessageDelegate(if_then_else(boost::lambda::_1, var(receivedStatuses) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Start non-INVITE request
    std::auto_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_REGISTER));
    const ACE_Time_Value startTime = ACE_OS::gettimeofday();
    transLayer.SendClientRequest(*req);

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_TRYING_STATE));

    // Run reactor until two retries are sent
    ACE_Time_Value retry[3];
    while (sentPackets < 3)
    {
        reactor.handle_events();
        
        if (sentPackets == 2 && !retry[0].msec())
            retry[0] = ACE_OS::gettimeofday();

        if (sentPackets == 3 && !retry[1].msec())
            retry[1] = ACE_OS::gettimeofday();
    }

    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_TRYING_STATE));
    _CPPUNIT_ASSERT((retry[0] - startTime + EPSILON) >= SipTransactionLayer::TIMER_T1);
    _CPPUNIT_ASSERT((retry[1] - retry[0] + EPSILON) >= (2 * SipTransactionLayer::TIMER_T1));
    
    // Receive provisional 1xx status message without matching Call-ID and CSeq value, should be passed to UA core outside transaction
    std::auto_ptr<SipMessage> status(MakeStatus(100));
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedStatuses == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_TRYING_STATE));

    // Receive provisional 1xx status message, should be passed to UA core
    status = MakeStatus(100, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedStatuses == 2 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_PROCEEDING_STATE));

    // Receive additional provisional 1xx status message, should be passed to UA core
    status = MakeStatus(183, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedStatuses == 3 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_PROCEEDING_STATE));

    // Run reactor until an additional retry is sent
    while (sentPackets < 4)
    {
        reactor.handle_events();
        
        if (sentPackets == 4 && !retry[2].msec())
            retry[2] = ACE_OS::gettimeofday();
    }

    _CPPUNIT_ASSERT(receivedStatuses == 3 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_PROCEEDING_STATE));
    _CPPUNIT_ASSERT((retry[2] - retry[1] + EPSILON) >= (4 * SipTransactionLayer::TIMER_T1));
    
    // Receive final 2xx status message, should be passed to UA core
    status = MakeStatus(200, req.get());
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 4 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_COMPLETED_STATE));

    // Receive duplicate final 2xx status message, should be filtered out
    status = MakeStatus(200, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 4 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_COMPLETED_STATE));

    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedStatuses == 4 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT((termTime - complTime + EPSILON) >= SipTransactionLayer::TIMER_T4);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testNonInviteClientRequestNonProvisional(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedStatuses = 0;
    bool timeout = false;
    std::string timeoutLocalTag;

    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetClientMessageDelegate(if_then_else(boost::lambda::_1, var(receivedStatuses) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Start non-INVITE request
    std::auto_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_REGISTER));
    const ACE_Time_Value startTime = ACE_OS::gettimeofday();
    transLayer.SendClientRequest(*req);

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 0 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_TRYING_STATE));

    // Receive final 2xx status message, should be passed to UA core
    std::auto_ptr<SipMessage> status(MakeStatus(200, req.get()));
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.ReceiveMessage(status);
    
    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_COMPLETED_STATE));

    // Receive duplicate final 2xx status message, should be filtered out
    status = MakeStatus(200, req.get());
    transLayer.ReceiveMessage(status);

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::CLIENT_NON_INVITE_COMPLETED_STATE));

    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedStatuses == 1);
    _CPPUNIT_ASSERT((termTime - complTime + EPSILON) >= SipTransactionLayer::TIMER_T4);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testInviteServerRequestSuccess(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedRequests = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetServerMessageDelegate(if_then_else(boost::lambda::_1, var(receivedRequests) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Receive INVITE request, internal 100 status should be sent and request should be passed to UA core
    boost::scoped_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_INVITE));
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);
    req->transactionId = trans->Id();

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_PROCEEDING_STATE));

    // Receive duplicate INVITE request, internal 100 status should be resent and request should be filtered out
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_PROCEEDING_STATE));

    // Send final 2xx status message, should terminate tranascation
    std::auto_ptr<SipMessage> status(MakeStatus(200, req.get()));
    transLayer.SendServerStatus(*status);
    
    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_TERMINATED_STATE));

    // Receive ACK request, should be passed to UA core outside transaction
    req->method = SipMessage::SIP_METHOD_ACK;
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedRequests == 2 && timeout == false && timeoutLocalTag.empty());
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testInviteServerRequestNonSuccess(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedRequests = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetServerMessageDelegate(if_then_else(boost::lambda::_1, var(receivedRequests) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Receive INVITE request, internal 100 status should be sent and request should be passed to UA core
    boost::scoped_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_INVITE));
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);
    req->transactionId = trans->Id();

    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_PROCEEDING_STATE));

    // Send final 4xx status message
    std::auto_ptr<SipMessage> status(MakeStatus(483, req.get()));
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.SendServerStatus(*status);
    
    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_COMPLETED_STATE));

    // Run reactor until three retries are sent
    ACE_Time_Value retry[3];
    while (sentPackets < 5)
    {
        reactor.handle_events();
        
        if (sentPackets == 3 && !retry[0].msec())
            retry[0] = ACE_OS::gettimeofday();

        if (sentPackets == 4 && !retry[1].msec())
            retry[1] = ACE_OS::gettimeofday();

        if (sentPackets == 5 && !retry[2].msec())
            retry[2] = ACE_OS::gettimeofday();
    }

    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_COMPLETED_STATE));
    _CPPUNIT_ASSERT((retry[0] - complTime + EPSILON) >= SipTransactionLayer::TIMER_T1);
    _CPPUNIT_ASSERT((retry[1] - retry[0] + EPSILON) >= (2 * SipTransactionLayer::TIMER_T1));
    _CPPUNIT_ASSERT((retry[2] - retry[1] + EPSILON) >= (4 * SipTransactionLayer::TIMER_T1));
    
    // Receive duplicate INVITE request, last status should be resent and request should be filtered out
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 6);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_COMPLETED_STATE));

    // Receive ACK request, should be filtered out
    req->method = SipMessage::SIP_METHOD_ACK;
    const ACE_Time_Value confTime = ACE_OS::gettimeofday();
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 6);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_INVITE_CONFIRMED_STATE));
    
    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 6);
    _CPPUNIT_ASSERT(receivedRequests == 1);
    _CPPUNIT_ASSERT((termTime - confTime + EPSILON) >= SipTransactionLayer::TIMER_T4);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testNonInviteServerRequestProvisional(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedRequests = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetServerMessageDelegate(if_then_else(boost::lambda::_1, var(receivedRequests) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Receive non-INVITE request, request should be passed to UA core
    boost::scoped_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_BYE));
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);
    req->transactionId = trans->Id();

    _CPPUNIT_ASSERT(sentPackets == 0);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_TRYING_STATE));

    // Send provisional 1xx status message
    std::auto_ptr<SipMessage> status(MakeStatus(100, req.get()));
    transLayer.SendServerStatus(*status);
    
    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_PROCEEDING_STATE));

    // Receive duplicate non-INVITE request, last status should be resent and request should be filtered out
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_PROCEEDING_STATE));

    // Send final 2xx status message
    status = MakeStatus(200, req.get());
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.SendServerStatus(*status);
    
    _CPPUNIT_ASSERT(sentPackets == 3);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_COMPLETED_STATE));
    
    // Receive duplicate non-INVITE request, last status should be resent and request should be filtered out
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_COMPLETED_STATE));

    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 4);
    _CPPUNIT_ASSERT(receivedRequests == 1);
    _CPPUNIT_ASSERT((termTime - complTime + EPSILON) >= (64 * SipTransactionLayer::TIMER_T1));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipTransactionLayer::testNonInviteServerRequestNonProvisional(void)
{
    ACE_Reactor reactor;
    uint32_t sentPackets = 0;
    uint32_t receivedRequests = 0;
    bool timeout = false;
    std::string timeoutLocalTag;
    
    SipTransactionLayer transLayer(0, 0, LOOPBACK_ADDR, &reactor);
    transLayer.SetSendDelegate((var(sentPackets) += 1, true));
    transLayer.SetServerMessageDelegate(if_then_else(boost::lambda::_1, var(receivedRequests) += 1, (var(timeout) = true, var(timeoutLocalTag) = boost::lambda::_2)));

    _CPPUNIT_ASSERT(transLayer.Enabled());

    // Receive non-INVITE request, request should be passed to UA core
    boost::scoped_ptr<SipMessage> req(MakeRequest(SipMessage::SIP_METHOD_BYE));
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    SipTransactionLayer::transSharedPtr_t trans(transLayer.LookupTransaction(transLayer.mLastTransId));
    _CPPUNIT_ASSERT(trans);
    req->transactionId = trans->Id();

    _CPPUNIT_ASSERT(sentPackets == 0);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_TRYING_STATE));

    // Send final 2xx status message
    std::auto_ptr<SipMessage> status(MakeStatus(200, req.get()));
    const ACE_Time_Value complTime = ACE_OS::gettimeofday();
    transLayer.SendServerStatus(*status);
    
    _CPPUNIT_ASSERT(sentPackets == 1);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_COMPLETED_STATE));
    
    // Receive duplicate non-INVITE request, last status should be resent and request should be filtered out
    transLayer.ReceiveMessage(std::auto_ptr<SipMessage>(new SipMessage(*req)));

    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedRequests == 1 && timeout == false && timeoutLocalTag.empty());
    _CPPUNIT_ASSERT(!trans.unique());
    _CPPUNIT_ASSERT(trans->mState == reinterpret_cast<const SipTransactionLayer::State *>(&SipTransactionLayer::SERVER_NON_INVITE_COMPLETED_STATE));

    // Run reactor until transaction termination
    while (!trans.unique())
        reactor.handle_events();
    
    const ACE_Time_Value termTime = ACE_OS::gettimeofday();
    _CPPUNIT_ASSERT(sentPackets == 2);
    _CPPUNIT_ASSERT(receivedRequests == 1);
    _CPPUNIT_ASSERT((termTime - complTime + EPSILON) >= (64 * SipTransactionLayer::TIMER_T1));
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSipTransactionLayer);
