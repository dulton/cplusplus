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

#include "private/tip_impl.h"
#include "private/tip_pres_negotiate_state.h"
using namespace LibTip;

// globals
CNoPresState          LibTip::gNoPresState;
CLocalAssertPresState LibTip::gLocalAssertPresState;
CLocalPresState       LibTip::gLocalPresState;
CRemotePresState      LibTip::gRemotePresState;
COverridePresState    LibTip::gOverridePresState;

CPresState::CPresState()
{

}

CPresState::~CPresState()
{

}

Status CPresState::LocalAssert(CTipPresImpl* am)
{
    return TIP_ERROR;
}

Status CPresState::LocalAssertAck(CTipPresImpl* am, CRtcpAppReqToSendAckPacket& packet)
{
    return TIP_ERROR;
}

Status CPresState::LocalAssertTimeout(CTipPresImpl* am)
{
    return TIP_ERROR;
}

Status CPresState::LocalRelease(CTipPresImpl* am)
{
    return TIP_ERROR;
}

Status CPresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return TIP_ERROR;
}

Status CPresState::RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return TIP_ERROR;
}

Status CPresState::OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate)
{
    return TIP_ERROR;
}

CNoPresState::CNoPresState()
{

}

CNoPresState::~CNoPresState()
{

}

Status CNoPresState::LocalAssert(CTipPresImpl* am)
{
    return am->OnLocalAssert();
}

Status CNoPresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteAssert(packet);
}

const char* const CNoPresState::GetName()
{
    return "No Presentation";
}

CLocalAssertPresState::CLocalAssertPresState()
{

}

CLocalAssertPresState::~CLocalAssertPresState()
{

}

Status CLocalAssertPresState::LocalRelease(CTipPresImpl* am)
{
    return am->OnLocalRelease();
}

Status CLocalAssertPresState::LocalAssertAck(CTipPresImpl* am,
                                             CRtcpAppReqToSendAckPacket& packet)
{
    return am->OnLocalAssertAck(packet);
}

Status CLocalAssertPresState::LocalAssertTimeout(CTipPresImpl* am)
{
    return am->OnLocalAssertTimeout();
}

Status CLocalAssertPresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnConflictRemoteAssert(packet);
}

Status CLocalAssertPresState::OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate)
{
    return am->OnOptionUpdateLocalAssert(oldRate);
}

const char* const CLocalAssertPresState::GetName()
{
    return "Local Assert";
}

CLocalPresState::CLocalPresState()
{

}

CLocalPresState::~CLocalPresState()
{

}

Status CLocalPresState::LocalRelease(CTipPresImpl* am)
{
    return am->OnLocalRelease();
}

Status CLocalPresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteOverride(packet);
}

Status CLocalPresState::OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate)
{
    return am->OnOptionUpdateLocal(oldRate);
}

const char* const CLocalPresState::GetName()
{
    return "Local Control";
}

CRemotePresState::CRemotePresState()
{

}

CRemotePresState::~CRemotePresState()
{

}

Status CRemotePresState::LocalAssert(CTipPresImpl* am)
{
    return am->OnLocalAssert();
}

Status CRemotePresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteAssertUpdate(packet);
}

Status CRemotePresState::RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteRelease(packet);
}

Status CRemotePresState::OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate)
{
    return am->OnOptionUpdateRemote(oldRate);
}

const char* const CRemotePresState::GetName()
{
    return "Remote Control";
}

COverridePresState::COverridePresState()
{

}

COverridePresState::~COverridePresState()
{

}

Status COverridePresState::LocalRelease(CTipPresImpl* am)
{
    return am->OnLocalReleaseOverride();
}

Status COverridePresState::RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteAssertUpdate(packet);
}

Status COverridePresState::RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet)
{
    return am->OnRemoteReleaseLocalAssert(packet);
}

Status COverridePresState::OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate)
{
    return am->OnOptionUpdateRemote(oldRate);
}

const char* const COverridePresState::GetName()
{
    return "Remote Override";
}
