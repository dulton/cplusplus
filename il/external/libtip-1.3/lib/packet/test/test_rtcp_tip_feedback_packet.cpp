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

#include "rtcp_tip_feedback_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppFeedbackPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppFeedbackPacket* packet;
    CRtcpAppExtendedFeedbackPacket* extp;

public:
    void setUp() {
        packet = new CRtcpAppFeedbackPacket();
        extp   = new CRtcpAppExtendedFeedbackPacket();
    }

    void tearDown() {
        delete packet;
        delete extp;
    }

    void testDefaults() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES] = { 0 };

        CPPUNIT_ASSERT_EQUAL( packet->GetType(), (uint8_t) CRtcpPacket::RTPFB );
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) CRtcpAppFeedbackPacket::APPFB_SUBTYPE );
        
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketID() , (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetPacketAcks(), bytes,
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }

    void testTarget() {
        packet->SetTarget(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), (uint32_t) 4 );
    }     

    void testPacketID() {
        packet->SetPacketID(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketID() , (uint16_t) 4 );
    }
        
    void testSetPacketAcks() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xA5, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);

        packet->SetPacketAcks(bytes);
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetPacketAcks(), bytes,
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }

    void testGetIndex() {
        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
            packet->SetPacketAckByIndex(i, CRtcpAppFeedbackPacket::APP_FB_ACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );
        }
    }

    void testGetIndexInvalid() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);

        packet->SetPacketAcks(bytes);
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex((CRtcpAppFeedbackPacket::NUM_ACK_BITS + 1)), 
                              CRtcpAppFeedbackPacket::APP_FB_NACK );
    }

    void testGetSeqNum() {
        packet->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(i),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
            packet->SetPacketAckBySeqNum(i, CRtcpAppFeedbackPacket::APP_FB_ACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(i),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );
        }
    }

    void testGetSeqNumPacketId() {
        packet->SetPacketID(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(4),
                              CRtcpAppFeedbackPacket::APP_FB_ACK );
    }
    
    void testGetSeqNumInvalid() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);

        packet->SetPacketAcks(bytes);
        packet->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum((CRtcpAppFeedbackPacket::NUM_ACK_BITS+1)),
                              CRtcpAppFeedbackPacket::APP_FB_NACK );
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(0xFFFF),
                              CRtcpAppFeedbackPacket::APP_FB_NACK );
    }

    void testSeqNumToIndex() {
        packet->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            packet->SetPacketAckByIndex(i, CRtcpAppFeedbackPacket::APP_FB_ACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(i),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );
        }
    }

    void testSeqNumWrapAround() {
        uint16_t base = 0;
        packet->SetPacketID(0);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            uint16_t seqnum = ((base - CRtcpAppFeedbackPacket::NUM_ACK_BITS) + i);

            packet->SetPacketAckBySeqNum(seqnum, CRtcpAppFeedbackPacket::APP_FB_ACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(seqnum),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );

            packet->SetPacketAckBySeqNum(seqnum, CRtcpAppFeedbackPacket::APP_FB_NACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(seqnum),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
        }
    }

    void testSeqNumWrapAround2() {
        uint16_t base = 96;
        packet->SetPacketID(base);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            uint16_t seqnum = ((base - CRtcpAppFeedbackPacket::NUM_ACK_BITS) + i);

            packet->SetPacketAckBySeqNum(seqnum, CRtcpAppFeedbackPacket::APP_FB_ACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(seqnum),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_ACK );

            packet->SetPacketAckBySeqNum(seqnum, CRtcpAppFeedbackPacket::APP_FB_NACK);
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckBySeqNum(seqnum),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
            CPPUNIT_ASSERT_EQUAL( packet->GetPacketAckByIndex(i),
                                  CRtcpAppFeedbackPacket::APP_FB_NACK );
        }
    }

#define RTCP_APP_FB_BYTES(ssrc, pid)                                                  \
    ((ssrc) >> 24), (((ssrc) >> 16) & 0xFF), (((ssrc) >> 8) & 0xFF), ((ssrc) & 0xFF), \
        (((pid) >> 8) & 0xFF), ((pid) & 0xFF)

