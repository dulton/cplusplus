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

#include <string.h>

#include "tip_system.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipSystemTest : public CppUnit::TestFixture {
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
    
    void testAMSInitd() {
        CPPUNIT_ASSERT_EQUAL( ams->GetTipVersion(), SUPPORTED_VERSION_MAX );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetConfID(), (uint64_t) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetParticipantIDLength(), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetFeedbackState(), false );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::REFRESH_FLAG),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::ACTIVITY_METRIC),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
    }

    void testAMSVersion() {
        CPPUNIT_ASSERT_EQUAL( ams->SetTipVersion(SUPPORTED_VERSION_MIN), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTipVersion(), SUPPORTED_VERSION_MIN );

        CPPUNIT_ASSERT_EQUAL( ams->SetTipVersion((ProtocolVersion) 0), TIP_ERROR );
    }

    void testAMSTxInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(MT_MAX, 0), TIP_ERROR );
    }
    
    void testAMSTxVideoInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(VIDEO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSTxVideod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(VIDEO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSTxVideoRemove() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(VIDEO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->RemoveTransmitter(VIDEO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
    }
    
    void testAMSRxInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddReceiver(MT_MAX, 0), TIP_ERROR );
    }
    
    void testAMSRxVideoInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddReceiver(VIDEO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSRxVideod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddReceiver(VIDEO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSTxRxVideoInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddBidirectional(VIDEO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSTxRxVideod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddBidirectional(VIDEO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 2 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSPresFrameRate() {
        CPPUNIT_ASSERT_EQUAL( ams->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testAMSPresFrameRateInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->SetPresentationFrameRate(CTipSystem::PRES_1FPS_AND_30FPS), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testAMSTxAudioInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(AUDIO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSTxAudiod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddTransmitter(AUDIO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSRxAudioInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddReceiver(AUDIO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSRxAudiod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddReceiver(AUDIO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSTxRxAudioInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->AddBidirectional(AUDIO, 0), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(), (long unsigned int) 0 );
    }

    void testAMSTxRxAudiod() {
        CPPUNIT_ASSERT_EQUAL( ams->AddBidirectional(AUDIO, 1), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetTransmitters(AUDIO).to_ulong(), (long unsigned int) 2 );
        CPPUNIT_ASSERT_EQUAL( ams->GetReceivers(AUDIO).to_ulong(), (long unsigned int) 2 );
    }

    void testAMSConfd() {
        uint64_t confID = 0x1234567890ABCDEFLL;
        const char* pID = "test";
        uint32_t pIDLen = strlen(pID);
    
        CPPUNIT_ASSERT_EQUAL( ams->JoinConference(confID, (uint8_t*) pID, pIDLen), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetConfID(), confID );
        CPPUNIT_ASSERT_EQUAL( strncmp((const char*) ams->GetParticipantID(), pID, pIDLen), 0 );
        CPPUNIT_ASSERT_EQUAL( ams->GetParticipantIDLength(), pIDLen );
    }

    void testAMSConfdInvd() {
        CPPUNIT_ASSERT_EQUAL( ams->JoinConference(0, NULL, 1), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( ams->JoinConference(0, (uint8_t*) "", (CTipSystem::MAX_PARTICIPANT_ID_LENGTH+1)), TIP_ERROR );
    }
    
    void testAMSSecurity() {
        CPPUNIT_ASSERT_EQUAL( ams->SetSecurityState(true), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetSecurityState(), true );
    }

    void testAMSMCU() {
        CPPUNIT_ASSERT_EQUAL( ams->SetMCUState(true), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetMCUState(), true );
    }
    
    void testAMSFeedback() {
        CPPUNIT_ASSERT_EQUAL( ams->SetFeedbackState(true), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetFeedbackState(), true );
    }

    void testAMSVMOd() {
        CTipVideoMediaOption mo(CTipVideoMediaOption::REFRESH_FLAG,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
    
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::REFRESH_FLAG),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
    }

    void testAMSVMO2d() {
        CTipVideoMediaOption mo(CTipVideoMediaOption::REFRESH_FLAG,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
        CTipVideoMediaOption mo2(CTipVideoMediaOption::EKT,
                                 CTipMediaOption::OPTION_NOT_SUPPORTED);
    
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo2), TIP_OK );

        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::REFRESH_FLAG),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::EKT),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
    }

    void testAMSVMO3d() {
        CTipVideoMediaOption mo(CTipVideoMediaOption::REFRESH_FLAG,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
        ams->AddMediaOption(mo);

        mo.SetState(CTipMediaOption::OPTION_SUPPORTED_TX);
        ams->AddMediaOption(mo);
        
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipVideoMediaOption::REFRESH_FLAG),
                              CTipMediaOption::OPTION_SUPPORTED_TX );
    }

    void testAMSAMOd() {
        CTipAudioMediaOption mo(CTipAudioMediaOption::ACTIVITY_METRIC,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
    
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::ACTIVITY_METRIC),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
    }

    void testAMSAMO2d() {
        CTipAudioMediaOption mo(CTipAudioMediaOption::ACTIVITY_METRIC,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
        CTipAudioMediaOption mo2(CTipAudioMediaOption::EKT,
                                 CTipMediaOption::OPTION_NOT_SUPPORTED);
    
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( ams->AddMediaOption(mo2), TIP_OK );

        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::ACTIVITY_METRIC),
                              CTipMediaOption::OPTION_SUPPORTED_BOTH );
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::EKT),
                              CTipMediaOption::OPTION_NOT_SUPPORTED );
    }

    void testAMSAMO3d() {
        CTipAudioMediaOption mo(CTipAudioMediaOption::ACTIVITY_METRIC,
                                CTipMediaOption::OPTION_SUPPORTED_BOTH);
        ams->AddMediaOption(mo);

        mo.SetState(CTipMediaOption::OPTION_SUPPORTED_TX);
        ams->AddMediaOption(mo);
        
        CPPUNIT_ASSERT_EQUAL( ams->GetMediaOptionState(CTipAudioMediaOption::ACTIVITY_METRIC),
                              CTipMediaOption::OPTION_SUPPORTED_TX );
    }

    CPPUNIT_TEST_SUITE( CTipSystemTest );
    CPPUNIT_TEST( testAMSInitd );
    CPPUNIT_TEST( testAMSVersion );
    CPPUNIT_TEST( testAMSTxInvd );
    CPPUNIT_TEST( testAMSTxVideoInvd );
    CPPUNIT_TEST( testAMSTxVideod );
    CPPUNIT_TEST( testAMSTxVideoRemove );
    CPPUNIT_TEST( testAMSRxInvd );
    CPPUNIT_TEST( testAMSRxVideoInvd );
    CPPUNIT_TEST( testAMSRxVideod );
    CPPUNIT_TEST( testAMSTxRxVideoInvd );
    CPPUNIT_TEST( testAMSTxRxVideod );
    CPPUNIT_TEST( testAMSPresFrameRate );
    CPPUNIT_TEST ( testAMSPresFrameRateInvd );
    CPPUNIT_TEST( testAMSTxAudioInvd );
    CPPUNIT_TEST( testAMSTxAudiod );
    CPPUNIT_TEST( testAMSRxAudioInvd );
    CPPUNIT_TEST( testAMSRxAudiod );
    CPPUNIT_TEST( testAMSTxRxAudioInvd );
    CPPUNIT_TEST( testAMSTxRxAudiod );
    CPPUNIT_TEST( testAMSConfd );
    CPPUNIT_TEST( testAMSConfdInvd );
    CPPUNIT_TEST( testAMSSecurity );
    CPPUNIT_TEST( testAMSMCU );
    CPPUNIT_TEST( testAMSFeedback );
    CPPUNIT_TEST( testAMSVMOd );
    CPPUNIT_TEST( testAMSVMO2d );
    CPPUNIT_TEST( testAMSVMO3d );
    CPPUNIT_TEST( testAMSAMOd );
    CPPUNIT_TEST( testAMSAMO2d );
    CPPUNIT_TEST( testAMSAMO3d );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipSystemTest );
