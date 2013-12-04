/*
 *   Copyright 2010 IMTC, Inc.  All rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
using namespace std;

#include "tip_debug_print.h"
#include "tip_debug_tools.h"
#include "tip_time.h"
#include "rtcp_packet_factory.h"
#include "rtcp_rr_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_feedback_packet.h"
#include "tip_media.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// test packet relay class, just saves off messages for later
// inspection
class CTipMediaTestXmit : public CTipPacketTransmit {
public:
    CTipMediaTestXmit() : rxREFRESH(NULL), rxACK_RXFLOWCTRL(NULL),
                          rxACK_TXFLOWCTRL(NULL), rxACK_REFRESH(NULL), rxFB(NULL) {}

    ~CTipMediaTestXmit() {
        delete rxREFRESH;
        delete rxACK_RXFLOWCTRL;
        delete rxACK_TXFLOWCTRL;
        delete rxACK_REFRESH;
        delete rxFB;
    }
    
    virtual Status Transmit(const uint8_t* pktBuffer, uint32_t pktSize, MediaType mType) {
        CPacketBuffer buffer((uint8_t*) pktBuffer, pktSize);
        while (buffer.GetBufferSize()) {
            CRtcpPacket* rtcp = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
            if (rtcp == NULL) {
                ostringstream oss;
                oss << "invalid packet for transmission:  " << HexDump(pktBuffer, pktSize);
                CPPUNIT_FAIL(oss.str());

                // not gonna get to here but just for fun...
                continue;
            }

            if (rtcp->GetType() == CRtcpPacket::RTPFB) {
                CRtcpAppFeedbackPacket* fb = dynamic_cast<CRtcpAppFeedbackPacket*>(rtcp);

                if (fb == NULL) {
                    delete rtcp;
                    continue;
                }
                
                rxFB = fb;

            } else if (rtcp->GetType() == CRtcpPacket::APP) {
            
                CRtcpTipPacket* packet = dynamic_cast<CRtcpTipPacket*>(rtcp);
                if (packet == NULL) {
                    delete rtcp;
                    continue;
                }
        
                switch (packet->GetTipPacketType()) {
                case REFRESH:
                    delete rxREFRESH;
                    rxREFRESH = packet;
                    break;

                case ACK_RXFLOWCTRL:
                    delete rxACK_RXFLOWCTRL;
                    rxACK_RXFLOWCTRL = packet;
                    break;
                    
                case ACK_TXFLOWCTRL:
                    delete rxACK_TXFLOWCTRL;
                    rxACK_TXFLOWCTRL = packet;
                    break;
                    
                case ACK_REFRESH:
                    delete rxACK_REFRESH;
                    rxACK_REFRESH = packet;
                    break;
                    
                default:
                    delete packet;
                    break;
                }

            } else {
                // something we don't handle
                delete rtcp;
            }
        }

        return TIP_OK;
    }

    CRtcpTipPacket* rxREFRESH;
    CRtcpTipPacket* rxACK_RXFLOWCTRL;
    CRtcpTipPacket* rxACK_TXFLOWCTRL;
    CRtcpTipPacket* rxACK_REFRESH;
    CRtcpAppFeedbackPacket* rxFB;
};

// test callback interface classes, just remembers when functions are called
class CTipMediaSinkTestCallback : public CTipMediaSinkCallback {
public:
    CTipMediaSinkTestCallback() : mStop(false), mStart(false)
    {}
    ~CTipMediaSinkTestCallback() {}

    virtual void Stop() { mStop = true; }
    virtual void Start() { mStart = true; }

    bool mStop;
    bool mStart;
};

class CTipMediaSourceTestCallback : public CTipMediaSourceCallback {
public:
    CTipMediaSourceTestCallback() : mStop(false), mStart(false),
                                    mRefresh(false),
                                    mRefreshIDR(false), mNacks(0), mAcks(0)
    {}
    ~CTipMediaSourceTestCallback() {}

    virtual void Stop() { mStop = true; }
    virtual void Start() { mStart = true; }
    virtual void Refresh(bool idr) { mRefresh = true; mRefreshIDR = idr;}
    virtual void ProcessNack(uint16_t seqno) {
        mNacks++;
        mBitset[seqno] = 0;
    }
    virtual void ProcessAck(uint16_t seqno) {
        mAcks++;
        mBitset[seqno] = 1;
    }

    bool mStop;
    bool mStart;
    bool mRefresh;
    bool mRefreshIDR;
    uint32_t mNacks;
    uint32_t mAcks;
    std::bitset<0x10000> mBitset;
};

class CTipMediaSinkTest : public CppUnit::TestFixture {
public:
    CTipMediaTestXmit* xmit;
    CTipMediaSink* am;
    CTipMediaSinkTestCallback* callback;

    uint8_t zero_bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
    uint8_t one_bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];

    void setUp() {
        // turn off debug prints
        if (getenv("TEST_TIP_DEBUG") == NULL) {
            gDebugFlags = 0;
        }
        
        xmit = new CTipMediaTestXmit();
        CPPUNIT_ASSERT( xmit != NULL );

        am = new CTipMediaSink(VIDEO, 0x12345678, 0xA5A5A011, *xmit);
        CPPUNIT_ASSERT( am != NULL );

        callback = new CTipMediaSinkTestCallback();
        CPPUNIT_ASSERT( callback != NULL );

        am->SetCallback(callback);
        am->SetSourceCSRC(0xABCDE011);

        memset(zero_bytes, 0, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
        memset(one_bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
    }

    void tearDown() {
        delete am;
        delete xmit;
    }

    void feedPacket(CRtcpPacket& packet, Status ret = TIP_OK) {
        CPacketBufferData buffer;
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize()),
                              ret );
    }
    
    void testCallback() {
        // just verify that setting the callback to NULL doesn't break
        // anything
        am->SetCallback(NULL);
    }

    void testReceiveInvalid() {
        uint8_t buffer[2048] = { 0 };
        
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(NULL, sizeof(buffer)), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, sizeof(buffer)), TIP_ERROR );
    }

    void testReceiveInvalid2() {
        // send in an RR packet, should be ignored
        CRtcpRRPacket rr;
        feedPacket(rr);
    }

    void testReceiveInvalid3() {
        // send in an FB packet, should be ignored by a sink
        CRtcpAppFeedbackPacket packet;
        feedPacket(packet);
    }
    
    void testReceiveInvalid4() {
        // send in a MUXCTRL packet, should be ignored
        CRtcpAppMuxCtrlPacket packet;
        feedPacket(packet, TIP_OK);
    }
    
    void testReceiveAckUnknown() {
        CRtcpTipAckPacket ack(ACK_MUXCTRL);
        feedPacket(ack);
    }
    
    void testRefresh() {
        CPPUNIT_ASSERT_EQUAL( am->RequestRefresh(false), TIP_OK );

        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH != NULL );

        CRtcpAppRefreshPacket* refresh =
            dynamic_cast<CRtcpAppRefreshPacket*>(xmit->rxREFRESH);
        CPPUNIT_ASSERT( refresh != NULL );

        CPPUNIT_ASSERT_EQUAL( refresh->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetFlags(),
                              (uint32_t) CRtcpAppRefreshPacket::REFRESH_PREFER_GDR );
        
        delete xmit->rxREFRESH;
        xmit->rxREFRESH = NULL;

        // do it a second time to verify retransmission
        usleep((am->GetIdleTime() * 1000));
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH != NULL );
        
        refresh = dynamic_cast<CRtcpAppRefreshPacket*>(xmit->rxREFRESH);
        CPPUNIT_ASSERT( refresh != NULL );

        CPPUNIT_ASSERT_EQUAL( refresh->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetFlags(),
                              (uint32_t) CRtcpAppRefreshPacket::REFRESH_PREFER_GDR );

        // send in an ACK and verify no more retransmission
        CRtcpTipAckPacket ack(*xmit->rxREFRESH);
        feedPacket(ack);

        CPPUNIT_ASSERT_EQUAL( am->GetIdleTime(), (uint64_t) -1 );
    }

    void testRefresh2() {
        CPPUNIT_ASSERT_EQUAL( am->RequestRefresh(true), TIP_OK );

        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH != NULL );

        CRtcpAppRefreshPacket* refresh =
            dynamic_cast<CRtcpAppRefreshPacket*>(xmit->rxREFRESH);
        CPPUNIT_ASSERT( refresh != NULL );

        CPPUNIT_ASSERT_EQUAL( refresh->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetFlags(),
                              (uint32_t) CRtcpAppRefreshPacket::REFRESH_REQUIRE_IDR );
        
        delete xmit->rxREFRESH;
        xmit->rxREFRESH = NULL;

        // do it a second time to verify retransmission
        usleep((am->GetIdleTime() * 1000));
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH != NULL );
        
        refresh = dynamic_cast<CRtcpAppRefreshPacket*>(xmit->rxREFRESH);
        CPPUNIT_ASSERT( refresh != NULL );

        CPPUNIT_ASSERT_EQUAL( refresh->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( refresh->GetFlags(),
                              (uint32_t) CRtcpAppRefreshPacket::REFRESH_REQUIRE_IDR );

        // send in an ACK and verify no more retransmission
        CRtcpTipAckPacket ack(*xmit->rxREFRESH);
        feedPacket(ack);

        CPPUNIT_ASSERT_EQUAL( am->GetIdleTime(), (uint64_t) -1 );
    }

    void testRefresh3() {
        am->SetRetransmissionInterval(1);
        am->SetRetransmissionLimit(1);

        CPPUNIT_ASSERT_EQUAL( am->RequestRefresh(false), TIP_OK );

        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH != NULL );

        delete xmit->rxREFRESH;
        xmit->rxREFRESH = NULL;

        // do it a second time to verify retransmission
        usleep((am->GetIdleTime() * 1000));
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxREFRESH == NULL );
    }
    
    void testRXFlowStart() {
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppRXFlowCtrlPacket::OPCODE_START);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, true );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_RXFLOWCTRL != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_RXFLOWCTRL->GetSSRC(), (uint32_t) 0x12345678 );

        // feed in the same thing twice, should only get one callback
        callback->mStart = false;
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
    }

    void testRXFlowStart2() {
        // media sink only requires that the source position in the
        // target CSRC field be correct.  verify that.
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0x00000010);
        packet.SetOpcode(CRtcpAppRXFlowCtrlPacket::OPCODE_START);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, true );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_RXFLOWCTRL != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_RXFLOWCTRL->GetSSRC(), (uint32_t) 0x12345678 );

        // feed in the same thing twice, should only get one callback
        callback->mStart = false;
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
    }
    
    void testRXFlowStop() {
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppRXFlowCtrlPacket::OPCODE_STOP);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, true );
        CPPUNIT_ASSERT( xmit->rxACK_RXFLOWCTRL != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_RXFLOWCTRL->GetSSRC(), (uint32_t) 0x12345678 );
    }

    void testRXFlowInvalid1() {
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(0xFFFF);

        // Tip will reject this packet b/c the opcode is invalid (ie
        // no callbacks invoked)
        feedPacket(packet, TIP_OK);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_RXFLOWCTRL == NULL );
    }

    void testRXFlowInvalid2() {
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A022);
        packet.SetOpcode(CRtcpAppRXFlowCtrlPacket::OPCODE_START);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_RXFLOWCTRL == NULL );
    }

    void testRegisterPacket1() {
        am->RegisterPacket(0, 1);
        CPPUNIT_ASSERT( xmit->rxFB != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetPacketID(), (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(xmit->rxFB->GetPacketAcks(), zero_bytes,
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }

    void doRegisterPacketBase(uint16_t base, uint16_t incr = 1) {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES] = { 0 };
        
        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i += incr) {
            am->RegisterPacket((base + i), 0);
            bytes[(i/8)] |= (1 << (i%8));
        }

        uint16_t pid = (base + CRtcpAppFeedbackPacket::NUM_ACK_BITS);
        am->RegisterPacket(pid, 1);
        
        CPPUNIT_ASSERT( xmit->rxFB != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetPacketID(), pid );
        if (memcmp(xmit->rxFB->GetPacketAcks(), bytes, CRtcpAppFeedbackPacket::NUM_ACK_BYTES) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(bytes, CRtcpAppFeedbackPacket::NUM_ACK_BYTES)
                << "\nPacked:    " << HexDump(xmit->rxFB->GetPacketAcks(), CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testRegisterPacket2() {
        doRegisterPacketBase(0);
    }
    
    void testRegisterPacket3() {
        doRegisterPacketBase(0x8000);
    }
    
    void testRegisterPacket4() {
        doRegisterPacketBase(0xFFF0);
    }
    
    void testRegisterPacket5() {
        uint16_t base = (0 - CRtcpAppFeedbackPacket::NUM_ACK_BITS);
        doRegisterPacketBase(base);
    }

    void testRegisterMissing1() {
        doRegisterPacketBase(0, 2);
    }
    
    void testRegisterMissing2() {
        doRegisterPacketBase(0x8000, 2);
    }
    
    void testRegisterMissing3() {
        doRegisterPacketBase(0xFFF0, 2);
    }
    
    void testRegisterMissing4() {
        doRegisterPacketBase(0, 4);
    }

    void doRegisterOOOBase(uint16_t base) {
        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i += 2) {
            am->RegisterPacket((base + i), 0);
        }

        for (uint16_t i = 1; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i += 2) {
            am->RegisterPacket((base + i), 0);
        }
        
        uint16_t pid = (base + CRtcpAppFeedbackPacket::NUM_ACK_BITS);
        am->RegisterPacket(pid, 1);
        
        CPPUNIT_ASSERT( xmit->rxFB != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetSSRC(), (uint32_t) 0x12345678 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetTarget(), (uint32_t) 0xABCDE011 );
        CPPUNIT_ASSERT_EQUAL( xmit->rxFB->GetPacketID(), pid );
        if (memcmp(xmit->rxFB->GetPacketAcks(), one_bytes, CRtcpAppFeedbackPacket::NUM_ACK_BYTES) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(one_bytes, CRtcpAppFeedbackPacket::NUM_ACK_BYTES)
                << "\nPacked:    " << HexDump(xmit->rxFB->GetPacketAcks(), CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testRegisterOOO1() {
        doRegisterOOOBase(0);
    }
    
    void testRegisterOOO2() {
        doRegisterOOOBase(0x8000);
    }
    
    void testRegisterOOO3() {
        doRegisterOOOBase(0xFFF0);
    }

    void testLogPrefix() {
        CRtcpAppRXFlowCtrlPacket packet;
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppRXFlowCtrlPacket::OPCODE_START);

        // set prefix then call something that will cause a print to go out
        am->SetLogPrefix("blah");
        packet.SetNtpTime(1);
        feedPacket(packet);
        
        am->SetLogPrefix("");
        packet.SetNtpTime(2);
        feedPacket(packet);
        
        am->SetLogPrefix(NULL);
        packet.SetNtpTime(3);
        feedPacket(packet);
    }
    
    CPPUNIT_TEST_SUITE( CTipMediaSinkTest );
    CPPUNIT_TEST( testCallback );
    CPPUNIT_TEST( testReceiveInvalid );
    CPPUNIT_TEST( testReceiveInvalid2 );
    CPPUNIT_TEST( testReceiveInvalid3 );
    CPPUNIT_TEST( testReceiveInvalid4 );
    CPPUNIT_TEST( testReceiveAckUnknown );
    CPPUNIT_TEST( testRefresh );
    CPPUNIT_TEST( testRefresh2 );
    CPPUNIT_TEST( testRefresh3 );
    CPPUNIT_TEST( testRXFlowStart );
    CPPUNIT_TEST( testRXFlowStart2 );
    CPPUNIT_TEST( testRXFlowStop );
    CPPUNIT_TEST( testRXFlowInvalid1 );
    CPPUNIT_TEST( testRXFlowInvalid2 );
    CPPUNIT_TEST( testRegisterPacket1 );
    CPPUNIT_TEST( testRegisterPacket2 );
    CPPUNIT_TEST( testRegisterPacket3 );
    CPPUNIT_TEST( testRegisterPacket4 );
    CPPUNIT_TEST( testRegisterPacket5 );
    CPPUNIT_TEST( testRegisterMissing1 );
    CPPUNIT_TEST( testRegisterMissing2 );
    CPPUNIT_TEST( testRegisterMissing3 );
    CPPUNIT_TEST( testRegisterMissing4 );
    CPPUNIT_TEST( testRegisterOOO1 );
    CPPUNIT_TEST( testRegisterOOO2 );
    CPPUNIT_TEST( testRegisterOOO3 );
    CPPUNIT_TEST( testLogPrefix );
    CPPUNIT_TEST_SUITE_END();
};

class CTipMediaSourceTest : public CppUnit::TestFixture {
public:
    CTipMediaTestXmit* xmit;
    CTipMediaSource* am;
    CTipMediaSourceTestCallback* callback;

    uint8_t zero_bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
    uint8_t one_bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];

    void setUp() {
        // turn off debug prints
        if (getenv("TEST_TIP_DEBUG") == NULL) {
            gDebugFlags = 0;
        }
        
        xmit = new CTipMediaTestXmit();
        CPPUNIT_ASSERT( xmit != NULL );

        am = new CTipMediaSource(VIDEO, 0x12345678, 0xA5A5A011, *xmit);
        CPPUNIT_ASSERT( am != NULL );

        callback = new CTipMediaSourceTestCallback();
        CPPUNIT_ASSERT( callback != NULL );

        am->SetCallback(callback);

        memset(zero_bytes, 0, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
        memset(one_bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
    }

    void tearDown() {
        delete am;
        delete xmit;
    }

    void feedPacket(CRtcpPacket& packet, Status ret = TIP_OK) {
        CPacketBufferData buffer;
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize()),
                              ret );
    }
    
    void testCallback() {
        // just verify that setting the callback to NULL doesn't break
        // anything
        am->SetCallback(NULL);
    }
    
    void testReceiveInvalid() {
        uint8_t buffer[2048] = { 0 };
        
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(NULL, sizeof(buffer)), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, sizeof(buffer)), TIP_ERROR );
    }

    void testReceiveInvalid2() {
        // send in an RR packet, should be ignored
        CRtcpRRPacket rr;
        feedPacket(rr);
    }

    void testReceiveInvalid3() {
        // send in an RXFLOWCTRL packet, should be ignored by a sink
        CRtcpAppRXFlowCtrlPacket packet;
        feedPacket(packet);
    }
    
    void testReceiveInvalid4() {
        // send in a MUXCTRL packet, should be ignored
        CRtcpAppMuxCtrlPacket packet;
        feedPacket(packet, TIP_OK);
    }
    
    void testRefresh() {
        CRtcpAppRefreshPacket packet;
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);

        packet.SetNtpTime(1);
        packet.SetFlags(CRtcpAppRefreshPacket::REFRESH_REQUIRE_IDR);
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mRefresh, true );
        CPPUNIT_ASSERT_EQUAL( callback->mRefreshIDR, true );
        callback->mRefresh = false;
        callback->mRefreshIDR = false;
        CPPUNIT_ASSERT( xmit->rxACK_REFRESH != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_REFRESH->GetSSRC(), (uint32_t) 0x12345678 );
        delete xmit->rxACK_REFRESH;
        xmit->rxACK_REFRESH = NULL;

        packet.SetNtpTime(2);
        packet.SetFlags(CRtcpAppRefreshPacket::REFRESH_REQUIRE_GDR);
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mRefresh, true );
        CPPUNIT_ASSERT_EQUAL( callback->mRefreshIDR, false );
        callback->mRefresh = false;
        callback->mRefreshIDR = false;
        CPPUNIT_ASSERT( xmit->rxACK_REFRESH != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_REFRESH->GetSSRC(), (uint32_t) 0x12345678 );
        delete xmit->rxACK_REFRESH;
        xmit->rxACK_REFRESH = NULL;
        
        packet.SetNtpTime(3);
        packet.SetFlags(CRtcpAppRefreshPacket::REFRESH_PREFER_IDR);
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mRefresh, true );
        CPPUNIT_ASSERT_EQUAL( callback->mRefreshIDR, false );
        callback->mRefresh = false;
        callback->mRefreshIDR = false;
        CPPUNIT_ASSERT( xmit->rxACK_REFRESH != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_REFRESH->GetSSRC(), (uint32_t) 0x12345678 );
        delete xmit->rxACK_REFRESH;
        xmit->rxACK_REFRESH = NULL;

        packet.SetNtpTime(4);
        packet.SetFlags(CRtcpAppRefreshPacket::REFRESH_PREFER_GDR);
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mRefresh, true );
        CPPUNIT_ASSERT_EQUAL( callback->mRefreshIDR, false );
        CPPUNIT_ASSERT( xmit->rxACK_REFRESH != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_REFRESH->GetSSRC(), (uint32_t) 0x12345678 );
    }

    void testRefreshInvalid() {
        CRtcpAppRefreshPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A022);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mRefresh, false );
        CPPUNIT_ASSERT( xmit->rxACK_REFRESH == NULL );
    }

    void testTXFlowStart() {
        CRtcpAppTXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppTXFlowCtrlPacket::OPCODE_START);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, true );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_TXFLOWCTRL != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_TXFLOWCTRL->GetSSRC(), (uint32_t) 0x12345678 );
    }

    void testTXFlowStop() {
        CRtcpAppTXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppTXFlowCtrlPacket::OPCODE_STOP);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, true );
        CPPUNIT_ASSERT( xmit->rxACK_TXFLOWCTRL != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACK_TXFLOWCTRL->GetSSRC(), (uint32_t) 0x12345678 );
    }

    void testTXFlowInvalid1() {
        CRtcpAppTXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(0xFFFF);

        // Tip will reject this packet b/c the opcode is invalid (ie
        // no callbacks will be invoked)
        feedPacket(packet, TIP_OK);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_TXFLOWCTRL == NULL );
    }

    void testTXFlowInvalid2() {
        CRtcpAppTXFlowCtrlPacket packet;
        packet.SetNtpTime(GetNtpTimestamp());
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A022);
        packet.SetOpcode(CRtcpAppTXFlowCtrlPacket::OPCODE_START);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mStart, false );
        CPPUNIT_ASSERT_EQUAL( callback->mStop, false );
        CPPUNIT_ASSERT( xmit->rxACK_TXFLOWCTRL == NULL );
    }

    void testFB1() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);

        // this if the first FB passed in, we should get 112 NACKs and
        // 1 ACK
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[100], 1 );
    }
    
    void testFB2() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 3 packets, 2 acks and 1 nack
        packet.SetPacketID(103);
        packet.SetPacketAckBySeqNum(101, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckBySeqNum(102, CRtcpAppFeedbackPacket::APP_FB_ACK);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 1 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[101], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[102], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[103], 1 );
    }

    // same thing as testFB2 just testing wrap-around
    void testFB3() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(0xFFFE);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 3 packets, 2 acks and 1 nack
        packet.SetPacketID(1);
        packet.SetPacketAckBySeqNum(0xFFFF, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckBySeqNum(0, CRtcpAppFeedbackPacket::APP_FB_ACK);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 1 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[0xFFFF], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[0], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[1], 1 );
    }

    void testFB4() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(0);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 115 packets simulating feedback loss.  ack
        // everything in the packet.  only the lost bits (seqno 1 and
        // 2) should be NACKS
        packet.SetPacketID(115);

        packet.SetPacketAcks(one_bytes);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS + 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[1], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[2], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[114], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[115], 1 );
    }

    void testFB5() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);

        feedPacket(packet);

        // save off old value
        uint32_t oldNacks = callback->mNacks;
        uint32_t oldAcks  = callback->mAcks;

        // feed in out of order feedback
        packet.SetPacketID(99);
        feedPacket(packet);

        // nothing should have changed
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, oldNacks );
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, oldAcks );
    }

    // same thing as testFB5 but with wrap-around
    void testFB6() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(1);

        feedPacket(packet);

        // save off old value
        uint32_t oldNacks = callback->mNacks;
        uint32_t oldAcks  = callback->mAcks;

        // feed in out of order feedback
        packet.SetPacketID(0xFFFE);
        feedPacket(packet);

        // nothing should have changed
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, oldNacks );
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, oldAcks );
    }

    void testFB7() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);

        am->SetSequenceNumber(100);

        // should get just a single ACK
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }
    
    void testFB8() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);

        am->SetSequenceNumber(100);

        // should get just a single ACK for the first time
        feedPacket(packet);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }

    void testFB9() {
        CRtcpAppFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketAcks(one_bytes);

        am->SetSequenceNumber(1);

        // feed in all possible seqnum twice, causing a wrap-around, verify callbacks
        uint32_t loop_count = ((65536 / CRtcpAppFeedbackPacket::NUM_ACK_BITS) * 2);
        for (uint32_t i = 0; i < loop_count; i++) {
            packet.SetPacketID((CRtcpAppFeedbackPacket::NUM_ACK_BITS * (i + 1)));
            //printf("%d - %hu\n", i, packet.GetPacketID());
            feedPacket(packet);

            CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS );
            CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );

            // reset for the next pass
            callback->mAcks = 0;
        }
    }
    
    // repeat above tests with extended FB packet (all VALID)
    void testExtFB1() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);
        packet.SetPacketAcksValid(one_bytes);

        // this if the first FB passed in, we should get 112 NACKs and
        // 1 ACK
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[100], 1 );
    }
    
    void testExtFB2() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);
        packet.SetPacketAcksValid(one_bytes);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 3 packets, 2 acks and 1 nack
        packet.SetPacketID(103);
        packet.SetPacketAckBySeqNum(101, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckBySeqNum(102, CRtcpAppFeedbackPacket::APP_FB_ACK);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 1 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[101], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[102], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[103], 1 );
    }

    // same thing as testExtFB2 just testing wrap-around
    void testExtFB3() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(0xFFFE);
        packet.SetPacketAcksValid(one_bytes);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 3 packets, 2 acks and 1 nack
        packet.SetPacketID(1);
        packet.SetPacketAckBySeqNum(0xFFFF, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckBySeqNum(0, CRtcpAppFeedbackPacket::APP_FB_ACK);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 1 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[0xFFFF], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[0], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[1], 1 );
    }

    void testExtFB4() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(0);
        packet.SetPacketAcksValid(one_bytes);

        feedPacket(packet);

        // reset counters
        callback->mAcks = 0;
        callback->mNacks = 0;

        // move forward 115 packets simulating feedback loss.  ack
        // everything in the packet.  only the lost bits (seqno 1 and
        // 2) should be NACKS
        packet.SetPacketID(115);

        packet.SetPacketAcks(one_bytes);
        
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS + 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );

        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[1], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[2], 0 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[114], 1 );
        CPPUNIT_ASSERT_EQUAL( (int) callback->mBitset[115], 1 );
    }

    void testExtFB5() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);
        packet.SetPacketAcksValid(one_bytes);

        feedPacket(packet);

        // save off old value
        uint32_t oldNacks = callback->mNacks;
        uint32_t oldAcks  = callback->mAcks;

        // feed in out of order feedback
        packet.SetPacketID(99);
        feedPacket(packet);

        // nothing should have changed
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, oldNacks );
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, oldAcks );
    }

    // same thing as testExtFB5 but with wrap-around
    void testExtFB6() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(1);
        packet.SetPacketAcksValid(one_bytes);

        feedPacket(packet);

        // save off old value
        uint32_t oldNacks = callback->mNacks;
        uint32_t oldAcks  = callback->mAcks;

        // feed in out of order feedback
        packet.SetPacketID(0xFFFE);
        feedPacket(packet);

        // nothing should have changed
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, oldNacks );
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, oldAcks );
    }

    void testExtFB7() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);
        packet.SetPacketAcksValid(one_bytes);

        am->SetSequenceNumber(100);

        // should get just a single ACK
        feedPacket(packet);
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }
    
    void testExtFB8() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(100);
        packet.SetPacketAcksValid(one_bytes);

        am->SetSequenceNumber(100);

        // should get just a single ACK for the first time
        feedPacket(packet);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }

    void testExtFB9() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketAcks(one_bytes);
        packet.SetPacketAcksValid(one_bytes);

        am->SetSequenceNumber(1);

        // feed in all possible seqnum twice, causing a wrap-around, verify callbacks
        uint32_t loop_count = ((65536 / CRtcpAppFeedbackPacket::NUM_ACK_BITS) * 2);
        for (uint32_t i = 0; i < loop_count; i++) {
            packet.SetPacketID((CRtcpAppFeedbackPacket::NUM_ACK_BITS * (i + 1)));
            //printf("%d - %hu\n", i, packet.GetPacketID());
            feedPacket(packet);

            CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) CRtcpAppFeedbackPacket::NUM_ACK_BITS );
            CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );

            // reset for the next pass
            callback->mAcks = 0;
        }
    }
    
    void testExtFB10() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(4);

        am->SetSequenceNumber(0);

        // feed in 0-1 INVALID ACK, 2-3 VALID NACK
        packet.SetPacketAckBySeqNum(0, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(1, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(2, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(2, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(3, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(3, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);

        feedPacket(packet);

        // should get no ACK callbacks and 2 NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );

        // feed in 0-1 VALID ACK, 2-3 VALID NACK
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        
        feedPacket(packet);

        // should get 3 ACK callbacks (including packet id) and 0 more NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 3 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );
    }

    void testExtFB11() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(8);

        am->SetSequenceNumber(0);

        // feed in 0-1 INVALID ACK, 2-3 VALID NACK, 4-5 INVALID ACK, 6-7 VALID NACK
        packet.SetPacketAckBySeqNum(0, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(1, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(2, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(2, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(3, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(3, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(4, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(4, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(5, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(5, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(6, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(6, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(7, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(7, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);

        feedPacket(packet);

        // should get no ACK callbacks and 4 NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 4 );

        // feed in 0-1 VALID ACK, 2-3 VALID NACK, 4-5 INVALID ACK, 6-7 VALID NACK
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        
        feedPacket(packet);

        // should get 2 ACK callbacks and 0 more NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 4 );

        // feed in 0-1 VALID ACK, 2-3 VALID NACK, 4-5 VALID ACK, 6-7 VALID NACK
        packet.SetPacketAckValidBySeqNum(4, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckValidBySeqNum(5, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        
        feedPacket(packet);

        // should get 3 more ACK callbacks and 0 more NACKs (0-3 will
        // not be processed again)
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 5 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 4 );
    }

    void testExtFB12() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(6);

        am->SetSequenceNumber(0);

        // feed in 0-1 INVALID ACK, 2-3 VALID NACK, 4-5 VALID ACK (which is no good)
        packet.SetPacketAckBySeqNum(0, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(1, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
        packet.SetPacketAckBySeqNum(2, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(2, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(3, CRtcpAppFeedbackPacket::APP_FB_NACK);
        packet.SetPacketAckValidBySeqNum(3, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(4, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(4, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckBySeqNum(5, CRtcpAppFeedbackPacket::APP_FB_ACK);
        packet.SetPacketAckValidBySeqNum(5, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);

        feedPacket(packet);

        // should get no ACK callbacks and 2 NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );

        // feed in 0-1 VALID ACK, 2-3 VALID NACK, 4-5 VALID ACK (which is ok now)
        packet.SetPacketAckValidBySeqNum(0, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        packet.SetPacketAckValidBySeqNum(1, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
        
        feedPacket(packet);

        // should get 5 ACK callbacks (including packet id) and 0 more NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 5 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 2 );
    }

    void testExtFB13() {
        CRtcpAppExtendedFeedbackPacket packet;
        packet.SetTarget(0xA5A5A011);
        packet.SetPacketID(6);
        packet.SetPacketAcks(one_bytes);
        packet.SetPacketAcksValid(zero_bytes);

        feedPacket(packet);

        // should get 0 ACK callbacks and 0 NACKs
        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }

    void testFBInvalid() {
        CRtcpAppFeedbackPacket packet;

        // send in invalid target
        packet.SetTarget(0xA5A5A012);
        feedPacket(packet);

        CPPUNIT_ASSERT_EQUAL( callback->mAcks, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mNacks, (uint32_t) 0 );
    }
    
    void testLogPrefix() {
        CRtcpAppTXFlowCtrlPacket packet;
        packet.SetSSRC(0x87654321);
        packet.SetTarget(0xA5A5A011);
        packet.SetOpcode(CRtcpAppTXFlowCtrlPacket::OPCODE_START);

        // set prefix then call something that will cause a print to go out
        am->SetLogPrefix("blah");
        packet.SetNtpTime(1);
        feedPacket(packet);
        
        am->SetLogPrefix("");
        packet.SetNtpTime(2);
        feedPacket(packet);
        
        am->SetLogPrefix(NULL);
        packet.SetNtpTime(3);
        feedPacket(packet);
    }
    
    CPPUNIT_TEST_SUITE( CTipMediaSourceTest );
    CPPUNIT_TEST( testCallback );
    CPPUNIT_TEST( testReceiveInvalid );
    CPPUNIT_TEST( testReceiveInvalid2 );
    CPPUNIT_TEST( testReceiveInvalid3 );
    CPPUNIT_TEST( testReceiveInvalid4 );
    CPPUNIT_TEST( testRefresh );
    CPPUNIT_TEST( testRefreshInvalid );
    CPPUNIT_TEST( testTXFlowStart );
    CPPUNIT_TEST( testTXFlowStop );
    CPPUNIT_TEST( testTXFlowInvalid1 );
    CPPUNIT_TEST( testTXFlowInvalid2 );
    CPPUNIT_TEST( testFB1 );
    CPPUNIT_TEST( testFB2 );
    CPPUNIT_TEST( testFB3 );
    CPPUNIT_TEST( testFB4 );
    CPPUNIT_TEST( testFB5 );
    CPPUNIT_TEST( testFB6 );
    CPPUNIT_TEST( testFB7 );
    CPPUNIT_TEST( testFB8 );
    CPPUNIT_TEST( testFB9 );
    CPPUNIT_TEST( testExtFB1 );
    CPPUNIT_TEST( testExtFB2 );
    CPPUNIT_TEST( testExtFB3 );
    CPPUNIT_TEST( testExtFB4 );
    CPPUNIT_TEST( testExtFB5 );
    CPPUNIT_TEST( testExtFB6 );
    CPPUNIT_TEST( testExtFB7 );
    CPPUNIT_TEST( testExtFB8 );
    CPPUNIT_TEST( testExtFB9 );
    CPPUNIT_TEST( testExtFB10 );
    CPPUNIT_TEST( testExtFB11 );
    CPPUNIT_TEST( testExtFB12 );
    CPPUNIT_TEST( testExtFB13 );
    CPPUNIT_TEST( testFBInvalid );
    CPPUNIT_TEST( testLogPrefix );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipMediaSinkTest );
CPPUNIT_TEST_SUITE_REGISTRATION( CTipMediaSourceTest );
