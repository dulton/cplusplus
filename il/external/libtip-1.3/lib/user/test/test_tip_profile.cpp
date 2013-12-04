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

#include "tip_profile.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipProfileTest : public CppUnit::TestFixture {
private:
    CTipSystem* ams;

public:
    void setUp() {
        ams = new CTipSystem();
        CPPUNIT_ASSERT( ams != NULL );
    }

    void tearDown() {
        delete ams;
    }

    void doMediaOptions() {

        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::REFRESH_FLAG),
                              CTipMediaOption::OPTION_SUPPORTED_TX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::INBAND_PARAM_SETS),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::CABAC),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::LTRP),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::GDR),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::HP_8X8_TRANSFORMS),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::UNRESTRICTED_XGA_1_5),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::UNRESTRICTED_720P),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::UNRESTRICTED_1080P),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::UNRESTRICED_XGA_30),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::EKT),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::ACTIVITY_METRIC),
                              CTipMediaOption::OPTION_SUPPORTED_TX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::DYNAMIC_OUTPUT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::CAPABLE_G722_LEGACY),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::USING_G722_LEGACY),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::EKT),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
    }
    
    void testSingleScreen() {
        CSingleScreenProfile::Configure(*ams, false, false);

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(),
                              (long unsigned int) 0x00000202 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(),
                              (long unsigned int) 0x00000002 );

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(),
                              (long unsigned int) 0x00001012 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(),
                              (long unsigned int) 0x0000101E );

        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );

        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetFeedbackState(), true );

        doMediaOptions();
    }

    void testSingleScreen30() {
        CSingleScreenProfile::Configure(*ams, true, false);

        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }

    void testSingleScreenSecure() {
        CSingleScreenProfile::Configure(*ams, false, true);
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), true );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
    }
    
    void testSingleScreenExtended() {
        CSingleScreenExtendedReachProfile::Configure(*ams, false);

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(),
                              (long unsigned int) 0x00000202 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(),
                              (long unsigned int) 0x00000002 );

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(),
                              (long unsigned int) 0x00001012 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(),
                              (long unsigned int) 0x0000101E );

        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
        
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetFeedbackState(), true );

        doMediaOptions();
    }
    
    void testSingleScreenExtendedSecure() {
        CSingleScreenExtendedReachProfile::Configure(*ams, true);
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), true );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
    }
    
    void testTripleScreen() {
        CTripleScreenProfile::Configure(*ams, false, false);

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(),
                              (long unsigned int) 0xE0E );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(),
                              (long unsigned int) 0x0000000E );

        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(),
                              (long unsigned int) 0x0000101E );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(),
                              (long unsigned int) 0x0000101E );

        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
        
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetFeedbackState(), true );

        doMediaOptions();
    }

    void testTripleScreen30() {
        CTripleScreenProfile::Configure(*ams, true, false);

        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }
    
    void testTripleScreenSecure() {
        CTripleScreenProfile::Configure(*ams, false, true);
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), true );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::EKT),
                              CTipMediaOption::OPTION_SUPPORTED_RX );
    }
    
    CPPUNIT_TEST_SUITE( CTipProfileTest );
    CPPUNIT_TEST( testSingleScreen );
    CPPUNIT_TEST( testSingleScreen30 );
    CPPUNIT_TEST( testSingleScreenSecure );
    CPPUNIT_TEST( testSingleScreenExtended );
    CPPUNIT_TEST( testSingleScreenExtendedSecure );
    CPPUNIT_TEST( testTripleScreen );
    CPPUNIT_TEST( testTripleScreen30 );
    CPPUNIT_TEST( testTripleScreenSecure );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipProfileTest );
