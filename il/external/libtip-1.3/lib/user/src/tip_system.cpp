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

#include <string.h>

#include <iostream>
#include <iomanip>

#include "tip_system.h"

using namespace LibTip;

CTipSystem::CTipSystem() :
    mVersion(TIP_V7), mPresFrameRate(PRES_1FPS_ONLY), mConfID(0),
    mPartIDLen(0), mSecurityState(false), mMCUState(false), mFeedbackState(false)
{
}

CTipSystem::~CTipSystem()
{
}

ProtocolVersion CTipSystem::GetTipVersion() const
{
    return mVersion;
}

Status CTipSystem::SetTipVersion(ProtocolVersion version)
{
    if (version < SUPPORTED_VERSION_MIN ||
        version > SUPPORTED_VERSION_MAX) {
        
        return TIP_ERROR;
    }
    
    mVersion = version;
    return TIP_OK;
}

Status CTipSystem::SetPosition(MediaType type, uint8_t pos, CPosBitset* pPos,
                               bool value)
{
	if (type >= MT_MAX) {
		return TIP_ERROR;
	}
	
	if (pos == POS_CONTROL ||
		pos >= POS_MAX) {

		return TIP_ERROR;
	}

	pPos[type][pos] = value;
	return TIP_OK;
}

Status CTipSystem::AddTransmitter(MediaType type, uint8_t pos)
{
	return SetPosition(type, pos, mTxPos, true);
}

Status CTipSystem::RemoveTransmitter(MediaType type, uint8_t pos)
{
    return SetPosition(type, pos, mTxPos, false);
}

Status CTipSystem::AddReceiver(MediaType type, uint8_t pos)
{
	return SetPosition(type, pos, mRxPos, true);
}

Status CTipSystem::AddBidirectional(MediaType type, uint8_t pos)
{
	Status ret = AddTransmitter(type, pos);
    if (ret == TIP_OK) {
        ret = AddReceiver(type, pos);
    }
    
	return ret;
}

Status CTipSystem::SetPresentationFrameRate(PresentationFrameRate rate)
{
    if (rate == PRES_1FPS_AND_30FPS) {
        // users cannot set this rate, it can only be used as the
        // result of negotiation.
        return TIP_ERROR;
    }
    
    mPresFrameRate = rate;
    return TIP_OK;
}

Status CTipSystem::JoinConference(uint64_t confID,
                                  uint8_t* participantID,
                                  uint32_t participantIDLength)
{
	if (participantID == NULL && participantIDLength != 0) {
		return TIP_ERROR;
	}

    if (participantIDLength > MAX_PARTICIPANT_ID_LENGTH) {
        return TIP_ERROR;
    }
    
	mConfID    = confID;
    mPartIDLen = participantIDLength;

	if (participantID != NULL) {
		memcpy(mPartID, participantID, participantIDLength);
	}

	return TIP_OK;
}

Status CTipSystem::AddMediaOption(CTipVideoMediaOption& option)
{
    MediaOptionMap::value_type v(option.GetOption(), option);
    mOptionMap[option.GetMediaType()].erase(option.GetOption());
    mOptionMap[option.GetMediaType()].insert(v);
    return TIP_OK;
}

Status CTipSystem::AddMediaOption(CTipAudioMediaOption& option)
{
    MediaOptionMap::value_type v(option.GetOption(), option);
    mOptionMap[option.GetMediaType()].erase(option.GetOption());
    mOptionMap[option.GetMediaType()].insert(v);
    return TIP_OK;
}

Status CTipSystem::SetSecurityState(bool enable)
{
    mSecurityState = enable;
    return TIP_OK;
}

Status CTipSystem::SetMCUState(bool enable)
{
    mMCUState = enable;
    return TIP_OK;
}

Status CTipSystem::SetFeedbackState(bool enable)
{
    mFeedbackState = enable;
    return TIP_OK;
}

CTipMediaOption::OptionState
CTipSystem::GetMediaOptionState(CTipVideoMediaOption::VideoOption option) const
{
    return GetMediaOptionState(VIDEO, option);
}

CTipMediaOption::OptionState
CTipSystem::GetMediaOptionState(CTipAudioMediaOption::AudioOption option) const
{
    return GetMediaOptionState(AUDIO, option);
}

bool CTipSystem::GetSecurityState() const
{
    return mSecurityState;
}

bool CTipSystem::GetMCUState() const
{
    return mMCUState;
}

bool CTipSystem::GetFeedbackState() const
{
    return mFeedbackState;
}

CTipMediaOption::OptionState
CTipSystem::GetMediaOptionState(MediaType type, uint16_t option) const
{
    MediaOptionMap::const_iterator i = mOptionMap[type].find(option);
    if (i == mOptionMap[type].end()) {
        // if its not there then its not supported
        return CTipMediaOption::OPTION_NOT_SUPPORTED;
    }        

    return (*i).second.GetState();
}

