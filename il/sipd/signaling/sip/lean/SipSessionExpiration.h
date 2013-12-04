/// @file
/// @brief SIP Session Expiration header file
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_SESSION_EXPIRATION_H_
#define _SIP_SESSION_EXPIRATION_H_

#include <stdint.h>

#include "SipCommon.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

struct SipSessionExpiration
{
    // Enumerated constants
    enum Refresher
    {
        NO_REFRESHER = 0,
        UAC_REFRESHER,
        UAS_REFRESHER
    };

    // Default ctor
    SipSessionExpiration(void)
      : deltaSeconds(0),
        refresher(NO_REFRESHER)
    {
    }

    // Other useful ctors
    explicit SipSessionExpiration(uint32_t theDeltaSeconds)
      : deltaSeconds(theDeltaSeconds),
        refresher(NO_REFRESHER)
    {
    }

    SipSessionExpiration(uint32_t theDeltaSeconds, Refresher theRefresher)
      : deltaSeconds(theDeltaSeconds),
        refresher(theRefresher)
    {
    }
    
    void Reset(void)
    {
        deltaSeconds = 0;
        refresher = NO_REFRESHER;
    }

    uint32_t deltaSeconds;
    Refresher refresher;
};
    
///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
