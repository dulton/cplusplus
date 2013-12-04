/// @file
/// @brief Address Info implementation 
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "AddrInfo.h"

USING_DPG_NS;

///////////////////////////////////////////////////////////////////////////////

std::ostream& DPG_NS::operator<<(std::ostream& os, const AddrInfo& addr)
{
    char buffer[48]; 
    addr.locAddr.addr_to_string(buffer, sizeof(buffer));
    os << addr.locIfName << "/" << buffer;

    if (!addr.hasRem)
        return os;

    addr.remAddr.addr_to_string(buffer, sizeof(buffer));
    return os << "/" << buffer; 
}

///////////////////////////////////////////////////////////////////////////////

