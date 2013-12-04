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

#ifndef __TIP_SYSTEM__
#define __TIP_SYSTEM__

#include <map>
#include <bitset>

#include "tip_constants.h"
#include "tip_media_option.h"

namespace LibTip {

    /**
     * Main system description class.  Provides APIs for describing a
     * system's components and capabilities.
     */
    class CTipSystem {
    public:
        CTipSystem();
        virtual ~CTipSystem();

        /**
         * Get the tip version associated with this system.
         *
         * @return the tip version is use
         */
        virtual ProtocolVersion GetTipVersion() const;

        /**
         * Set the tip version associated with this system.  This
         * function is only necessary when the user is manually
         * handling a version mis-match between two systems.  In most
         * cases, users should not need to call this function.
         *
         * @param version the new tip version
         * @return TIP_OK if the new version is accepted, otherwise TIP_ERROR
         */
        virtual Status SetTipVersion(ProtocolVersion version);

        /**
         * Add a media transmitter to this system.  Adds a transmitter
         * of the given type at the given position.  Positions are
         * media specific and are defined in tip_constants.h.
         *
         * @param type the transmitter's media type
         * @param pos the transmitter's position
         * @see VideoPosition
         * @see AudioPosition
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */         
        virtual Status AddTransmitter(MediaType type,
                                      uint8_t pos);

        /**
         * Remove a media transmitter from this system.  Removes a
         * transmitter of the given type at the given position.
         * Positions are media specific and are defined in
         * tip_constants.h.  Users should not invoke this
         * function, it is for Tip internal use only.
         *
         * @param type the transmitter's media type
         * @param pos the transmitter's position
         * @see VideoPosition
         * @see AudioPosition
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */         
        virtual Status RemoveTransmitter(MediaType type,
                                         uint8_t pos);
        
        /**
         * Add a media receiver to this system.  Adds a receiver of
         * the given type at the given position.  Positions are media
         * specific and are defined in tip_constants.h.
         *
         * @param type the receiver's media type
         * @param pos the receiver's position
         * @return TIP_OK if successful, otherwise TIP_ERROR
         * @see VideoPosition
         * @see AudioPosition
         */         
        virtual Status AddReceiver(MediaType type,
                                   uint8_t pos);

        /**
         * Add a media transmitter and receiver to this system.  Adds
         * a transmitter and receiver of the given type at the given
         * position.  Positions are media specific and are defined in
         * tip_constants.h.
         *
         * @param type the media type
         * @param pos the media position
         * @return TIP_OK if successful, otherwise TIP_ERROR
         * @see VideoPosition
         * @see AudioPosition
         */         
        virtual Status AddBidirectional(MediaType type,
                                        uint8_t pos);

        /**
         * Possible presentation frame rates.  The following constants
         * represents the possible presentation frame rates.
         */
        enum PresentationStreamFrameRate {
            PRES_INVALID_RATE = 0x0,
            PRES_1FPS_RATE    = 0x1,
            PRES_5FPS_RATE    = 0x2,
            PRES_30FPS_RATE   = 0x4
        };
        
        /**
         * Valid system presentation frame rates.  The following enum
         * represents the valid presentation frame rates supported by
         * this system.
         */
        enum PresentationFrameRate {
            PRES_1FPS_ONLY      = PRES_1FPS_RATE,
            PRES_5FPS_ONLY      = PRES_5FPS_RATE,
            PRES_30FPS_ONLY     = PRES_30FPS_RATE,
            PRES_1FPS_AND_30FPS = (PRES_1FPS_RATE | PRES_30FPS_RATE),
            PRES_5FPS_AND_30FPS = (PRES_5FPS_RATE | PRES_30FPS_RATE),
        };
        
        /**
         * Set the supported presentation frame rate(s).  Configures
         * the system to support the given presentation frame rate(s).
         * Note that PRES_1FPS_AND_30FPS is not a valid frame rate to
         * be set, it is only used as the result of negotiation.
         *
         * @param rate the supported presentation frame rates
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */
        virtual Status SetPresentationFrameRate(PresentationFrameRate rate);
         
        /**
         * Maximum participant id length
         */
        static const uint32_t MAX_PARTICIPANT_ID_LENGTH = 128;

        /**
         * Request to join a conference.  When tip negotiation occurs,
         * this system will request to join the given conference.  The
         * system may also advertise its own participant id as part of
         * joining the conference.
         *
         * @param confID the conference id to join
         * @param participantID the id of the local system
         * @param participantIDLength length of the participant id
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */
        virtual Status JoinConference(uint64_t confID,
                                      uint8_t* participantID = NULL,
                                      uint32_t participantIDLength = 0);

        /**
         * Add a video media option to this system.  Add support for
         * a video media option to this system.  Media options will
         * be negotiated as part of tip negotiation.
         *
         * @param option video media option to be negotiated
         * @return TIP_OK if successful, otherwise TIP_ERROR
         * @see CTipVideoMediaOption
         */
        virtual Status AddMediaOption(CTipVideoMediaOption& option);

        /**
         * Add an audio media option to this system.  Add support for
         * an audio media option to this system.  Media options will
         * be negotiated as part of tip negotiation.
         *
         * @param option video media option to be negotiated
         * @return TIP_OK if successful, otherwise TIP_ERROR
         * @see CTipAudioMediaOption
         */
        virtual Status AddMediaOption(CTipAudioMediaOption& option);

