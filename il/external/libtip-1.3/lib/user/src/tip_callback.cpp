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

#include "tip_callback.h"
using namespace LibTip;

CTipCallback::CTipCallback()
{

}

CTipCallback::~CTipCallback()
{

}

void CTipCallback::TipNegotiationEarly(MediaType mType)
{

}

void CTipCallback::TipNegotiationFailed(MediaType mType)
{

}

bool CTipCallback::TipNegotiationMismatch(MediaType mType, uint8_t version)
{
    return true;
}

void CTipCallback::TipNegotiationIncompatible(MediaType mType, uint8_t version)
{

}

void CTipCallback::TipNegotiationLastAckReceived(MediaType mType)
{

}

bool CTipCallback::TipNegotiationLastAckTransmit(MediaType mType, bool doReinvite,
                                                 void* id)
{
    return true;
}

bool CTipCallback::TipNegotiationLastAckTransmit(MediaType mType, void* id)
{
    return true;
}

bool CTipCallback::TipNegotiationUpdate(MediaType mType, void* id)
{
    return true;
}

bool CTipCallback::TipSecurityKeyUpdate(MediaType mType, uint16_t spi, 
                                        const uint8_t* salt, const uint8_t* kek, 
                                        void* id)
{
    return true;
}

void CTipCallback::TipSecurityStateUpdate(MediaType mType, bool secure)
{
}

void
CTipCallback::LocalPresentationStart(uint8_t position,
                                     CTipSystem::PresentationStreamFrameRate fps)
{
    
}

void CTipCallback::LocalPresentationStop()
{

}

void CTipCallback::LocalPresentationFailure()
{

}

bool
CTipCallback::RemotePresentationStart(uint8_t vpos,
                                      CTipSystem::PresentationStreamFrameRate fps,
                                      void* id)
{
    return true;
}

void CTipCallback::RemotePresentationStop()
{

}

bool CTipCallback::ResolvePresentationConflict()
{
    return false;
}


