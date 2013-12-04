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

#include "rtcp_tip_mediaopts_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppMediaoptsPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppMediaoptsPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppMediaoptsPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefault() {
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint16_t) CRtcpAppMediaoptsPacket::MAXIMUM_VERSION );
    }

    void testVersion() {
        CRtcpAppMediaoptsPacket packet2(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        CPPUNIT_ASSERT_EQUAL( packet2.GetVersion(), (uint16_t) CRtcpAppMediaoptsPacket::MINIMUM_VERSION );
    }
    
    void testAddSsrc() {
        uint32_t xmit;
        uint32_t rcv;
        list<uint32_t> ssrc;
    
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, xmit, rcv), -1 );

        ssrc = packet->GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 0 );

        packet->AddSSRC(0, 1, 1);
        packet->AddSSRC(1, 2, 2);
    
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 1 );
    
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(1, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 2 );
    
        ssrc = packet->GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 2 );
        CPPUNIT_ASSERT_EQUAL( *(ssrc.begin()), (unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( *(++(ssrc.begin())), (unsigned int) 1 );

        packet->AddSSRC(0, 3, 3);
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 3 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 3 );
    }

    void testAddSSRCV2() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);

        uint32_t xmit;
        uint32_t rcv;
        list<uint32_t> ssrc;
        
        packet->AddSSRC(0, 1, 1);
        ssrc = packet->GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 1 );
        
        packet->AddSSRC(1, 2, 2);
        ssrc = packet->GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(1, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 2 );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 2 );
    }
    
    void testAddOpt() {
        list<CRtcpAppMediaoptsPacket::OptionTag> olist;
        uint32_t option;
    
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 0), -1 );

        olist = packet->GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( olist.size(), (size_t) 0 );
    
        packet->AddSSRC(0, 1, 1);
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 1), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, option), 0);
        CPPUNIT_ASSERT_EQUAL( option, (uint32_t) 1 );

        olist = packet->GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( olist.size(), (size_t) 1 );
    
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 2), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, option), 0);
        CPPUNIT_ASSERT_EQUAL( option, (uint32_t) 2 );
    
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::PROFILE, 3), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::PROFILE, option), 0);
        CPPUNIT_ASSERT_EQUAL( option, (uint32_t) 3 );
    
        olist = packet->GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( olist.size(), (size_t) 2 );
        CPPUNIT_ASSERT_EQUAL( *(olist.begin()), CRtcpAppMediaoptsPacket::RESERVED );
        CPPUNIT_ASSERT_EQUAL( *(++(olist.begin())), CRtcpAppMediaoptsPacket::PROFILE );
    }    

    void testAddOptV2() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);

        list<CRtcpAppMediaoptsPacket::OptionTag> olist;
        uint32_t option;

        packet->AddSSRC(0, 1, 1);
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 1), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, option), 0);
        CPPUNIT_ASSERT_EQUAL( option, (uint32_t) 1 );

        olist = packet->GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( olist.size(), (size_t) 1 );

        CPPUNIT_ASSERT_EQUAL( packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 2), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(1, CRtcpAppMediaoptsPacket::RESERVED, option), 0);
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, option), 0);
        CPPUNIT_ASSERT_EQUAL( option, (uint32_t) 2 );
    }
    
    void testAddOptInvalid() {
        CPPUNIT_ASSERT_EQUAL( packet->AddOption(0, CRtcpAppMediaoptsPacket::LASTTAG, 0), -1 );
    }

    void testGetOptInvalid1() {
        uint32_t value;
        
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, value), -1 );
    }

    void testGetOptInvalid2() {
        uint32_t value;

        packet->AddSSRC(0, 1, 1);
        CPPUNIT_ASSERT_EQUAL( packet->GetOption(0, CRtcpAppMediaoptsPacket::RESERVED, value), -1 );
    }
    
    void testSize() {
        uint32_t baseSize = 24;

        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), baseSize );

        // ssrc adds 16 bytes
        packet->AddSSRC(0, 1, 1);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 16) );

        // duplicate ssrc should add nothing
        packet->AddSSRC(0, 2, 2);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 16) );

        // tag adds 4 bytes
        packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 0);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 16 + 4) );

        // duplicate tag adds nothing
        packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 0);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 16 + 4) );

        // re-adding the same SSRC doesn't change anything (including
        // added options)
        packet->AddSSRC(0, 2, 2);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 16 + 4) );
    }    

    void testSize2() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        
        uint32_t baseSize = 32;
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), baseSize );

        // ssrc doesn't add anything b/c the packet contains one by default
        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), baseSize );

        // tag adds 4 bytes
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 4) );

        // a different ssrc doesn't add anything b/c the packet can
        // only contain one ssrc.  note options are preserved so need
        // to include the 4 bytes for that.
        packet->AddSSRC(0x87654321, 0x90ABCDEF, 0x13579ACE);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 4) );

        // duplicate tag adds nothing, even with a different ssrc
        packet->AddOption(0x87654321, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), (baseSize + 4) );
    }
    