static void PrintMediaOptionState(std::ostream& o, CTipMediaOption::OptionState state)
{
    switch (state) {
    case CTipMediaOption::OPTION_NOT_SUPPORTED:
        o << "NOT SUPPORTED";
        break;
        
    case CTipMediaOption::OPTION_SUPPORTED_TX:
        o << "TX SUPPORTED";
        break;

    case CTipMediaOption::OPTION_SUPPORTED_RX:
        o << "RX SUPPORTED";
        break;
        
    case CTipMediaOption::OPTION_SUPPORTED_BOTH:
        o << "TX & RX SUPPORTED";
        break;
    }
}

static void PrintMediaOption(std::ostream& o, const CTipSystem& ams,
                             CTipVideoMediaOption::VideoOption opt, 
                             const char* optstr)
{
    o << "\n\t  " << std::setw(22) << optstr << " -> ";

    CTipMediaOption::OptionState state = ams.GetMediaOptionState(opt);
    PrintMediaOptionState(o, state);
}

static void PrintMediaOption(std::ostream& o, const CTipSystem& ams,
                             CTipAudioMediaOption::AudioOption opt, 
                             const char* optstr)
{
    o << "\n\t  " << std::setw(22) << optstr << " -> ";

    CTipMediaOption::OptionState state = ams.GetMediaOptionState(opt);
    PrintMediaOptionState(o, state);
}

std::ostream& LibTip::operator<<(std::ostream& o, const CTipSystem& ams)
{
    o << "\n\tVERSION:  " << std::dec << static_cast<int>(ams.GetTipVersion())
      << "\n\tVIDEO TX: 0x" << std::hex << ams.GetTransmitters(VIDEO).to_ulong()
      << "\n\tVIDEO RX: 0x" << ams.GetReceivers(VIDEO).to_ulong()
      << "\n\tAUDIO TX: 0x" << std::hex << ams.GetTransmitters(AUDIO).to_ulong()
      << "\n\tAUDIO RX: 0x" << ams.GetReceivers(AUDIO).to_ulong()
      << "\n\tPRES:     " << ams.GetPresentationFrameRate()
      << "\n\tCONFID:   " << std::dec << ams.GetConfID()
      << "\n\tSECURITY: " << (ams.GetSecurityState() ? "YES" : "NO")
      << "\n\tMCU:      " << (ams.GetMCUState() ? "YES" : "NO")
      << "\n\tFEEDBACK: " << (ams.GetFeedbackState() ? "YES" : "NO")
      << "\n\tVIDEO OPT:";

    PrintMediaOption(o, ams, CTipVideoMediaOption::REFRESH_FLAG, "REFRESH_FLAG");
    PrintMediaOption(o, ams, CTipVideoMediaOption::INBAND_PARAM_SETS, "INBAND_PARAM_SETS");
    PrintMediaOption(o, ams, CTipVideoMediaOption::CABAC, "CABAC");
    PrintMediaOption(o, ams, CTipVideoMediaOption::LTRP, "LTRP");
    PrintMediaOption(o, ams, CTipVideoMediaOption::GDR, "GDR");
    PrintMediaOption(o, ams, CTipVideoMediaOption::EKT, "EKT");
    PrintMediaOption(o, ams, CTipVideoMediaOption::HP_8X8_TRANSFORMS, "HP_8X8_TRANSFORMS");
    PrintMediaOption(o, ams, CTipVideoMediaOption::UNRESTRICTED_XGA_1_5, "UNRESTRICTED_XGA_1_5");
    PrintMediaOption(o, ams, CTipVideoMediaOption::UNRESTRICTED_720P, "UNRESTRICTED_720P");
    PrintMediaOption(o, ams, CTipVideoMediaOption::UNRESTRICTED_1080P, "UNRESTRICTED_1080P");
    PrintMediaOption(o, ams, CTipVideoMediaOption::UNRESTRICED_XGA_30, "UNRESTRICED_XGA_30");
    
    o << "\n\tAUDIO OPT:";
    PrintMediaOption(o, ams, CTipAudioMediaOption::ACTIVITY_METRIC, "ACTIVITY_METRIC");
    PrintMediaOption(o, ams, CTipAudioMediaOption::DYNAMIC_OUTPUT, "DYNAMIC_OUTPUT");
    PrintMediaOption(o, ams, CTipAudioMediaOption::CAPABLE_G722_LEGACY, "CAPABLE_G722_LEGACY");
    PrintMediaOption(o, ams, CTipAudioMediaOption::USING_G722_LEGACY, "USING_G722_LEGACY");
    PrintMediaOption(o, ams, CTipAudioMediaOption::EKT, "EKT");

    return o;
}

std::ostream& LibTip::operator<<(std::ostream& o, CTipSystem::PresentationFrameRate rate) {
    switch (rate) {
    case CTipSystem::PRES_1FPS_ONLY:
        o << "1FPS";
        break;

    case CTipSystem::PRES_5FPS_ONLY:
        o << "5FPS";
        break;
        
    case CTipSystem::PRES_30FPS_ONLY:
        o << "30FPS";
        break;
        
    case CTipSystem::PRES_1FPS_AND_30FPS:
        o << "1FPS 30FPS";
        break;
        
    case CTipSystem::PRES_5FPS_AND_30FPS:
        o << "5FPS 30FPS";
        break;
    }

    return o;
}
