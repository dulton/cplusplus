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
#include "tip_debug_tools.h"
#include "tip_time.h"
#include "rtcp_packet_factory.h"
#include "rtcp_rr_packet.h"
#include "rtcp_tip_echo_packet.h"
#include "rtcp_tip_reqtosend_packet.h"
#include "rtcp_tip_spimap_packet.h"
#include "rtcp_tip_notify_packet.h"
#include "private/map_tip_system.h"
#include "tip.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// test packet relay class, just saves off messages for later
// inspection
class CTipTestXmit : public CTipPacketTransmit {
public:
    CTipTestXmit() : rxMO(NULL), rxRTS(NULL), rxACKMC(NULL), rxACKMO(NULL),
                     rxACKRTS(NULL), rxACKECHO(NULL), rxACKSPIMAP(NULL),
                     rxACKNOTIFY(NULL) {
        rxMC[VIDEO] = NULL;
        rxMC[AUDIO] = NULL;
    }

    ~CTipTestXmit() {
        delete rxMC[VIDEO];
        delete rxMC[AUDIO];
        delete rxMO;
        delete rxRTS;
        delete rxACKMC;
        delete rxACKMO;
        delete rxACKRTS;
        delete rxACKECHO;
        delete rxACKSPIMAP;
        delete rxACKNOTIFY;
    }
    
    virtual Status Transmit(const uint8_t* pktBuffer, uint32_t pktSize, MediaType mType) {
        CPacketBuffer buffer((uint8_t*) pktBuffer, pktSize);

        while (buffer.GetBufferSize()) {
            CRtcpPacket* rtcp = CRtcpPacketFactory::CreatePacketFromBuffer(buffer);
            if (rtcp == NULL) {
                ostringstream oss;
                oss << "invalid packet for transmission:  " << HexDump(pktBuffer, pktSize);
                CPPUNIT_FAIL(oss.str());

                // not gonna get to here but just for fun...
                continue;
            }
            
            // only handle APP packets
            CRtcpTipPacket* packet = dynamic_cast<CRtcpTipPacket*>(rtcp);
            if (packet == NULL) {
                // might just be a compound packet, ignore it and try the
                // next one
                delete rtcp;
                continue;
            }
        
            switch (packet->GetTipPacketType()) {
            case MUXCTRL:
                delete rxMC[mType];
                rxMC[mType] = packet;
                break;
                
            case MEDIAOPTS:
                delete rxMO;
                rxMO = packet;
                break;
                
            case REQTOSEND:
                delete rxRTS;
                rxRTS = packet;
                break;
                
            case ACK_MUXCTRL:
                delete rxACKMC;
                rxACKMC = packet;
                break;
                
            case ACK_MEDIAOPTS:
                delete rxACKMO;
                rxACKMO = packet;
                break;
                
            case ACK_REQTOSEND:
                delete rxACKRTS;
                rxACKRTS = packet;
                break;

            case TIPECHO:
                delete rxACKECHO;
                rxACKECHO = packet;
                break;
		
            case ACK_SPIMAP:
                delete rxACKSPIMAP;
                rxACKSPIMAP = packet;
                break;

            case ACK_NOTIFY:
                delete rxACKNOTIFY;
                rxACKNOTIFY = packet;
                break;
                
            default:
                delete packet;
                break;
            }
        }

        return TIP_OK;
    }

    CRtcpTipPacket* rxMC[MT_MAX];
    CRtcpTipPacket* rxMO;
    CRtcpTipPacket* rxRTS;
    CRtcpTipPacket* rxACKMC;
    CRtcpTipPacket* rxACKMO;
    CRtcpTipPacket* rxACKRTS;
    CRtcpTipPacket* rxACKECHO;
    CRtcpTipPacket* rxACKSPIMAP;
    CRtcpTipPacket* rxACKNOTIFY;
};

// test callback interface class, just remembers when functions are called
class CTipTestCallback : public CTipCallback {
public:
    CTipTestCallback() : mnEarly(0), mnFailed(0), mnMismatch(0),
                         mnIncompatible(0), mnLastAckRx(0), mnLastAckTx(0), mnLastAckTxOld(0),
                         mnUpdate(0), mnLocalPresStart(0), mnLocalPresStop(0),
                         mnLocalPresFailed(0), mnRemotePresStart(0), mnRemotePresStop(0),
                         mnResolvePresentationConflict(0),
                         mnLocalPresStart4(0),
                         mnLocalPresStart5(0),
                         mnRemotePresStart4(0),
                         mnRemotePresStart5(0),
                         mnLocalPresStart4FPS(CTipSystem::PRES_INVALID_RATE),
                         mnLocalPresStart5FPS(CTipSystem::PRES_INVALID_RATE),
                         mnRemotePresStart4FPS(CTipSystem::PRES_INVALID_RATE),
                         mnRemotePresStart5FPS(CTipSystem::PRES_INVALID_RATE),
                         mnMismatchReturn(true), mnLastAckTxReturn(true), 
                         mnLastAckTxReturnOld(true),
                         mnUpdateReturn(true), mnRemotePresStartReturn(true),
                         mnLastAckTxReinvite(true), mnResolvePresentationConflictReturn(true),
                         mnLastAckTxPacket(NULL), mnLastAckTxPacketOld(NULL),
                         mnUpdatePacket(NULL), mnRemotePresStartPacket(NULL),
                         mnSecurityKeyUpdate(0), mnSecurityKeyUpdateReturn(true),
                         mnSecuritySPI(0), mnSecurityPacket(NULL),
                         mnSecurityStateUpdate(0), mnSecurityStateFlag(false)
    {}
    ~CTipTestCallback() {}

    void TipNegotiationEarly(MediaType mType) {
        mnEarly = 1;
    }
    
    void TipNegotiationFailed(MediaType mType) {
        mnFailed = 1;
    }
    
    bool TipNegotiationMismatch(MediaType mType, uint8_t version) {
        mnMismatch = version;
        return mnMismatchReturn;
    }
    
    void TipNegotiationIncompatible(MediaType mType, uint8_t version) {
        mnIncompatible = version;
    }
    
    void TipNegotiationLastAckReceived(MediaType mType) {
        mnLastAckRx = 1;
    }
    
    bool TipNegotiationLastAckTransmit(MediaType mType, void* id) {
        mnLastAckTxOld = 1;
        mnLastAckTxPacketOld = id;
        return mnLastAckTxReturnOld;
    }

    bool TipNegotiationLastAckTransmit(MediaType mType, bool reinvite, void* id) {
        mnLastAckTx = 1;
        mnLastAckTxReinvite = reinvite;
        mnLastAckTxPacket = id;
        return mnLastAckTxReturn;
    }
    
    bool TipNegotiationUpdate(MediaType mType, void* id) {
        mnUpdate = 1;
        mnUpdatePacket = id;
        return mnUpdateReturn;
    }

    void LocalPresentationStart(uint8_t position,
                                CTipSystem::PresentationStreamFrameRate fps)
    {
        mnLocalPresStart++;

        if (position == POS_VIDEO_AUX_1_5FPS) {
            mnLocalPresStart4 = 1;
            mnLocalPresStart4FPS = fps;
        } else {
            mnLocalPresStart5 = 1;
            mnLocalPresStart5FPS = fps;
        }
    }
    
    void LocalPresentationStop() {
        mnLocalPresStop = 1;
    }

    void LocalPresentationFailure() {
        mnLocalPresFailed = 1;
    }

    bool RemotePresentationStart(uint8_t position,
                                 CTipSystem::PresentationStreamFrameRate fps,
                                 void* id)
    {
        mnRemotePresStart++;
        if (position == POS_VIDEO_AUX_1_5FPS) {
            mnRemotePresStart4 = 1;
            mnRemotePresStart4FPS = fps;
        } else {
            mnRemotePresStart5 = 1;
            mnRemotePresStart5FPS = fps;
        }

        mnRemotePresStartPacket = id;
        return mnRemotePresStartReturn;
    }

    void RemotePresentationStop() {
        mnRemotePresStop = 1;
    }

    bool ResolvePresentationConflict() {
        mnResolvePresentationConflict = 1;
        return mnResolvePresentationConflictReturn;
    }

    bool TipSecurityKeyUpdate(MediaType mType, uint16_t spi, 
                              const uint8_t* salt, const uint8_t* kek, 
                              void* id)
    {
        mnSecurityKeyUpdate = 1;

        mnSecuritySPI = spi;
        memcpy(mnSecuritySalt, salt, sizeof(mnSecuritySalt));
        memcpy(mnSecurityKek, kek, sizeof(mnSecurityKek));
        mnSecurityPacket = id;

        return mnSecurityKeyUpdateReturn;
    }

    void TipSecurityStateUpdate(MediaType mType, bool secure)
    {
        mnSecurityStateUpdate = 1;
        mnSecurityStateFlag = secure;
    }
    
    uint8_t mnEarly;
    uint8_t mnFailed;
    uint8_t mnMismatch;
    uint8_t mnIncompatible;
    uint8_t mnLastAckRx;
    uint8_t mnLastAckTx;
    uint8_t mnLastAckTxOld;
    uint8_t mnUpdate;
    uint8_t mnLocalPresStart;
    uint8_t mnLocalPresStop;
    uint8_t mnLocalPresFailed;
    uint8_t mnRemotePresStart;
    uint8_t mnRemotePresStop;
    uint8_t mnResolvePresentationConflict;
    uint8_t mnLocalPresStart4;
    uint8_t mnLocalPresStart5;
    uint8_t mnRemotePresStart4;
    uint8_t mnRemotePresStart5;
    CTipSystem::PresentationStreamFrameRate mnLocalPresStart4FPS;
    CTipSystem::PresentationStreamFrameRate mnLocalPresStart5FPS;
    CTipSystem::PresentationStreamFrameRate mnRemotePresStart4FPS;
    CTipSystem::PresentationStreamFrameRate mnRemotePresStart5FPS;

    bool mnMismatchReturn;
    bool mnLastAckTxReturn;
    bool mnLastAckTxReturnOld;
    bool mnUpdateReturn;
    bool mnRemotePresStartReturn;
    bool mnLastAckTxReinvite;
    bool mnResolvePresentationConflictReturn;

    void* mnLastAckTxPacket;
    void* mnLastAckTxPacketOld;
    void* mnUpdatePacket;
    void* mnRemotePresStartPacket;

    uint8_t mnSecurityKeyUpdate;
    bool mnSecurityKeyUpdateReturn;
    uint16_t mnSecuritySPI;
    uint8_t mnSecuritySalt[CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH];
    uint8_t mnSecurityKek[CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH];
    void* mnSecurityPacket;

    uint8_t mnSecurityStateUpdate;
    bool mnSecurityStateFlag;
};

class CTipTest : public CppUnit::TestFixture {
private:
    CTipTestXmit* xmit;
    CTip* am;
    CTipTestCallback* callback;
    CMapTipSystem* rs;
    
public:
    void setUp() {
        // turn off debug prints 
        if (getenv("TEST_TIP_DEBUG") == NULL) {
            gDebugFlags = 0;
        }
        
        xmit = new CTipTestXmit();
        CPPUNIT_ASSERT( xmit != NULL );
        
        am = new CTip(*xmit);
        CPPUNIT_ASSERT( am != NULL );

        callback = new CTipTestCallback();
        CPPUNIT_ASSERT( callback != NULL );
        am->SetCallback(callback);

        rs = new CMapTipSystem();
        CPPUNIT_ASSERT( rs != NULL );
        rs->SetTipVersion(TIP_V7);
    }

    void tearDown() {
        delete am;
        delete xmit;
        delete rs;
    }

    void testCallback() {
        // just verify that setting the callback to NULL doesn't break anything
        am->SetCallback(NULL);
        callback->TipNegotiationEarly(VIDEO);

        // set the callback to the same value and verify it still works
        am->SetCallback(callback);
        callback->TipNegotiationEarly(VIDEO);
    }

