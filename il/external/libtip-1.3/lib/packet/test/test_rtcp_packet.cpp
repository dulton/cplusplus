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

#include "rtcp_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpPacketTest : public CppUnit::TestFixture {
private:
    CRtcpPacket* packet;

public:
    void setUp() {
        packet = new CRtcpPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint8_t) CRtcpPacket::RTCP_VERSION );
        CPPUNIT_ASSERT_EQUAL( packet->GetPadding(), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetType(), (uint8_t) 0 );
    }

    void testSubType() {
        // check the default value
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 0 );

        // check simple set/get
        packet->SetSubType(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 4 );

        // check the range of valid subtypes
        packet->SetSubType(0);
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 0 );

        packet->SetSubType(0x1F);
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 0x1F );

        // 0x1F is the max value, anything bigger should get masked out
        packet->SetSubType(0xFF);
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), (uint8_t) 0x1F );
    }
    
    void testType() {
        CPPUNIT_ASSERT_EQUAL( packet->GetType(), (uint8_t) 0 );

        packet->SetType(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetType(), (uint8_t) 4 );
    }
    
    void testLength() {
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 0 );
    }

    void testLengthConversion() {
        // note only works for multiples of 4 bytes
        CPPUNIT_ASSERT_EQUAL( CRtcpPacket::BytesToRtcpLength(4), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( CRtcpPacket::BytesToRtcpLength(256), (uint32_t) 63 );

        CPPUNIT_ASSERT_EQUAL( CRtcpPacket::RtcpLengthToBytes(0), (uint32_t) 4 );
        CPPUNIT_ASSERT_EQUAL( CRtcpPacket::RtcpLengthToBytes(63), (uint32_t) 256 );
    }
    
    void testPackSize() {
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) 4 );
    }
    
    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_HEADER_BYTES(0, 0, packet->GetLength()) };
    
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );

        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpPacket packet2;
        CPacketBufferData buffer;

        packet->SetSubType(4);
        packet->SetType(4);

        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), packet2.GetVersion() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPadding(), packet2.GetPadding() );
        CPPUNIT_ASSERT_EQUAL( packet->GetSubType(), packet2.GetSubType() );
        CPPUNIT_ASSERT_EQUAL( packet->GetType(), packet2.GetType() );
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail() {
        CPacketBufferData buffer;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        CPacketBufferData buffer;
        packet->Pack(buffer);

        // force set RTCP version to 0
        uint8_t* data = buffer.GetBuffer();
        data[0] = 0;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail3() {
        CPacketBufferData buffer;
        packet->Pack(buffer);

        // force set RTCP version to a large number
        uint16_t* data = (uint16_t*) buffer.GetBuffer();
        data[1] = 0x8000;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    class CRtcpTest : public CRtcpPacket {
    public:
        CRtcpTest(uint32_t pad) { mPad = pad; IncrSize(pad); }
        ~CRtcpTest() {}

    protected:
        virtual uint32_t PackData(CPacketBuffer& buffer) const {
            CRtcpPacket::PackData(buffer);
            
            uint8_t pad = 0;
            for (uint32_t i = 0; i < mPad; i++) {
                buffer.Add(pad);
            }

            return buffer.GetBufferSize();
        }

        virtual int UnpackData(CPacketBuffer& buffer) {
            CRtcpPacket::UnpackData(buffer);
            
            uint8_t pad;
            for (uint32_t i = 0; i < mPad; i++) {
                buffer.Rem(pad);
            }

            return 0;
        }
        
        uint32_t mPad;
    };
            
    void testPackPad() {
        for (uint32_t i = 0; i < 4; i++) {
            CRtcpTest test(i);
            CPacketBufferData buffer;

            CPPUNIT_ASSERT_EQUAL( (test.GetPackSize() % 4), (uint32_t) 0 );
            CPPUNIT_ASSERT_EQUAL( (test.Pack(buffer) % 4), (uint32_t) 0 );
            CPPUNIT_ASSERT_EQUAL( (buffer.GetBufferSize() % 4), (uint32_t) 0 );
            CPPUNIT_ASSERT_EQUAL( test.Unpack(buffer), 0 );
            CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), (uint32_t) 0 );
        }
    }

    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }
    
    CPPUNIT_TEST_SUITE( CRtcpPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testSubType );
    CPPUNIT_TEST( testType );
    CPPUNIT_TEST( testLength );
    CPPUNIT_TEST( testLengthConversion );
    CPPUNIT_TEST( testPackSize );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testUnpackFail3 );
    CPPUNIT_TEST( testPackPad );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

