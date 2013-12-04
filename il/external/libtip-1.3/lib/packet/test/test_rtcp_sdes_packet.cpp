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
#include "rtcp_sdes_packet.h"
#include "test_packet_data.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpSDESPacketTest : public CppUnit::TestFixture {
private:
    CRtcpSDESPacket* packet;

public:
    void setUp() {
        packet = new CRtcpSDESPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetChunkSSRC(0), (uint32_t) 0 );
    }

    void testAddChunk() {
        packet->AddChunk(0x12345678);
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 2 );
        CPPUNIT_ASSERT_EQUAL( packet->GetChunkSSRC(0), (uint32_t) 0x12345678 );

        packet->AddChunk(0x90ABCDEF);
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 4 );
        CPPUNIT_ASSERT_EQUAL( packet->GetChunkSSRC(1), (uint32_t) 0x90ABCDEF );
    }

    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_HEADER_BYTES(0, CRtcpPacket::SDES, packet->GetLength()) };
    
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;

            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpSDESPacket packet2;
        CPacketBufferData buffer;
        
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpack2() {
        CRtcpSDESPacket packet2;
        CPacketBufferData buffer;
        
        packet->AddChunk(0x12345678);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
        CPPUNIT_ASSERT_EQUAL( packet->GetChunkSSRC(0), packet2.GetChunkSSRC(0) );
    }

    void testUnpack3() {
        CRtcpSDESPacket packet2;
        CPacketBufferData buffer;
        
        packet->AddChunk(0x12345678);
        packet->AddChunk(0x90ABCDEF);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        // only the first chunk will be unpacked
        CPPUNIT_ASSERT_EQUAL( (uint16_t) (packet->GetLength() - 2), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( (uint32_t) (packet->GetPackSize() - 8), packet2.GetPackSize() );
        CPPUNIT_ASSERT_EQUAL( packet->GetChunkSSRC(0), packet2.GetChunkSSRC(0) );
        CPPUNIT_ASSERT_EQUAL( (uint32_t) 0, packet2.GetChunkSSRC(1) );
    }

    void testUnpack4() {
        CRtcpSDESPacket packet2;
        CPacketBufferData buffer;
        
        packet->Pack(buffer);

        // mess up the SC count
        uint8_t* data = buffer.GetBuffer();
        data[0] &= 0xE0;
        data[0] |= 0x1F;
        
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail() {
        CPacketBufferData buffer;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        CRtcpSDESPacket packet2;
        CPacketBufferData buffer;

        packet->SetType(CRtcpPacket::RR);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), -1 );
    }

    // just verify the stream functions don't crash anything
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpSDESPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testAddChunk );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpack3 );
    CPPUNIT_TEST( testUnpack4 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpSDESPacketTest );
