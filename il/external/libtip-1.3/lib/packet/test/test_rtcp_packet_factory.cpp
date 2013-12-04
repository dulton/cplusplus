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

#include "tip_debug_tools.h"
#include "test_packet_data.h"
#include "rtcp_packet_factory.h"
#include "rtcp_rr_packet.h"
#include "rtcp_sdes_packet.h"
#include "rtcp_tip_echo_packet.h"
#include "rtcp_tip_feedback_packet.h"
#include "rtcp_tip_flowctrl_packet.h"
#include "rtcp_tip_mediaopts_packet.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_refresh_packet.h"
#include "rtcp_tip_reqtosend_packet.h"
#include "rtcp_tip_spimap_packet.h"
#include "rtcp_tip_notify_packet.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpPacketFactoryTest : public CppUnit::TestFixture {
public:
    void testCreate() {
        CPacketBufferData buf;
    
        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buf) == NULL );
    }

    void testInvalid() {
        for (uint32_t i = 0; i < MAX_PACKET_TYPE; i++) {
            CRtcpTipPacket packet((TipPacketType) i);
            CPacketBufferData buf;

            // acks will pass the check b/c they contain no extra
            // data, so skip them
            if (IsAckTipPacketType((TipPacketType) i)) {
                continue;
            }

            // NOTIFY will pass the check b/c they don't have to
            // contain extra data either.
            if (i == NOTIFY) {
                continue;
            }
            
            packet.Pack(buf);
            CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buf) == NULL );
            CPPUNIT_ASSERT_EQUAL( buf.GetBufferSize(), (uint32_t) 0 );
        }
    }

    void testInvalid2() {
        CPacketBufferData buf;

        uint8_t data = 0;
        buf.Add(data);

        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buf) == NULL );
        CPPUNIT_ASSERT_EQUAL( buf.GetBufferSize(), (uint32_t) 0 );
    }

    void testInvalid3() {
        CRtcpPacket packet;
        packet.SetType(CRtcpPacket::APP);

        CPacketBufferData buf;
        packet.Pack(buf);

        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buf) == NULL );
        CPPUNIT_ASSERT_EQUAL( buf.GetBufferSize(), (uint32_t) 0 );
    }

    void testInvalid4() {
        CRtcpPacket packet;
        packet.SetType(0);

        CPacketBufferData buffer;
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), (uint32_t) 0 );
    }
    
    void testCreateMuxCtrlV6() {
        CRtcpAppMuxCtrlPacket packet;
        CRtcpAppMuxCtrlPacketBase* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(ret);
        CPPUNIT_ASSERT( packet2 == NULL );
        
        packet2 = dynamic_cast<CRtcpAppMuxCtrlPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetVersion(), packet2->GetVersion() );
        delete ret;
    }
    
    void testCreateMuxCtrl() {
        CRtcpAppMuxCtrlV7Packet packet;
        CRtcpAppMuxCtrlV7Packet* packet2;
        CPacketBufferData buffer;

        packet.SetProfile(4);
        packet.SetNumXmit(4);
        packet.SetNumRcv(4);
        packet.SetNtpTime(4);
        packet.SetConfID(4);
        packet.SetXmitPositions(4);
        packet.SetRcvPositions(4);
        packet.SetNumShared(4);
        packet.SetSharedPositions(4);

        uint8_t* testID = (uint8_t*) "abcdABCD1234";
        uint32_t testIDLen = strlen((char*) testID);
        packet.SetParticipantID(testID, testIDLen);
    
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );
    
        CPPUNIT_ASSERT_EQUAL( packet.GetVersion(), packet2->GetVersion() );
        CPPUNIT_ASSERT_EQUAL( packet.GetProfile(), packet2->GetProfile() );
        CPPUNIT_ASSERT_EQUAL( packet.GetNumXmit(), packet2->GetNumXmit() );
        CPPUNIT_ASSERT_EQUAL( packet.GetNumRcv(), packet2->GetNumRcv() );
        CPPUNIT_ASSERT_EQUAL( packet.GetConfID(), packet2->GetConfID() );
        CPPUNIT_ASSERT_EQUAL( packet.GetXmitPositions(), packet2->GetXmitPositions() );
        CPPUNIT_ASSERT_EQUAL( packet.GetRcvPositions(), packet2->GetRcvPositions() );
        CPPUNIT_ASSERT_EQUAL( packet.GetNumShared(), packet2->GetNumShared() );
        CPPUNIT_ASSERT_EQUAL( packet.GetSharedPositions(), packet2->GetSharedPositions() );

        uint8_t getBuffer[CTipTlv::MAX_TLV_DATA_LEN];
        CPPUNIT_ASSERT( packet2->GetParticipantID(getBuffer) == testIDLen );
        CPPUNIT_ASSERT( memcmp(getBuffer, testID, testIDLen) == 0 );

        CPPUNIT_ASSERT_EQUAL( packet.GetLength(), packet2->GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet.GetPackSize(), packet2->GetPackSize() );

        delete ret;
    }

    // this will fail b/c the version is 8 but the data is not the
    // correct format
    void testCreateMuxCtrlInvalid() {
        CRtcpAppMuxCtrlPacket packet;
        packet.SetVersion(8);

        CPacketBufferData buffer;

        packet.Pack(buffer);

        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buffer) == NULL );
    }
    
    void testCreateMuxCtrlInvalid2() {
        CRtcpAppMuxCtrlV7Packet packet;
        CPacketBufferData buffer;

        packet.SetVersion(5);
        packet.Pack(buffer);

        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buffer) == NULL );
    }
    
    void testCreateMuxCtrlInvalid3() {
        CRtcpAppMuxCtrlPacket packet;
        CPacketBufferData buffer;

        packet.SetVersion(7);
        packet.Pack(buffer);

        CPPUNIT_ASSERT( CRtcpPacketFactory::CreatePacketFromBuffer(buffer) == NULL );
    }
    
    void testCreateMO() {
        CRtcpAppMediaoptsPacket packet;
        CRtcpAppMediaoptsPacket* packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        packet.AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet.AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        packet.AddSSRC(0x00000000, 0x90ABCDEF, 0x13579ACE);
        packet.AddOption(0x00000000, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
    
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppMediaoptsPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        buffer.ResetHead();
        packet2->Pack(buffer2);
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), buffer2.GetBufferSize() );
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  "
                << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    "
                << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());
            
            CPPUNIT_FAIL(oss.str());
        }
    
        list<uint32_t> ssrc = packet2->GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 2 );
        CPPUNIT_ASSERT_EQUAL( *(ssrc.begin()), (unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( *(++(ssrc.begin())), (unsigned int) 1 );

        uint32_t xmit;
        uint32_t rcv;
        CPPUNIT_ASSERT_EQUAL( packet2->GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 0x90ABCDEF );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 0x13579ACE );

        CPPUNIT_ASSERT_EQUAL( packet2->GetSSRC(1, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 0x90ABCDEF );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 0x13579ACE );

        uint32_t value;
        list<CRtcpAppMediaoptsPacket::OptionTag> opt = packet2->GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( opt.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet2->GetOption(0, *(opt.begin()), value), 0 );
        CPPUNIT_ASSERT_EQUAL( value, (uint32_t) 0x123456 );

        opt = packet2->GetOptions(1);
        CPPUNIT_ASSERT_EQUAL( opt.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet2->GetOption(0, *(opt.begin()), value), 0 );
        CPPUNIT_ASSERT_EQUAL( value, (uint32_t) 0x123456 );

        CPPUNIT_ASSERT_EQUAL( packet.GetLength(), packet2->GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet.GetPackSize(), packet2->GetPackSize() );

        delete ret;
    }

    void testCreateMCAck() {
        CRtcpTipAckPacket packet(ACK_MUXCTRL);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_MUXCTRL );

        delete ret;
    }

    void testCreateMOAck() {
        CRtcpTipAckPacket packet(ACK_MEDIAOPTS);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_MEDIAOPTS );

        delete ret;
    }

    void testCreateTXFlowCtrl() {
        CRtcpAppTXFlowCtrlPacket packet;
        CRtcpAppTXFlowCtrlPacket* packet2;
        CPacketBufferData buffer;

        packet.SetOpcode(0xA5A5A5A5);
        packet.SetTarget(0x12345678);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppTXFlowCtrlPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetOpcode(), packet2->GetOpcode() );
        CPPUNIT_ASSERT_EQUAL( packet.GetTarget(), packet2->GetTarget() );

        delete ret;
    }
    
    void testCreateTXFlowCtrlAck() {
        CRtcpTipAckPacket packet(ACK_TXFLOWCTRL);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_TXFLOWCTRL );

        delete ret;
    }

    void testCreateRXFlowCtrl() {
        CRtcpAppRXFlowCtrlPacket packet;
        CRtcpAppRXFlowCtrlPacket* packet2;
        CPacketBufferData buffer;

        packet.SetOpcode(0xA5A5A5A5);
        packet.SetTarget(0x12345678);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppRXFlowCtrlPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetOpcode(), packet2->GetOpcode() );
        CPPUNIT_ASSERT_EQUAL( packet.GetTarget(), packet2->GetTarget() );

        delete ret;
    }
    
    void testCreateRXFlowCtrlAck() {
        CRtcpTipAckPacket packet(ACK_RXFLOWCTRL);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_RXFLOWCTRL );

        delete ret;
    }

    void testCreateRefresh() {
        CRtcpAppRefreshPacket packet;
        CRtcpAppRefreshPacket* packet2;
        CPacketBufferData buffer;

        packet.SetTarget(0x12345678);
        packet.SetFlags(0x90ABCDEF);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppRefreshPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetTarget(), packet2->GetTarget() );
        CPPUNIT_ASSERT_EQUAL( packet.GetFlags(), packet2->GetFlags() );

        delete ret;
    }
    
    void testCreateRefreshAck() {
        CRtcpTipAckPacket packet(ACK_REFRESH);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_REFRESH );

        delete ret;
    }

    void testCreateRTS() {
        CRtcpAppReqToSendPacket packet;
        CRtcpAppReqToSendPacket* packet2;
        CPacketBufferData buffer;

        packet.SetFlags(1);
        packet.SetVideoPos(0x1234);
        packet.SetAudioPos(0x5678);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppReqToSendPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetFlags(), packet2->GetFlags() );
        CPPUNIT_ASSERT_EQUAL( packet.GetVideoPos(), packet2->GetVideoPos() );
        CPPUNIT_ASSERT_EQUAL( packet.GetAudioPos(), packet2->GetAudioPos() );

        delete ret;
    }
    
    void testCreateRTSAck() {
        CRtcpAppReqToSendAckPacket packet;
        CRtcpAppReqToSendAckPacket* packet2;
        CPacketBufferData buffer;

        packet.SetFlags(1);
        packet.SetVideoPos(0x1234);
        packet.SetAudioPos(0x5678);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppReqToSendAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetFlags(), packet2->GetFlags() );
        CPPUNIT_ASSERT_EQUAL( packet.GetVideoPos(), packet2->GetVideoPos() );
        CPPUNIT_ASSERT_EQUAL( packet.GetAudioPos(), packet2->GetAudioPos() );

        delete ret;
    }        

    void testCreateRTSAckInvalid() {
        CRtcpTipPacket packet(ACK_REQTOSEND);
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
    }        

    void testCreateSpiMap() {
        CRtcpAppSpiMapPacket packet;
        CRtcpAppSpiMapPacket* packet2;
        CPacketBufferData buffer;

        packet.SetSPI(0x1234);

        uint8_t salt[CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH] = { 0x01, 0x02, 0x03, 0x04 };
        packet.SetSrtpSalt(salt);

        uint8_t kek[CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH] = { 0x01, 0x02, 0x03, 0x04 };
        packet.SetKek(kek);
        
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppSpiMapPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetSPI(), packet2->GetSPI() );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet.GetSrtpSalt(), packet2->GetSrtpSalt(),
                                     CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH), 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet.GetKek(), packet2->GetKek(),
                                     CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH), 0 );

        delete ret;
    }
    
    void testCreateSpiMapAck() {
        CRtcpTipAckPacket packet(ACK_SPIMAP);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_SPIMAP );

        delete ret;
    }        

    void testCreateNotify() {
        CRtcpAppNotifyPacket packet;
        CRtcpAppNotifyPacket* packet2;
        CPacketBufferData buffer;

        uint8_t data = 1;
        packet.AddTLV(CRtcpAppNotifyPacket::SECURITYICON, &data, sizeof(data));
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppNotifyPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        uint8_t data2;
        CPPUNIT_ASSERT_EQUAL( packet.GetTLVByTag(CRtcpAppNotifyPacket::SECURITYICON, &data2), (int) sizeof(data2) );
        CPPUNIT_ASSERT_EQUAL( data, data2 );

        delete ret;
    }
    
    void testCreateNotifyAck() {
        CRtcpTipAckPacket packet(ACK_NOTIFY);
        CRtcpTipAckPacket* packet2;
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpTipAckPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), ACK_NOTIFY );

        delete ret;
    }

    void testCreateMultiple() {
        CRtcpAppMuxCtrlV7Packet mc;
        CRtcpAppMediaoptsPacket mo;
    
        CPacketBufferData buffer;

        mc.Pack(buffer);
        mo.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(ret) != NULL );
        delete ret;
    
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpAppMediaoptsPacket*>(ret) != NULL );

        delete ret;
    }

    void testCreateMultiple2() {
        CRtcpRRPacket rr;

        CRtcpPacket app;
        app.SetType(CRtcpPacket::APP);

        CRtcpSDESPacket sdes;

        CPacketBufferData buffer;

        // insert an un-parsable APP into the middle.  verify that we
        // still parse the RR and SDES
        rr.Pack(buffer);
        app.Pack(buffer);
        sdes.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpRRPacket*>(ret) != NULL );
        delete ret;
    
        // this should be NULL
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );

        // this should be the SDES
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpSDESPacket*>(ret) != NULL );

        delete ret;
    }

    void testCreateMultiple3() {
        CRtcpRRPacket rr;
        CRtcpSDESPacket sdes;

        CRtcpPacket app;
        app.SetType(CRtcpPacket::APP);

        CPacketBufferData buffer;

        // insert an un-parsable APP at the end.  verify that we
        // still parse the RR and SDES
        rr.Pack(buffer);
        sdes.Pack(buffer);
        app.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpRRPacket*>(ret) != NULL );
        delete ret;
    
        // this should be the SDES
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpSDESPacket*>(ret) != NULL );
        delete ret;

        // this should be NULL
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), (uint32_t) 0 );
    }

    void testCreateMultipleInvalid() {
        CRtcpRRPacket rr;
        CRtcpSDESPacket sdes;

        CPacketBufferData buffer;

        // insert an un-parsable section in the middle.  verify that
        // we get the RR but not the SDES
        rr.Pack(buffer);

        // add a few bytes of useless data
        uint8_t data = 0;
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);
        buffer.Add(data);

        // and now the SDES
        sdes.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpRRPacket*>(ret) != NULL );
        delete ret;
    
        // this should be NULL and we should be done with the buffer
        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), (uint32_t) 0 );
    }

    void doCreateAck(CRtcpTipPacket& packet) {
        packet.SetNtpTime(1);

        CRtcpTipPacket* ret = CRtcpPacketFactory::CreateAckPacket(packet);
        CPPUNIT_ASSERT( ret != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.IsMyAck(*ret), true );
        delete ret;
    }
    
    void testCreateAckMuxCtrl() {
        CRtcpAppMuxCtrlV7Packet packet;
        doCreateAck(packet);
    }
    
    void testCreateAckMO() {
        CRtcpAppMediaoptsPacket packet;
        doCreateAck(packet);
    }
    
    void testCreateAckTXFlowCtrl() {
        CRtcpAppTXFlowCtrlPacket packet;
        doCreateAck(packet);
    }
    
    void testCreateAckRXFlowCtrl() {
        CRtcpAppRXFlowCtrlPacket packet;
        doCreateAck(packet);
    }
    
    void testCreateAckRefresh() {
        CRtcpAppRefreshPacket packet;
        doCreateAck(packet);
    }

    void testCreateAckRTS() {
        CRtcpAppReqToSendPacket packet;
        doCreateAck(packet);
    }

    void testCreateAckRTSInvalid() {
        CRtcpTipPacket packet(REQTOSEND);
        
        CRtcpTipPacket* ret = CRtcpPacketFactory::CreateAckPacket(packet);
        CPPUNIT_ASSERT( ret == NULL );
    }

    void testCreateAckSpiMap() {
        CRtcpAppSpiMapPacket packet;
        doCreateAck(packet);
    }
    
    void testCreateAckNotify() {
        CRtcpAppNotifyPacket packet;
        doCreateAck(packet);
    }

    void testCreateFB() {
        CRtcpAppFeedbackPacket packet;
        CRtcpAppFeedbackPacket* packet2;
        CPacketBufferData buffer;

        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xA5, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
        
        packet.SetTarget(0x12345678);
        packet.SetPacketID(0x90AB);
        packet.SetPacketAcks(bytes);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppFeedbackPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetTarget(), packet2->GetTarget() );
        CPPUNIT_ASSERT_EQUAL( packet.GetPacketID(), packet2->GetPacketID() );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet.GetPacketAcks(), packet2->GetPacketAcks(),
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );

        delete ret;
    }

    void testCreateExtFB() {
        CRtcpAppExtendedFeedbackPacket packet;
        CRtcpAppExtendedFeedbackPacket* packet2;
        CPacketBufferData buffer;

        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xA5, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);
        
        packet.SetTarget(0x12345678);
        packet.SetPacketID(0x90AB);
        packet.SetPacketAcks(bytes);
        packet.SetPacketAcksValid(bytes);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppExtendedFeedbackPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetTarget(), packet2->GetTarget() );
        CPPUNIT_ASSERT_EQUAL( packet.GetPacketID(), packet2->GetPacketID() );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet.GetPacketAcks(), packet2->GetPacketAcks(),
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet.GetPacketAcksValid(),
                                     packet2->GetPacketAcksValid(),
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );

        delete ret;
    }
    
    void testCreateFBInvalid() {
        CRtcpPacket packet;
        packet.SetType(CRtcpPacket::RTPFB);

        CPacketBufferData buffer;
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
    }

    void testCreateRR() {
        CRtcpRRPacket packet;

        CPacketBufferData buffer;
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpRRPacket*>(ret) != NULL );
        delete ret;
    }
    
    void testCreateSDES() {
        CRtcpSDESPacket packet;

        CPacketBufferData buffer;
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpSDESPacket*>(ret) != NULL );
        delete ret;
    }
    
    void testWrapper() {
        CRtcpRRPacket rr;
        CRtcpSDESPacket sdes;
        CRtcpAppMuxCtrlV7Packet mc;
        CPacketBufferData buffer;

        rr.Pack(buffer);
        sdes.Pack(buffer);
        mc.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpRRPacket*>(ret) != NULL );
        delete ret;

        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpSDESPacket*>(ret) != NULL );
        delete ret;

        ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );
        CPPUNIT_ASSERT( dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(ret) != NULL );
        delete ret;
    }

    void testCreateEcho() {
        CRtcpAppEchoPacket packet;
        CRtcpAppEchoPacket* packet2;
        CPacketBufferData buffer;

        packet.SetNtpTime(0x1234567890ABCDEFLL);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppEchoPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet.GetNtpTime(), packet2->GetNtpTime() );
        CPPUNIT_ASSERT_EQUAL( packet.GetRcvNtpTime(), packet2->GetRcvNtpTime() );

        delete ret;
    }
    
    void testCreateEchoAck() {
        CRtcpAppEchoPacket packet;
        CRtcpAppEchoPacket* packet2;
        CPacketBufferData buffer;

        packet.SetRcvNtpTime(0x1234567890ABCDEFLL);
        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret != NULL );

        packet2 = dynamic_cast<CRtcpAppEchoPacket*>(ret);
        CPPUNIT_ASSERT( packet2 != NULL );

        CPPUNIT_ASSERT_EQUAL( packet2->GetTipPacketType(), TIPECHO );
        CPPUNIT_ASSERT_EQUAL( packet2->GetRcvNtpTime(), packet.GetRcvNtpTime() );
        
        delete ret;
    }

    void testCreateEchoAckInvalid() {
        CRtcpTipPacket packet(ACK_TIPECHO);
        CPacketBufferData buffer;

        packet.Pack(buffer);

        CRtcpPacket* ret = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
        CPPUNIT_ASSERT( ret == NULL );
    }        

    void testCreateAckEcho() {
        CRtcpAppEchoPacket packet;
        doCreateAck(packet);
    }

    void testCreateAckEchoInvalid() {
        CRtcpTipPacket packet(TIPECHO);
        
        CRtcpTipPacket* ret = CRtcpPacketFactory::CreateAckPacket(packet);
        CPPUNIT_ASSERT( ret == NULL );
    }

    CPPUNIT_TEST_SUITE( CRtcpPacketFactoryTest );
    CPPUNIT_TEST( testInvalid );
    CPPUNIT_TEST( testInvalid2 );
    CPPUNIT_TEST( testInvalid3 );
    CPPUNIT_TEST( testInvalid4 );
    CPPUNIT_TEST( testCreate );
    CPPUNIT_TEST( testCreateMuxCtrlV6 );
    CPPUNIT_TEST( testCreateMuxCtrl );
    CPPUNIT_TEST( testCreateMuxCtrlInvalid );
    CPPUNIT_TEST( testCreateMuxCtrlInvalid2 );
    CPPUNIT_TEST( testCreateMuxCtrlInvalid3 );
    CPPUNIT_TEST( testCreateMO );
    CPPUNIT_TEST( testCreateMCAck );
    CPPUNIT_TEST( testCreateMOAck );
    CPPUNIT_TEST( testCreateTXFlowCtrl );
    CPPUNIT_TEST( testCreateTXFlowCtrlAck );
    CPPUNIT_TEST( testCreateRXFlowCtrl );
    CPPUNIT_TEST( testCreateRXFlowCtrlAck );
    CPPUNIT_TEST( testCreateRefresh );
    CPPUNIT_TEST( testCreateRefreshAck );
    CPPUNIT_TEST( testCreateRTS );
    CPPUNIT_TEST( testCreateRTSAck );
    CPPUNIT_TEST( testCreateRTSAckInvalid );
    CPPUNIT_TEST( testCreateSpiMap );
    CPPUNIT_TEST( testCreateSpiMapAck );
    CPPUNIT_TEST( testCreateNotify );
    CPPUNIT_TEST( testCreateNotifyAck );
    CPPUNIT_TEST( testCreateMultiple );
    CPPUNIT_TEST( testCreateMultiple2 );
    CPPUNIT_TEST( testCreateMultiple3 );
    CPPUNIT_TEST( testCreateMultipleInvalid );
    CPPUNIT_TEST( testCreateAckMuxCtrl );
    CPPUNIT_TEST( testCreateAckMO );
    CPPUNIT_TEST( testCreateAckTXFlowCtrl );
    CPPUNIT_TEST( testCreateAckRXFlowCtrl );
    CPPUNIT_TEST( testCreateAckRefresh );
    CPPUNIT_TEST( testCreateAckRTS );
    CPPUNIT_TEST( testCreateAckRTSInvalid );
    CPPUNIT_TEST( testCreateAckSpiMap );
    CPPUNIT_TEST( testCreateAckNotify );
    CPPUNIT_TEST( testCreateFB );
    CPPUNIT_TEST( testCreateExtFB );
    CPPUNIT_TEST( testCreateFBInvalid );
    CPPUNIT_TEST( testCreateRR );
    CPPUNIT_TEST( testCreateSDES );
    CPPUNIT_TEST( testWrapper );
    CPPUNIT_TEST( testCreateEcho );
    CPPUNIT_TEST( testCreateEchoAck );
    CPPUNIT_TEST( testCreateEchoAckInvalid );
    CPPUNIT_TEST( testCreateAckEcho );
    CPPUNIT_TEST( testCreateAckEchoInvalid );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpPacketFactoryTest );
