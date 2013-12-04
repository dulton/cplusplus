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

#include "tip_debug_print.h"
#include "tip.h"
#include "private/tip_impl.h"
using namespace LibTip;

CTip::CTip(CTipPacketTransmit& xmit)
{
    mImpl = new CTipImpl(xmit);
    if (mImpl == NULL) {
        AMDEBUG(INTERR, ("impl allocation failed!"));
    }
}

CTip::~CTip() {
    delete mImpl;
}

CTipSystem& CTip::GetTipSystem()
{
    return mImpl->GetTipSystem();
}

const CTipSystem& CTip::GetTipNegotiatedSystem() const
{
    return mImpl->GetTipNegotiatedSystem();
}

void CTip::SetCallback(CTipCallback* callback)
{
    mImpl->SetCallback(callback);
}

Status CTip::StartTipNegotiate(MediaType mType)
{
    return mImpl->StartTipNegotiate(mType);
}

Status CTip::StopTipNegotiate(MediaType mType)
{
    return mImpl->StopTipNegotiate(mType);
}

Status CTip::StartPresentation()
{
    return mImpl->StartPresentation();
}

Status CTip::StopPresentation()
{
    return mImpl->StopPresentation();
}

Status CTip::ReceivePacket(uint8_t* buffer, uint32_t size, MediaType mType)
{
    return mImpl->ReceivePacket(buffer, size, mType);
}

Status CTip::SendDelayedAck(void* id, MediaType mType)
{
    return mImpl->SendDelayedAck(id, mType);
}

void CTip::SetRetransmissionInterval(uint32_t intervalMS)
{
    mImpl->SetRetransmissionInterval(intervalMS);
}

void CTip::SetRetransmissionLimit(uint32_t limit)
{
    mImpl->SetRetransmissionLimit(limit);
}

uint64_t CTip::GetIdleTime() const
{
    return mImpl->GetIdleTime();
}

void CTip::DoPeriodicActivity()
{
    mImpl->DoPeriodicActivity();
}

uint32_t CTip::GetRTCPSSRC(MediaType mType) const
{
    return mImpl->GetRTCPSSRC(mType);
}

void CTip::SetRTCPSSRC(MediaType mType, uint32_t ssrc)
{
    mImpl->SetRTCPSSRC(mType, ssrc);
}
