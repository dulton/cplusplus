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

#ifndef TIP_H_
#define TIP_H_

#include "tip_constants.h"
#include "tip_system.h"
#include "tip_packet_transmit.h"
#include "tip_callback.h"

namespace LibTip {

    // pre-declare used classes
    class CTipImpl;

    /**
     * Main user interface class.  CTip provides APIs to configure
     * and drive all system Tip actions.
     */
    class CTip {
    public:
        /**
         * Public constructor.  Only user accessible constructor.
         *
         * @param xmit reference to an implementation of the
         * CTipPacketTransmit interface
         */
        CTip(CTipPacketTransmit& xmit);
        ~CTip();

        /**
         * Get access to the system description object.  The system
         * description object provides users an API to describe the
         * local system using the Tip.  The local system object
         * should be fully configured before the user starts tip
         * negotiation.
         *
         * @return reference to the local system object
         * @see CTipSystem
         */         
        CTipSystem& GetTipSystem();

        /**
         * Get access to the negotiated system description object.
         * The negotiated system description object provides users an
         * API to get information about the behavior that has been
         * negotiated through tip negotiation.  This object is based
         * on the local system object configured via GetTipSystem
         * combined with the data received from the remote peer.
         * While GetTipSystem() provides users a way to describe
         * all of the capabilities of the local system, this function
         * provides users a system object that describes which
         * capabilities should be enabled/disabled to properly
         * interopate with the peer.  This object can be retrieved at
         * any time but will only contain valid data after tip
         * negotiation has completed.
         *
         * @return the negotiated local system object
         * @see CTipSystem
         */
        const CTipSystem& GetTipNegotiatedSystem() const;

        /**
         * Set the callback interface.  The callback interface allows
         * users to implement specific methods in the callback
         * interface and receive updates when the associated events
         * occur.
         *
         * @param callback pointer to an implementation of the CTipCallback interface
         * @see CTipCallback
         */         
        void SetCallback(CTipCallback* callback);

        /**
         * Start tip negotiation on the given media type.  Calling
         * this function will cause the Tip stack to being the tip
         * negotiation process.  The data configured in the system
         * object will be used to advertise and negotiate this
         * system's capabilities.
         *
         * @param mType the type of media to start tip negotiation on
         * @see GetTipSystem
         */
        Status StartTipNegotiate(MediaType mType);

        /**
         * Stop tip negotiation on the given media type.  Calling this
         * function will cause tip negotiation to stop.
         *
         * @param mType the type of media to start tip negotiation on
         */         
        Status StopTipNegotiate(MediaType mType);

        /**
         * Request to take control of presentation (aux video).
         * Calling this function will cause the Tip stack to assert
         * control over the shared presentation streams.
         */
        Status StartPresentation();

        /**
         * Give up control of presentation (aux video).  Calling this
         * function will cause the Tip stack to give up control
         * over the shared presentation streams.
         */
        Status StopPresentation();
        
        /**
         * Process a received packet.  This API is used to feed
         * packets into the Tip library.  Valid packets received may
         * invoke callback events from within this function.  Users
         * can pass all received packets into this function, the
         * library will only process the Tip specific packets.  If a
         * packet contains a single TIP packet, TIP_OK will be
         * returned, if a packet does not contain any TIP packets
         * TIP_ERROR will be returned.
         *
         * @param buffer pointer to the packet received
         * @param size length of the received packet
         * @param mType the type of media associated with the packet
         * @return TIP_OK if the packet was processed, otherwise TIP_ERROR
         */
        Status ReceivePacket(uint8_t* buffer, uint32_t size, MediaType mType);

        /**
         * Send a delayed ack.  Some callbacks allow the user to delay
         * sending an ACK for a tip event.  This gives the user
         * an opportunity to handle the associated change before the
         * remote side is notified.  After the user has processed the
         * data, an ACK should be sent using this interface.
         *
         * @param id tip event id given as part of the callback
         * @param mType the type of media associated with the packet
         * @return TIP_OK if the ACK was sent successfully, otherwise TIP_ERROR
         */
        Status SendDelayedAck(void* id, MediaType mType);

        /**
         * Set the interval between Tip packet retransmissions.  If
         * an Tip packet is not ack'ed it will be retransmitted
         * after this many milliseconds.  The default is 250ms.
         *
         * @param intervalMS the interval time, in milliseconds
         * @see DEFAULT_RETRANS_INTERVAL
         */
        void SetRetransmissionInterval(uint32_t intervalMS);

        /**
         * Set a limit on the max number of retransmissions.  If an
         * Tip packet is not ack'ed it will be retransmitted this
         * many times.
         *
         * @param limit maximum number of transmissions
         * @see DEFAULT_RETRANS_LIMIT
         */
        void SetRetransmissionLimit(uint32_t limit);

        /**
         * Get the idle time until the next Tip action.  Get the
         * amount of time until the next Tip action needs to occur
         * (in milliseconds).  After this amount of time has pased the
         * user should call DoPeriodicActivity()
         *
         * @return idle time in milliseconds, (uint64_t) -1 means
         * there are no scheduled actions
         * @see DoPeriodicActivity()
         */
        uint64_t GetIdleTime() const;

        /**
         * Perform periodic activity.  The Tip will take periodic
         * actions, handle timeouts, and any other time related
         * events.  This should be called periodically by the user,
         * the amount of delay between invocations can be determined
         * by calling GetIdleTime()
         *
         * @see GetIdleTime()
         */
        void DoPeriodicActivity();

        /**
         * Get the RTCP SSRC used by this system on the given media
         * type.
         *
         * @param mType the media type SSRC requested
         * @return the requested SSRC value
         */
        uint32_t GetRTCPSSRC(MediaType mType) const;

        /**
         * Set the RTCP SSRC used by this system on the given media
         * type.  All future packets will be sent using this SSRC.
         * The default SSRC is created using a random number seeded
         * off the time at which the CTip object was created.
         *
         * @param mType the media type SSRC being set
         * @param ssrc the new SSRC value
         */
        void SetRTCPSSRC(MediaType mType, uint32_t ssrc);
        
    private:
        CTipImpl* mImpl;

        // do not allow copy or assignment
        CTip(const CTip&);
        CTip& operator=(const CTip&);
    };
};

#endif
