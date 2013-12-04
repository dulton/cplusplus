/// @file
/// @brief TCP source-port utilities
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <engine/ModifierBlock.h>
#include <engine/PlaylistInstance.h>

#include "TcpPortUtils.h"


USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

uint16_t TcpPortUtils::MakePort(uint16_t flowIdx, L4L7_ENGINE_NS::PlaylistInstance * pi)
{
    uint16_t port = pi->IsAttack() ? 
                    MakeAtkPort(flowIdx, pi->GetModifiers().GetCursor(), pi->GetAtkLoopIteration(flowIdx)) :
                    MakeStmPort(flowIdx, pi->GetModifiers().GetCursor()) ;

    return port;
}

///////////////////////////////////////////////////////////////////////////////

uint32_t TcpPortUtils::GetCursor(uint16_t port, L4L7_ENGINE_NS::PlaylistInstance * pi)
{
    L4L7_ENGINE_NS::ModifierBlock& modifiers = pi->GetModifiers();

    uint32_t cursor = GetCursor(modifiers.GetCursor(), port, pi->IsAttack());

    // NOTE: update the cursor so next time around the old cursor 
    //       will be accurate. We don't rely on this setting in Clone
    //       because another thread could be spawning at the same time
    pi->GetModifiers().SetCursor(cursor);

    return cursor;
}

///////////////////////////////////////////////////////////////////////////////

