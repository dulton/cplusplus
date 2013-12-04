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

#include "tip_debug_print.h"
#include "private/map_tip_system.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CMapTipSystemTest : public CppUnit::TestFixture {
private:
    CMapTipSystem* system;

public:
    void setUp() {
        system = new CMapTipSystem();
        CPPUNIT_ASSERT( system != NULL );

        // turn off debug prints to keep the test output clean
        gDebugAreas = 0;
    }

    void tearDown() {
        delete system;
    }
    
    void testMapCreateMuxCtrl() {
        CRtcpAppMuxCtrlPacketBase* packet;

        packet = system->MapToMuxCtrl(VIDEO);    
        CPPUNIT_ASSERT( packet != NULL );
        delete packet;

        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT( packet != NULL );
        delete packet;
    }

    void testMapCreateMuxCtrlInvalid() {
        CRtcpAppMuxCtrlPacketBase* packet;

        packet = system->MapToMuxCtrl(MT_MAX);
        CPPUNIT_ASSERT( packet == NULL );

        if (packet != NULL) {
            delete packet;
        }
    }

    void testMapProfile() {
        CRtcpAppMuxCtrlPacketBase* packet;

        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::AVP );
        delete packet;

        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::AVP );
        delete packet;

        system->SetSecurityState(true);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::SAVP );
        delete packet;

        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::SAVP );
        delete packet;

        system->SetFeedbackState(true);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::SAVPF );
        delete packet;

        // audio does not support feedback
        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::SAVP );
        delete packet;
	
        system->SetSecurityState(false);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::AVPF );
        delete packet;
	
        // audio does not support feedback
        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT_EQUAL( packet->GetProfile(), (uint8_t) CRtcpAppMuxCtrlPacketBase::AVP );
        delete packet;
    }

    void testMapOptions() {
        CRtcpAppMuxCtrlPacketBase* packet;

        system->SetMCUState(false);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetOptions(), (uint8_t) 0 );
        delete packet;

        system->SetMCUState(true);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetOptions(), (uint8_t) CRtcpAppMuxCtrlPacketBase::IS_FOCUS );
        delete packet;
    }
    
    void testMapXmit() {
        CRtcpAppMuxCtrlPacketBase* packet;

        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), (uint8_t) 0 );
        delete packet;
    
        system->AddTransmitter(VIDEO, 1);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetXmitPositions(), (uint16_t) 0x0002 );
        delete packet;
    }

    void testMapRecv() {
        CRtcpAppMuxCtrlPacketBase* packet;

        // for pre-V7 video we always have aux receive for video
        system->SetTipVersion(TIP_V6);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) (1 << POS_VIDEO_AUX_1_5FPS) );
        delete packet;

        // for pre-V7 audio we should get no default receive positions
        packet = system->MapToMuxCtrl(AUDIO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0 );
        delete packet;

        // for pre-V7 MCU we should get no default receive positions
        system->SetMCUState(true);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0 );
        delete packet;
        
        // for V7 we should get no default receive positions
        system->SetMCUState(false);
        system->SetTipVersion(TIP_V7);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0 );
        delete packet;
        
        system->AddReceiver(VIDEO, 1);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0x0002 );
        delete packet;
    }

    void testMapConf() {
        CRtcpAppMuxCtrlPacketBase* packet;
        CRtcpAppMuxCtrlV7Packet* v7packet;

        system->SetTipVersion(TIP_V7);
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetConfID(), (uint64_t) 0 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetParticipantID(NULL), (uint32_t) 0 );
        delete packet;

        uint64_t confID = 0x1234567887654321LL;
        uint8_t* testID = (uint8_t*) "abcdABCD1234";
        uint32_t testIDLen = strlen((char*) testID);
    
        system->JoinConference(confID, testID, testIDLen);
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetConfID(), confID );

        uint8_t partID[256];
        CPPUNIT_ASSERT_EQUAL( v7packet->GetParticipantID(partID), testIDLen );
        CPPUNIT_ASSERT_EQUAL( memcmp(partID, testID, testIDLen), 0 );
        delete packet;
    }

    void testMapPres() {
        CRtcpAppMuxCtrlPacketBase* packet;
        CRtcpAppMuxCtrlV7Packet* v7packet;

        system->SetTipVersion(TIP_V7);
        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetNumShared(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetSharedPositions(), (uint16_t) 0x0010 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetNumShared(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetSharedPositions(), (uint16_t) 0x0010 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetNumShared(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetSharedPositions(), (uint16_t) 0x0030 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetNumShared(), (uint8_t) 2 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetSharedPositions(), (uint16_t) 0x0030 );
        delete packet;

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        
        packet = system->MapToMuxCtrl(VIDEO);
        v7packet = dynamic_cast<CRtcpAppMuxCtrlV7Packet*>(packet);

        CPPUNIT_ASSERT( v7packet != NULL );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetNumShared(), (uint8_t) 2 );
        CPPUNIT_ASSERT_EQUAL( v7packet->GetSharedPositions(), (uint16_t) 0x0030 );
        delete packet;
    }
    
    void testMapPresV6() {
        uint32_t tx, rx;
        CRtcpAppMediaoptsPacket* packet;
        
        system->SetTipVersion(TIP_V6);

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        packet = system->MapToMediaOpts(VIDEO);
        packet->GetSSRC(0, tx, rx);
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx),
                              CRtcpAppMediaoptsPacket::AUX_1FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx),
                              CRtcpAppMediaoptsPacket::AUX_1FPS );
        delete packet;
        
        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        packet = system->MapToMediaOpts(VIDEO);
        packet->GetSSRC(0, tx, rx);
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx),
                              CRtcpAppMediaoptsPacket::AUX_5FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx),
                              CRtcpAppMediaoptsPacket::AUX_5FPS );
        delete packet;
        
        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        packet = system->MapToMediaOpts(VIDEO);
        packet->GetSSRC(0, tx, rx);
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        packet = system->MapToMediaOpts(VIDEO);
        packet->GetSSRC(0, tx, rx);
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        delete packet;

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        
        packet = system->MapToMediaOpts(VIDEO);
        packet->GetSSRC(0, tx, rx);
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx),
                              CRtcpAppMediaoptsPacket::AUX_30FPS );
        delete packet;
    }

    void testMapBiDir() {
        CRtcpAppMuxCtrlPacketBase* packet;

        system->SetTipVersion(TIP_V7);
        system->AddBidirectional(VIDEO, 1);
        packet = system->MapToMuxCtrl(VIDEO);
        CPPUNIT_ASSERT_EQUAL( packet->GetNumXmit(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetXmitPositions(), (uint16_t) 0x0002 );
        CPPUNIT_ASSERT_EQUAL( packet->GetNumRcv(), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( packet->GetRcvPositions(), (uint16_t) 0x0002 );
        delete packet;
    }

    void testMapCreateMO() {
        CRtcpAppMediaoptsPacket* packet;

        system->SetTipVersion(TIP_V7);

        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint16_t) 3 );
        delete packet;

        packet = system->MapToMediaOpts(AUDIO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint16_t) 3 );
        delete packet;
    }

    void testMapCreateMOV2() {
        CRtcpAppMediaoptsPacket* packet;

        system->SetTipVersion(TIP_V6);
        
        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint16_t) 2 );
        delete packet;

        packet = system->MapToMediaOpts(AUDIO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVersion(), (uint16_t) 2 );
        delete packet;
    }
    
    void testMapCreateMOInvalid() {
        CRtcpAppMediaoptsPacket* packet;

        packet = system->MapToMediaOpts(MT_MAX);
        CPPUNIT_ASSERT( packet == NULL );

        if (packet != NULL) {
            delete packet;
        }
    }

    void testMapCreateMOOption1() {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        packet = system->MapToMediaOpts(AUDIO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( tx, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( rx, (uint32_t) 0 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( tx, (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( rx, (uint32_t) 0 );
        delete packet;
    }

    void doMapCreateMOOptionState(uint32_t opt1,
                                  CTipVideoMediaOption::VideoOption opt2,
                                  CTipMediaOption::OptionState state)
    {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        CTipVideoMediaOption opt(opt2, state);
        system->AddMediaOption(opt);

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( tx, ((state & CTipMediaOption::OPTION_SUPPORTED_TX) ? opt1 : 0) );
        CPPUNIT_ASSERT_EQUAL( rx, ((state & CTipMediaOption::OPTION_SUPPORTED_RX) ? opt1 : 0) );
        delete packet;
    }

    void doMapCreateMOOptionState(uint32_t opt1, CTipVideoMediaOption::VideoOption opt2) {
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_TX);
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_RX);
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_BOTH);

        // do "not supported" last so it leaves us in a clean state
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_NOT_SUPPORTED);
    }
    
    void doMapCreateMOOptionState(uint32_t opt1,
                                  CTipAudioMediaOption::AudioOption opt2,
                                  CTipMediaOption::OptionState state)
    {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        CTipAudioMediaOption opt(opt2, state);
        system->AddMediaOption(opt);
    
        packet = system->MapToMediaOpts(AUDIO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( tx, ((state & CTipMediaOption::OPTION_SUPPORTED_TX) ? opt1 : 0) );
        CPPUNIT_ASSERT_EQUAL( rx, ((state & CTipMediaOption::OPTION_SUPPORTED_RX) ? opt1 : 0) );
        delete packet;
    }

    void doMapCreateMOOptionState(uint32_t opt1, CTipAudioMediaOption::AudioOption opt2) {
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_TX);
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_RX);
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_BOTH);

        // do "not supported" last so it leaves us in a clean state
        doMapCreateMOOptionState(opt1, opt2, CTipMediaOption::OPTION_NOT_SUPPORTED);
    }

    void testMapCreateVideoMOOption() {
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::REFRESH_FLAG,
                                 CTipVideoMediaOption::REFRESH_FLAG);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS,
                                 CTipVideoMediaOption::INBAND_PARAM_SETS);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::CABAC,
                                 CTipVideoMediaOption::CABAC);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::LTRP,
                                 CTipVideoMediaOption::LTRP);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::GDR,
                                 CTipVideoMediaOption::GDR);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS,
                                 CTipVideoMediaOption::HP_8X8_TRANSFORMS);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5,
                                 CTipVideoMediaOption::UNRESTRICTED_XGA_1_5);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::UNRESTRICTED_720P,
                                 CTipVideoMediaOption::UNRESTRICTED_720P);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P,
                                 CTipVideoMediaOption::UNRESTRICTED_1080P);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30,
                                 CTipVideoMediaOption::UNRESTRICED_XGA_30);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::EKT,
                                 CTipVideoMediaOption::EKT);
    }

    void testMapCreateAudioMOOption() {
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::ACTIVITY_METRIC,
                                 CTipAudioMediaOption::ACTIVITY_METRIC);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT,
                                 CTipAudioMediaOption::DYNAMIC_OUTPUT);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY,
                                 CTipAudioMediaOption::CAPABLE_G722_LEGACY);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::USING_G722_LEGACY,
                                 CTipAudioMediaOption::USING_G722_LEGACY);
        doMapCreateMOOptionState(CRtcpAppMediaoptsPacket::EKT,
                                 CTipAudioMediaOption::EKT);
    }
    
    void testMapCreateMOPres1() {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx), CRtcpAppMediaoptsPacket::AUX_1FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx), CRtcpAppMediaoptsPacket::AUX_1FPS );
        delete packet;
    }

    void testMapCreateMOPres5() {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx), CRtcpAppMediaoptsPacket::AUX_5FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx), CRtcpAppMediaoptsPacket::AUX_5FPS );

        delete packet;
    }

    void testMapCreateMOPres30() {
        CRtcpAppMediaoptsPacket* packet;
        uint32_t tx;
        uint32_t rx;

        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        packet = system->MapToMediaOpts(VIDEO);
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetSSRC(0, tx, rx), 0 );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(tx), CRtcpAppMediaoptsPacket::AUX_30FPS );
        CPPUNIT_ASSERT_EQUAL( CRtcpAppMediaoptsPacket::GetAuxFrameRateForBitValue(rx), CRtcpAppMediaoptsPacket::AUX_30FPS );

        delete packet;
    }

    void testMapFromMuxCtrl() {
        CRtcpAppMuxCtrlPacket packet;

        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetTipVersion(), TIP_V6 );
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetConfID(), (uint64_t) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY );
        CPPUNIT_ASSERT_EQUAL( system->GetParticipantIDLength(), (uint32_t) 0 );
    }

    void testMapFromMuxCtrlV7() {
        CRtcpAppMuxCtrlV7Packet packet;

        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetTipVersion(), TIP_V7 );
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetMCUState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );
        CPPUNIT_ASSERT_EQUAL( system->GetConfID(), (uint64_t) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY );
        CPPUNIT_ASSERT_EQUAL( system->GetParticipantIDLength(), (uint32_t) 0 );
    }

    void testMapFromMuxCtrlInv() {
        CRtcpAppMuxCtrlPacket packet;
        
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, MT_MAX), TIP_ERROR );
    }

    void testMapFromMuxCtrlSecure() {
        CRtcpAppMuxCtrlPacket packet;
        packet.SetProfile(CRtcpAppMuxCtrlPacketBase::SAVP);

        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), true );
    }

    void testMapFromMuxCtrlMCU() {
        CRtcpAppMuxCtrlPacket packet;

        packet.SetOptions(0);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetMCUState(), false );

        packet.SetOptions(CRtcpAppMuxCtrlPacketBase::IS_FOCUS);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetMCUState(), true );
    }

    void testMapFromMuxCtrlFeedback() {
        CRtcpAppMuxCtrlPacket packet;
        packet.SetProfile(CRtcpAppMuxCtrlPacketBase::AVPF);

        // audio does not affect feedback state
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, AUDIO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );

        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), true );

        packet.SetProfile(CRtcpAppMuxCtrlPacketBase::AVP);
        // audio does not affect feedback state
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, AUDIO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), true );
    }

    void testMapFromMuxCtrlXmit() {
        CRtcpAppMuxCtrlPacket packet;

        packet.SetNumXmit(4);
        packet.SetXmitPositions(0x1111);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetTransmitters(VIDEO).to_ulong(), (long unsigned int) 0x1111 );
    }

    void testMapFromMuxCtrlRcv() {
        CRtcpAppMuxCtrlPacket packet;

        packet.SetNumRcv(4);
        packet.SetRcvPositions(0x1111);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetReceivers(VIDEO).to_ulong(), (long unsigned int) 0x1111 );
    }

    void testMapFromMuxCtrlConfID() {
        CRtcpAppMuxCtrlPacket packet;

        uint64_t confID = 0x1234567890ABCDEFLL;
        packet.SetConfID(confID);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetConfID(), confID );
    }

    void testMapFromMuxCtrlPartID() {
        CRtcpAppMuxCtrlV7Packet packet;

        uint8_t partID[] = { 'a', 'b', 'c', 'd' };
        uint32_t partIDLen = 4;
    
        packet.SetParticipantID(partID, partIDLen);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMuxCtrl(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( memcmp(system->GetParticipantID(), partID, partIDLen), 0 );
        CPPUNIT_ASSERT_EQUAL( system->GetParticipantIDLength(), partIDLen );
    }

    void testMapFromMOInv() {
        CRtcpAppMediaoptsPacket packet;

        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_ERROR );
    }

    void testMapFromMO() {
        CRtcpAppMediaoptsPacket packet;

        packet.AddSSRC(0, 0, 0);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
    }

    void doMapFromMOMediaOptionState(uint32_t opt1,
                                     CTipAudioMediaOption::AudioOption opt2,
                                     CTipMediaOption::OptionState state)
    {
        CRtcpAppMediaoptsPacket packet;

        packet.AddSSRC(0, ((state & CTipMediaOption::OPTION_SUPPORTED_TX) ? opt1 : 0),
                       ((state & CTipMediaOption::OPTION_SUPPORTED_RX) ? opt1 : 0));

        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, AUDIO), TIP_OK );
        if (system->GetMediaOptionState(opt2) != state) {
            ostringstream oss;

            oss << "Audio media state test failed opt1 0x" << hex << opt1
                << " opt2 " << dec << opt2 << " expected state " << state
                << " actual state " << system->GetMediaOptionState(opt2);

            CPPUNIT_FAIL(oss.str());
        }
    }

    void doMapFromMOMediaOption(uint32_t opt1,
                                CTipAudioMediaOption::AudioOption opt2)
    {
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_NOT_SUPPORTED);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_TX);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_RX);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_BOTH);
    }

    void doMapFromMOMediaOptionState(uint32_t opt1,
                                     CTipVideoMediaOption::VideoOption opt2,
                                     CTipMediaOption::OptionState state)
    {
        CRtcpAppMediaoptsPacket packet;

        packet.AddSSRC(0, ((state & CTipMediaOption::OPTION_SUPPORTED_TX) ? opt1 : 0),
                       ((state & CTipMediaOption::OPTION_SUPPORTED_RX) ? opt1 : 0));

        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
        if (system->GetMediaOptionState(opt2) != state) {
            ostringstream oss;

            oss << "Video media state test failed opt1 0x" << hex << opt1
                << " opt2 " << dec << opt2 << " expected state " << state
                << " actual state " << system->GetMediaOptionState(opt2);

            CPPUNIT_FAIL(oss.str());
        }
    }

    void doMapFromMOMediaOption(uint32_t opt1,
                                CTipVideoMediaOption::VideoOption opt2)
    {
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_NOT_SUPPORTED);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_TX);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_RX);
        doMapFromMOMediaOptionState(opt1, opt2, CTipMediaOption::OPTION_SUPPORTED_BOTH);
    }

    void testMapFromMOVideoOptions() {
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::REFRESH_FLAG,
                               CTipVideoMediaOption::REFRESH_FLAG);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::INBAND_PARAM_SETS,
                               CTipVideoMediaOption::INBAND_PARAM_SETS);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::CABAC,
                               CTipVideoMediaOption::CABAC);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::LTRP,
                               CTipVideoMediaOption::LTRP);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::GDR,
                               CTipVideoMediaOption::GDR);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::HP_8X8_TRANSFORMS,
                               CTipVideoMediaOption::HP_8X8_TRANSFORMS);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_XGA_1_5,
                               CTipVideoMediaOption::UNRESTRICTED_XGA_1_5);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_720P,
                               CTipVideoMediaOption::UNRESTRICTED_720P);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::UNRESTRICTED_1080P,
                               CTipVideoMediaOption::UNRESTRICTED_1080P);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::UNRESTRICED_XGA_30,
                               CTipVideoMediaOption::UNRESTRICED_XGA_30);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::EKT,
                               CTipVideoMediaOption::EKT);
    }

    void testMapFromMOAudioOptions() {
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::ACTIVITY_METRIC,
                               CTipAudioMediaOption::ACTIVITY_METRIC);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::DYNAMIC_OUTPUT,
                               CTipAudioMediaOption::DYNAMIC_OUTPUT);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::CAPABLE_G722_LEGACY,
                               CTipAudioMediaOption::CAPABLE_G722_LEGACY);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::USING_G722_LEGACY,
                               CTipAudioMediaOption::USING_G722_LEGACY);
        doMapFromMOMediaOption(CRtcpAppMediaoptsPacket::EKT,
                               CTipAudioMediaOption::EKT);
    }

    // NOTE that we only derive the minimum required frame rate from
    // the MO not multi-streaming.
    
    void testMapFromMOPres1() {
        CRtcpAppMediaoptsPacket packet;

        uint32_t opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_1FPS);
        packet.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY );
    }

    void testMapFromMOPres5() {
        CRtcpAppMediaoptsPacket mo;

        uint32_t opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_5FPS);
        mo.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(mo, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testMapFromMOPres30() {
        CRtcpAppMediaoptsPacket mo;

        uint32_t opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_30FPS);
        mo.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(mo, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_30FPS_ONLY );
    }
    
    void testMapFromMOPresV6() {
        CRtcpAppMediaoptsPacket packet;

        system->SetTipVersion(TIP_V6);
        
        uint32_t opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_1FPS);
        packet.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY );

        opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_5FPS);
        packet.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY );

        opt = CRtcpAppMediaoptsPacket::GetBitValueForAuxFrameRate(CRtcpAppMediaoptsPacket::AUX_30FPS);
        packet.AddSSRC(0, opt, opt);
        CPPUNIT_ASSERT_EQUAL( system->MapFromMediaOpts(packet, VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_30FPS_ONLY );
    }
    
    void testMapCreateRTS() {
        CRtcpAppReqToSendPacket* packet;

        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        delete packet;
    }

    void testMapRTS() {
        CRtcpAppReqToSendPacket* packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0x0010 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0x0000 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0x0010 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0x0000 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0x0020 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0x0000 );
        delete packet;

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0x0030 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0x0000 );
        delete packet;

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        
        packet = system->MapToReqToSend();
        CPPUNIT_ASSERT( packet != NULL );
        CPPUNIT_ASSERT_EQUAL( packet->GetVideoPos(), (uint16_t) 0x0030 );
        CPPUNIT_ASSERT_EQUAL( packet->GetAudioPos(), (uint16_t) 0x0000 );
        delete packet;
    }
    
    void testNegVersion() {
        CTipSystem local;
        CTipSystem remote;

        local.SetTipVersion(TIP_V6);
        remote.SetTipVersion(TIP_V7);

        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetTipVersion(), TIP_V6 );

        local.SetTipVersion(TIP_V7);
        remote.SetTipVersion(TIP_V6);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetTipVersion(), TIP_V6 );

        remote.SetTipVersion(TIP_V7);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetTipVersion(), TIP_V7 );
    }

    void testNegXmit() {
        CTipSystem local;
        CTipSystem remote;

        for (MediaType type = VIDEO; type < MT_MAX; type++) {
            local.AddTransmitter(type, 1);
            local.AddTransmitter(type, 2);
            remote.AddReceiver(type, 1);
            remote.AddReceiver(type, 3);

            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetTransmitters(type).test(1), true );
            CPPUNIT_ASSERT_EQUAL( system->GetTransmitters(type).test(2), false );
        }
    }
    
    void testNegRecv() {
        CTipSystem local;
        CTipSystem remote;

        for (MediaType type = VIDEO; type < MT_MAX; type++) {
            local.AddReceiver(type, 1);
            local.AddReceiver(type, 2);
            remote.AddTransmitter(type, 1);
            remote.AddTransmitter(type, 3);

            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetReceivers(type).test(1), true );
            CPPUNIT_ASSERT_EQUAL( system->GetReceivers(type).test(2), false );
        }
    }

    void testNegPres() {
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY);

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY);

        local.SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_ONLY);

        remote.SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_30FPS_ONLY);

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_AND_30FPS);

        remote.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_AND_30FPS);

        remote.SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_5FPS_AND_30FPS);
    }
    
    void testNegConfID() {
        CTipSystem local;
        CTipSystem remote;

        local.JoinConference(0x1234567890ABCDEFLL);
        remote.JoinConference(0xFEDCBA0987654321LL);

        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetConfID(), (uint64_t) 0x1234567890ABCDEFLL );
    }
    
    void testNegPartID() {
        CTipSystem local;
        CTipSystem remote;

        uint32_t lpid = 0xA5A5A5A5;
        uint16_t rpid = 0x5A5A;
        
        local.JoinConference(0x1234567890ABCDEFLL, (uint8_t*) &lpid, sizeof(lpid));
        remote.JoinConference(0xFEDCBA0987654321LL, (uint8_t*) &rpid, sizeof(rpid));

        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( *(uint32_t*) system->GetParticipantID(), lpid );
        CPPUNIT_ASSERT_EQUAL( system->GetParticipantIDLength(), (uint32_t) sizeof(lpid) );
    }

    void testNegSecurity() {
        CTipSystem local;
        CTipSystem remote;
	
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), false );

        local.SetSecurityState(true);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), false );

        remote.SetSecurityState(true);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), true );

        local.SetSecurityState(false);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetSecurityState(), false );
    }
    
    void testNegMCU() {
        CTipSystem local;
        CTipSystem remote;

        local.SetMCUState(true);
        remote.SetMCUState(false);

        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMCUState(), true );
    }

    void testNegFeedback() {
        CTipSystem local;
        CTipSystem remote;
	
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );

        local.SetFeedbackState(true);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );

        remote.SetFeedbackState(true);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), true );

        local.SetFeedbackState(false);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetFeedbackState(), false );
    }
    
    template < class T, class E >
    void doNegOpt(E min, E max) {
        for (E i = min; i < max; i++) {
            CTipSystem local;
            CTipSystem remote;

            T ns(i, CTipMediaOption::OPTION_NOT_SUPPORTED);
            T tx(i, CTipMediaOption::OPTION_SUPPORTED_TX);
            T rx(i, CTipMediaOption::OPTION_SUPPORTED_RX);
            T both(i, CTipMediaOption::OPTION_SUPPORTED_BOTH);

            // local ns & remote ns, result ns
            local.AddMediaOption(ns);
            remote.AddMediaOption(ns);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local ns & remote tx, result ns
            local.AddMediaOption(ns);
            remote.AddMediaOption(tx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local ns & remote rx, result ns
            local.AddMediaOption(ns);
            remote.AddMediaOption(rx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local ns & remote both, result ns
            local.AddMediaOption(ns);
            remote.AddMediaOption(both);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            // local tx & remote ns, result ns
            local.AddMediaOption(tx);
            remote.AddMediaOption(ns);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local tx & remote tx, result ns
            local.AddMediaOption(tx);
            remote.AddMediaOption(tx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local tx & remote rx, result tx
            local.AddMediaOption(tx);
            remote.AddMediaOption(rx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
            // local tx & remote both, result tx
            local.AddMediaOption(tx);
            remote.AddMediaOption(both);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
            // local rx & remote ns, result ns
            local.AddMediaOption(rx);
            remote.AddMediaOption(ns);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local rx & remote tx, result rx
            local.AddMediaOption(rx);
            remote.AddMediaOption(tx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
            // local rx & remote rx, result ns
            local.AddMediaOption(rx);
            remote.AddMediaOption(rx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local rx & remote both, result rx
            local.AddMediaOption(rx);
            remote.AddMediaOption(both);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );

            // local both & remote ns, result ns
            local.AddMediaOption(both);
            remote.AddMediaOption(ns);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
            // local both & remote tx, result rx
            local.AddMediaOption(both);
            remote.AddMediaOption(tx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
            // local both & remote rx, result tx
            local.AddMediaOption(both);
            remote.AddMediaOption(rx);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
            // local both & remote both, result both
            local.AddMediaOption(both);
            remote.AddMediaOption(both);
            system->NegotiateLocalRemote(local, remote);
            CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_BOTH );
        }
    }

    template < class T, class E >
    void doNegOptRemoteControl(E i) {
        CTipSystem local;
        CTipSystem remote;

        T ns(i, CTipMediaOption::OPTION_NOT_SUPPORTED);
        T tx(i, CTipMediaOption::OPTION_SUPPORTED_TX);
        T rx(i, CTipMediaOption::OPTION_SUPPORTED_RX);
        T both(i, CTipMediaOption::OPTION_SUPPORTED_BOTH);
        
        // local ns & remote ns, result ns
        local.AddMediaOption(ns);
        remote.AddMediaOption(ns);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
        // local ns & remote tx, result rx
        local.AddMediaOption(ns);
        remote.AddMediaOption(tx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
        // local ns & remote rx, result tx
        local.AddMediaOption(ns);
        remote.AddMediaOption(rx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
        // local ns & remote both, result both
        local.AddMediaOption(ns);
        remote.AddMediaOption(both);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_BOTH );

        // local tx & remote ns, result ns
        local.AddMediaOption(tx);
        remote.AddMediaOption(ns);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
        // local tx & remote tx, result rx
        local.AddMediaOption(tx);
        remote.AddMediaOption(tx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
        // local tx & remote rx, result tx
        local.AddMediaOption(tx);
        remote.AddMediaOption(rx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
        // local tx & remote both, result both
        local.AddMediaOption(tx);
        remote.AddMediaOption(both);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_BOTH );
            
        // local rx & remote ns, result ns
        local.AddMediaOption(rx);
        remote.AddMediaOption(ns);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
        // local rx & remote tx, result rx
        local.AddMediaOption(rx);
        remote.AddMediaOption(tx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
        // local rx & remote rx, result tx
        local.AddMediaOption(rx);
        remote.AddMediaOption(rx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
        // local rx & remote both, result both
        local.AddMediaOption(rx);
        remote.AddMediaOption(both);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_BOTH );

        // local both & remote ns, result ns
        local.AddMediaOption(both);
        remote.AddMediaOption(ns);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_NOT_SUPPORTED );
            
        // local both & remote tx, result rx
        local.AddMediaOption(both);
        remote.AddMediaOption(tx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_RX );
            
        // local both & remote rx, result tx
        local.AddMediaOption(both);
        remote.AddMediaOption(rx);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_TX );
            
        // local both & remote both, result both
        local.AddMediaOption(both);
        remote.AddMediaOption(both);
        system->NegotiateLocalRemote(local, remote);
        CPPUNIT_ASSERT_EQUAL( system->GetMediaOptionState(i), CTipMediaOption::OPTION_SUPPORTED_BOTH );
    }

    void testNegVideoOpt() {
        doNegOpt<CTipVideoMediaOption>(CTipVideoMediaOption::REFRESH_FLAG,
                                       CTipVideoMediaOption::MAX_VIDEO_OPTION);
    }
    
    void testNegAudioOpt() {
        doNegOpt<CTipAudioMediaOption>(CTipAudioMediaOption::ACTIVITY_METRIC,
                                       CTipAudioMediaOption::USING_G722_LEGACY);
        doNegOpt<CTipAudioMediaOption>(CTipAudioMediaOption::EKT,
                                       CTipAudioMediaOption::MAX_AUDIO_OPTION);

        // controlled by remote side only
        doNegOptRemoteControl<CTipAudioMediaOption>(CTipAudioMediaOption::USING_G722_LEGACY);
    }

    void testMapToMuxCtrlSharedPos() {
        uint16_t shpos;
        
        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(AUDIO, shpos), (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) 0 );

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(VIDEO, shpos), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(VIDEO, shpos), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(VIDEO, shpos), (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(VIDEO, shpos), (uint8_t) 2 );
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);

        CPPUNIT_ASSERT_EQUAL( system->MapToMuxCtrlSharedPos(VIDEO, shpos), (uint8_t) 2 );
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );
    }

    void testMapToActiveSharedPos() {
        uint16_t shpos;

        // v7 tests
        system->MapToActiveSharedPos(AUDIO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) 0 );

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) PositionToMask(POS_VIDEO_AUX_30FPS) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_AND_30FPS);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );

        // v6 tests
        system->SetTipVersion(TIP_V6);
        
        system->MapToActiveSharedPos(AUDIO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) 0 );

        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, PositionToMask(POS_VIDEO_AUX_1_5FPS) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) PositionToMask(POS_VIDEO_AUX_1_5FPS) );
        
        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        system->MapToActiveSharedPos(VIDEO, shpos);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        system->NegotiateLocalRemote(local, remote);
        system->MapToActiveSharedPos(VIDEO, shpos);
        
        CPPUNIT_ASSERT_EQUAL( system->GetPresentationFrameRate(), CTipSystem::PRES_1FPS_AND_30FPS);
        CPPUNIT_ASSERT_EQUAL( shpos, (uint16_t) (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)) );
    }

    void testMapPositionToFrameRate() {
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_CONTROL), CTipSystem::PRES_INVALID_RATE );

        // v7 tests
        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_30FPS_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_30FPS_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_30FPS_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_30FPS_RATE );

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        CTipSystem local;
        CTipSystem remote;

        local.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        remote.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        system->NegotiateLocalRemote(local, remote);
        
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_30FPS_RATE );

        // V6 tests
        system->SetTipVersion(TIP_V6);
        system->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_INVALID_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_INVALID_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_30FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_INVALID_RATE );

        system->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_INVALID_RATE );

        // do some special stuff to get the system into the 1 and 30
        // mode as we can't set that directly
        local.SetTipVersion(TIP_V6);
        system->NegotiateLocalRemote(local, remote);

        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_1_5FPS),
                              CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( system->MapPositionToFrameRate(POS_VIDEO_AUX_30FPS),
                              CTipSystem::PRES_INVALID_RATE );
    }
    
    CPPUNIT_TEST_SUITE( CMapTipSystemTest );
    CPPUNIT_TEST( testMapCreateMuxCtrl );
    CPPUNIT_TEST( testMapCreateMuxCtrlInvalid );
    CPPUNIT_TEST( testMapProfile );
    CPPUNIT_TEST( testMapOptions );
    CPPUNIT_TEST( testMapXmit );
    CPPUNIT_TEST( testMapRecv );
    CPPUNIT_TEST( testMapConf );
    CPPUNIT_TEST( testMapPres );
    CPPUNIT_TEST( testMapPresV6 );
    CPPUNIT_TEST( testMapBiDir );
    CPPUNIT_TEST( testMapCreateMO );
    CPPUNIT_TEST( testMapCreateMOV2 );
    CPPUNIT_TEST( testMapCreateMOInvalid );
    CPPUNIT_TEST( testMapCreateMOOption1 );
    CPPUNIT_TEST( testMapCreateVideoMOOption );
    CPPUNIT_TEST( testMapCreateAudioMOOption );
    CPPUNIT_TEST( testMapCreateMOPres1 );
    CPPUNIT_TEST( testMapCreateMOPres5 );
    CPPUNIT_TEST( testMapCreateMOPres30 );
    CPPUNIT_TEST( testMapFromMuxCtrl );
    CPPUNIT_TEST( testMapFromMuxCtrlV7 );
    CPPUNIT_TEST( testMapFromMuxCtrlInv );
    CPPUNIT_TEST( testMapFromMuxCtrlSecure );
    CPPUNIT_TEST( testMapFromMuxCtrlMCU );
    CPPUNIT_TEST( testMapFromMuxCtrlFeedback );
    CPPUNIT_TEST( testMapFromMuxCtrlXmit );
    CPPUNIT_TEST( testMapFromMuxCtrlRcv );
    CPPUNIT_TEST( testMapFromMuxCtrlConfID );
    CPPUNIT_TEST( testMapFromMuxCtrlPartID );
    CPPUNIT_TEST( testMapFromMOInv );
    CPPUNIT_TEST( testMapFromMO );
    CPPUNIT_TEST( testMapFromMOVideoOptions );
    CPPUNIT_TEST( testMapFromMOAudioOptions );
    CPPUNIT_TEST( testMapFromMOPres1 );
    CPPUNIT_TEST( testMapFromMOPres5 );
    CPPUNIT_TEST( testMapFromMOPres30 );
    CPPUNIT_TEST( testMapFromMOPresV6 );
    CPPUNIT_TEST( testMapCreateRTS );
    CPPUNIT_TEST( testMapRTS);
    CPPUNIT_TEST( testNegVersion );
    CPPUNIT_TEST( testNegXmit );
    CPPUNIT_TEST( testNegRecv );
    CPPUNIT_TEST( testNegPres );
    CPPUNIT_TEST( testNegConfID );
    CPPUNIT_TEST( testNegPartID );
    CPPUNIT_TEST( testNegSecurity );
    CPPUNIT_TEST( testNegMCU );
    CPPUNIT_TEST( testNegFeedback );
    CPPUNIT_TEST( testNegVideoOpt );
    CPPUNIT_TEST( testNegAudioOpt );
    CPPUNIT_TEST( testMapToMuxCtrlSharedPos );
    CPPUNIT_TEST( testMapToActiveSharedPos );
    CPPUNIT_TEST( testMapPositionToFrameRate );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CMapTipSystemTest );
    