#define RTCP_APP_MEDIAOPTS_BYTES(ver, pad)       0x00, (ver), ((pad) >> 8), ((pad) & 0xFF)
#define RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT_DEF 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
#define RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT 0x90, 0xAB, 0xCD, 0xEF, 0x13, 0x57, 0x9A, 0xCE
#define RTCP_APP_MEDIAOPTS_BYTES_SSRC  0x12, 0x34, 0x56, 0x78, RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT
#define RTCP_APP_MEDIAOPTS_BYTES_SSRC2 0x00, 0x00, 0x00, 0x00, 0x90, 0xAB, 0xCD, 0xEF, 0x13, 0x57, 0x9A, 0xCE
#define RTCP_APP_MEDIAOPTS_BYTES_LAST  0xFF, 0x00, 0x00, 0x00
#define RTCP_APP_MEDIAOPTS_BYTES_OPT   0x00, 0x12, 0x34, 0x56
#define RTCP_APP_MEDIAOPTS_BYTES_OPT2  0x01, 0x12, 0x34, 0x56

    void testPack() {
        CPacketBufferData buffer;

        uint8_t def[] = { RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MAXIMUM_VERSION, 0) };
    

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

        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);

        uint8_t def[] = { RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MAXIMUM_VERSION, 0),
                          RTCP_APP_MEDIAOPTS_BYTES_SSRC, RTCP_APP_MEDIAOPTS_BYTES_LAST };
    
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

    void testPack3() {
        CPacketBufferData buffer;

        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        uint8_t def[] = { RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MAXIMUM_VERSION, 0),
                          RTCP_APP_MEDIAOPTS_BYTES_SSRC, RTCP_APP_MEDIAOPTS_BYTES_OPT,
                          RTCP_APP_MEDIAOPTS_BYTES_LAST };
    
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

    void testPack4() {
        CPacketBufferData buffer;

        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::PROFILE, 0x123456);

        packet->AddSSRC(0x00000000, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x00000000, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        uint8_t def[] = { RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MAXIMUM_VERSION, 0),
                          RTCP_APP_MEDIAOPTS_BYTES_SSRC2, RTCP_APP_MEDIAOPTS_BYTES_OPT,
                          RTCP_APP_MEDIAOPTS_BYTES_LAST,
                          RTCP_APP_MEDIAOPTS_BYTES_SSRC, RTCP_APP_MEDIAOPTS_BYTES_OPT,
                          RTCP_APP_MEDIAOPTS_BYTES_OPT2, RTCP_APP_MEDIAOPTS_BYTES_LAST };
    
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

    void testPack5() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);

        CPacketBufferData buffer;

        uint8_t def[] = {
            RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
            RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
            RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MINIMUM_VERSION, 0xFFFF),
            RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT_DEF
        };
    

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

    void testPack6() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);

        CPacketBufferData buffer;

        uint8_t def[] = {
            RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
            RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
            RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MINIMUM_VERSION, 0xFFFF),
            RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT
        };
    

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
    
    void testPack7() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);

        packet->AddSSRC(0x12345678, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        packet->AddOption(0x12345678, CRtcpAppMediaoptsPacket::PROFILE, 0x123456);

        // just to verify we are really only adding one SSRC section in V2
        packet->AddSSRC(0x87654321, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x87654321, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        
        
        CPacketBufferData buffer;

        uint8_t def[] = {
            RTCP_PACKET_BYTES(MEDIAOPTS, CRtcpPacket::APP, packet->GetLength()),
            RTCP_APP_PACKET_BYTES_TP1, RTCP_TIP_PACKET_BYTES,
            RTCP_APP_MEDIAOPTS_BYTES(CRtcpAppMediaoptsPacket::MINIMUM_VERSION, 0xFFFF),
            RTCP_APP_MEDIAOPTS_BYTES_TX_RX_OPT, RTCP_APP_MEDIAOPTS_BYTES_OPT,
            RTCP_APP_MEDIAOPTS_BYTES_OPT2
        };
    

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
        CRtcpAppMediaoptsPacket packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        packet->AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        packet->AddSSRC(0x00000000, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(0x00000000, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
    
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        packet->Pack(buffer);
        packet2.Pack(buffer2);
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), buffer2.GetBufferSize() );
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    " << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());

            CPPUNIT_FAIL(oss.str());
        }
    
        list<uint32_t> ssrc = packet2.GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 2 );
        CPPUNIT_ASSERT_EQUAL( *(ssrc.begin()), (unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( *(++(ssrc.begin())), (unsigned int) 1 );

        uint32_t xmit;
        uint32_t rcv;
        CPPUNIT_ASSERT_EQUAL( packet2.GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 0x90ABCDEF );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 0x13579ACE );

        CPPUNIT_ASSERT_EQUAL( packet2.GetSSRC(1, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 0x90ABCDEF );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 0x13579ACE );

        uint32_t value;
        list<CRtcpAppMediaoptsPacket::OptionTag> opt = packet2.GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( opt.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet2.GetOption(0, *(opt.begin()), value), 0 );
        CPPUNIT_ASSERT_EQUAL( value, (uint32_t) 0x123456 );

        opt = packet2.GetOptions(1);
        CPPUNIT_ASSERT_EQUAL( opt.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet2.GetOption(0, *(opt.begin()), value), 0 );
        CPPUNIT_ASSERT_EQUAL( value, (uint32_t) 0x123456 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpack2() {
        // this test the case where an option tag segment is not
        // terminated by a LAST_TAG tag, which is not required for the
        // last option tag segment in a packet.  we need to do some
        // manual wrangling of the packet for this to happen as our
        // code always adds LAST_TAG for every segment.

        CRtcpAppMediaoptsPacket packet2;
        packet->AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        
        CPacketBufferData buffer;
        packet->Pack(buffer);

        // change the last 4 bytes to all 0
        buffer.ResetTail(buffer.GetBufferSize() - 4);
        uint32_t data = 0;
        buffer.Add(data);

        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
    }
        
    void testUnpack3() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        
        CRtcpAppMediaoptsPacket packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        packet->AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);

        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        packet->Pack(buffer);
        packet2.Pack(buffer2);
        CPPUNIT_ASSERT_EQUAL( buffer.GetBufferSize(), buffer2.GetBufferSize() );
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    " << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());

            CPPUNIT_FAIL(oss.str());
        }
    
        list<uint32_t> ssrc = packet2.GetAllSSRC();
        CPPUNIT_ASSERT_EQUAL( ssrc.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( *(ssrc.begin()), (unsigned int) 0 );

        uint32_t xmit;
        uint32_t rcv;
        CPPUNIT_ASSERT_EQUAL( packet2.GetSSRC(0, xmit, rcv), 0 );
        CPPUNIT_ASSERT_EQUAL( xmit, (uint32_t) 0x90ABCDEF );
        CPPUNIT_ASSERT_EQUAL( rcv, (uint32_t) 0x13579ACE );

        uint32_t value;
        list<CRtcpAppMediaoptsPacket::OptionTag> opt = packet2.GetOptions(0);
        CPPUNIT_ASSERT_EQUAL( opt.size(), (size_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet2.GetOption(0, *(opt.begin()), value), 0 );
        CPPUNIT_ASSERT_EQUAL( value, (uint32_t) 0x123456 );

        CPPUNIT_ASSERT_EQUAL( packet->GetLength(), packet2.GetLength() );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }

    void testUnpackFail1() {
        CRtcpAppMediaoptsPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackFail2() {
        CRtcpAppMediaoptsPacket packet2;
        CPacketBufferData buffer;

        packet2.Pack(buffer);

        // add some random data (between 4 and 12 bytes required to make it fail)
        uint32_t data = 0;
        buffer.Add(data);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail3() {
        CRtcpAppMediaoptsPacket packet2((CRtcpAppMediaoptsPacket::Version) (CRtcpAppMediaoptsPacket::MINIMUM_VERSION - 1));
        CPacketBufferData buffer;

        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail4() {
        CRtcpAppMediaoptsPacket packet2((CRtcpAppMediaoptsPacket::Version) (CRtcpAppMediaoptsPacket::MAXIMUM_VERSION + 1));
        CPacketBufferData buffer;

        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail5() {
        CRtcpAppMediaoptsPacket packet2(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        CPacketBufferData buffer;
        
        packet2.Pack(buffer);
        buffer.ResetTail(buffer.GetBufferSize() - sizeof(uint32_t));

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }
    
    void testUnpackFail6() {
        CRtcpAppMediaoptsPacket packet2(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        CPacketBufferData buffer;

        packet2.Pack(buffer);
        buffer.ResetTail(buffer.GetBufferSize() - sizeof(uint32_t));

        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackClear() {
        CRtcpAppMediaoptsPacket packet2;
        CPacketBufferData buffer;

        packet->AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        
        packet2.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }
    
    void testUnpackClear2() {
        delete packet;
        packet = new CRtcpAppMediaoptsPacket(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);

        CRtcpAppMediaoptsPacket packet2(CRtcpAppMediaoptsPacket::MINIMUM_VERSION);
        CPacketBufferData buffer;

        packet->AddSSRC(1, 0x90ABCDEF, 0x13579ACE);
        packet->AddOption(1, CRtcpAppMediaoptsPacket::RESERVED, 0x123456);
        
        packet2.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetPackSize(), packet2.GetPackSize() );
    }
    
    void testAuxBitVal() {
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_1FPS), (uint32_t) CRtcpAppMediaoptsPacket::AUX_BIT1 );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_5FPS), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_30FPS), (uint32_t) CRtcpAppMediaoptsPacket::AUX_BIT0 );

        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(CRtcpAppMediaoptsPacket::AUX_BIT1), CRtcpAppMediaoptsPacket::AUX_1FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(0), CRtcpAppMediaoptsPacket::AUX_5FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(CRtcpAppMediaoptsPacket::AUX_BIT0), CRtcpAppMediaoptsPacket::AUX_30FPS );
    }
    
    void testToStream() {
        packet->AddSSRC(0, 0xFFFFFFFF, 0xFFFFFFFF);
        packet->AddOption(0, CRtcpAppMediaoptsPacket::RESERVED, 1234);
        packet->AddOption(0, CRtcpAppMediaoptsPacket::PROFILE, 1234);
        packet->AddOption(0, CRtcpAppMediaoptsPacket::LEGACYMIX, 1234);
        packet->AddOption(0, CRtcpAppMediaoptsPacket::RXRESOLUTION, 1234);

        ostringstream oss;
        packet->ToStream(oss);
    }

    CPPUNIT_TEST_SUITE( CRtcpAppMediaoptsPacketTest );
    CPPUNIT_TEST( testDefault );
    CPPUNIT_TEST( testVersion );
    CPPUNIT_TEST( testAddSsrc );
    CPPUNIT_TEST( testAddSSRCV2 );
    CPPUNIT_TEST( testAddOpt );
    CPPUNIT_TEST( testAddOptV2 );
    CPPUNIT_TEST( testAddOptInvalid );
    CPPUNIT_TEST( testGetOptInvalid1 );
    CPPUNIT_TEST( testGetOptInvalid2 );
    CPPUNIT_TEST( testSize );
    CPPUNIT_TEST( testSize2 );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testPack2 );
    CPPUNIT_TEST( testPack3 );
    CPPUNIT_TEST( testPack4 );
    CPPUNIT_TEST( testPack5 );
    CPPUNIT_TEST( testPack6 );
    CPPUNIT_TEST( testPack7 );
    CPPUNIT_TEST( testUnpack );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpack3 );
    CPPUNIT_TEST( testUnpackFail1 );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testUnpackFail3 );
    CPPUNIT_TEST( testUnpackFail4 );
    CPPUNIT_TEST( testUnpackFail5 );
    CPPUNIT_TEST( testUnpackFail6 );
    CPPUNIT_TEST( testUnpackClear );
    CPPUNIT_TEST( testUnpackClear2 );
    CPPUNIT_TEST( testAuxBitVal );
    CPPUNIT_TEST( testToStream );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppMediaoptsPacketTest );
