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

#include "packet_buffer.h"
#include "tip_debug_tools.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CPacketBufferTest : public CppUnit::TestFixture {
private:
    uint32_t data_size;
    uint8_t* data;
    CPacketBuffer* buf;

public:
    void setUp() {
        data_size = 1024;
        data = (uint8_t*) malloc(data_size);
        buf = new CPacketBuffer(data, data_size);
    }

    void tearDown() {
        delete buf;
        free(data);
    }

    void testDefaults() {
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), data );
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), data_size );
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferOffset(), (uint32_t) 0 );
    }

    void testPointer() {
        // make buffer empty
        buf->Reset();
        
        uint8_t add[32];
        uint8_t rem[32];
        memset(add, 0xA5, sizeof(add));

        buf->Add(add, sizeof(add));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(add) );
        
        buf->Rem(rem, sizeof(rem));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );

        if (memcmp(add, rem, sizeof(add))) {
            ostringstream oss;
            
            oss << "\nExpected:  "
                << HexDump(add, sizeof(add))
                << "\nSet:       "
                << HexDump(rem, sizeof(rem));

            CPPUNIT_FAIL(oss.str());
        }            
    }

    void test16Bit() {
        // make buffer empty
        buf->Reset();

        uint16_t add = 0xA5A5;
        uint16_t rem;
        uint16_t rem2;

        buf->Add(add);
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(add) );

        // remove as uint8_t first so no byte swap
        buf->Rem((uint8_t*) &rem, sizeof(rem));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );

        // move head back so we can read again
        buf->ResetHead();

        // remove a uint16_t so byte swap
        buf->Rem(rem2);

        CPPUNIT_ASSERT_EQUAL( add, ntohs(rem) );
        CPPUNIT_ASSERT_EQUAL( add, rem2 );
    }
    
    void test32Bit() {
        // make buffer empty
        buf->Reset();

        uint32_t add = 0xA5A5;
        uint32_t rem;
        uint32_t rem2;

        buf->Add(add);
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(add) );

        // remove as uint8_t first so no byte swap
        buf->Rem((uint8_t*) &rem, sizeof(rem));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );

        // move head back so we can read again
        buf->ResetHead();

        // remove a uint32_t so byte swap
        buf->Rem(rem2);

        CPPUNIT_ASSERT_EQUAL( add, ntohl(rem) );
        CPPUNIT_ASSERT_EQUAL( add, rem2 );
    }
    
    void test64Bit() {
        // make buffer empty
        buf->Reset();

        uint64_t add = 0xA5A5;
        uint32_t rem[2];
        uint64_t rem2;

        buf->Add(add);
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(add) );

        // remove as uint8_t first so no byte swap
        buf->Rem((uint8_t*) &rem, sizeof(rem));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );

        // move head back so we can read again
        buf->ResetHead();

        // remove a uint64_t so byte swap
        buf->Rem(rem2);

        uint64_t tmp = ((uint64_t) ntohl(rem[0])) << 32;
        tmp |= ntohl(rem[1]);
        CPPUNIT_ASSERT_EQUAL( add, tmp );
        CPPUNIT_ASSERT_EQUAL( add, rem2 );
    }

    struct TestStruct {
        uint32_t one;
        uint32_t two;
    };

    void testAddTemplate() {
        TestStruct s;
        
        // make buffer empty
        buf->Reset();
        
        buf->Add(s);
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(s) );
    }
        
    void testAddInvalid() {
        // make buffer empty
        buf->Reset();

        uint8_t add[2048];
        buf->Add(add, sizeof(add));
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), data_size );
    }

    void testRemInvalid() {
        uint8_t rem[2048];
        buf->Rem(rem, sizeof(rem));

        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
    }
    
    void testReset() {
        // make buffer empty
        buf->Reset();

        uint8_t add[32];
        buf->Add(add, sizeof(add));

        uint8_t rem[16];
        buf->Rem(rem, sizeof(rem));

        buf->Reset();
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), data );
    }

    void testResetHead() {
        // make buffer empty
        buf->Reset();

        uint8_t add[32];
        buf->Add(add, sizeof(add));

        uint8_t rem[16];
        buf->Rem(rem, sizeof(rem));

        buf->ResetHead();

        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) sizeof(add) );
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), data );

        // move head past tail (invalid)
        buf->ResetHead(data_size);
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), (data + sizeof(add)) );
    }

    void testResetTail() {
        // make buffer empty
        buf->ResetTail();

        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), data );

        uint8_t add[32];
        buf->Add(add, sizeof(add));

        uint8_t rem[16];
        buf->Rem(rem, sizeof(rem));

        // try to move tail to be before head (invalid)
        buf->ResetTail();
        
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( buf->GetBuffer(), (data + sizeof(rem)) );
    }

    void testRemAll() {
        // make buffer empty
        buf->Reset();

        uint8_t add[32];
        buf->Add(add, sizeof(add));

        buf->RemAll();
        CPPUNIT_ASSERT_EQUAL( buf->GetBufferSize(), (uint32_t) 0 );
    }

    void testBufferData() {
        CPacketBufferData data;
        CPPUNIT_ASSERT_EQUAL( data.GetBufferSize(), (uint32_t) 0 );
    }
    
    CPPUNIT_TEST_SUITE( CPacketBufferTest );
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testPointer );
    CPPUNIT_TEST( test16Bit );
    CPPUNIT_TEST( test32Bit );
    CPPUNIT_TEST( test64Bit );
    CPPUNIT_TEST( testAddTemplate );
    CPPUNIT_TEST( testAddInvalid );
    CPPUNIT_TEST( testRemInvalid );
    CPPUNIT_TEST( testReset );
    CPPUNIT_TEST( testResetHead );
    CPPUNIT_TEST( testResetTail );
    CPPUNIT_TEST( testRemAll );
    CPPUNIT_TEST( testBufferData );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CPacketBufferTest );
