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

#ifndef TIP_PRES_IMPL_H
#define TIP_PRES_IMPL_H

#include "rtcp_tip_reqtosend_packet.h"
#include "private/tip_pres_negotiate_state.h"

namespace LibTip {

    // pre-declare classes
    class CTipImpl;
    
    /**
     * Tip presentation implementation class.  This class
     * implements behaviors associated with presentation (aux) media
     * sharing and negotiation.  It is a friend class to TipImpl.
     */
    class CTipPresImpl {
    public:
        /**
         * Constructor
         */
        CTipPresImpl(CTipImpl* impl);

        /**
         * Destructor
         */
        ~CTipPresImpl();

        /**
         * Request to take control of presentation.  Invoked by
         * CTipImpl when the user requests to take control of
         * presentation.
         */
        Status StartPresentation();

        /**
         * Give up control of presentation.  Invoked by CTipImpl
         * when the user requests to give up control of presentation.
         */
        Status StopPresentation();
        
        /**
         * Receive a presentation negotiation packet.  Invoked by
         * CTipImpl when a presentation related packet is received.
         *
         * @param rts Tip REQTOSEND packet to process
         */
        Status ProcessPacket(CRtcpAppReqToSendPacket* rts, MediaType mType);

        /**
         * Receive a V6 presentation negotiation packet.  Invoked by
         * CTipImpl when a presentation related packet is received.
         * Note that the packet may or may not contain presentation
         * specific data.
         *
         * @param muxctrl Tip MUXCTRL packet to process
         */
        Status ProcessPacket(CRtcpAppMuxCtrlPacketBase* muxctrl, MediaType mType);
        
        /**
         * Receive a presentation ack packet.  Invoked by CTipImpl
         * when a presentation ack packet is received.
         *
         * @param packet Tip REQTOSEND ACK packet
         */
        void ProcessAckPacket(CRtcpAppReqToSendAckPacket* packet);

        /**
         * Receive a V6 presentation ack packet.  Invoked by
         * CTipImpl when a V6 presentation ack packet is received.
         *
         * @param packet Tip MUXCTRL ACK packet
         */
        void ProcessAckPacket(CRtcpTipPacket* packet);

        /**
         * Process a presentation packet timeout.  Invoked by
         * CTipImpl when a transmitted presentation related packet
         * times out.
         */
        void ProcessPacketTimeout();

        /**
         * Process a negotiation update.  Invoked by CTipImpl when a
         * packet is received which may affect the negotiated
         * presentation options (e.g. frame rate).
         */
        void NegotiationUpdate(CTipSystem::PresentationFrameRate oldRate);
        
        /**
         * Local presentation assert.  Invoked by pres state machine
         * when the local Tip asserts control over the
         * presentation.
         */
        Status OnLocalAssert();

        /**
         * Local presentation assert ack.  Invoked by pres state
         * machine when the local Tip assert has been ack'ed by the
         * remote Tip.
         */
        Status OnLocalAssertAck(CRtcpAppReqToSendAckPacket& packet);

        /**
         * Local presentation assert timeout.  Invoked by pres state
         * machine when the local Tip assert is not ack'ed by the
         * remote Tip.
         */
        Status OnLocalAssertTimeout();
        
        /**
         * Remote presentation assert.  Invoked by pres state machine
         * when the remote Tip asserts control over the
         * presentation.
         */
        Status OnRemoteAssert(CRtcpAppReqToSendPacket& mPacket);

        /**
         * Remote presentation assert during remote control.  Invoked
         * by the pres state machine when the remote has presentation
         * control and they send re-assert control (possibly changing
         * avaialable streams).
         */
        Status OnRemoteAssertUpdate(CRtcpAppReqToSendPacket& mPacket);
        
        /**
         * Local presentation release.  Invoked by the pres state
         * machine when the local Tip releases control over the
         * presentation.
         */
        Status OnLocalRelease();


        /**
         * Local presentation release when remote has control.
         * Invoked by the pres state machine when the local Tip
         * releases control over the presentation while the remote
         * currently has control.
         */
        Status OnLocalReleaseOverride();
        
        /**
         * Presentation conflict.  Invoked by the pres state machine
         * when we have a pending presentation assert request and the
         * remote Tip sends us a presentation assert.
         */
        Status OnConflictRemoteAssert(CRtcpAppReqToSendPacket& packet);

        /**
         * Remote presentation overide.  Invoked by the pres state
         * machine when we have control of the presentation and the
         * remote side asserts control.
         */
        Status OnRemoteOverride(CRtcpAppReqToSendPacket& packet);

        /**
         * Remote presentation release.  Invoked by the pres state
         * machine when the remote side releases control of the
         * presentation.
         */
        Status OnRemoteRelease(CRtcpAppReqToSendPacket& packet);

        /**
         * Remote presentation release and local assert.  Invoked by
         * the pres state machine when the remote side releases
         * control of the presentation and the local side should
         * reassert control.
         */
        Status OnRemoteReleaseLocalAssert(CRtcpAppReqToSendPacket& packet);

        /**
         * Presentation option update during local assert.  Invoked by
         * the pres state machine when a presentation option changes
         * while we are asserting local control.
         */
        Status OnOptionUpdateLocalAssert(CTipSystem::PresentationFrameRate oldRate);
        
        /**
         * Presentation option update during local control.  Invoked by
         * the pres state machine when a presentation option changes
         * while we have local control.
         */
        Status OnOptionUpdateLocal(CTipSystem::PresentationFrameRate oldRate);
        
        /**
         * Presentation option update during remote control.  Invoked by
         * the pres state machine when a presentation option changes
         * while the remote has control.
         */
        Status OnOptionUpdateRemote(CTipSystem::PresentationFrameRate oldRate);

    protected:
        /**
         * Helper function to process a local release.
         */
        Status ProcessLocalRelease();

        /**
         * Helper function to process a remote assert.
         */
        Status ProcessRemoteAssert(CRtcpAppReqToSendPacket& packet);

        /**
         * Helper function to change states
         */
        void ChangeState(CPresState* newState);

        /**
         * Helper function to ack pending packets.
         */
        void TransmitAck();
       
        CTipImpl* mpImpl;
        CPresState*  mpState;
        CRtcpTipPacket* mpPacketToAck;
        MediaType mPacketToAckMediaType;

        uint16_t mRemoteAssertVideoPos;
        uint16_t mRemoteAssertAckedVideoPos;

    private:
        // do not allow copy or assignment
        CTipPresImpl(const CTipPresImpl&);
        CTipPresImpl& operator=(const CTipPresImpl&);
    };
    
};

#endif