        /**
         * Enable security for this system.  A secure RTP profile will
         * be signaled and callbacks related to security will be
         * enabled.
         *
         * @param enable if true security is enabled
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */
        virtual Status SetSecurityState(bool enable);

        /**
         * Enable MCU behavior for this system.  MCU behavior affects
         * the Tip negotiations and is advertised via the Tip.
         * MCU behavior defaults to disabled.
         *
         * @param enable if true mcu behavior is enabled
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */
        virtual Status SetMCUState(bool enable);

        /**
         * Enable RTP feedback for this system.  RTP feedback is used
         * to provide packet reception information from a receiver to
         * a transmitter.  Feedback is only supported for video media
         * types.
         *
         * @param enable if true feedback is enabled, otherwise disabled
         * @return TIP_OK if successful, otherwise TIP_ERROR
         */
        virtual Status SetFeedbackState(bool enable);
        
        // typedefs used for data return below
        typedef std::bitset<16> CPosBitset;

        /**
         * Get the enabled transmitters.  Return a std::bitset
         * containing a mask of the enabled transmitters.
         *
         * @param type transmitter media type to return @return
         * std::bitset of enabled transmitters, if a bit is 1 then
         * that transmitter is enabled
         */
        const CPosBitset& GetTransmitters(MediaType type) const { return mTxPos[type]; }

        /**
         * Get the enabled receivers.  Return a std::bitset
         * containing a mask of the enabled receivers.
         *
         * @param type receiver media type to return
         * @return std::bitset of enabled receivers, if a bit is 1 then
         * that receiver is enabled
         */
        const CPosBitset& GetReceivers(MediaType type) const { return mRxPos[type]; }

        /**
         * Get the maximum supported presentation frame rate.
         *
         * @return the max supported presentation frame rate
         */
        PresentationFrameRate GetPresentationFrameRate() const { return mPresFrameRate; }
        
        /**
         * Get the conference id.  Returns the configured conference
         * id for this system.
         *
         * @return the conference id for this system
         */
        uint64_t GetConfID() const { return mConfID; }

        /**
         * Get the participant id.  Returns the configured participant
         * id for this system.
         *
         * @return the participant id
         */
        const uint8_t* const GetParticipantID() const { return mPartID; }

        /**
         * Get the participant id length.  Returns the length of the
         * configured participant id for this system.
         *
         * @return the participant id length
         */
        uint32_t GetParticipantIDLength() const { return mPartIDLen; }

        /**
         * Get the state of a video media option.  Returns the state
         * of the given video media option.
         *
         * @param option the option to retrieve state on
         * @return the state of the requested option
         * @see CTipVideoMediaOption
         */
        CTipMediaOption::OptionState
            GetMediaOptionState(CTipVideoMediaOption::VideoOption option) const;

        /**
         * Get the state of a audio media option.  Returns the state
         * of the given audio media option.
         *
         * @param option the option to retrieve state on
         * @return the state of the requested option
         * @see CTipAudioMediaOption
         */
        CTipMediaOption::OptionState
            GetMediaOptionState(CTipAudioMediaOption::AudioOption option) const;

        /**
         * Get the security state of this system.  Returns the
         * security state as configured in this TIP system.
         *
         * @return true if security is enabled, false otherwise
         */
        virtual bool GetSecurityState() const;

        /**
         * Get the MCU state of this system.  Returns the MCU state as
         * configured in this TIP system.
         *
         * @return true if MCU behavior is enabled, otherwise false
         */
        virtual bool GetMCUState() const;
        
        /**
         * Get the feedback state of this system.  Returns the
         * feedback state as configured in this TIP system.
         *
         * @return true if feedbak is enabled, false otherwise
         */
        virtual bool GetFeedbackState() const;

    protected:
    
        /**
         * Helper function.  Applies generic checking logic for changing
         * positions of any type.
         */
        virtual Status SetPosition(MediaType type, uint8_t pos,
                                   CPosBitset* pPos, bool value);

        /**
         * Helper function.  Does a generic lookup for an option of
         * the given type and value.
         */
        CTipMediaOption::OptionState
            GetMediaOptionState(MediaType type, uint16_t option) const;
        
        ProtocolVersion mVersion;
        CPosBitset mTxPos[MT_MAX];
        CPosBitset mRxPos[MT_MAX];
        PresentationFrameRate mPresFrameRate;
        uint64_t mConfID;
        uint8_t  mPartID[MAX_PARTICIPANT_ID_LENGTH];
        uint32_t mPartIDLen;
        bool mSecurityState;
        bool mMCUState;
        bool mFeedbackState;

        typedef std::map<uint16_t, CTipMediaOption> MediaOptionMap;
        MediaOptionMap mOptionMap[MT_MAX];

    private:
        // do not allow copy or assignment
        CTipSystem(const CTipSystem&);
        CTipSystem& operator=(const CTipSystem&);
    };

    /**
     * Pretty print operators.
     */
    std::ostream& operator<<(std::ostream& o, const CTipSystem& ams);
    std::ostream& operator<<(std::ostream& o, CTipSystem::PresentationFrameRate rate);
    
};

#endif
