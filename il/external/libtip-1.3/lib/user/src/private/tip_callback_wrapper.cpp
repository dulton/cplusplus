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

#include "tip_callback_wrapper.h"
#include "tip_debug_print.h"
using namespace LibTip;

CTipCallbackWrapper::CTipCallbackWrapper(CTipCallback* callback) :
    mpCallback(callback)
{

}

CTipCallbackWrapper::~CTipCallbackWrapper()
{
    delete mpCallback;
}

void CTipCallbackWrapper::SetCallback(CTipCallback* callback)
{
    if (callback == NULL) {
        return;
    }

    if (callback == mpCallback) {
        return;
    }
    
    delete mpCallback;
    mpCallback = callback;
}

void CTipCallbackWrapper::TipNegotiationEarly(MediaType mType)
{
    AMDEBUG(USER, ("callback with %s", GetMediaString(mType)));
    mpCallback->TipNegotiationEarly(mType);
}

void CTipCallbackWrapper::TipNegotiationFailed(MediaType mType)
{
    AMDEBUG(USER, ("callback with %s", GetMediaString(mType)));
    mpCallback->TipNegotiationFailed(mType);
}

bool CTipCallbackWrapper::TipNegotiationMismatch(MediaType mType, uint8_t version)
{
    AMDEBUG(USER, ("callback with %s %d", GetMediaString(mType), version));

    bool ret = mpCallback->TipNegotiationMismatch(mType, version);

    AMDEBUG(USER, ("callback returned %d", ret));
    return ret;
}

void CTipCallbackWrapper::TipNegotiationIncompatible(MediaType mType, uint8_t version)
{
    AMDEBUG(USER, ("callback with %s", GetMediaString(mType), version));
    mpCallback->TipNegotiationIncompatible(mType, version);
}

void CTipCallbackWrapper::TipNegotiationLastAckReceived(MediaType mType)
{
    AMDEBUG(USER, ("callback with %s", GetMediaString(mType)));
    mpCallback->TipNegotiationLastAckReceived(mType);
}

bool
CTipCallbackWrapper::TipNegotiationLastAckTransmit(MediaType mType, bool doReinvite,
                                                   void* id)
{
    AMDEBUG(USER, ("callback with %s %s %p", GetMediaString(mType),
                   (doReinvite ? "TRUE" : "FALSE"), id));

    bool ret1 = mpCallback->TipNegotiationLastAckTransmit(mType, doReinvite, id);
    bool ret2 = mpCallback->TipNegotiationLastAckTransmit(mType, id);

    AMDEBUG(USER, ("callbacks returned %d %d (%d)", ret1, ret2, (ret1 && ret2)));
    return (ret1 && ret2);
}

bool CTipCallbackWrapper::TipNegotiationUpdate(MediaType mType, void* id)
{
    AMDEBUG(USER, ("callback with %s %p", GetMediaString(mType), id));

    bool ret = mpCallback->TipNegotiationUpdate(mType, id);

    AMDEBUG(USER, ("callback returned %d", ret));
    return ret;
}

bool CTipCallbackWrapper::TipSecurityKeyUpdate(MediaType mType, uint16_t spi, 
                                               const uint8_t* salt, const uint8_t* kek, 
                                               void* id)
{
    AMDEBUG(USER, ("callback with %s %hu %p %p %p",
                   GetMediaString(mType), spi, salt, kek, id));

    bool ret = mpCallback->TipSecurityKeyUpdate(mType, spi, salt, kek, id);

    AMDEBUG(USER, ("callback returned %d", ret));
    return ret;
}

void CTipCallbackWrapper::TipSecurityStateUpdate(MediaType mType, bool secure)
{
    AMDEBUG(USER, ("callback with %s %s",
                   GetMediaString(mType), (secure ? "TRUE" : "FALSE")));

    mpCallback->TipSecurityStateUpdate(mType, secure);
}

void
CTipCallbackWrapper::LocalPresentationStart(uint8_t position,
                                            CTipSystem::PresentationStreamFrameRate fps)
{
    AMDEBUG(USER, ("callback with %d %d", position, fps));
    mpCallback->LocalPresentationStart(position, fps);
}

void CTipCallbackWrapper::LocalPresentationStop()
{
    AMDEBUG(USER, ("callback"));
    mpCallback->LocalPresentationStop();
}

void CTipCallbackWrapper::LocalPresentationFailure()
{
    AMDEBUG(USER, ("callback"));
    mpCallback->LocalPresentationFailure();
}

bool
CTipCallbackWrapper::RemotePresentationStart(uint8_t vpos,
                                             CTipSystem::PresentationStreamFrameRate fps,
                                             void* id)
{
    AMDEBUG(USER, ("callback with %d %d %p", vpos, fps, id));

    bool ret = mpCallback->RemotePresentationStart(vpos, fps, id);

    AMDEBUG(USER, ("callback returned %d", ret));
    return ret;
}

void CTipCallbackWrapper::RemotePresentationStop()
{
    AMDEBUG(USER, ("callback"));
    mpCallback->RemotePresentationStop();
}

bool CTipCallbackWrapper::ResolvePresentationConflict()
{
    AMDEBUG(USER, ("callback"));

    bool ret = mpCallback->ResolvePresentationConflict();

    AMDEBUG(USER, ("callback returned %d", ret));
    return ret;
}