#define RTCP_APP_FB_ACK_BYTES 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#define RTCP_APP_FB_ACK_BYTES2 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, \
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE

    void testPack() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, packet->GetLength()),
                          RTCP_APP_FB_BYTES(0, 0), RTCP_APP_FB_ACK_BYTES };

        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (packet->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testPack2() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, packet->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB), RTCP_APP_FB_ACK_BYTES2 };

        uint8_t acks[] = { RTCP_APP_FB_ACK_BYTES2 };
        
        packet->SetTarget(0x12345678);
        packet->SetPacketID(0x90AB);
        packet->SetPacketAcks(acks);
        
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (packet->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpAppFeedbackPacket packet2;
        CPacketBufferData buffer;

        uint8_t acks[] = { RTCP_APP_FB_ACK_BYTES2 };
        
        packet->SetTarget(0x12345678);
        packet->SetPacketID(0x90AB);
        packet->SetPacketAcks(acks);

        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetTarget(), packet2.GetTarget() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPacketID(), packet2.GetPacketID() );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetPacketAcks(),
                                     packet2.GetPacketAcks(),
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }

    void testUnpackFail() {
        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, packet->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB), RTCP_APP_FB_ACK_BYTES2 };

        // force invalid version
        def[0] = 0;

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        // invalid sub-type
        uint8_t def[] = { RTCP_PACKET_BYTES(0,
                                            CRtcpPacket::RTPFB, packet->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB), RTCP_APP_FB_ACK_BYTES2 };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackFail3() {
        // invalid type
        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            0, packet->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB), RTCP_APP_FB_ACK_BYTES2 };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testExtDefaults() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES] = { 0 };

        CPPUNIT_ASSERT_EQUAL( memcmp(extp->GetPacketAcksValid(), bytes,
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }
        
    void testExtGetIndex() {
        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
            extp->SetPacketAckValidByIndex(i, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
        }
    }

    void testExtGetIndexInvalid() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);

        extp->SetPacketAcksValid(bytes);
        CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex((CRtcpAppFeedbackPacket::NUM_ACK_BITS+1)), 
                              CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
    }

    void testExtGetSeqNum() {
        extp->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
            extp->SetPacketAckValidBySeqNum(i, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
        }
    }

    void testExtGetSeqNumPacketId() {
        extp->SetPacketID(4);
        CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(4),
                              CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
    }
    
    void testExtGetSeqNumInvalid() {
        uint8_t bytes[CRtcpAppFeedbackPacket::NUM_ACK_BYTES];
        memset(bytes, 0xFF, CRtcpAppFeedbackPacket::NUM_ACK_BYTES);

        extp->SetPacketAcksValid(bytes);
        extp->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        // out of range seq nums are VALID
        CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(CRtcpAppFeedbackPacket::NUM_ACK_BITS),
                              CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
        CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(0xFFFF),
                              CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
    }

    void testExtSeqNumToIndex() {
        extp->SetPacketID(CRtcpAppFeedbackPacket::NUM_ACK_BITS);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            extp->SetPacketAckValidByIndex(i, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
            extp->SetPacketAckValidByIndex(i, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
        }
    }

    void testExtSeqNumWrapAround() {
        uint16_t base = 0;
        extp->SetPacketID(0);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            uint16_t seqnum = ((base - CRtcpAppFeedbackPacket::NUM_ACK_BITS) + i);

            extp->SetPacketAckValidBySeqNum(seqnum, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(seqnum),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );

            extp->SetPacketAckValidBySeqNum(seqnum, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(seqnum),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
        }
    }

    void testExtSeqNumWrapAround2() {
        uint16_t base = 96;
        extp->SetPacketID(base);

        for (uint16_t i = 0; i < CRtcpAppFeedbackPacket::NUM_ACK_BITS; i++) {
            uint16_t seqnum = ((base - CRtcpAppFeedbackPacket::NUM_ACK_BITS) + i);

            extp->SetPacketAckValidBySeqNum(seqnum, CRtcpAppExtendedFeedbackPacket::APP_FB_VALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(seqnum),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_VALID );

            extp->SetPacketAckValidBySeqNum(seqnum, CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID);
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidBySeqNum(seqnum),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
            CPPUNIT_ASSERT_EQUAL( extp->GetPacketAckValidByIndex(i),
                                  CRtcpAppExtendedFeedbackPacket::APP_FB_INVALID );
        }
    }

    void testExtPack() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0, 0), RTCP_APP_FB_ACK_BYTES,
                          0x00, 0x00, RTCP_APP_FB_ACK_BYTES };

        CPPUNIT_ASSERT_EQUAL( extp->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (extp->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( extp->GetPackSize(), extp->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, extp->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, extp->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), extp->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testExtPack2() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB),
                          RTCP_APP_FB_ACK_BYTES2, 0x00, 0x00, RTCP_APP_FB_ACK_BYTES2 };

        uint8_t acks[] = { RTCP_APP_FB_ACK_BYTES2 };
        
        extp->SetTarget(0x12345678);
        extp->SetPacketID(0x90AB);
        extp->SetPacketAcks(acks);
        extp->SetPacketAcksValid(acks);
        
        CPPUNIT_ASSERT_EQUAL( extp->GetPackSize(), (uint32_t) sizeof(def) );
        CPPUNIT_ASSERT_EQUAL( (extp->GetPackSize() % 4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( extp->GetPackSize(), extp->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, extp->GetPackSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(def, extp->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), extp->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testExtUnpack() {
        CRtcpAppExtendedFeedbackPacket packet2;
        CPacketBufferData buffer;

        uint8_t acks[] = { RTCP_APP_FB_ACK_BYTES2 };
        
        extp->SetTarget(0x12345678);
        extp->SetPacketID(0x90AB);
        extp->SetPacketAcks(acks);
        extp->SetPacketAcksValid(acks);

        extp->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( extp->GetTarget(), packet2.GetTarget() );
        CPPUNIT_ASSERT_EQUAL( extp->GetPacketID(), packet2.GetPacketID() );
        CPPUNIT_ASSERT_EQUAL( memcmp(extp->GetPacketAcksValid(),
                                     packet2.GetPacketAcksValid(),
                                     CRtcpAppFeedbackPacket::NUM_ACK_BYTES), 0 );
    }

    void testExtUnpackFail() {
        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            CRtcpPacket::RTPFB, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB),
                          RTCP_APP_FB_ACK_BYTES2, 0x00, 0x00, RTCP_APP_FB_ACK_BYTES2 };

        // force invalid version
        def[0] = 0;

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( extp->Unpack(buffer), -1 );
    }

    void testExtUnpackFail2() {
        // invalid sub-type
        uint8_t def[] = { RTCP_PACKET_BYTES(0,
                                            CRtcpPacket::RTPFB, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB),
                          RTCP_APP_FB_ACK_BYTES2, 0x00, 0x00, RTCP_APP_FB_ACK_BYTES2 };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( extp->Unpack(buffer), -1 );
    }
    
    void testExtUnpackFail3() {
        // invalid type
        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            0, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB),
                          RTCP_APP_FB_ACK_BYTES2, 0x00, 0x00, RTCP_APP_FB_ACK_BYTES2 };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( extp->Unpack(buffer), -1 );
    }

    void testExtUnpackFail4() {
        // invalid size
        uint8_t def[] = { RTCP_PACKET_BYTES(CRtcpAppFeedbackPacket::APPFB_SUBTYPE,
                                            0, extp->GetLength()),
                          RTCP_APP_FB_BYTES(0x12345678, 0x90AB),
                          RTCP_APP_FB_ACK_BYTES2 };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( extp->Unpack(buffer), -1 );
    }
    
    CPPUNIT_TEST_SUITE( CRtcpAppFeedbackPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testTarget );
    CPPUNIT_TEST( testPacketID );
    CPPUNIT_TEST( testSetPacketAcks );
    CPPUNIT_TEST( testGetIndex );
    CPPUNIT_TEST( testGetIndexInvalid );
    CPPUNIT_TEST( testGetSeqNum );
    CPPUNIT_TEST( testGetSeqNumPacketId );
    CPPUNIT_TEST( testGetSeqNumInvalid );
    CPPUNIT_TEST( testSeqNumToIndex );
    CPPUNIT_TEST( testSeqNumWrapAround );
    CPPUNIT_TEST( testSeqNumWrapAround2 );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testUnpackFail3 );
    CPPUNIT_TEST( testExtDefaults );
    CPPUNIT_TEST( testExtGetIndex );
    CPPUNIT_TEST( testExtGetIndexInvalid );
    CPPUNIT_TEST( testExtGetSeqNum );
    CPPUNIT_TEST( testExtGetSeqNumInvalid );
    CPPUNIT_TEST( testExtSeqNumToIndex );
    CPPUNIT_TEST( testExtSeqNumWrapAround );
    CPPUNIT_TEST( testExtSeqNumWrapAround2 );
    CPPUNIT_TEST( testExtPack );
    CPPUNIT_TEST( testExtPack2 );
    CPPUNIT_TEST( testExtUnpack );
    CPPUNIT_TEST( testExtUnpackFail );
    CPPUNIT_TEST( testExtUnpackFail2 );
    CPPUNIT_TEST( testExtUnpackFail3 );
    CPPUNIT_TEST( testExtUnpackFail4 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppFeedbackPacketTest );
