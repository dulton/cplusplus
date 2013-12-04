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

#include "tip_media_option.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CMediaOptionTest : public CppUnit::TestFixture {
private:
    CTipMediaOption* mo;

public:
    void setUp() {
        mo = new CTipMediaOption(VIDEO, 0, CTipMediaOption::OPTION_NOT_SUPPORTED);
        CPPUNIT_ASSERT( mo != NULL );
    }
    
    void tearDown() {
        delete mo;
    }
        
    void testMOInit() {
        CPPUNIT_ASSERT_EQUAL( mo->GetMediaType(), VIDEO );
        CPPUNIT_ASSERT_EQUAL( mo->GetOption(), (uint16_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mo->GetState(), CTipMediaOption::OPTION_NOT_SUPPORTED );
    }

    void testMOType() {
        mo->SetMediaType(AUDIO);
        CPPUNIT_ASSERT_EQUAL( mo->GetMediaType(), AUDIO );

        mo->SetMediaType(VIDEO);
        CPPUNIT_ASSERT_EQUAL( mo->GetMediaType(), VIDEO );

        mo->SetMediaType(MT_MAX);
        CPPUNIT_ASSERT_EQUAL( mo->GetMediaType(), MT_MAX );
    }

    void testMOOpt() {
        mo->SetOption(1);
        CPPUNIT_ASSERT_EQUAL( mo->GetOption(), (uint16_t) 1 );

        mo->SetOption(0xFFFF);
        CPPUNIT_ASSERT_EQUAL( mo->GetOption(), (uint16_t) 0xFFFF );
    }

    void testMOState() {
        mo->SetState(CTipMediaOption::OPTION_SUPPORTED_BOTH);
        CPPUNIT_ASSERT_EQUAL( mo->GetState(), CTipMediaOption::OPTION_SUPPORTED_BOTH);

        mo->SetState(CTipMediaOption::OPTION_NOT_SUPPORTED);
        CPPUNIT_ASSERT_EQUAL( mo->GetState(), CTipMediaOption::OPTION_NOT_SUPPORTED);

        mo->SetState(CTipMediaOption::OPTION_SUPPORTED_TX);
        CPPUNIT_ASSERT_EQUAL( mo->GetState(), CTipMediaOption::OPTION_SUPPORTED_TX);
    
        mo->SetState(CTipMediaOption::OPTION_SUPPORTED_RX);
        CPPUNIT_ASSERT_EQUAL( mo->GetState(), CTipMediaOption::OPTION_SUPPORTED_RX);
    }

    void testVMOInit() {
        CTipVideoMediaOption vmo(CTipVideoMediaOption::LTRP,
                                 CTipMediaOption::OPTION_NOT_SUPPORTED);

        CPPUNIT_ASSERT_EQUAL( vmo.GetMediaType(), VIDEO );
        CPPUNIT_ASSERT_EQUAL( vmo.GetOption(), (uint16_t) CTipVideoMediaOption::LTRP );
        CPPUNIT_ASSERT_EQUAL( vmo.GetState(), CTipMediaOption::OPTION_NOT_SUPPORTED );
    }
    
    void testAMOInit() {
        CTipAudioMediaOption amo(CTipAudioMediaOption::DYNAMIC_OUTPUT,
                                 CTipMediaOption::OPTION_SUPPORTED_TX);

        CPPUNIT_ASSERT_EQUAL( amo.GetMediaType(), AUDIO );
        CPPUNIT_ASSERT_EQUAL( amo.GetOption(), (uint16_t) CTipAudioMediaOption::DYNAMIC_OUTPUT );
        CPPUNIT_ASSERT_EQUAL( amo.GetState(), CTipMediaOption::OPTION_SUPPORTED_TX );
    }

    void testIncrement() {
        CTipAudioMediaOption::AudioOption aopt = CTipAudioMediaOption::ACTIVITY_METRIC;
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::DYNAMIC_OUTPUT );
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::CAPABLE_G722_LEGACY );
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::USING_G722_LEGACY );
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::EKT );
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::MAX_AUDIO_OPTION );
        CPPUNIT_ASSERT_EQUAL( ++aopt, CTipAudioMediaOption::ACTIVITY_METRIC );

        CTipVideoMediaOption::VideoOption vopt = CTipVideoMediaOption::REFRESH_FLAG;
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::INBAND_PARAM_SETS );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::CABAC );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::LTRP );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::GDR );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::HP_8X8_TRANSFORMS );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::UNRESTRICTED_XGA_1_5 );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::UNRESTRICTED_720P );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::UNRESTRICTED_1080P );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::UNRESTRICED_XGA_30 );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::EKT );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::MAX_VIDEO_OPTION );
        CPPUNIT_ASSERT_EQUAL( ++vopt, CTipVideoMediaOption::REFRESH_FLAG );
    }
    
    CPPUNIT_TEST_SUITE( CMediaOptionTest );
    CPPUNIT_TEST( testMOInit );
    CPPUNIT_TEST( testMOType );
    CPPUNIT_TEST( testMOOpt );
    CPPUNIT_TEST( testMOState );
    CPPUNIT_TEST( testVMOInit );
    CPPUNIT_TEST( testAMOInit );
    CPPUNIT_TEST( testIncrement );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CMediaOptionTest );
