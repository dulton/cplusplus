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

#ifndef TIP_MEDIA_H_
#define TIP_MEDIA_H_

#include <bitset>

#include "tip_csrc.h"
#include "tip_constants.h"
#include "tip_packet_transmit.h"
#include "rtcp_tip_packet_manager.h"
#include "tip_media_callback.h"
#include "private/tip_packet_receiver.h"

namespace LibTip {

    // predeclare used classes
    class CRtcpTipPacket;
    class CRtcpAppFeedbackPacket;
    
    /**
     * Abstract base class for media Tip implementations.  Base
     * class for Tip media APIs, not directly usable.  Users should
     * use CTipMediaSink or CTipMediaSource instead.
     */
    class CTipMedia {
    public:
        /**
         * Public constructor.
         *
         * @param type Media type used by this media implementation
         * @param ssrc SSRC value to be used when sending Tip
         * messages.
         * @param csrc CSRC of this media, used to validate incoming messages
         * @param xmit reference to an implementation of the
         * CTipPacketTransmit interface
         * @param string should be either "SINK" or "SOURCE"
         */
        CTipMedia(MediaType type, uint32_t ssrc, uint32_t csrc,
                  CTipPacketTransmit& xmit, const char* string);

        /**
         * Pure virtual destructor.
         */
        virtual ~CTipMedia() = 0;

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
         * @return TIP_OK if the packet was processed, otherwise TIP_ERROR
         */
        Status ReceivePacket(uint8_t* buffer, uint32_t size);

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
         * Set a log prefix string.  As there may be multiple media
         * sinks and sources all producing log output at the same
         * time, it is sometimes useful to prefix each log message
         * with a string uniquely identifying who generated the log
         * statement.  The default value is the media type, followed
         * by sink or source, followed by the CSRC value (in hex).  For
         * example: "VIDEO SINK 0x12345678".
         *
         * @param string the prefix string to use (a copy is made)
         */
        void SetLogPrefix(const char* string);
        
    protected:
        void StartPacketTx(CRtcpTipPacket* packet);
        void StopPacketTx(TipPacketType pType);

        void PrintPacketTx(const CRtcpPacket& packet, MediaType mType) const;
        void PrintPacketRx(const CRtcpPacket& packet, MediaType mType) const;
        
        virtual void ProcessPacket(CRtcpTipPacket* packet);
        virtual void ProcessAckPacket(CRtcpTipPacket* packet);
        virtual void ProcessFBPacket(CRtcpAppFeedbackPacket* packet);

        void AckPacket(const CRtcpTipPacket* packet);
        
        MediaType              mMediaType;
        uint32_t               mSSRC;
        CTipCSRC               mCSRC;
        CTipPacketTransmit& mPacketXmit;
        CTipPacketManager   mPacketManager;
        CTipPacketReceiver  mPacketReceiver;
        std::string            mLogPrefix;
        
    private:
        // do not allow copy or assignment
        CTipMedia(const CTipMedia&);
        CTipMedia& operator=(const CTipMedia&);
    };
        
    /**
     * User interface class for media sink Tip implementations.
     * CTipMediaSink provides functions to transmit and receive tip
     * messages which directly affect the media sink flows.
     */
    class CTipMediaSink : public CTipMedia {
    public:
        /**
         * Public constructor.
         *
         * @param type Media type used by this media implementation
         * @param ssrc SSRC value to be used when sending Tip
         * messages.
         * @param csrc CSRC of this sink, used to validate incoming messages
         * @param xmit reference to an implementation of the
         * CTipPacketTransmit interface
         */
        CTipMediaSink(MediaType type, uint32_t ssrc, uint32_t csrc,
                      CTipPacketTransmit& xmit);
        virtual ~CTipMediaSink();

        /**
         * Set the callback interface.  The callback interface allows
         * users to implement specific methods in the callback
         * interface and receive updates when the associated events
         * occur.
         *
         * @param callback pointer to an implementation of the
         * CTipMediaSink Callback interface
         * @see CTipMediaSinkCallback
         */         
        void SetCallback(CTipMediaSinkCallback* callback);

