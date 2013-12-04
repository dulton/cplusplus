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
#include "rtcp_rr_packet.h"
#include "test_packet_data.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpRRPacketTest : public CppUnit::TestFixture {
private:
    CRtcpRRPacket* packet;

public:
    void setUp() {
        packet = new CRtcpRRPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), (uint16_t) 1 );
    }

    void testPack() {
        CPacketBufferData buffer;
        uint8_t def[] = { RTCP_PACKET_BYTES(0, CRtcpPacket::RR, packet->GetLength()) };
    
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet->Pack(buffer) );
        if (memcmp(buffer.GetBuffer(), def, packet->GetPackSize()) != 0) {
            ostringstream oss;

            oss << "\nExpected:  " << HexDump(def, packet->GetPackSize())
                << "\nPacked:    " << HexDump(buffer.GetBuffer(), packet->GetPackSize());

            CPPUNIT_FAIL(oss.str());
        }
    }

    void testUnpack() {
        CRtcpRRPacket packet2;
        CPacketBufferData buffer;
        
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail() {
        CPacketBufferData buffer;

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        CRtcpRRPacket packet2;
        CPacketBufferData buffer;

        packet->SetType(CRtcpPacket::SDES);
        packet->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), -1 );
    }

    // just verify the stream functions don't crash anything
    void testToStream() {
        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpRRPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpRRPacketTest );
