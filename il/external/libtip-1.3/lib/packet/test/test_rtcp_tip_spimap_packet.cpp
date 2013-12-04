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

#include "rtcp_tip_spimap_packet.h"
#include "test_packet_data.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpAppSpiMapPacketTest : public CppUnit::TestFixture {
private:
    CRtcpAppSpiMapPacket* packet;

public:
    void setUp() {
        packet = new CRtcpAppSpiMapPacket();
    }

    void tearDown() {
        delete packet;
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( packet->GetSPI(), (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetSrtpProtectionProfile(), (uint8_t) CRtcpAppSpiMapPacket::SPIMAP_SRTP_PROFILE_AES128_CM_SHA1_80 );
        CPPUNIT_ASSERT_EQUAL( packet->GetEktProtectionProfile(), (uint8_t) CRtcpAppSpiMapPacket::SPIMAP_EKT_PROFILE_AES128_ECB );
    }

    void testSPI() {
        packet->SetSPI(0x1234);
        CPPUNIT_ASSERT_EQUAL( packet->GetSPI(), (uint16_t) 0x1234 );
    }
                 
    void testSrtpSalt() {
        uint8_t salt[CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH] =
            { 0x12, 0x34, 0x56, 0x78, 0x90 };
        
        packet->SetSrtpSalt(salt);
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetSrtpSalt(), salt, CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH), 0 );

        packet->SetSrtpSalt(NULL);
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetSrtpSalt(), salt, CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH), 0 );
    }

    void testKek() {
        uint8_t kek[CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH] =
            { 0x12, 0x34, 0x56, 0x78, 0x90 };
        
        packet->SetKek(kek);
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetKek(), kek, CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH), 0 );

        packet->SetKek(NULL);
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetKek(), kek, CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH), 0 );
    }

#define RTCP_APP_SPIMAP_BYTES(spi, srtp, ekt)       \
    ((spi >> 8) & 0xFF), (spi & 0xFF), srtp, ekt

#define RTCP_APP_SPIMAP_SALT                                            \
    0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x09, 0x87, 0x65
    
#define RTCP_APP_SPIMAP_KEK                                             \
    0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x09, 0x87, 0x65, 0x43, 0x21
    
    void testPack() {
        CPacketBufferData buffer;

        uint8_t salt[] = { RTCP_APP_SPIMAP_SALT };
        uint8_t kek[] = { RTCP_APP_SPIMAP_KEK };
        
        uint8_t def[] = { RTCP_PACKET_BYTES(SPIMAP,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_SPIMAP_BYTES(0x1234, 1, 1),
                          RTCP_APP_SPIMAP_SALT,
                          RTCP_APP_SPIMAP_KEK,
                          0x00, 0x00 // padding
        };

        packet->SetSPI(0x1234);
        packet->SetSrtpSalt(salt);
        packet->SetKek(kek);

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
    
    void testUnpack1() {
        CRtcpAppSpiMapPacket packet2;
        CPacketBufferData buffer;
        CPacketBufferData buffer2;

        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );
        packet2.Pack(buffer2);
        if (memcmp(buffer.GetBuffer(), buffer2.GetBuffer(), buffer.GetBufferSize()) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  " << HexDump(buffer.GetBuffer(), buffer.GetBufferSize())
                << "\nPacked:    " << HexDump(buffer2.GetBuffer(), buffer.GetBufferSize());

            CPPUNIT_FAIL(oss.str());
        }
    }
    
    void testUnpack2() {
        CRtcpAppSpiMapPacket packet2;
        CPacketBufferData buffer;
        uint8_t salt[] = { RTCP_APP_SPIMAP_SALT };
        uint8_t kek[] = { RTCP_APP_SPIMAP_KEK };

        packet->SetSPI(0x1234);
        packet->SetSrtpSalt(salt);
        packet->SetKek(kek);
        
        packet->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( packet2.Unpack(buffer), 0 );

        CPPUNIT_ASSERT_EQUAL( packet->GetSPI(), packet2.GetSPI() );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetSrtpSalt(), packet2.GetSrtpSalt(), CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH), 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(packet->GetKek(), packet2.GetKek(), CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH), 0 );
    }

    void testUnpackFail() {
        CRtcpAppSpiMapPacket packet2;
        CPacketBufferData buffer;

        packet2.SetType(CRtcpPacket::RR);
        packet2.Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail2() {
        uint8_t def[] = { RTCP_PACKET_BYTES(SPIMAP,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_SPIMAP_BYTES(0x1234, 0, 1),
                          RTCP_APP_SPIMAP_SALT,
                          RTCP_APP_SPIMAP_KEK,
                          0x00, 0x00 // padding
        };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    void testUnpackFail3() {
        uint8_t def[] = { RTCP_PACKET_BYTES(SPIMAP,
                                            CRtcpPacket::APP,
                                            packet->GetLength()),
                          RTCP_APP_PACKET_BYTES_TP1,
                          RTCP_TIP_PACKET_BYTES,
                          RTCP_APP_SPIMAP_BYTES(0x1234, 1, 0),
                          RTCP_APP_SPIMAP_SALT,
                          RTCP_APP_SPIMAP_KEK,
                          0x00, 0x00 // padding
        };

        CPacketBuffer buffer(def, sizeof(def));
        CPPUNIT_ASSERT_EQUAL( packet->Unpack(buffer), -1 );
    }

    CPPUNIT_TEST_SUITE( CRtcpAppSpiMapPacketTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testSPI );
    CPPUNIT_TEST( testSrtpSalt );
    CPPUNIT_TEST( testKek );
    CPPUNIT_TEST( testPack );
    CPPUNIT_TEST( testUnpack1 );
    CPPUNIT_TEST( testUnpack2 );
    CPPUNIT_TEST( testUnpackFail );
    CPPUNIT_TEST( testUnpackFail2 );
    CPPUNIT_TEST( testUnpackFail3 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpAppSpiMapPacketTest );
