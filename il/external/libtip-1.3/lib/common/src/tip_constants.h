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

#ifndef TIP_CONSTANTS_H
#define TIP_CONSTANTS_H

#include <stdint.h>

namespace LibTip {
    /**
     * Tip return codes.
     */
    enum Status {
        TIP_OK    = 0,
        TIP_ERROR = 1
    };

    /**
     * Tip versions.
     */
    enum ProtocolVersion {
        TIP_V6 = 6,
        TIP_V7 = 7
    };

    /**
     * Range of version supported by this library.
     */
    extern const ProtocolVersion SUPPORTED_VERSION_MIN; // V6
    extern const ProtocolVersion SUPPORTED_VERSION_MAX; // V7

    /**
     * Tip media types.
     */
	enum MediaType {
		VIDEO  = 0,
		AUDIO  = 1,
		MT_MAX = 2
	};

    /**
     * Convert an Tip media type into a string
     */
    inline const char* GetMediaString(MediaType mt) {
        return (mt == VIDEO) ? "VIDEO" : "AUDIO";
    }

    /**
     * Generic position.  This position enum maps position values to
     * their uses in a media independent way.  Note that in some case
     * it is not possible to map a position without specific knowledge
     * of the media type in use.
     */
	enum GenericPosition {
		POS_CONTROL = 0,
		POS_CENTER  = 1,
		POS_LEFT    = 2,
		POS_RIGHT   = 3,
		POS_AUX1    = 4,
		POS_AUX2    = 5,
		POS_AUX3    = 6,
		POS_UNUSED1 = 7,
		POS_UNUSED2 = 8,
		POS_LEGACY1 = 9,
		POS_LEGACY2 = 10,
		POS_LEGACY3 = 11,
		POS_LEGACY4 = 12,
		POS_UNUSED3 = 13,
		POS_UNUSED4 = 14,
		POS_MAX     = 15
	};

    /**
     * Video Positions.  This enum maps video positions to their uses.
     */
	enum VideoPosition {
		POS_VIDEO_INVALID       = 0,
		POS_VIDEO_CENTER        = 1,
		POS_VIDEO_LEFT          = 2,
		POS_VIDEO_RIGHT         = 3,
		POS_VIDEO_AUX_1_5FPS    = 4,
		POS_VIDEO_AUX_30FPS     = 5,
		POS_VIDEO_UNUSED1       = 6,
		POS_VIDEO_UNUSED2       = 7,
		POS_VIDEO_UNUSED3       = 8,
		POS_VIDEO_LEGACY_CENTER = 9,
		POS_VIDEO_LEGACY_LEFT   = 10,
		POS_VIDEO_LEGACY_RIGHT  = 11,
		POS_VIDEO_UNUSED4       = 12,
		POS_VIDEO_UNUSED5       = 13,
		POS_VIDEO_UNUSED6       = 14,
		POS_VIDEO_MAX           = 15
	};
	
    /**
     * Audio Positions.  This enum maps audio positions to their uses.
     */
	enum AudioPosition {
		POS_AUDIO_INVALID = 0,
		POS_AUDIO_CENTER  = 1,
		POS_AUDIO_LEFT    = 2,
		POS_AUDIO_RIGHT   = 3,
		POS_AUDIO_AUX     = 4,
		POS_AUDIO_UNUSED1 = 5,
		POS_AUDIO_UNUSED2 = 6,
		POS_AUDIO_UNUSED3 = 7,
		POS_AUDIO_UNUSED4 = 8,
		POS_AUDIO_UNUSED5 = 9,
		POS_AUDIO_UNUSED6 = 10,
		POS_AUDIO_UNUSED7 = 11,
		POS_AUDIO_LEGACY  = 12,
		POS_AUDIO_UNUSED8 = 13,
		POS_AUDIO_UNUSED9 = 14,
		POS_AUDIO_MAX     = 15
	};

    /**
     * Convert a position to the bitmask version of that position.
     */
    inline uint16_t PositionToMask(uint8_t pos) {
        uint16_t ret = (1 << pos);
        return ret;
    }
    
    /**
     * Default retransmission interval.  The default retransmission
     * interval is 250 milliseconds.  When combined with the default
     * retransmission limit of 40, this allows for 10 seconds of
     * retransmissions.
     */
    const uint32_t DEFAULT_RETRANS_INTERVAL = 250;

    /**
     * Default retransmission limit.  The default retransmission limit
     * is 40.  When combined with the default retransmission interval
     * of 250ms, this allows for 10 seconds of retransmissions.
     */             
    const uint32_t DEFAULT_RETRANS_LIMIT    = 40;

    /**
     * Tip media type incrementer
     */
    inline MediaType& operator++(MediaType& mType) { // prefix ++
        if (mType == VIDEO) {
            mType = AUDIO;
        } else if (mType == AUDIO) {
            mType = MT_MAX;
        } else {
            mType = VIDEO;
        }
        return mType;
    }
    inline MediaType operator++(MediaType& mType, int) { // postfix ++
        MediaType tmp = mType;
        ++mType;
        return tmp;
    }
};

#endif
