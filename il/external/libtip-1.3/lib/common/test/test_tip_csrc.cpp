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

#include "tip_csrc.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipCSRCTest : public CppUnit::TestFixture {
private:
    CTipCSRC* csrc;

public:
    void setUp() {
        csrc = new CTipCSRC();
        CPPUNIT_ASSERT( csrc != NULL );
    }

    void tearDown() {
        delete csrc;
    }

    void testInit() {
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0 );
    }

    void testInit2() {
        delete csrc;
        csrc = new CTipCSRC(0x12345678);
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x12345678 );
    }
    
    void testSetCSRC() {
        csrc->SetCSRC(0x12345678);
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x12345678 );
    }

    void testSetClockID() {
        csrc->SetClockID(0x12345);
        CPPUNIT_ASSERT_EQUAL( csrc->GetClockID(), (uint32_t) 0x12345 );
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x12345000 );
    }

    void testSetOutputPos() {
        csrc->SetOutputPos(0xA);
        CPPUNIT_ASSERT_EQUAL( csrc->GetOutputPos(), (uint8_t) 0xA );
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x00000A00 );
    }

    void testSetSourcePos() {
        csrc->SetSourcePos(0xA);
        CPPUNIT_ASSERT_EQUAL( csrc->GetSourcePos(), (uint8_t) 0xA );
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x000000A0 );
    }

    void testSetSinkPos() {
        csrc->SetSinkPos(0xA);
        CPPUNIT_ASSERT_EQUAL( csrc->GetSinkPos(), (uint8_t) 0xA );
        CPPUNIT_ASSERT_EQUAL( csrc->GetCSRC(), (uint32_t) 0x0000000A );
    }

    CPPUNIT_TEST_SUITE( CTipCSRCTest );
    CPPUNIT_TEST( testInit );
    CPPUNIT_TEST( testInit2 );
    CPPUNIT_TEST( testSetCSRC );
    CPPUNIT_TEST( testSetClockID );
    CPPUNIT_TEST( testSetOutputPos );
    CPPUNIT_TEST( testSetSourcePos );
    CPPUNIT_TEST( testSetSinkPos );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipCSRCTest );
