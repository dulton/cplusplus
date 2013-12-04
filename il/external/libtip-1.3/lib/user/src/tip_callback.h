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

#ifndef TIP_CALLBACK_H
#define TIP_CALLBACK_H

#include "tip_constants.h"
#include "tip_system.h"

namespace LibTip {

    /**
     * Callback interface class.  Users should inherit from this class
     * and provide their own implementation of the callbacks they are
     * interested in.  Users can be notified of many different events
     * related to packet reception, tip negotiation, errors, and more.
     * For some events the user has the opportunity to affect how that
     * event is handled.  More information on the specifics is
     * available for each event.
     *
     * @see CTip
     */     
    class CTipCallback {
    public:
        /**
         * Constructor
         */
        CTipCallback();

        /**
         * Destructor
         */
        virtual ~CTipCallback();

        /**
         * Tip negotiation early.  The remote Tip is sending tip
         * negotiation related packets but local tip negotiation has
         * not started yet.  It is possible for this callback to be
         * invoked multiple times, in most cases the user can ignore
         * all but the first invocation.
         *
         * @param mType the media type the event was detected on
         */
        virtual void TipNegotiationEarly(MediaType mType);

        /**
         * Tip negotiation failed.  The remote Tip did not send
         * all of the required packets or did not ack all received
         * packets within the time window.
         *
         * @param mType the media type the event was detected on
         */
        virtual void TipNegotiationFailed(MediaType mType);

        /**
         * Tip negotiation version mismatch.  The remote Tip is
         * using a different tip version than we are configured to
         * use.
         *
         * @param mType the media type the event was detected on
         * @param version the version of the remote Tip
         *
         * @return If true is returned (the default) the tip library
         * will downgrade to the received version and restart tip
         * negotiation.  If false is returned tip negotiation will
         * stop pending user reconfiguration and restart.
         */
        virtual bool TipNegotiationMismatch(MediaType mType, uint8_t version);

        /**
         * Tip negotiation invalid version.  The remote Tip is
         * using an unsupported tip version
         *
         * @param mType the media type the event was detected on
         * @param version the version of the remote Tip
         */
        virtual void TipNegotiationIncompatible(MediaType mType, uint8_t version);

        /**
         * Tip negotiation final ack received.  we have sent all tip
         * negotiation packets and received acks for each one.
         *
         * @param mType the media type the event was detected on
         */
        virtual void TipNegotiationLastAckReceived(MediaType mType);

        /**
         * Tip negotation final ack is ready to be sent.  We have
         * received all packets from the remote Tip.  This callback is
         * invoked prior to sending the ACK for the last received
         * packet.  The doReinvite parameter is used by some TIP
         * profiles to control which side of the call should initiate
         * a call control reinvite now that TIP negotiation has
         * completed.  This is always false for TIP V6 and audio media
         * types.
         *
         * @param mType the media type the event was detected on
         * @param doReinvite if true, the user should initiate the reinvite
         * @param id transaction id of this callback
         *         
         * @return If true is returned (the default) the ACK will be
         * sent.  If false is returned the ACK will not be sent, and
         * should be sent later using the transaction id and
         * SendDelayedAck() CTip method.
         * @see CTip::SendDelayedAck()
         */
        virtual bool TipNegotiationLastAckTransmit(MediaType mType, bool doReinvite,
                                                   void* id);

        /**
         * Deprecated version of TipNegotiationLastAckTransmit, use
         * the above version instead.  This version was present in the
         * TIP library 1.0 release and has been deprecated in the 1.1
         * release.  The callback will still be invoked as per the
         * above description but this callback may be removed in
         * future versions.
         *
         * @param mType the media type the event was detected on
         * @param id transaction id of this callback
         *         
         * @return If true is returned (the default) the ACK will be
         * sent.  If false is returned the ACK will not be sent, and
         * should be sent later using the transaction id and
         * SendDelayedAck() CTip method.
         * @see CTip::SendDelayedAck()
         */
        virtual bool TipNegotiationLastAckTransmit(MediaType mType, void* id);
        
