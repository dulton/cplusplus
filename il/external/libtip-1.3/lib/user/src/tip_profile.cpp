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

#include "tip_profile.h"

using namespace LibTip;

static void addVideoMediaOption(CTipSystem& system,
                                CTipVideoMediaOption::VideoOption opt,
                                CTipMediaOption::OptionState state)
{
    CTipVideoMediaOption vmo(opt, state);
    system.AddMediaOption(vmo);
}

static void addAudioMediaOption(CTipSystem& system,
                                CTipAudioMediaOption::AudioOption opt,
                                CTipMediaOption::OptionState state)
{
    CTipAudioMediaOption amo(opt, state);
    system.AddMediaOption(amo);
}

static void mediaOptionsConfigure(CTipSystem& system, bool secure)
{
    /* add video media options */
    addVideoMediaOption(system, CTipVideoMediaOption::REFRESH_FLAG,
                        CTipMediaOption::OPTION_SUPPORTED_TX);
    addVideoMediaOption(system, CTipVideoMediaOption::INBAND_PARAM_SETS,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);
    addVideoMediaOption(system, CTipVideoMediaOption::CABAC,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);
    addVideoMediaOption(system, CTipVideoMediaOption::LTRP,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);
    addVideoMediaOption(system, CTipVideoMediaOption::GDR,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);
    addVideoMediaOption(system, CTipVideoMediaOption::HP_8X8_TRANSFORMS,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);
    

    /* add audio media options */
    addAudioMediaOption(system, CTipAudioMediaOption::ACTIVITY_METRIC,
                        CTipMediaOption::OPTION_SUPPORTED_TX);
    addAudioMediaOption(system, CTipAudioMediaOption::DYNAMIC_OUTPUT,
                        CTipMediaOption::OPTION_SUPPORTED_RX);
    addAudioMediaOption(system, CTipAudioMediaOption::CAPABLE_G722_LEGACY,
                        CTipMediaOption::OPTION_SUPPORTED_BOTH);

    /* enable EKT only for secure calls */
    if (secure) {
        addVideoMediaOption(system, CTipVideoMediaOption::EKT,
                            CTipMediaOption::OPTION_SUPPORTED_RX);
        addAudioMediaOption(system, CTipAudioMediaOption::EKT,
                            CTipMediaOption::OPTION_SUPPORTED_RX);
    }

    /* enable RTP feedback */
    system.SetFeedbackState(true);
}

static void singleScreenConfigure(CTipSystem& system)
{
    /* add video streams */
    system.AddBidirectional(VIDEO, POS_VIDEO_CENTER);
    system.AddTransmitter(VIDEO, POS_VIDEO_LEGACY_CENTER);

    /* add audio streams */
    system.AddBidirectional(AUDIO, POS_AUDIO_CENTER);
    system.AddBidirectional(AUDIO, POS_AUDIO_AUX);
    system.AddBidirectional(AUDIO, POS_AUDIO_LEGACY);
    system.AddReceiver(AUDIO, POS_AUDIO_LEFT);
    system.AddReceiver(AUDIO, POS_AUDIO_RIGHT);
}

void CSingleScreenProfile::Configure(CTipSystem& system, bool aux30FPS, bool secure)
{
    singleScreenConfigure(system);
    mediaOptionsConfigure(system, secure);
    
    if (aux30FPS) {
        system.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
    } else {
        system.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
    }

    system.SetSecurityState(secure);
}

void CSingleScreenExtendedReachProfile::Configure(CTipSystem& system, bool secure)
{
    singleScreenConfigure(system);
    mediaOptionsConfigure(system, secure);

    system.SetPresentationFrameRate(CTipSystem::PRES_1FPS_ONLY);

    system.SetSecurityState(secure);
}

void CTripleScreenProfile::Configure(CTipSystem& system, bool aux30FPS, bool secure)
{
    /* add video streams */
    system.AddBidirectional(VIDEO, POS_VIDEO_CENTER);
    system.AddBidirectional(VIDEO, POS_VIDEO_LEFT);
    system.AddBidirectional(VIDEO, POS_VIDEO_RIGHT);
    system.AddTransmitter(VIDEO, POS_VIDEO_LEGACY_CENTER);
    system.AddTransmitter(VIDEO, POS_VIDEO_LEGACY_LEFT);
    system.AddTransmitter(VIDEO, POS_VIDEO_LEGACY_RIGHT);

    if (aux30FPS) {
        system.SetPresentationFrameRate(CTipSystem::PRES_5FPS_AND_30FPS);
    } else {
        system.SetPresentationFrameRate(CTipSystem::PRES_5FPS_ONLY);
    }

    /* add audio streams */
    system.AddBidirectional(AUDIO, POS_AUDIO_CENTER);
    system.AddBidirectional(AUDIO, POS_AUDIO_LEFT);
    system.AddBidirectional(AUDIO, POS_AUDIO_RIGHT);
    system.AddBidirectional(AUDIO, POS_AUDIO_AUX);
    system.AddBidirectional(AUDIO, POS_AUDIO_LEGACY);

    mediaOptionsConfigure(system, secure);

    system.SetSecurityState(secure);
}