class CRtcpPacketSSRCTest : public CppUnit::TestFixture {
private:
    CRtcpPacketSSRC* packet;

public:
    void setUp() {
        packet = new CRtcpPacketSSRC();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(), (uint32_t) 0 );
    }

    void testLength() {
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 1 );
    }

    void testSSRC() {
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(), (uint32_t) 0 );

        packet->SetSSRC(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(), (uint32_t) 4 );
    }

    void testPackSize() {
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (uint32_t) 8 );
    }
    
    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_PACKET_BYTES(0, 0, packet->GetLength()) };
    
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );

        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpPacketSSRC packet2;
        CPacketBufferData buffer;

        packet->SetSSRC(0x12345678);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(), packet2.GetSSRC() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail() {
        CPacketBufferData buffer;
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    CPPUNIT_TEST_SUITE( CRtcpPacketSSRCTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testLength );
    CPPUNIT_TEST( testSSRC );
    CPPUNIT_TEST( testPackSize );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST_SUITE_END();
};

class CRtcpAppPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testLength() {
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 2 );
    }

    void testName() {
        CRtcpAppPacket::RtcpAppName def;
        memset(&def, 0, sizeof(def));
    
        CRtcpAppPacket::RtcpAppName name = packet->GetAppName();
        CPPUNIT_ASSERT_EQUAL( memcmp(&name, &def, sizeof(name)), 0 );

        memset(&def, 'a', sizeof(def));
        packet->SetAppName(def);
        name = packet->GetAppName();
        CPPUNIT_ASSERT_EQUAL( memcmp(&name, &def, sizeof(name)), 0 );
    }

    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_PACKET_BYTES(0, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES };

        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;
            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpAppPacket packet2;
        CPacketBufferData buffer;

        CRtcpAppPacket::RtcpAppName def;
        memset(&def, 'a', sizeof(def));

        packet->SetAppName(def);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
        CRtcpAppPacket::RtcpAppName name = packet2.GetAppName();

        CPPUNIT_ASSERT_EQUAL( memcmp(&name, &def, sizeof(name)), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail1() {
        CPacketBufferData buffer;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        CRtcpAppPacket packet2;
        CPacketBufferData buffer;

        packet->SetType(CRtcpPacket::RR);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), -1 );
    }

    CPPUNIT_TEST_SUITE( CRtcpAppPacketTest );
    CPPUNIT_TEST( testName );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail1 );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST_SUITE_END();
};

class CRtcpTipPacketTest : public CppUnit::TestFixture {
private:
    CRtcpTipPacket* packet;

public:
    void setUp() {
        packet = new CRtcpTipPacket(MUXCTRL);
    }

    void tearDown() {
        delete packet;
    }

    void testNtpTime() {
        CPPUNIT_ASSERT_EQUAL( packet->GetNtpTime(), (uint64_t) 0 );

        packet->SetNtpTime(4);
        CPPUNIT_ASSERT_EQUAL( packet->GetNtpTime(), (uint64_t) 4 );
    }

    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_PACKET_BYTES(MUXCTRL, CRtcpPacket::APP, packet->GetLength()),
                        RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES };
    
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;

            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpTipPacket packet2(MUXCTRL);
        CPacketBufferData buffer;

        packet->SetNtpTime(0x1234567890ABCDEFLL);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetNtpTime(), packet2.GetNtpTime() );
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail() {
        CPacketBufferData buffer;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        CRtcpTipPacket packet2(MUXCTRL);
        CPacketBufferData buffer;

        packet->SetAppName("abcd");
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), -1 );
    }
    
    void testPacketTypeString() {
        CPPUNIT_ASSERT_EQUAL( strcmp(packet->GetTipPacketTypeString(), "MUXCTRL"), 0 );
    }

    void testAck() {
        CRtcpTipPacket ack(ConvertTipNonAckToAck(packet->GetTipPacketType()));
        ack.SetNtpTime(packet->GetNtpTime());

        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), true );
    }

    void testNotAck1() {
        CRtcpTipPacket ack(ACK_MEDIAOPTS);
        ack.SetNtpTime(packet->GetNtpTime());

        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }
    
    void testNotAck2() {
        CRtcpTipPacket ack(ConvertTipNonAckToAck(packet->GetTipPacketType()));
        ack.SetNtpTime((packet->GetNtpTime()+1));

        CPPUNIT_ASSERT_EQUAL( packet->IsMyAck(ack), false );
    }

    // just verify the stream functions don't crash anything
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    void testPosToStream() {
        ostringstream oss;
        
        CRtcpTipPacket::PositionsToStream(oss, 0xFFFF, AUDIO);
        CRtcpTipPacket::PositionsToStream(oss, 0xFFFF, VIDEO);
        CRtcpTipPacket::PositionsToStream(oss, 0xFFFF, MT_MAX);
    }
    
    CPPUNIT_TEST_SUITE( CRtcpTipPacketTest );
    CPPUNIT_TEST( testNtpTime );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testPacketTypeString );
    CPPUNIT_TEST( testAck );
    CPPUNIT_TEST( testNotAck1 );
    CPPUNIT_TEST( testNotAck2 );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST( testPosToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpPacketTest );
CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpPacketSSRCTest );
CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppPacketTest );
CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpTipPacketTest );
