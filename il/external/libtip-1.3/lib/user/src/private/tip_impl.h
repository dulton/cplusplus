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

#ifndef TIP_IMPL_H
#define TIP_IMPL_H

#include <iostream>
#include <sstream>

#include "tip_constants.h"
#include "rtcp_packet.h"
#include "rtcp_tip_packet_manager.h"
#include "rtcp_tip_types.h"
#include "tip_callback_wrapper.h"
#include "tip_packet_transmit.h"
#include "private/tip_negotiate_state.h"
#include "private/map_tip_system.h"
#include "private/tip_packet_receiver.h"
#include "private/tip_timer.h"
#include "private/tip_pres_impl.h"

namespace LibTip {

    /**
     * Tip implementation class.  This class provides the
     * implementation of the interfaces defined in CTip.
     */
    class CTipImpl {
    public:
        /**
         * Constructor
         */
        CTipImpl(CTipPacketTransmit& xmit);

        /**
         * Destructor
         */
        ~CTipImpl();

        //
        // methods "inherited" from CTip
        //
        
        /**
         * Get access to the system description object.  The system
         * description object provides users an API to describe the
         * system using the Tip.
         *
         * @return reference to the system object
         * @see CTipSystem
         */         
        CTipSystem& GetTipSystem() { return mSystem; }

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
         * @return pointer the negotiated local system object
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
         * sending an ACK for a tip event.  This gives the user an
         * opportunity to handle the associated change before the
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
         */
        void SetRetransmissionInterval(uint32_t intervalMS);

        /**
         * Set a limit on the max number of retransmissions.  If an
         * Tip packet is not ack'ed it will be retransmitted this
         * many times.
         *
         * @param limit maximum number of transmissions
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
        
        //
        // Impl specific public methods, not exposed to user
        //

        /**
         * Local tip negotiation start.  Invoked by state machine when
         * local tip negotiation begins.
         */
        Status OnLocalStart(MediaType mType);

        /**
         * RX MUXCTRL ACK.  Invoked by state machine when a MUXCTRL
         * ACK packet is received.
         */
        void OnRxMCAck(MediaType mType);
        
        /**
         * RX MUXCTRL ACK after.  Invoked by state machine when a
         * MUXCTRL ACK packet is received after tip negotiation has
         * finished.
         */
        void OnRxMCAckAfterNegotiate(CRtcpTipPacket& ack, MediaType mType);
        
        /**
         * RX MEDIAOPTS ACK.  Invoked by state machine when a
         * MEDIAOPTS ACK packet is received.
         */
        void OnRxMOAck(MediaType mType);

        /**
         * Local tip timeout.  Invoked by state machine when a timeout
         * occurs.
         */
        void OnLocalTimeout(MediaType mType);

        /**
         * Local tip timeout after tip negotiation is finished.
         * Invoked by state machine when a timeout occurs after tip
         * negotiation has finished.
         */
        void OnLocalTimeoutAfterNegotiate(MediaType mType);
        
        /**
         * Local tip stop.  Invoked by state machine when local tip
         * negotiation is stopped.
         */
        void OnLocalStop(MediaType mType);

        /**
         * Remote tip start.  Invoked by state machine when remote tip
         * negotiation is started.
         */
        void OnRemoteStart(MediaType mType);

        /**
         * RX MUXCTRL early.  Invoked by state machine when MUXCTRL is
         * received before tip negotiation is started.
         */
        void OnRxMCBeforeNegotiate(const CRtcpAppMuxCtrlPacketBase& muxctrl,
                                   MediaType mType);

        /**
         * RX MUXCTRL.  Invoked by state machine when MUXCTRL is
         * received during tip negotiation.
         */
        void OnRxMCDuringNegotiate(CRtcpAppMuxCtrlPacketBase& muxctrl,
                                   MediaType mType);

        /**
         * RX MUXCTRL after.  Invoked by state machine when MUXCTRL is
         * received after tip negotiation has finished.
         */
        void OnRxMCAfterNegotiate(CRtcpAppMuxCtrlPacketBase& muxctrl,
                                  MediaType mType);

        /**
         * RX MEDIAOPTS.  Invoked by state machine when MEDIAOPTS is
         * received during tip negotiation.
         */
        void OnRxMODuringNegotiate(CRtcpAppMediaoptsPacket& mo, MediaType mType);

        /**
         * RX MEDIAOPTS after.  Invoked by state machine when
         * MEDIAOPTS is received after tip negotiation has finished.
         */
        void OnRxMOAfterNegotiate(CRtcpAppMediaoptsPacket& mo, MediaType mType);

        /**
         * RX REQTOSEND.  Invoked by state machine when REQTOSEND is
         * received after tip negotiation has finished.
         */
        void OnRxReqToSend(CRtcpAppReqToSendPacket& rts, MediaType mType);
        
        /**
         * RX SPIMAP.  Invoked by state machine when SPIMAP is
         * received after tip negotiation has finished.
         */
        void OnRxSpiMap(CRtcpAppSpiMapPacket& spimap, MediaType mType);

        /**
         * RX NOTIFY.  Invoked by state machine when NOTIFY is
         * received after tip negotiation has finished.
         */
        void OnRxNotify(const CRtcpAppNotifyPacket& notify, MediaType mType);

        /**
         * Remote tip timeout. Invoked by state machine when a timeout
         * occurs.
         */
        void OnRemoteTimeout(MediaType mType);

        /**
         * Remote tip stop.  Invoked by state machine when remote tip
         * negotiation is stopped.
         */
        void OnRemoteStop(MediaType mType);
        
    protected:
        /**
         * Friend classes.  These classes implement specific portions
         * of the Tip and can access internal methods and data
         * directly.
         */
        friend class CTipPresImpl;
        
        Status ConfigureTipNegPackets(MediaType mType);

        void StartPacketTx(CRtcpTipPacket* packet, MediaType mType);
        void StopPacketTx(TipPacketType pType, MediaType mType);

        void HandleTimeout(CRtcpTipPacket* packet, MediaType mType);
        void RelayPacket(CPacketBuffer* buffer, MediaType mType);

        void AckPacket(const CRtcpTipPacket* packet, MediaType mType);
        void AckDuplicatePacket(const CRtcpTipPacket* packet, MediaType mType);
        void TransmitAck(CRtcpTipPacket* ack, const CRtcpTipPacket* packet,
                         MediaType mType);

        void ProcessPacket(CRtcpTipPacket* packet, MediaType mType);
        void ProcessAckPacket(CRtcpTipPacket* packet, MediaType mType);

        void SetLocalState(MediaType mType, CLocalState* state);
        void SetRemoteState(MediaType mType, CRemoteState* state);

        void UpdateNegotiatedSystem();

        void PrintPacket(const CRtcpTipPacket* packet, MediaType mType, bool isRX) const;
        
        CMapTipSystem        mSystem;
        CMapTipSystem        mRemoteSystem;
        CMapTipSystem        mNegotiatedSystem;

        CLocalState*         mpTipNegLocalState[MT_MAX];
        CRemoteState*        mpTipNegRemoteState[MT_MAX];
        
        CTipPacketManager    mPacketManager[MT_MAX];
        uint32_t             mSSRC[MT_MAX];
        CTipPacketReceiver   mPacketReceiver[MT_MAX];
        CTipTimer            mTimer;
        CTipPacketTransmit&  mPacketXmit;
        CTipCallbackWrapper* mpCallback;
        CTipPresImpl         mPresImpl;
        
        uint32_t             mTipNegTimerId[MT_MAX];
        uint64_t             mMuxCtrlTime[MT_MAX];

    private:
        // do not allow copy or assignment
        CTipImpl(const CTipImpl&);
        CTipImpl& operator=(const CTipImpl&);
    };

};

#endif