        /**
         * Set the current media source.  This API allows the user to
         * set the current source for this media sink.  Where
         * appropriate Tip messages will be sent to and only
         * received from the current media source.
         *
         * @param csrc CSRC value of the current source
         */
        void SetSourceCSRC(uint32_t csrc);
        
        /**
         * Request a media refresh.  This API is used by a sink when a
         * media refresh is required.
         *
         * @param idr if true an IDR will be requested, if false the
         * source will choose the refresh type.
         *
         * @return TIP_OK if the request was sent, otherwise TIP_ERROR
         */
        Status RequestRefresh(bool idr);

        /**
         * Register a received packet sequence number.  This API
         * allows the user to register that a packet has been
         * received.  This information is then used to create the
         * Tip feedback packet when requested.  Gaps in received
         * sequence numbers will be treated as lost packets, but users
         * may later register those skipped sequence numbers (ie you
         * can register out of order packets in the order they are
         * received).  If the end-of-frame flag is true, a feedback
         * packet will be generated and transmitted via the user
         * supplied transmit object.
         *
         * @param seqno RTP sequence number of the received packet
         * @param eof if true, this is the last RTP sequence number in
         * the current frame
         */
        void RegisterPacket(uint16_t seqno, bool eof);

    protected:
        virtual void ProcessPacket(CRtcpTipPacket* packet);

        uint32_t                  mSourceCSRC;
        CTipMediaSinkCallback*    mpSinkCallback;

        bool                      mHaveLastSeqNum;
        uint16_t                  mLastSeqNum;

        // enough bits and bytes to store the all possible sequence numbers
        static const uint32_t     NUM_ACK_BITS  = 65536;
        std::bitset<NUM_ACK_BITS> mAcks;
        
    private:
        // do not allow copy or assignment
        CTipMediaSink(const CTipMediaSink&);
        CTipMediaSink& operator=(const CTipMediaSink&);
    };

    /**
     * User interface class for media source Tip implementations.
     * CTipMediaSource provides functions to transmit and receive tip
     * messages which directly affect the media source flows.
     */
    class CTipMediaSource : public CTipMedia {
    public:
        /**
         * Public constructor.
         *
         * @param type Media type used by this media implementation
         * @param ssrc SSRC value to be used when sending Tip
         * messages.
         * @param csrc CSRC of this source, used to validate incoming messages
         * @param xmit reference to an implementation of the
         * CTipPacketTransmit interface
         */
        CTipMediaSource(MediaType type, uint32_t ssrc, uint32_t csrc,
                        CTipPacketTransmit& xmit);
        virtual ~CTipMediaSource();

        /**
         * Set the callback interface.  The callback interface allows
         * users to implement specific methods in the callback
         * interface and receive updates when the associated events
         * occur.
         *
         * @param callback pointer to an implementation of the
         * CTipMediaSource Callback interface
         * @see CTipMediaSourceCallback
         */         
        void SetCallback(CTipMediaSourceCallback* callback);

        /**
         * Set the first transmitted sequence number.  This API allows
         * the user to set the first transmitted RTP sequence number.
         * This sequence number will be used to validate incoming
         * Tip packets that have sequence number information,
         * assuring that the packets are in range.  This API only
         * needs to be called once when the first RTP packet is sent.
         * This API should also be invoked anytime the RTP sequence
         * number is reset or changed non-linearly (e.g. if the
         * sequence number is reset on a stop - start event).
         *
         * @param seqno the RTP sequence number
         */
        void SetSequenceNumber(uint16_t seqno);
        
    protected:
        virtual void ProcessPacket(CRtcpTipPacket* packet);
        virtual void ProcessFBPacket(CRtcpAppFeedbackPacket* packet);

        void DoAckNackCallback(uint16_t seqNum, bool isAck);

        
        bool                      mHaveLastSeqNum;
        uint16_t                  mLastSeqNum;
        
        // enough bits and bytes to store the all possible sequence numbers
        static const uint32_t     NUM_ACK_BITS  = 65536;
        std::bitset<NUM_ACK_BITS> mCallbackEnable;
        
        CTipMediaSourceCallback*  mpSourceCallback;

    private:
        // do not allow copy or assignment
        CTipMediaSource(const CTipMediaSource&);
        CTipMediaSource& operator=(const CTipMediaSource&);
    };

};

#endif
