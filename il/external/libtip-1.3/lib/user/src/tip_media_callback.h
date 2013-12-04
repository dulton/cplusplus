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

#ifndef TIP_MEDIA_CALLBACK_H
#define TIP_MEDIA_CALLBACK_H

#include <stdint.h>

namespace LibTip {

    /**
     * Media sink callback interface class.  Users should inherit from
     * this class and provide their own implementation of the
     * callbacks they are interested in.  Users can be notified of
     * many different events related to packet reception, media
     * events, errors, and more.
     *
     * @see CTipMediaSink
     */     
    class CTipMediaSinkCallback {
    public:
        /**
         * Constructor
         */
        CTipMediaSinkCallback();

        /**
         * Destructor
         */
        virtual ~CTipMediaSinkCallback();

        /**
         * Stop the media sink.  This callback is invoked when a sink
         * flow control message is received asking the sink to stop
         * rendering media.  The packet processing should continue but
         * any media packets should be silently dropped.
         */
        virtual void Stop();

        /**
         * Start the media sink.  This callback is invoked when a sink
         * flow control message is received asking the sink to start
         * rendering media.
         */
        virtual void Start();
    };
    
    /**
     * Media source callback interface class.  Users should inherit from
     * this class and provide their own implementation of the
     * callbacks they are interested in.  Users can be notified of
     * many different events related to packet reception, media
     * events, errors, and more.
     *
     * @see CTipMediaSource
     */     
    class CTipMediaSourceCallback {
    public:
        /**
         * Constructor
         */
        CTipMediaSourceCallback();

        /**
         * Destructor
         */
        virtual ~CTipMediaSourceCallback();

        /**
         * Stop the media source.  This callback is invoked when a
         * source flow control message is received asking the source
         * to stop rendering media.  The packet reception processing
         * should continue but no media packets should be sent.
         */
        virtual void Stop();

        /**
         * Start the media source.  This callback is invoked when a
         * source flow control message is received asking the source
         * to start transmitting media.
         */
        virtual void Start();

        /**
         * Request a refresh frame from the media source.  This
         * callback is invoked when a media source refresh request
         * message is received asking the source to transmit a refresh
         * frame.  In most cases this is only applicable to video
         * media.  If the idr parameter is true than an IDR must be
         * sent, if false then he type of refresh frame should be
         * determined by the options negotiated during tip
         * negotiation.
         *
         * @param idr if true an IDR should be generated, otherwise
         * the refresh type should be determined by the tip
         * negotiation.
         */
        virtual void Refresh(bool idr);

        /**
         * Process a feedback NACK.  This callback is invoked when a
         * media source receives a packet loss NACK from the remote
         * sink.  The source should take action to recover from the
         * packet loss.
         *
         * @param seqno the RTP sequence number of the lost packet
         */
        virtual void ProcessNack(uint16_t seqno);

        /**
         * Process a feedback ACK.  This callback is invoked when a
         * media source receives a packet reception ACK from the
         * remote sink.
         *
         * @param seqno the RTP sequence number of the received packet
         */
        virtual void ProcessAck(uint16_t seqno);
    };

};

#endif

