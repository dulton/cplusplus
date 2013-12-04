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

#include "rtcp_tip_tlv.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CRtcpTipTlvTest : public CppUnit::TestFixture {
private:
    CTipTlv* tlv;

public:
    void setUp() {
        tlv = new CTipTlv();
    }

    void tearDown() {
        delete tlv;
    }
    
    void testTag() {
        CPPUNIT_ASSERT_EQUAL( tlv->GetTag(), (uint8_t) 0 );

        tlv->SetTag(4);
        CPPUNIT_ASSERT_EQUAL( tlv->GetTag(), (uint8_t) 4 );
    }

    void testLength() {
        CPPUNIT_ASSERT_EQUAL( tlv->GetLength(), (uint8_t) 0 );
    }

    void testData() {
        CPPUNIT_ASSERT( tlv->GetData() != NULL );

        uint8_t data[CTipTlv::MAX_TLV_DATA_LEN];
        memset(data, 'a', CTipTlv::MAX_TLV_DATA_LEN);

        tlv->SetData(data, CTipTlv::MAX_TLV_DATA_LEN);
        CPPUNIT_ASSERT_EQUAL( tlv->GetLength(), (uint8_t) CTipTlv::MAX_TLV_DATA_LEN );

        if (memcmp(data, tlv->GetData(), CTipTlv::MAX_TLV_DATA_LEN) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  "
                << HexDump(data, CTipTlv::MAX_TLV_DATA_LEN)
                << "\nSet:       "
                << HexDump(tlv->GetData(), CTipTlv::MAX_TLV_DATA_LEN);
            
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testDataBounds() {
        tlv->SetData(NULL, 1);
        CPPUNIT_ASSERT_EQUAL( tlv->GetLength(), (uint8_t) 0 );

        uint8_t data[CTipTlv::MAX_TLV_DATA_LEN];
        memset(data, 'a', CTipTlv::MAX_TLV_DATA_LEN);

        tlv->SetData(data, (CTipTlv::MAX_TLV_DATA_LEN + 1));
        CPPUNIT_ASSERT_EQUAL( tlv->GetLength(), (uint8_t) CTipTlv::MAX_TLV_DATA_LEN );
    }

    void testDataPackSize() {
        CPPUNIT_ASSERT_EQUAL( tlv->GetPackSize(), (uint32_t) 2 );

        tlv->SetData((const uint8_t*) "aaaa", 4);
        CPPUNIT_ASSERT_EQUAL( tlv->GetPackSize(), (uint32_t) 6 );
    }

    void testDataPack() {
        uint8_t data[CTipTlv::MAX_TLV_DATA_LEN];
        memset(data, 'a', CTipTlv::MAX_TLV_DATA_LEN);

        tlv->SetTag(4);
        tlv->SetData(data, CTipTlv::MAX_TLV_DATA_LEN);
    
        CPacketBufferData buffer;
        CPPUNIT_ASSERT_EQUAL( tlv->Pack(buffer), (uint32_t) CTipTlv::MAX_TLV_PACK_LEN );

        const uint8_t* pack_data = buffer.GetBuffer();
        CPPUNIT_ASSERT_EQUAL( pack_data[0], (uint8_t) 4 );
        CPPUNIT_ASSERT_EQUAL( pack_data[1], (uint8_t) CTipTlv::MAX_TLV_DATA_LEN );
    
        if (memcmp(&(pack_data[2]), data, CTipTlv::MAX_TLV_DATA_LEN) != 0) {
            ostringstream oss;
            
            oss << "\nExpected:  "
                << HexDump(data, CTipTlv::MAX_TLV_DATA_LEN)
                << "\nSet:       "
                << HexDump(&(pack_data[2]), CTipTlv::MAX_TLV_DATA_LEN);
            
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testDataUnpack() {
        uint8_t data[CTipTlv::MAX_TLV_DATA_LEN];
        memset(data, 'a', CTipTlv::MAX_TLV_DATA_LEN);

        tlv->SetTag(4);
        tlv->SetData(data, CTipTlv::MAX_TLV_DATA_LEN);
    
        CPacketBufferData buffer;
        tlv->Pack(buffer);

        CTipTlv tlv2;
        tlv2.Unpack(buffer);

        CPPUNIT_ASSERT_EQUAL( tlv->GetTag(), tlv2.GetTag() );
        CPPUNIT_ASSERT_EQUAL( tlv->GetLength(), tlv2.GetLength() );
    
        if (memcmp(tlv->GetData(), tlv2.GetData(), tlv->GetLength())) {
            ostringstream oss;
            
            oss << "\nExpected:  "
                << HexDump(tlv->GetData(), CTipTlv::MAX_TLV_DATA_LEN)
                << "\nSet:       "
                << HexDump(tlv2.GetData(), CTipTlv::MAX_TLV_DATA_LEN);
            
            CPPUNIT_FAIL(oss.str());
        }
    }

    void testDataUnpackFail1() {
        CPacketBufferData buffer;
        CPPUNIT_ASSERT_EQUAL( tlv->Unpack(buffer), -1 );
    }
    
    void testDataUnpackFail2() {
        CPacketBufferData buffer;

        uint8_t data = 2;
        buffer.Add(data); // add tlv tag
        buffer.Add(data); // add invalid tlv length
        
        CPPUNIT_ASSERT_EQUAL( tlv->Unpack(buffer), -1 );
    }
    
    CPPUNIT_TEST_SUITE( CRtcpTipTlvTest );
    CPPUNIT_TEST( testTag );
    CPPUNIT_TEST( testLength );
    CPPUNIT_TEST( testData );
    CPPUNIT_TEST( testDataBounds );
    CPPUNIT_TEST( testDataPackSize );
    CPPUNIT_TEST( testDataPack );
    CPPUNIT_TEST( testDataUnpack );
    CPPUNIT_TEST( testDataUnpackFail1 );
    CPPUNIT_TEST( testDataUnpackFail2 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CRtcpTipTlvTest );
