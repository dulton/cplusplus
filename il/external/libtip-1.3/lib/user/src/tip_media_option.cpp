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

#include "tip_media_option.h"
using namespace LibTip;

CTipMediaOption::CTipMediaOption(MediaType type, uint16_t option, OptionState state) :
    mType(type), mOption(option), mState(state)
{
}

CTipMediaOption::~CTipMediaOption()
{
}

void CTipMediaOption::SetMediaType(MediaType type)
{
    mType = type;
}

MediaType CTipMediaOption::GetMediaType() const
{
    return mType;
}
    
void CTipMediaOption::SetOption(uint16_t option)
{
    mOption = option;
}

uint16_t CTipMediaOption::GetOption() const
{
    return mOption;
}

void CTipMediaOption::SetState(OptionState state)
{
    mState = state;
}

CTipMediaOption::OptionState CTipMediaOption::GetState() const
{
    return mState;
}

CTipVideoMediaOption::CTipVideoMediaOption(VideoOption option, OptionState state) :
    CTipMediaOption(VIDEO, option, state)
{

}

CTipVideoMediaOption::~CTipVideoMediaOption()
{

}

CTipAudioMediaOption::CTipAudioMediaOption(AudioOption option, OptionState state) :
    CTipMediaOption(AUDIO, option, state)
{

}

CTipAudioMediaOption::~CTipAudioMediaOption()
{

}