    void testTipNegInvalid() {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(MT_MAX), TIP_ERROR );
    }

    void testTipNegDouble() {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_ERROR );
    }

    void testStopTipNegInvalid() {
        CPPUNIT_ASSERT_EQUAL( am->StopTipNegotiate(MT_MAX), TIP_ERROR );
    }

    void testReceiveAckUnknown() {
        CRtcpTipAckPacket ack(ACK_MUXCTRL);
        CPacketBufferData buffer;

        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
    }
    
    void testReceiveInvalid() {
        uint8_t buffer[2048] = { 0 };
        
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(NULL, sizeof(buffer), VIDEO), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, 0, VIDEO), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer, sizeof(buffer), MT_MAX), TIP_ERROR );
    }

    void testReceiveInvalid2() {
        // send in an RR packet, should be ignored
        CRtcpRRPacket rr;
        CPacketBufferData buffer;

        rr.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_ERROR );
    }
    
    void testDelayedAckInvalid() {
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(NULL, VIDEO), TIP_ERROR );
    }

    void testSSRCEndZero() {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        am->DoPeriodicActivity();

        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );
        CPPUNIT_ASSERT_EQUAL( (xmit->rxMC[VIDEO]->GetSSRC() & 0x000000FF), (uint32_t) 0 );

        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );
        am->DoPeriodicActivity();

        CPPUNIT_ASSERT( xmit->rxMC[AUDIO] != NULL );
        CPPUNIT_ASSERT_EQUAL( (xmit->rxMC[AUDIO]->GetSSRC() & 0x000000FF), (uint32_t) 0 );
    }

    void testSetSSRC() {
        am->SetRTCPSSRC(VIDEO, 0x12345678);

        // last byte must be 00 per spec
        CPPUNIT_ASSERT_EQUAL( am->GetRTCPSSRC(VIDEO), (uint32_t) 0x12345600 );

        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        am->DoPeriodicActivity();

        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxMC[VIDEO]->GetSSRC(), (uint32_t) 0x12345600 );

        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        am->SetRTCPSSRC(AUDIO, 0x87654321);

        // last byte must be 00 per spec
        CPPUNIT_ASSERT_EQUAL( am->GetRTCPSSRC(AUDIO), (uint32_t) 0x87654300 );

        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );
        am->DoPeriodicActivity();

        CPPUNIT_ASSERT( xmit->rxMC[AUDIO] != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxMC[AUDIO]->GetSSRC(), (uint32_t) 0x87654300 );

        // ask for invalid media type should get 0
        CPPUNIT_ASSERT_EQUAL( am->GetRTCPSSRC(MT_MAX), (uint32_t) 0 );

        // set SSRC for invalid media type
        am->SetRTCPSSRC(MT_MAX, 0);
        CPPUNIT_ASSERT_EQUAL( am->GetRTCPSSRC(VIDEO), (uint32_t) 0x12345600 );
        CPPUNIT_ASSERT_EQUAL( am->GetRTCPSSRC(AUDIO), (uint32_t) 0x87654300 );
    }
    
    void doTipNegLocal(MediaType mType) {
        CPacketBufferData buffer;

        // send out first packets
        am->DoPeriodicActivity();

        // verify we got MUXCTRL
        CPPUNIT_ASSERT( xmit->rxMC[mType] != NULL );

        // ack MUXCTRL
        CRtcpTipAckPacket mcack(*xmit->rxMC[mType]);
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), mType),
                              TIP_OK );

        // send out next packets
        am->DoPeriodicActivity();

        // verify we got MEDIAOPTS
        CPPUNIT_ASSERT( xmit->rxMO != NULL );

        // ack MEDIAOPTS
        CRtcpTipAckPacket moack(*xmit->rxMO);
        buffer.Reset();
        moack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), mType),
                              TIP_OK );

        // verify callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckRx, (uint8_t) 1 );
    }

    void doTipNegRemote(MediaType mType, bool ack = true, uint64_t mc_time = 0,
                        bool old = false) {

        CPacketBufferData buffer;

        // send in MUXCTRL
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(mType);

        if (mc_time == 0) {
            mc->SetNtpTime(GetNtpTimestamp());
        } else {
            mc->SetNtpTime(mc_time);
        }
        
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), mType),
                              TIP_OK );
        delete mc;

        // verify ACK went out
        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(mType) );

        if (old) {
            callback->mnLastAckTxReturnOld = ack;
        } else {
            callback->mnLastAckTxReturn = ack;
        }

        // send in MO
        buffer.Reset();
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(mType);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), mType),
                              TIP_OK );
        delete mo;

        // verify ACK went out if not delayed, otherwise verify ACK
        // did not go out
        if (ack) {
            CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
            CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(mType) );
        } else {
            CPPUNIT_ASSERT( xmit->rxACKMO == NULL );
        }

        // verify callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTx, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxOld, (uint8_t) 1 );
    }

    void doTipNeg(MediaType mType) {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(mType), TIP_OK );
        doTipNegLocal(mType);
        doTipNegRemote(mType);
    }
    
    void testTipNegLocal() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        doTipNegLocal(VIDEO);
    }

    void testTipNegRemote() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        doTipNegRemote(VIDEO);
    }

    void testTipNegRemoteDelay() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        doTipNegRemote(VIDEO, false);

        // tip neg is finished but we haven't sent the last ack yet
        CPPUNIT_ASSERT( callback->mnLastAckTxPacket != NULL );

        // send it now
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnLastAckTxPacket, VIDEO), TIP_OK );

        // verify ack went out
        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }
    
    // delay the last ACK using the old, deprecated callback
    void testTipNegRemoteDelayOld() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // delay the ack on the old deprecated callback only
        doTipNegRemote(VIDEO, false, 0, true);

        // tip neg is finished but we haven't sent the last ack yet
        CPPUNIT_ASSERT( callback->mnLastAckTxPacketOld != NULL );

        // send it now
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnLastAckTxPacketOld, VIDEO), TIP_OK );

        // verify ack went out
        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    // local timeout -- we do not receive acks for our transmitted
    // MUXCTRL
    void testTipNegLocalTimeout() {
        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // do the remote side so it doesn't time out first
        doTipNegRemote(VIDEO);
        
        // spin through all the retransmissions of MUXCTRL
        for (uint32_t i = 0; i <= DEFAULT_RETRANS_LIMIT; i++) {
            usleep((am->GetIdleTime() * 1000));
            am->DoPeriodicActivity();
        }

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnFailed, (uint8_t) 1 );
    }

    // local timeout -- we do not receive acks for our transmitted
    // MEDIAOPTS
    void testTipNegLocalTimeout2() {
        CPacketBufferData buffer;
        
        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // do the remote side so it doesn't time out first
        doTipNegRemote(VIDEO);

        // send out first packets
        am->DoPeriodicActivity();

        // verify we got MUXCTRL
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );

        // ack MUXCTRL
        CRtcpTipAckPacket mcack(*xmit->rxMC[VIDEO]);
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // spin through all the retransmissions of MEDIAOPTS
        for (uint32_t i = 0; i <= DEFAULT_RETRANS_LIMIT; i++) {
            usleep((am->GetIdleTime() * 1000));
            am->DoPeriodicActivity();
        }

        // verify we got MEDIAOPTS
        CPPUNIT_ASSERT( xmit->rxMO != NULL );
        
        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnFailed, (uint8_t) 1 );
    }

    // remote timeout -- we do not receive any packets from the remote
    // side
    void testTipNegRemoteTimeout() {
        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // do the local side so it doesn't time out first
        doTipNegLocal(VIDEO);

        // wait a bit for the remote side to timeout
        usleep((am->GetIdleTime() * 1000));

        am->DoPeriodicActivity();

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnFailed, (uint8_t) 1 );
    }
    
    // remote timeout -- we receive MUXCTRL but not MEDIAOPTS from the
    // remote side
    void testTipNegRemoteTimeout2() {
        CPacketBufferData buffer;

        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // do the local side so it doesn't time out first
        doTipNegLocal(VIDEO);

        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;
        
        // verify ACK went out
        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );

        // wait a bit for the remote side to timeout
        usleep((am->GetIdleTime() * 1000));

        am->DoPeriodicActivity();

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnFailed, (uint8_t) 1 );
    }
    
    void testTipNegEarly() {
        CRtcpAppMuxCtrlPacket mc;
        CPacketBufferData buffer;

        uint64_t mc_time = GetNtpTimestamp();
        mc.SetNtpTime(mc_time);
        mc.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // verify that we got the early callback
        CPPUNIT_ASSERT_EQUAL( callback->mnEarly, (uint8_t) 1 );

        // verify normal tip negotiation works after we start
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        doTipNegRemote(VIDEO, true, mc_time);
    }

    void testTipNegIncompatibleMin() {
        CPacketBufferData buffer;

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // feed in the remote packets.  we won't be able to parse this
        // packet as the version is no good.
        CRtcpAppMuxCtrlPacket mc;
        mc.SetVersion((SUPPORTED_VERSION_MIN - 1));
        mc.SetNtpTime(GetNtpTimestamp());
        mc.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_ERROR );

        // note incompatible callback not invoked as the packet is not
        // parsed
    }

    void testTipNegMismatchRemoteLower() {
        CPacketBufferData buffer;

        // set remote system version to 6
        rs->SetTipVersion(TIP_V6);
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // feed in the remote packets
        uint64_t mc_time = GetNtpTimestamp();
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(mc_time);
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // verify mismatch callback
        CPPUNIT_ASSERT_EQUAL( callback->mnMismatch, (uint8_t) 6 );

        // verify that tip negotiation finishes as it should.  we
        // don't want to send in a new MUXCTRL here, so we make
        // it look like a dup to keep the code common.
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO, true, mc_time);

        // verify transmitted packets are the right version
        mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[VIDEO]);
        CPPUNIT_ASSERT( mc != NULL );
        CPPUNIT_ASSERT_EQUAL( mc->GetVersion(), (uint8_t) 6 );

        CRtcpAppMediaoptsPacket* mo = dynamic_cast<CRtcpAppMediaoptsPacket*>(xmit->rxMO);
        CPPUNIT_ASSERT( mo != NULL );
        CPPUNIT_ASSERT_EQUAL( mo->GetVersion(), (uint16_t) 2 );

        // verify the audio packets were not sent when the mismatch
        // caused a restart of tip negotiation
        CPPUNIT_ASSERT( xmit->rxMC[AUDIO] == NULL );
    }
    
    void testTipNegMismatchRemoteLowerDelay() {
        CPacketBufferData buffer;

        // set remote system version to 6
        rs->SetTipVersion(TIP_V6);

        // configure callback to stop tip negotiation on mismatch
        callback->mnMismatchReturn = false;
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // feed in the remote packets
        uint64_t mc_time = GetNtpTimestamp();
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(mc_time);
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // verify mismatch callback
        CPPUNIT_ASSERT_EQUAL( callback->mnMismatch, (uint8_t) 6 );

        // verify packet was not ACK'ed
        CPPUNIT_ASSERT( xmit->rxACKMC == NULL );

        // set local version manually and restart
        am->GetTipSystem().SetTipVersion(TIP_V6);
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        
        // verify tip negotiation works
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO, true, mc_time);

        // verify transmitted packets are the right version
        mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[VIDEO]);
        CPPUNIT_ASSERT( mc != NULL );
        CPPUNIT_ASSERT_EQUAL( mc->GetVersion(), (uint8_t) 6 );

        CRtcpAppMediaoptsPacket* mo = dynamic_cast<CRtcpAppMediaoptsPacket*>(xmit->rxMO);
        CPPUNIT_ASSERT( mo != NULL );
        CPPUNIT_ASSERT_EQUAL( mo->GetVersion(), (uint16_t) 2 );
    }
    
    void testTipNegMismatchRemoteHigher() {
        CPacketBufferData buffer;

        // set local system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // feed in the remote packets
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // verify mismatch callback is not asserted
        CPPUNIT_ASSERT_EQUAL( callback->mnMismatch, (uint8_t) 0 );

        // remote side will now downgrade and we should finish as normal
        rs->SetTipVersion(TIP_V6);
        
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);
    }
    
    void testTipNegMismatchRemoteHigherTimeout() {
        CPacketBufferData buffer;

        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);

        // set local system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // do the local side so it doesn't time out first
        doTipNegLocal(VIDEO);

        // feed in the remote packets
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // wait a bit for the remote side to timeout
        usleep((am->GetIdleTime() * 1000));

        // verify mismatch callback is not asserted
        CPPUNIT_ASSERT_EQUAL( callback->mnMismatch, (uint8_t) 0 );

        am->DoPeriodicActivity();

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnFailed, (uint8_t) 1 );
    }

    // verify that a mismatch on one media type forces a version
    // change on the other media type as well.
    void testTipNegMismatchAllMedia() {
        CPacketBufferData buffer;

        // set remote system version to 6
        rs->SetTipVersion(TIP_V6);
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // feed in the remote packets
        uint64_t mc_time = GetNtpTimestamp();
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(mc_time);
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // verify mismatch callback
        CPPUNIT_ASSERT_EQUAL( callback->mnMismatch, (uint8_t) 6 );

        // verify that tip negotiation finishes as it should.  we
        // don't want to send in a new MUXCTRL here, so we make
        // it look like a dup to keep the code common.
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO, true, mc_time);

        doTipNegLocal(AUDIO);
        doTipNegRemote(AUDIO);
        
        // verify transmitted packets are the right version
        mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[VIDEO]);
        CPPUNIT_ASSERT( mc != NULL );
        CPPUNIT_ASSERT_EQUAL( mc->GetVersion(), (uint8_t) 6 );

        CRtcpAppMediaoptsPacket* mo = dynamic_cast<CRtcpAppMediaoptsPacket*>(xmit->rxMO);
        CPPUNIT_ASSERT( mo != NULL );
        CPPUNIT_ASSERT_EQUAL( mo->GetVersion(), (uint16_t) 2 );

        mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[AUDIO]);
        CPPUNIT_ASSERT( mc != NULL );
        CPPUNIT_ASSERT_EQUAL( mc->GetVersion(), (uint8_t) 6 );
    }

    void testTipNegReinviteRemoteNtpHigher() {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // do local TIP negotiation
        doTipNegLocal(AUDIO);
        doTipNegLocal(VIDEO);

        // do remote TIP negotiation using a higher MUXCTRL NTP
        // timestamp
        doTipNegRemote(VIDEO, true, (xmit->rxMC[VIDEO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, true );

        doTipNegRemote(AUDIO, true, (xmit->rxMC[AUDIO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );
    }

    void testTipNegReinviteRemoteNtpLower() {
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // do local TIP negotiation
        doTipNegLocal(AUDIO);
        doTipNegLocal(VIDEO);

        // do remote TIP negotiation using a lower MUXCTRL NTP
        // timestamp
        doTipNegRemote(VIDEO, true, (xmit->rxMC[VIDEO]->GetNtpTime() - 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );

        doTipNegRemote(AUDIO, true, (xmit->rxMC[AUDIO]->GetNtpTime() - 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );
    }
    
    // local MCU should be true
    void testTipNegReinviteLocalMCU() {
        am->GetTipSystem().SetMCUState(true);

        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // do local TIP negotiation
        doTipNegLocal(AUDIO);
        doTipNegLocal(VIDEO);

        doTipNegRemote(VIDEO, true, (xmit->rxMC[VIDEO]->GetNtpTime() - 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, true );

        doTipNegRemote(AUDIO, true, (xmit->rxMC[AUDIO]->GetNtpTime() - 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );
    }

    // remote MCU should always be false
    void testTipNegReinviteRemoteMCU() {
        rs->SetMCUState(true);

        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // do local TIP negotiation
        doTipNegLocal(AUDIO);
        doTipNegLocal(VIDEO);

        // do remote TIP negotiation using a higher MUXCTRL NTP
        // timestamp
        doTipNegRemote(VIDEO, true, (xmit->rxMC[VIDEO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );

        doTipNegRemote(AUDIO, true, (xmit->rxMC[AUDIO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );
    }

    // V6 callback should always be false
    void testTipNegReinviteV6() {
        rs->SetTipVersion(TIP_V6);
        am->GetTipSystem().SetTipVersion(TIP_V6);
    
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(AUDIO), TIP_OK );

        // do local TIP negotiation
        doTipNegLocal(AUDIO);
        doTipNegLocal(VIDEO);

        doTipNegRemote(VIDEO, true, (xmit->rxMC[VIDEO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );

        doTipNegRemote(AUDIO, true, (xmit->rxMC[AUDIO]->GetNtpTime() + 1));
        CPPUNIT_ASSERT_EQUAL( callback->mnLastAckTxReinvite, false );
    }

    void testTipNegUpdateMUXCTRL() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        // delete the old MC ack
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;

        // now send in a new muxctrl
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnUpdate, (uint8_t) 1 );
    }

    void testTipNegUpdateMUXCTRLV6() {
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);
        
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        // delete the old MC ack
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;

        // now send in a new muxctrl
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnUpdate, (uint8_t) 1 );
    }

    void testTipNegUpdateMO() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        // delete the old MO ack
        delete xmit->rxACKMO;
        xmit->rxACKMO = NULL;
        
        // now send in a new mediaopts
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        CPacketBufferData buffer;

        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnUpdate, (uint8_t) 1 );

        delete mo;
    }

    void testTipNegUpdateDelay() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        // setup to not send an ACK
        callback->mnUpdateReturn = false;

        // clean up old MO ack data
        delete xmit->rxACKMO;
        xmit->rxACKMO = NULL;
        
        // now send in a new mediaopts
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        CPacketBufferData buffer;
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        CPPUNIT_ASSERT_EQUAL( callback->mnUpdate, (uint8_t) 1 );

        // verify ack did not go out
        CPPUNIT_ASSERT( xmit->rxACKMO == NULL );

        // ack now
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnUpdatePacket, VIDEO), TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testSpiMapUnsecure() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);
	
        // send in a SPIMAP, verify that the callback is not invoked
        CRtcpAppSpiMapPacket spimap;
        spimap.SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        spimap.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityKeyUpdate, (uint8_t) 0 );
        CPPUNIT_ASSERT( xmit->rxACKSPIMAP == NULL );
    }

    void testSpiMapSecure() {
        // make both systems secure
        am->GetTipSystem().SetSecurityState(true);
        rs->SetSecurityState(true);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);
	
        // send in a SPIMAP, verify that the callback is invoked
        CRtcpAppSpiMapPacket spimap;
        spimap.SetNtpTime(GetNtpTimestamp());
        spimap.SetSPI(0x1234);

        uint8_t salt[CRtcpAppSpiMapPacket::SPIMAP_SRTP_SALT_LENGTH] = {
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB
        };
        spimap.SetSrtpSalt(salt);

        uint8_t kek[CRtcpAppSpiMapPacket::SPIMAP_KEK_LENGTH] = {
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF
        };
        spimap.SetKek(kek);

        CPacketBufferData buffer;
        spimap.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityKeyUpdate, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnSecuritySPI, spimap.GetSPI() );
        CPPUNIT_ASSERT_EQUAL( memcmp(callback->mnSecuritySalt, salt, sizeof(salt)), 0 );
        CPPUNIT_ASSERT_EQUAL( memcmp(callback->mnSecurityKek, kek, sizeof(kek)), 0 );
        CPPUNIT_ASSERT( xmit->rxACKSPIMAP != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKSPIMAP->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testSpiMapSecureDelay() {
        // make both systems secure
        am->GetTipSystem().SetSecurityState(true);
        rs->SetSecurityState(true);

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);
	
        // send in a SPIMAP, verify that the callback is invoked
        CRtcpAppSpiMapPacket spimap;
        spimap.SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        spimap.Pack(buffer);

        // setup for the callback to return false and delay the ack
        callback->mnSecurityKeyUpdateReturn = false;

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityKeyUpdate, (uint8_t) 1 );
        CPPUNIT_ASSERT( xmit->rxACKSPIMAP == NULL );

        // send the ack now
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnSecurityPacket, VIDEO), TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKSPIMAP != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKSPIMAP->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testSpiMapEarly() {
        // make both systems secure
        am->GetTipSystem().SetSecurityState(true);
        rs->SetSecurityState(true);

        // send in a SPIMAP, verify that the callback is not invoked
        CRtcpAppSpiMapPacket spimap;
        spimap.SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        spimap.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityKeyUpdate, (uint8_t) 0 );
        CPPUNIT_ASSERT( xmit->rxACKSPIMAP == NULL );
    }

    void testNotifySecure() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        CRtcpAppNotifyPacket notify;
        notify.SetNtpTime(GetNtpTimestamp());

        uint8_t data = 1;
        notify.AddTLV(CRtcpAppNotifyPacket::SECURITYICON, &data, sizeof(data));

        CPacketBufferData buffer;
        notify.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityStateUpdate, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityStateFlag, true );
        CPPUNIT_ASSERT( xmit->rxACKNOTIFY != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKNOTIFY->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }
    
    void testNotifyNonSecure() {
        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        // finish both local and remote tip negotiation
        doTipNegLocal(VIDEO);
        doTipNegRemote(VIDEO);

        CRtcpAppNotifyPacket notify;
        notify.SetNtpTime(GetNtpTimestamp());

        uint8_t data = 0;
        notify.AddTLV(CRtcpAppNotifyPacket::SECURITYICON, &data, sizeof(data));

        CPacketBufferData buffer;
        notify.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityStateUpdate, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityStateFlag, false );
        CPPUNIT_ASSERT( xmit->rxACKNOTIFY != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKNOTIFY->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testNotifyEarly() {
        CRtcpAppNotifyPacket notify;
        notify.SetNtpTime(GetNtpTimestamp());

        uint8_t data = 0;
        notify.AddTLV(CRtcpAppNotifyPacket::SECURITYICON, &data, sizeof(data));

        CPacketBufferData buffer;
        notify.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
	
        CPPUNIT_ASSERT_EQUAL( callback->mnSecurityStateUpdate, (uint8_t) 0 );
        CPPUNIT_ASSERT( xmit->rxACKNOTIFY == NULL );
    }   
    
    void doPresNeg(uint16_t rts_vid_pos, uint16_t ack_vid_pos = 0) {
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify REQTOSEND packet went out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        CRtcpAppReqToSendPacket* rts = dynamic_cast<CRtcpAppReqToSendPacket*>(xmit->rxRTS);
        CPPUNIT_ASSERT( rts != NULL );

        // verify REQTOSEND has the right options
        CPPUNIT_ASSERT_EQUAL( rts->GetFlags(), (uint32_t) CRtcpAppReqToSendPacketBase::REQTOSEND_START );
        CPPUNIT_ASSERT_EQUAL( rts->GetVideoPos(), rts_vid_pos );
        CPPUNIT_ASSERT_EQUAL( rts->GetAudioPos(), (uint16_t) 0 );
        
        // ack RTS
        CRtcpAppReqToSendAckPacket ack(*rts);

        if (ack_vid_pos != 0) {
            ack.SetVideoPos(ack_vid_pos);
        }
        
        CPacketBufferData buffer;
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // callback will be invoked once per ack'ed video position
        uint8_t exp_count = 0;
        if (ack.GetVideoPos() & PositionToMask(POS_VIDEO_AUX_1_5FPS)) {
            exp_count++;
        }
        if (ack.GetVideoPos() & PositionToMask(POS_VIDEO_AUX_30FPS)) {
            exp_count++;
        }
        
        // verify callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, exp_count );
    }
    
    void testTipPresStartPreTipNeg() {
        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_ERROR );
    }

    void testTipPresStart() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testTipPresStart2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }
    
    void testTipPresStart3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        doPresNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                  PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }
    
    void testTipPresStart4() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        doPresNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                  PositionToMask(POS_VIDEO_AUX_30FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }

    void testTipPresStart5() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }
    
    void testTipPresStart6() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }
    
    void testTipPresStart7() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testTipPresStart8() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_30FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_30FPS_ONLY );
    }
    
    void testTipPresStart9() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testTipPresStart10() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doPresNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                  PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }

    void testTipPresStartUpdate1() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify REQTOSEND packet went out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // delete old RTS
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;

        // update with a new MO for 5FPS before the RTS-ACK arrives
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        delete mo;

        // send out packets
        am->DoPeriodicActivity();
        
        // verify that a new RTS went out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // ack RTS
        buffer.Reset();
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        ack.SetVideoPos(1 << POS_VIDEO_AUX_1_5FPS);
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void doPresNegUpdate(CTipSystem::PresentationFrameRate newRate, bool newRTS,
                         uint16_t rts_vid_pos, uint16_t ack_vid_pos) {
        // clean up old packets
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;
        delete xmit->rxACKMO;
        xmit->rxACKMO = NULL;

        // clean up old state
        callback->mnLocalPresStart = 0;
        callback->mnLocalPresStart4 = 0;
        callback->mnLocalPresStart5 = 0;
        
        // now we will send in a new MO that changes the required frame rate
        rs->SetPresentationFrameRate(newRate);

        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKMO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        delete mo;

        // verify that local presentation was stopped
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStop, (uint8_t) 1 );

        // send out packets
        am->DoPeriodicActivity();

        if (newRTS) {
            // verify that a new RTS went out
            CPPUNIT_ASSERT( xmit->rxRTS != NULL );

            CRtcpAppReqToSendPacket* rts = dynamic_cast<CRtcpAppReqToSendPacket*>(xmit->rxRTS);
            CPPUNIT_ASSERT( rts != NULL );
            CPPUNIT_ASSERT_EQUAL( rts->GetFlags(), (uint32_t) CRtcpAppReqToSendPacketBase::REQTOSEND_START );
            CPPUNIT_ASSERT_EQUAL( rts->GetVideoPos(), rts_vid_pos );
            CPPUNIT_ASSERT_EQUAL( rts->GetAudioPos(), (uint16_t) 0 );

            // ack RTS
            buffer.Reset();
            CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
            ack.SetVideoPos(ack_vid_pos);
            ack.Pack(buffer);
            CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                                  TIP_OK );
        } else {
            // verify that a new RTS did not go out
            CPPUNIT_ASSERT( xmit->rxRTS == NULL );
        }

        uint8_t exp_count = 0;
        if (ack_vid_pos & PositionToMask(POS_VIDEO_AUX_1_5FPS)) {
            exp_count++;
        }
        if (ack_vid_pos & PositionToMask(POS_VIDEO_AUX_30FPS)) {
            exp_count++;
        }
        
        // verify callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, exp_count );
    }
    
    void testTipPresStartUpdate2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_30FPS));

        doPresNegUpdate(CTipSystem::PRES_5FPS_ONLY, true,
                        PositionToMask(POS_VIDEO_AUX_1_5FPS),
                        PositionToMask(POS_VIDEO_AUX_1_5FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }

    void testTipPresStartUpdate3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_30FPS));

        doPresNegUpdate(CTipSystem::PRES_1FPS_ONLY, true,
                        PositionToMask(POS_VIDEO_AUX_1_5FPS),
                        PositionToMask(POS_VIDEO_AUX_1_5FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }

    void testTipPresStartUpdate4() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        doPresNegUpdate(CTipSystem::PRES_1FPS_ONLY, false,
                        PositionToMask(POS_VIDEO_AUX_1_5FPS),
                        PositionToMask(POS_VIDEO_AUX_1_5FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }

    void testTipPresStartUpdate5() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        doPresNegUpdate(CTipSystem::PRES_30FPS_ONLY, true,
                        PositionToMask(POS_VIDEO_AUX_30FPS),
                        PositionToMask(POS_VIDEO_AUX_30FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5FPS, CTipSystem::PRES_30FPS_RATE );
    }

    void testTipPresStartUpdate6() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        doPresNegUpdate(CTipSystem::PRES_30FPS_ONLY, true,
                        PositionToMask(POS_VIDEO_AUX_30FPS),
                        PositionToMask(POS_VIDEO_AUX_30FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart5FPS, CTipSystem::PRES_30FPS_RATE );
    }

    void testTipPresStartUpdate7() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        doPresNegUpdate(CTipSystem::PRES_5FPS_ONLY, false,
                        PositionToMask(POS_VIDEO_AUX_1_5FPS),
                        PositionToMask(POS_VIDEO_AUX_1_5FPS));

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }

    void testTipPresStopInv() {
        CPPUNIT_ASSERT_EQUAL( am->StopPresentation(), TIP_ERROR );
    }

    void testTipPresStop() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // reset pointer as we should get a new REQTOSEND
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;
        
        CPPUNIT_ASSERT_EQUAL( am->StopPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify REQTOSEND packet went out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );
    }

    void testTipPresTimeout() {
        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // spin through all the retransmissions of REQTOSEND
        for (uint32_t i = 0; i <= DEFAULT_RETRANS_LIMIT; i++) {
            usleep((am->GetIdleTime() * 1000));
            am->DoPeriodicActivity();
        }

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresFailed, (uint8_t) 1 );
    }

    void testTipPresRemoteStartPreTipNeg() {
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        
        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 0 );
    }

    void testTipPresRemoteStartInvalid() {
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 0 );
    }
    
    void testTipPresRemoteStartInvalid2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 0 );
    }
    
    void testTipPresRemoteStartInvalid3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_30FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 0 );
    }

    void doPresRemoteNeg(uint16_t rts_vid_pos = 0, uint16_t rts_ack_vid_pos = 0,
                         MediaType mType = VIDEO)
    {
        // handle being called more than once, only do neg the first time
        if (callback->mnLastAckRx == 0) {
            doTipNeg(VIDEO);
            doTipNeg(AUDIO);
        }

        // clean up old stuff
        delete xmit->rxACKRTS;
        xmit->rxACKRTS = NULL;

        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        callback->mnRemotePresStart5 = 0;
        
        CRtcpAppReqToSendPacket* rts = rs->MapToReqToSend();
        rts->SetNtpTime(GetNtpTimestamp());
        rts->SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);

        if (rts_vid_pos != 0) {
            rts->SetVideoPos(rts_vid_pos);
        }

        CPacketBufferData buffer;
        rts->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), mType),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(mType) );

        CRtcpAppReqToSendAckPacket* ack = dynamic_cast<CRtcpAppReqToSendAckPacket*>(xmit->rxACKRTS);
        CPPUNIT_ASSERT( ack != NULL );
        CPPUNIT_ASSERT_EQUAL( ack->GetFlags(), (uint32_t) CRtcpAppReqToSendPacketBase::REQTOSEND_START );

        if (rts_ack_vid_pos == 0) {
            CPPUNIT_ASSERT_EQUAL( ack->GetVideoPos(), rts->GetVideoPos() );
        } else {
            CPPUNIT_ASSERT_EQUAL( ack->GetVideoPos(), rts_ack_vid_pos );
        }
        CPPUNIT_ASSERT_EQUAL( ack->GetAudioPos(), (uint16_t) 0 );

        uint8_t exp_count = 0;
        if (ack->GetVideoPos() & PositionToMask(POS_VIDEO_AUX_1_5FPS)) {
            exp_count++;
        }
        if (ack->GetVideoPos() & PositionToMask(POS_VIDEO_AUX_30FPS)) {
            exp_count++;
        }
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, exp_count );
        delete rts;
    }
    
    void testTipPresRemoteStart() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testTipPresRemoteStart2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        doPresRemoteNeg(0, PositionToMask(POS_VIDEO_AUX_30FPS));

        // endpoints can only handle a single presentation stream at a
        // time, verify only the highest rate is started.
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }

    void testTipPresRemoteStart3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testTipPresRemoteStart4() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        am->GetTipSystem().SetMCUState(true);
        
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        doPresRemoteNeg(0, (PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)));

        // MCUs can handle multiple simultaneous presentation streams
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );
    }

    void testTipPresRemoteStart5() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        doPresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_30FPS_ONLY );
    }

    void testTipPresRemoteStart6() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testTipPresRemoteStart7() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        // send in an RTS, note this is a bit off as its only
        // advertising 5 FPS yet our system is configured for 30FPS.
        // even so the end result should be 5FPS enabled presentation.
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_30FPS_ONLY );
    }

    void testTipPresRemoteStart8() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                        PositionToMask(POS_VIDEO_AUX_30FPS));
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testTipPresRemoteStart9() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testTipPresRemoteStart10() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresRemoteNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                        PositionToMask(POS_VIDEO_AUX_30FPS));
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testTipPresRemoteStartMOUpdate1() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg();

        // clean up old state info
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        
        // now send in an MO update changing the frame rate to 1FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify presentation was stopped and then restarted as 1FPS
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }
    
    void testTipPresRemoteStartMOUpdate2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        doPresRemoteNeg();
        
        // clean up old state info
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        
        // now send in an MO update changing the frame rate to 5FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify presentation was stopped and then restarted as 5FPS
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }
    
    void testTipPresRemoteStartMOUpdate3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg();

        // clean up old state info
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        
        // now send in an MO update changing the frame rate to 1FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify presentation was stopped and then restarted as 5FPS
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }
    
    void testTipPresRemoteStartMOUpdate4() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresRemoteNeg((PositionToMask(POS_VIDEO_AUX_1_5FPS) | PositionToMask(POS_VIDEO_AUX_30FPS)),
                        PositionToMask(POS_VIDEO_AUX_30FPS));

        // clean up old state info
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart5 = 0;
        
        // now send in an MO update changing the frame rate to 1FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify presentation was NOT stopped
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 0 );
    }

    void testTipPresRemoteStartRTSUpdate1() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        // send in an RTS that advertises 5+30
        doPresRemoteNeg(0, PositionToMask(POS_VIDEO_AUX_30FPS));

        // now send in a new RTS that only advertises 5 FPS
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));
    }

    void testTipPresRemoteStartRTSUpdate2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        // send in an RTS that advertises 5
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // now send in a new RTS that advertises 5+30
        doPresRemoteNeg(0, PositionToMask(POS_VIDEO_AUX_30FPS));
    }

    void testTipPresRemoteStartRTSUpdate3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        // send in an RTS that advertises 5
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        // now send in a new RTS that only advertises 30
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_30FPS), PositionToMask(POS_VIDEO_AUX_30FPS));
    }

    void testTipPresRemoteStartRTSUpdate4() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);

        // send in an RTS that advertises 30
        doPresRemoteNeg(PositionToMask(POS_VIDEO_AUX_30FPS));

        // now send in a new RTS that advertises 5+30
        doPresRemoteNeg(0, PositionToMask(POS_VIDEO_AUX_30FPS));
    }

    void testTipPresRemoteStartDup() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        uint64_t ntptime = GetNtpTimestamp();
        
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(ntptime);
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS) | (1 << POS_VIDEO_AUX_30FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CRtcpAppReqToSendAckPacket* ack = dynamic_cast<CRtcpAppReqToSendAckPacket*>(xmit->rxACKRTS);
        CPPUNIT_ASSERT( ack != NULL );
        CPPUNIT_ASSERT_EQUAL( ack->GetFlags(), (uint32_t) CRtcpAppReqToSendPacketBase::REQTOSEND_START );
        CPPUNIT_ASSERT_EQUAL( ack->GetVideoPos(), (uint16_t) (1 << POS_VIDEO_AUX_30FPS) );
        CPPUNIT_ASSERT_EQUAL( ack->GetAudioPos(), (uint16_t) 0 );

        // endpoints can only handle a single presentation stream at a
        // time, verify only the highest rate is started.
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_AND_30FPS );

        delete xmit->rxACKRTS;
        xmit->rxACKRTS = NULL;
        
        // send in a duplicate RTS and verify it gets ACK'ed correctly
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        ack = dynamic_cast<CRtcpAppReqToSendAckPacket*>(xmit->rxACKRTS);
        CPPUNIT_ASSERT( ack != NULL );
        CPPUNIT_ASSERT_EQUAL( ack->GetFlags(), (uint32_t) CRtcpAppReqToSendPacketBase::REQTOSEND_START );
        CPPUNIT_ASSERT_EQUAL( ack->GetVideoPos(), (uint16_t) (1 << POS_VIDEO_AUX_30FPS) );
        CPPUNIT_ASSERT_EQUAL( ack->GetAudioPos(), (uint16_t) 0 );
    }

    void testTipPresRemoteStartDelay() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // setup to not ack
        callback->mnRemotePresStartReturn = false;
        
        CRtcpAppReqToSendPacket* rts = rs->MapToReqToSend();
        rts->SetNtpTime(GetNtpTimestamp());
        rts->SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        

        CPacketBufferData buffer;
        rts->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete rts;

        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );
        CPPUNIT_ASSERT( callback->mnRemotePresStartPacket != NULL );

        // endpoints can only handle a single presentation stream at a
        // time, verify only the highest rate is started.
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 0 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart5FPS, CTipSystem::PRES_30FPS_RATE );

        // send out delayed ACK
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnRemotePresStartPacket, VIDEO), TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testTipPresAudio() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        // send in on AUDIO channel
        doPresRemoteNeg(0, 0, AUDIO);
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testTipPresRemoteStop() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );

        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_STOP);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        // delete ACK so we can see if we get a new one
        delete xmit->rxACKRTS;
        xmit->rxACKRTS = NULL;

        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
    }

    void testTipPresOverride() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doPresNeg(PositionToMask(POS_VIDEO_AUX_1_5FPS));

        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }

    void testTipPresOverrideUpdate1() {
        // use testTipPresOverride (5FPS local and remote) to get things initialized
        testTipPresOverride();
        
        // clean up old state info
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        
        // now send in an MO update changing the frame rate to 1FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify presentation was stopped and then restarted as 1FPS
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }
    
    void testTipPresOverrideRelease() {
        // use logic in above test case to set things up
        testTipPresOverride();

        // clean up old stuff
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;
        delete xmit->rxACKRTS;
        xmit->rxACKRTS = NULL;
        callback->mnLocalPresStart = 0;
        callback->mnLocalPresStart4 = 0;
        callback->mnLocalPresStart5 = 0;
        
        // now give up remote control
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_STOP);
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify REQTOSEND packet went out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // ack RTS
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        buffer.Reset();
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }

    void testTipPresOverrideLocalRelease() {
        // use logic in above test case to set things up
        testTipPresOverride();

        // clean up old stuff
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;
        delete xmit->rxACKRTS;
        xmit->rxACKRTS = NULL;
        
        // give up local control
        am->StopPresentation();
        am->DoPeriodicActivity();

        // verify a new REQTOSEND goes out
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // ack RTS
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        CPacketBufferData buffer;
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // clean up old stuff
        delete xmit->rxRTS;
        xmit->rxRTS = NULL;
        
        // give up remote control, verify local does not assert
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_STOP);
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        buffer.Reset();
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify REQTOSEND packet did not go out
        CPPUNIT_ASSERT( xmit->rxRTS == NULL );
    }

    void testTipPresConflictRemoteWinnerMCU() {
        // setup local as an MCU and presentation
        am->GetTipSystem().SetMCUState(true);
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should win so we should get an ACK and a callback
        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1);
    }

    void testTipPresConflictLocalWinnerMCU() {
        // add shared position to local system
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        // make remote system an mcu
        rs->SetMCUState(true);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(GetNtpTimestamp());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should lose so we should not get an ACK
        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );

        // send in an ACK for the local RTS
        buffer.Reset();
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
    }

    void testTipPresConflictRemoteWinnerNTP() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with a smaller NTP time
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime((xmit->rxRTS->GetNtpTime() - 1));
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should win so we should get an ACK and a callback
        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1);
    }

    void testTipPresConflictLocalWinnerNTP() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with a larger NTP time
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime((xmit->rxRTS->GetNtpTime() + 1));
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should lose so we should not get an ACK
        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );

        // send in an ACK for the local RTS
        buffer.Reset();
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
    }

    void testTipPresConflictRemoteWinnerSSRC() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with a smaller SSRC
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(xmit->rxRTS->GetNtpTime());
        rts.SetSSRC((xmit->rxRTS->GetSSRC() - 1));
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should win so we should get an ACK and a callback
        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1);
    }

    void testTipPresConflictLocalWinnerSSRC() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with a larger SSRC
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(xmit->rxRTS->GetNtpTime());
        rts.SetSSRC((xmit->rxRTS->GetSSRC() + 1));
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        // the remote should lose so we should not get an ACK
        CPPUNIT_ASSERT( xmit->rxACKRTS == NULL );

        // send in an ACK for the local RTS
        buffer.Reset();
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
    }

    void testTipPresConflictRemoteWinnerUser() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // setup for user conflict callback to return remote winner
        callback->mnResolvePresentationConflictReturn = false;
        
        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with equal NTP and SSRC
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(xmit->rxRTS->GetNtpTime());
        rts.SetSSRC(xmit->rxRTS->GetSSRC());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // verify user decision callback invoked
        CPPUNIT_ASSERT_EQUAL( callback->mnResolvePresentationConflict, (uint8_t) 1 );

        // the remote should win so we should get an ACK and a callback
        CPPUNIT_ASSERT( xmit->rxACKRTS != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKRTS->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1);
    }

    void testTipPresConflictLocalWinnerUser() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // setup for user conflict callback to return remote winner
        callback->mnResolvePresentationConflictReturn = true;
        
        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxRTS != NULL );

        // send in a remote RTS with equal NTP and SSRC
        CRtcpAppReqToSendPacket rts;
        rts.SetNtpTime(xmit->rxRTS->GetNtpTime());
        rts.SetSSRC(xmit->rxRTS->GetSSRC());
        rts.SetFlags(CRtcpAppReqToSendPacketBase::REQTOSEND_START);        
        rts.SetVideoPos((1 << POS_VIDEO_AUX_1_5FPS));

        CPacketBufferData buffer;
        rts.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // verify user decision callback invoked
        CPPUNIT_ASSERT_EQUAL( callback->mnResolvePresentationConflict, (uint8_t) 1 );

        // user decision callback returned true so we should assert
        // control.  ack and make sure that is true.
        buffer.Reset();
        CRtcpAppReqToSendAckPacket ack(* (CRtcpAppReqToSendPacket*) xmit->rxRTS);
        ack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
    }

    void doV6PresNeg(bool expectCallback = true) {
        CPacketBufferData buffer;

        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old video MUXCTRL
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;
        
        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify MUXCTRL packet went out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );

        // verify presentation position is set
        CRtcpAppMuxCtrlPacket* mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[VIDEO]);
        CPPUNIT_ASSERT_EQUAL( (uint16_t) (mc->GetXmitPositions() & PositionToMask(POS_VIDEO_AUX_1_5FPS)), PositionToMask(POS_VIDEO_AUX_1_5FPS) );

        // ack MUXCTRL
        CRtcpTipAckPacket mcack(*xmit->rxMC[VIDEO]);
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        if (expectCallback) {
            // verify callback
            CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
        } else {
            // verify no callback
            CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 0);
        }
    }
    
    // V6 presentation tests
    void testV6TipPresStart() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresStart2() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doV6PresNeg();

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_30FPS_ONLY );
        
        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_30FPS_RATE );
    }

    void testV6TipPresStart3() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresStart4() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresStart5() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresStart6() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresStart7() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresStart8() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresStart9() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_1FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresStartUpdate1() {
        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old MUXCTRL
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify MUXCTRL went out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );
        
        // delete old MUXCTRL
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        // update with a new MO for 5FPS before the RTS-ACK arrives
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // send out packets
        am->DoPeriodicActivity();
        
        // verify that a new MUXCTRL went out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );

        // ack MUXCTRL
        buffer.Reset();
        CRtcpTipAckPacket mcack(*xmit->rxMC[VIDEO]);
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresStartUpdate2() {
        // set things up just like in testV6TipPresStart2
        testV6TipPresStart2();

        // delete old MUXCTRL
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        // clear out old cached callback data
        callback->mnLocalPresStart = 0;
        callback->mnLocalPresStart4 = 0;
        
        // now send in a new MO
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;
        
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        // verify negotiated object is correct
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresStop() {
        doV6PresNeg();

        // reset pointer as we should get a new MUXCTRL
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        // stop presentation
        CPPUNIT_ASSERT_EQUAL( am->StopPresentation(), TIP_OK );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify MUXCTRL packet went out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );

        // verify position is not set
        CRtcpAppMuxCtrlPacket* mc = dynamic_cast<CRtcpAppMuxCtrlPacket*>(xmit->rxMC[VIDEO]);
        CPPUNIT_ASSERT_EQUAL( (mc->GetXmitPositions() & PositionToMask(POS_VIDEO_AUX_1_5FPS)), 0 );
    }
    
    void testV6TipPresTimeout() {
        // make retransmission fast to speed up the test
        am->SetRetransmissionInterval(1);
        
        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // kick off presentation
        CPPUNIT_ASSERT_EQUAL( am->StartPresentation(), TIP_OK );

        // spin through all the retransmissions of MUXCTRL
        for (uint32_t i = 0; i <= DEFAULT_RETRANS_LIMIT; i++) {
            usleep((am->GetIdleTime() * 1000));
            am->DoPeriodicActivity();
        }

        // verify that we got the timeout callback
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresFailed, (uint8_t) 1 );
    }

    void doV6PresRemoteNeg() {
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old ACKs
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
    }
    
    void testV6TipPresRemoteStart() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doV6PresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testV6TipPresRemoteStart2() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);

        doV6PresRemoteNeg();
        
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_30FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_30FPS_ONLY );
    }
    
    void testV6TipPresRemoteStart3() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }
    
    void testV6TipPresRemoteStart4() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresRemoteStart5() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresRemoteStart6() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresRemoteStart7() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresRemoteStart8() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresRemoteStart9() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        
        doV6PresRemoteNeg();

        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_1FPS_ONLY );
    }

    void testV6TipPresRemoteUpdate1() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        doV6PresRemoteNeg();

        // clear out old data
        callback->mnRemotePresStart = 0;
        callback->mnRemotePresStart4 = 0;
        
        // send in a new MO with 1FPS
        rs->SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

        CPacketBufferData buffer;
        CRtcpAppMediaoptsPacket* mo = rs->MapToMediaOpts(VIDEO);
        mo->SetNtpTime(GetNtpTimestamp());
        mo->Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mo;

        // verify the presentation was stopped and restarted
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_1FPS_RATE );
    }
    
    void testV6TipPresRemoteStartDelay() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_30FPS_ONLY);
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);
        
        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old ACKs
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        // setup to not ack
        callback->mnRemotePresStartReturn = false;
        
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC == NULL );
        CPPUNIT_ASSERT( callback->mnRemotePresStartPacket != NULL );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_30FPS_RATE );

        // send out delayed ACK
        CPPUNIT_ASSERT_EQUAL( am->SendDelayedAck(callback->mnRemotePresStartPacket, VIDEO), TIP_OK );
        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testV6TipPresRemoteStop() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);
        
        // set local and remote system version to 6
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old ACKs
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );

        // delete old ACKs
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        rs->RemoveTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );
    }

    void testV6TipPresOverride() {
        // setup presentation
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);
        
        doV6PresNeg();

        // verify callback data is correct
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());

        CPacketBufferData buffer;
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStop, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );

        CPPUNIT_ASSERT_EQUAL( am->GetTipNegotiatedSystem().GetPresentationFrameRate(),
                              CTipSystem::PRES_5FPS_ONLY );
    }

    void testV6TipPresOverrideRelease() {
        // use logic in above test case to set things up
        testV6TipPresOverride();

        // delete old packets
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;
        
        // now give up remote control
        rs->RemoveTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        CPacketBufferData buffer;
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );

        // for V6 control is automatic, should start right away
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 2 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }

    void testV6TipPresOverrideLocalRelease() {
        // use logic in above test case to set things up
        testV6TipPresOverride();

        // clean up old stuff
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        // give up local control
        am->StopPresentation();
        am->DoPeriodicActivity();

        // verify a new MUXCTRL goes out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );

        // ack MUXCTRL
        CRtcpTipAckPacket mcack(*xmit->rxMC[VIDEO]);
        CPacketBufferData buffer;
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        // clean up old stuff
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;

        // remove gives up control
        rs->RemoveTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(GetNtpTimestamp());
        buffer.Reset();
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStop, (uint8_t) 1 );

        // send out packets
        am->DoPeriodicActivity();
        
        // verify MUXCTRL packet did not go out
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] == NULL );
    }

    void testV6TipPresConflictRemoteWinner() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old packets
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        // setup for user conflict callback to return remote winner
        callback->mnResolvePresentationConflictReturn = false;

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );
        
        // send in a remote MUXCTRL
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        CPacketBufferData buffer;
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        delete mc;
        
        // verify user decision callback invoked
        CPPUNIT_ASSERT_EQUAL( callback->mnResolvePresentationConflict, (uint8_t) 1 );

        // the remote should win so we should get an ACK and a callback
        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnRemotePresStart, (uint8_t) 1);
    }
    
    void testV6TipPresConflictLocalWinner() {
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        am->GetTipSystem().SetTipVersion(TIP_V6);
        rs->SetTipVersion(TIP_V6);
        rs->AddTransmitter(VIDEO, POS_VIDEO_AUX_1_5FPS);

        doTipNeg(VIDEO);
        doTipNeg(AUDIO);

        // delete old packets
        delete xmit->rxMC[VIDEO];
        xmit->rxMC[VIDEO] = NULL;
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;
        
        // setup for user conflict callback to return local winner
        callback->mnResolvePresentationConflictReturn = true;

        // send out RTS but do not ack it
        am->StartPresentation();
        am->DoPeriodicActivity();
        CPPUNIT_ASSERT( xmit->rxMC[VIDEO] != NULL );
        
        // send in a remote MUXCTRL
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        CPacketBufferData buffer;
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);
        
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        delete mc;
        
        // verify user decision callback invoked
        CPPUNIT_ASSERT_EQUAL( callback->mnResolvePresentationConflict, (uint8_t) 1 );

        // user decision callback returned true so we should assert
        // control.  ack and make sure that is true.
        buffer.Reset();
        CRtcpTipAckPacket mcack(*xmit->rxMC[VIDEO]);
        mcack.Pack(buffer);
        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1);
    }

    void testV6TipPresMCULocalStart() {
        am->GetTipSystem().SetTipVersion(TIP_V6);
        am->GetTipSystem().SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);

        rs->SetTipVersion(TIP_V6);
        rs->SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
        rs->SetMCUState(true);

        doV6PresNeg(false);

        // send in remote MUXCTRL granting permission to send
        rs->AddReceiver(VIDEO, POS_VIDEO_AUX_1_5FPS);
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        CPacketBufferData buffer;
        mc->SetNtpTime(GetNtpTimestamp());
        mc->Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        delete mc;

        // verify ACK
        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4, (uint8_t) 1 );
        CPPUNIT_ASSERT_EQUAL( callback->mnLocalPresStart4FPS, CTipSystem::PRES_5FPS_RATE );
    }
    
    void testOldPacketIgnored() {
        CPacketBufferData buffer;

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        doTipNegRemote(VIDEO);

        // now we send in an "old" MUXCTRL and verify it is not acked
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime((xmit->rxACKMC->GetNtpTime() - 1));
        mc->Pack(buffer);
        delete mc;

        // delete old ack from earlier tip neg
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );
        
        CPPUNIT_ASSERT( xmit->rxACKMC == NULL );
    }

    void testDupPacketAcked() {
        CPacketBufferData buffer;

        // start tip negotiation
        CPPUNIT_ASSERT_EQUAL( am->StartTipNegotiate(VIDEO), TIP_OK );

        doTipNegRemote(VIDEO);

        // now we send in an "duplicate" MUXCTRL and verify it is acked
        CRtcpAppMuxCtrlPacketBase* mc = rs->MapToMuxCtrl(VIDEO);
        mc->SetNtpTime(xmit->rxACKMC->GetNtpTime());
        mc->Pack(buffer);
        delete mc;

        // delete old ack from earlier tip neg
        delete xmit->rxACKMC;
        xmit->rxACKMC = NULL;

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKMC != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKMC->GetSSRC(), am->GetRTCPSSRC(VIDEO) );
    }

    void testEcho() {
        CRtcpAppEchoPacket packet;
        CPacketBufferData buffer;

        packet.SetNtpTime(GetNtpTimestamp());
        packet.Pack(buffer);

        CPPUNIT_ASSERT_EQUAL( am->ReceivePacket(buffer.GetBuffer(), buffer.GetBufferSize(), VIDEO),
                              TIP_OK );

        CPPUNIT_ASSERT( xmit->rxACKECHO != NULL );
        CPPUNIT_ASSERT_EQUAL( xmit->rxACKECHO->GetSSRC(), am->GetRTCPSSRC(VIDEO) );

        CRtcpAppEchoPacket* ack = dynamic_cast<CRtcpAppEchoPacket*>(xmit->rxACKECHO);
        CPPUNIT_ASSERT( ack != NULL );
        CPPUNIT_ASSERT_EQUAL( ack->GetNtpTime(), packet.GetNtpTime() );
        CPPUNIT_ASSERT( ack->GetRcvNtpTime() >= packet.GetNtpTime() );
    }
    
    CPPUNIT_TEST_SUITE( CTipTest );
    CPPUNIT_TEST( testCallback );
    CPPUNIT_TEST( testTipNegInvalid );
    CPPUNIT_TEST( testTipNegDouble );
    CPPUNIT_TEST( testStopTipNegInvalid );
    CPPUNIT_TEST( testReceiveAckUnknown );
    CPPUNIT_TEST( testReceiveInvalid );
    CPPUNIT_TEST( testReceiveInvalid2 );
    CPPUNIT_TEST( testDelayedAckInvalid );
    CPPUNIT_TEST( testSSRCEndZero );
    CPPUNIT_TEST( testSetSSRC );
    CPPUNIT_TEST( testTipNegLocal );
    CPPUNIT_TEST( testTipNegRemote );
    CPPUNIT_TEST( testTipNegRemoteDelay );
    CPPUNIT_TEST( testTipNegRemoteDelayOld );
    CPPUNIT_TEST( testTipNegLocalTimeout );
    CPPUNIT_TEST( testTipNegLocalTimeout2 );
    CPPUNIT_TEST( testTipNegRemoteTimeout );
    CPPUNIT_TEST( testTipNegRemoteTimeout2 );
    CPPUNIT_TEST( testTipNegEarly );
    CPPUNIT_TEST( testTipNegIncompatibleMin );
    CPPUNIT_TEST( testTipNegMismatchRemoteLower );
    CPPUNIT_TEST( testTipNegMismatchRemoteLowerDelay );
    CPPUNIT_TEST( testTipNegMismatchRemoteHigher );
    CPPUNIT_TEST( testTipNegMismatchRemoteHigherTimeout );
    CPPUNIT_TEST( testTipNegReinviteRemoteNtpHigher );
    CPPUNIT_TEST( testTipNegReinviteRemoteNtpLower );
    CPPUNIT_TEST( testTipNegReinviteLocalMCU );
    CPPUNIT_TEST( testTipNegReinviteRemoteMCU );
    CPPUNIT_TEST( testTipNegReinviteV6 );
    CPPUNIT_TEST( testTipNegUpdateMUXCTRL );
    CPPUNIT_TEST( testTipNegUpdateMUXCTRLV6 );
    CPPUNIT_TEST( testTipNegUpdateMO );
    CPPUNIT_TEST( testTipNegMismatchAllMedia );
    CPPUNIT_TEST( testTipNegUpdateDelay );
    CPPUNIT_TEST( testSpiMapUnsecure );
    CPPUNIT_TEST( testSpiMapSecure );
    CPPUNIT_TEST( testSpiMapSecureDelay );
    CPPUNIT_TEST( testSpiMapEarly );
    CPPUNIT_TEST( testNotifySecure );
    CPPUNIT_TEST( testNotifyNonSecure );
    CPPUNIT_TEST( testNotifyEarly );
    CPPUNIT_TEST( testTipPresStartPreTipNeg );
    CPPUNIT_TEST( testTipPresStart );
    CPPUNIT_TEST( testTipPresStart2 );
    CPPUNIT_TEST( testTipPresStart3 );
    CPPUNIT_TEST( testTipPresStart4 );
    CPPUNIT_TEST( testTipPresStart5 );
    CPPUNIT_TEST( testTipPresStart6 );
    CPPUNIT_TEST( testTipPresStart7 );
    CPPUNIT_TEST( testTipPresStart8 );
    CPPUNIT_TEST( testTipPresStart9 );
    CPPUNIT_TEST( testTipPresStart10 );
    CPPUNIT_TEST( testTipPresStartUpdate1 );
    CPPUNIT_TEST( testTipPresStartUpdate2 );
    CPPUNIT_TEST( testTipPresStartUpdate3 );
    CPPUNIT_TEST( testTipPresStartUpdate4 );
    CPPUNIT_TEST( testTipPresStartUpdate5 );
    CPPUNIT_TEST( testTipPresStartUpdate6 );
    CPPUNIT_TEST( testTipPresStartUpdate7 );
    CPPUNIT_TEST( testTipPresTimeout );
    CPPUNIT_TEST( testTipPresStopInv );
    CPPUNIT_TEST( testTipPresStop );
    CPPUNIT_TEST( testTipPresRemoteStartPreTipNeg );
    CPPUNIT_TEST( testTipPresRemoteStartInvalid );
    CPPUNIT_TEST( testTipPresRemoteStartInvalid2 );
    CPPUNIT_TEST( testTipPresRemoteStartInvalid3 );
    CPPUNIT_TEST( testTipPresRemoteStart );
    CPPUNIT_TEST( testTipPresRemoteStart2 );
    CPPUNIT_TEST( testTipPresRemoteStart3 );
    CPPUNIT_TEST( testTipPresRemoteStart4 );
    CPPUNIT_TEST( testTipPresRemoteStart5 );
    CPPUNIT_TEST( testTipPresRemoteStart6 );
    CPPUNIT_TEST( testTipPresRemoteStart7 );
    CPPUNIT_TEST( testTipPresRemoteStart8 );
    CPPUNIT_TEST( testTipPresRemoteStart9 );
    CPPUNIT_TEST( testTipPresRemoteStart10 );
    CPPUNIT_TEST( testTipPresRemoteStartMOUpdate1 );
    CPPUNIT_TEST( testTipPresRemoteStartMOUpdate2 );
    CPPUNIT_TEST( testTipPresRemoteStartMOUpdate3 );
    CPPUNIT_TEST( testTipPresRemoteStartMOUpdate4 );
    CPPUNIT_TEST( testTipPresRemoteStartRTSUpdate1 );
    CPPUNIT_TEST( testTipPresRemoteStartRTSUpdate2 );
    CPPUNIT_TEST( testTipPresRemoteStartRTSUpdate3 );
    CPPUNIT_TEST( testTipPresRemoteStartRTSUpdate4 );
    CPPUNIT_TEST( testTipPresRemoteStartDup );
    CPPUNIT_TEST( testTipPresRemoteStartDelay );
    CPPUNIT_TEST( testTipPresRemoteStop );
    CPPUNIT_TEST( testTipPresAudio );
    CPPUNIT_TEST( testTipPresOverride );
    CPPUNIT_TEST( testTipPresOverrideUpdate1 );
    CPPUNIT_TEST( testTipPresOverrideRelease );
    CPPUNIT_TEST( testTipPresOverrideLocalRelease );
    CPPUNIT_TEST( testTipPresConflictRemoteWinnerMCU );
    CPPUNIT_TEST( testTipPresConflictLocalWinnerMCU );
    CPPUNIT_TEST( testTipPresConflictRemoteWinnerNTP );
    CPPUNIT_TEST( testTipPresConflictLocalWinnerNTP );
    CPPUNIT_TEST( testTipPresConflictRemoteWinnerSSRC );
    CPPUNIT_TEST( testTipPresConflictLocalWinnerSSRC );
    CPPUNIT_TEST( testTipPresConflictRemoteWinnerUser );
    CPPUNIT_TEST( testTipPresConflictLocalWinnerUser );
    CPPUNIT_TEST( testV6TipPresStart );
    CPPUNIT_TEST( testV6TipPresStart2 );
    CPPUNIT_TEST( testV6TipPresStart3 );
    CPPUNIT_TEST( testV6TipPresStart4 );
    CPPUNIT_TEST( testV6TipPresStart5 );
    CPPUNIT_TEST( testV6TipPresStart6 );
    CPPUNIT_TEST( testV6TipPresStart7 );
    CPPUNIT_TEST( testV6TipPresStart8 );
    CPPUNIT_TEST( testV6TipPresStart9 );
    CPPUNIT_TEST( testV6TipPresStartUpdate1 );
    CPPUNIT_TEST( testV6TipPresStartUpdate2 );
    CPPUNIT_TEST( testV6TipPresStop );
    CPPUNIT_TEST( testV6TipPresTimeout );
    CPPUNIT_TEST( testV6TipPresRemoteStart );
    CPPUNIT_TEST( testV6TipPresRemoteStart2 );
    CPPUNIT_TEST( testV6TipPresRemoteStart3 );
    CPPUNIT_TEST( testV6TipPresRemoteStart4 );
    CPPUNIT_TEST( testV6TipPresRemoteStart5 );
    CPPUNIT_TEST( testV6TipPresRemoteStart6 );
    CPPUNIT_TEST( testV6TipPresRemoteStart7 );
    CPPUNIT_TEST( testV6TipPresRemoteStart8 );
    CPPUNIT_TEST( testV6TipPresRemoteStart9 );
    CPPUNIT_TEST( testV6TipPresRemoteUpdate1 );
    CPPUNIT_TEST( testV6TipPresRemoteStartDelay );
    CPPUNIT_TEST( testV6TipPresRemoteStop );
    CPPUNIT_TEST( testV6TipPresOverride );
    CPPUNIT_TEST( testV6TipPresOverrideRelease );
    CPPUNIT_TEST( testV6TipPresOverrideLocalRelease );
    CPPUNIT_TEST( testV6TipPresConflictRemoteWinner );
    CPPUNIT_TEST( testV6TipPresConflictLocalWinner );
    CPPUNIT_TEST( testV6TipPresMCULocalStart );
    CPPUNIT_TEST( testOldPacketIgnored );
    CPPUNIT_TEST( testDupPacketAcked );
    CPPUNIT_TEST( testEcho );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipTest );
