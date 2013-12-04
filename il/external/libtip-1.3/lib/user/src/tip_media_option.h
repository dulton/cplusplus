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

#ifndef __MEDIA_OPTION__
#define __MEDIA_OPTION__

#include <stdint.h>
#include "tip_constants.h"

namespace LibTip {

    /**
     * Base media option class.  Base class for media options, should
     * not be used directly.
     *
     * @see CTipVideoMediaOption
     * @see CTipAudioMediaOption
     */
    class CTipMediaOption {
    public:
        enum OptionState {
            OPTION_NOT_SUPPORTED  = 0x00,
            OPTION_SUPPORTED_TX   = 0x01,
            OPTION_SUPPORTED_RX   = 0x02,
            OPTION_SUPPORTED_BOTH = (OPTION_SUPPORTED_TX | OPTION_SUPPORTED_RX)
        };

        /**
         * Constructor.  Constructs a media option of the give type,
         * option type, and option state.
         *
         * @param type the media type
         * @param option generic option type
         * @param state the option state
         */
        CTipMediaOption(MediaType type, uint16_t option, OptionState state);

        /**
         * Destructor.
         */
        virtual ~CTipMediaOption();

        /**
         * Set the media type.
         *
         * @param type the media type
         */
        virtual void SetMediaType(MediaType type);

        /**
         * Get the media type.
         *
         * @return the media type
         */
        virtual MediaType GetMediaType() const;

        /**
         * Set the option type.  Option types are defined by the
         * derived classes but are generically a 16 bit integer.
         *
         * @param option the option type
         */
        virtual void SetOption(uint16_t option);

        /**
         * Get the option type.  Option types are defined by the
         * derived classes but are generically a 16 bit integer.
         *
         * @return the option type
         */
        virtual uint16_t GetOption() const;

        /**
         * Set the option state.
         *
         * @param state the option state
         */
        virtual void SetState(OptionState state);

        /**
         * Get the option state.
         *
         * @return the option state
         */
        virtual OptionState GetState() const;
    
    protected:
        MediaType   mType;
        uint16_t    mOption;
        OptionState mState;
    };

    /**
     * Video media option class.  This class implements the
     * CTipMediaOption interface providing an interface for
     * manipulating options associated with video media.
     */
    class CTipVideoMediaOption : public CTipMediaOption {
    public:
        /**
         * Supported video options.
         */
        enum VideoOption {
            REFRESH_FLAG = 0,
            INBAND_PARAM_SETS,
            CABAC,
            LTRP,
            GDR,
            HP_8X8_TRANSFORMS,
            UNRESTRICTED_XGA_1_5,
            UNRESTRICTED_720P,
            UNRESTRICTED_1080P,
            UNRESTRICED_XGA_30,
            EKT,
            MAX_VIDEO_OPTION
        };

        /**
         * Constructor.  Construct a media option for the given option
         * with the given state.
         *
         * @param option the video option type
         * @param state the option state
         */
        CTipVideoMediaOption(VideoOption option, OptionState state);

        /**
         * Destructor.
         */
        virtual ~CTipVideoMediaOption();
    };

    /**
     * Audio media option class.  This class implements the
     * MediaOption interface providing an interface for manipulating
     * options associated with audio media.
     */
    class CTipAudioMediaOption : public CTipMediaOption {
    public:
        /**
         * Supported audio options.
         */
        enum AudioOption {
            ACTIVITY_METRIC = 0,
            DYNAMIC_OUTPUT,
            CAPABLE_G722_LEGACY,
            USING_G722_LEGACY,
            EKT,
            MAX_AUDIO_OPTION
        };
    
        /**
         * Constructor.  Construct a media option for the given option
         * with the given state.
         *
         * @param option the audio option type
         * @param state the option state
         */
        CTipAudioMediaOption(AudioOption option, OptionState state);

        /**
         * Destructor.
         */
        virtual ~CTipAudioMediaOption();
    };

    /**
     * VideoOption type prefix incrementer
     */
    inline CTipVideoMediaOption::VideoOption&
        operator++(CTipVideoMediaOption::VideoOption& opt)
    {
        // rollover case
        if (opt == CTipVideoMediaOption::MAX_VIDEO_OPTION) {
            opt = CTipVideoMediaOption::REFRESH_FLAG;
        } else {
            uint32_t tmp = opt;
            tmp++;
            opt = static_cast<CTipVideoMediaOption::VideoOption>(tmp);
        }
        
        return opt;
    }
    
    /**
     * VideoOption type postfix incrementer
     */
    inline CTipVideoMediaOption::VideoOption
        operator++(CTipVideoMediaOption::VideoOption& opt, int)
    {
        CTipVideoMediaOption::VideoOption ret = opt;
        ++opt;
        return ret;
    }

    /**
     * AudioOption type prefix incrementer
     */
    inline CTipAudioMediaOption::AudioOption&
        operator++(CTipAudioMediaOption::AudioOption& opt)
    {
        // rollover case
        if (opt == CTipAudioMediaOption::MAX_AUDIO_OPTION) {
            opt = CTipAudioMediaOption::ACTIVITY_METRIC;
        } else {
            uint32_t tmp = opt;
            tmp++;
            opt = static_cast<CTipAudioMediaOption::AudioOption>(tmp);
        }
        return opt;
    }

    /**
     * AudioOption type postfix incrementer
     */
    inline CTipAudioMediaOption::AudioOption
        operator++(CTipAudioMediaOption::AudioOption& opt, int)
    {
        CTipAudioMediaOption::AudioOption ret = opt;
        ++opt;
        return ret;
    }

};

#endif