        /**
         * New tip negotiation packet received.  The remote Tip
         * sent an updated version of one of the packets used in tip
         * negotiation, after initial tip negotiation had finished.
         *
         * @param mType the media type the event was detected on
         * @param id transaction id of this callback
         *         
         * @return If true is returned (the default) the ACK will be
         * sent.  If false is returned the ACK will not be sent, and
         * should be sent later using the transaction id and
         * SendDelayedAck() CTip method.
         * @see CTip::SendDelayedAck()
         */
        virtual bool TipNegotiationUpdate(MediaType mType, void* id);

        /**
         * Tip security key packet received.  The remote Tip sent a
         * security key message (SPIMAP).  This callback will only be
         * invoked when both the local and remote systems have
         * security enabled.
         *
         * @param mType the media type the event was detected on
         * @param spi session protection index received from the
         * remote Tip
         * @param salt the SRTP salt value (112 bits, 14 bytes per spec)
         * @param kek the key encrypting key (128 bits, 18 bytes per spec)
         * @param id transaction id of this callback
         *
         * @return If true is returned (the default) the tip library
         * will ACK the SPIMAP.  If false is returned the ACK will not
         * be sent and should be sent later using the transaction id
         * and SendDelayedAck() CTip method.  
         * @see CTip::SendDelayedAck()
         */
        virtual bool TipSecurityKeyUpdate(MediaType mType, uint16_t spi, 
                                          const uint8_t* salt, const uint8_t* kek, 
                                          void* id);

        /**
         * Tip security state packet received.  The remote Tip sent a
         * security state update message.  This message informs the
         * user when the overall security state of a call has changed.
         * This does not affect the RTP/SRTP being used for media
         * transport, it is simply an indication of whether the
         * overall call is secure or not.
         *
         * @param mType the media type the event was detected on
         * @param secure if true the call is secure, if false the call
         * is non-secure
         */
        virtual void TipSecurityStateUpdate(MediaType mType, bool secure);
        
        /**
         * Local presentation start.  The local system now has control
         * and should start transmitting the video presentation (aux)
         * media streams.  Note this callback will be invoked once for
         * every video position which should be enabled.
         *
         * @param position the position to be started
         * @param fps the framerate to be started
         */
        virtual void
            LocalPresentationStart(uint8_t position,
                                   CTipSystem::PresentationStreamFrameRate fps);

        /**
         * Local presentation control lost.  The local system has lost
         * control and should stop transmitting the presentation (aux)
         * media streams.  Note this callback will only be invoked
         * once even if multiple presentation streams are running.
         * The callback should stop all presentation streams.
         */
        virtual void LocalPresentationStop();

        /**
         * Local presentation failure.  The local system has timed out
         * while trying to assert control over the presentation.
         */
        virtual void LocalPresentationFailure();
        
        /**
         * Remote presentation control.  The remote system has control
         * and the local system should prepare to receive the video
         * presentation (aux) media streams.  Note that this callback
         * will be invoked once per stream position to be enabled.
         *
         * @param position the stream position to be started
         * @param fps the frame rate to be received on the given position
         *
         * @return if true the remote presentation control will be
         * ACK'ed (the default).  If false control will not be ACK'ed.
         * The user can later ACK by calling SendDelayedAck() on the
         * CTip object and passing in the transaction id.  When
         * this callback is invoked multiple times, returning false
         * for any callback will cause the ACK to not be sent.
         *
         * @param position the position to be started
         * @param fps the framerate to be started
         * @param id transaction id of this callback
         *
         * @see CTip::SendDelayedAck()
         */
        virtual bool
            RemotePresentationStart(uint8_t position,
                                    CTipSystem::PresentationStreamFrameRate fps,
                                    void* id);

        /**
         * Remote presentation stop.  The remote system has given up
         * control of the presentation (aux) media.  The local system
         * should stop receiving.  This should stop reception on all
         * receiving presentation video streams.
         */
        virtual void RemotePresentationStop();

        /**
         * Presentation conflict.  The local and remote systems have
         * both requested control over presentation.  In most case the
         * Tip stack can internally determine the winner but in
         * some cases it cannot.  When a winner cannot be determined
         * this callback will be invoked.  The user should return true
         * if the local system should take control and false if the
         * remote system should take control.
         *
         * return true if local should control, false otherwise
         */
        virtual bool ResolvePresentationConflict();

    };

};

#endif
