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

#include "rtcp_tip_types.h"
using namespace LibTip;

const char* LibTip::GetTipPacketTypeString(TipPacketType type) {
    switch (type) {
    case RESERVED0:
        return "RESERVED0";
            
    case MUXCTRL:
        return "MUXCTRL";
            
    case RESERVED2:
        return "RESERVED2";
            
    case NOTIFY:
        return "NOTIFY";
            
    case TIPECHO:
        return "ECHO";
            
    case TXFLOWCTRL:
        return "TXFLOWCTRL";
            
    case RXFLOWCTRL:
        return "RXFLOWCTRL";
            
    case MEDIAOPTS:
        return "MEDIAOPTS";
            
    case REFRESH:
        return "REFRESH";
            
    case RESERVED9:
        return "RESERVED9";
            
    case RESERVED10:
        return "RESERVED10";
            
    case RESERVED11:
        return "RESERVED11";
            
    case SPIMAP:
        return "SPIMAP";
            
    case RESERVED13:
        return "RESERVED13";
            
    case RESERVED14:
        return "RESERVED14";
            
    case REQTOSEND:
        return "REQTOSEND";
            
    case ACK_RESERVED0:
        return "ACK_RESERVED1";
            
    case ACK_MUXCTRL:
        return "ACK_MUXCTRL";
            
    case ACK_RESERVED2:
        return "ACK_RESERVED2";
            
    case ACK_NOTIFY:
        return "ACK_NOTIFY";
            
    case ACK_TIPECHO:
        return "ACK_ECHO";
            
    case ACK_TXFLOWCTRL:
        return "ACK_TXFLOWCTRL";
            
    case ACK_RXFLOWCTRL:
        return "ACK_RXFLOWCTRL";
            
    case ACK_MEDIAOPTS:
        return "ACK_MEDIAOPTS";
            
    case ACK_REFRESH:
        return "ACK_REFRESH";
            
    case ACK_RESERVED9:
        return "ACK_RESERVED9";
            
    case ACK_RESERVED10:
        return "ACK_RESERVED10";
            
    case ACK_RESERVED11:
        return "ACK_RESERVED11";
            
    case ACK_SPIMAP:
        return "ACK_SPIMAP";
            
    case ACK_RESERVED13:
        return "ACK_RESERVED13";
            
    case ACK_RESERVED14:
        return "ACK_14";
            
    case ACK_REQTOSEND:
        return "ACK_REQTOSEND";
            
    default:
        return "INVALID";
    }
}
