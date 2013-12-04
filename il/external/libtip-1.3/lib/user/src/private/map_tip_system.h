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

#ifndef MAP_TIP_SYSTEM_H
#define MAP_TIP_SYSTEM_H

#include "tip_constants.h"
#include "tip_system.h"
#include "rtcp_tip_muxctrl_packet.h"
#include "rtcp_tip_mediaopts_packet.h"
#include "rtcp_tip_reqtosend_packet.h"

namespace LibTip {
    
    class CMapTipSystem : public CTipSystem {
    public:
        CMapTipSystem();
        virtual ~CMapTipSystem();

        CRtcpAppMuxCtrlPacketBase* MapToMuxCtrl(MediaType type) const;
        CRtcpAppMediaoptsPacket* MapToMediaOpts(MediaType type) const;
        CRtcpAppReqToSendPacket* MapToReqToSend() const;

        Status MapFromMuxCtrl(CRtcpAppMuxCtrlPacketBase& packet, MediaType type);
        Status MapFromMediaOpts(CRtcpAppMediaoptsPacket& packet, MediaType type);

        void NegotiateLocalRemote(const CTipSystem& local, const CTipSystem& remote);

        uint8_t MapToMuxCtrlSharedPos(MediaType type, uint16_t& shpos) const;
        void MapToActiveSharedPos(MediaType type, uint16_t& shpos) const;
        PresentationStreamFrameRate MapPositionToFrameRate(uint8_t position) const;
        
    protected:
        void MapVideoOption(const CTipMediaOption& opt, uint32_t& tx, uint32_t& rx) const;
        void MapAudioOption(const CTipMediaOption& opt, uint32_t& tx, uint32_t& rx) const;

        void AddVideoOption(CRtcpAppMediaoptsPacket::VideoOption opt, uint32_t tx, uint32_t rx);
        void AddVideoOption(CRtcpAppMediaoptsPacket::SharedOption opt, uint32_t tx, uint32_t rx);

        void AddAudioOption(CRtcpAppMediaoptsPacket::AudioOption opt, uint32_t tx, uint32_t rx);
        void AddAudioOption(CRtcpAppMediaoptsPacket::SharedOption opt, uint32_t tx, uint32_t rx);

        void AddSharedOption(MediaType type, CRtcpAppMediaoptsPacket::SharedOption opt,
                             uint32_t tx, uint32_t rx);
        void AddOption(MediaType type, uint16_t opt, bool tx, bool rx);

        void NegotiateOption(CTipVideoMediaOption::VideoOption opt,
                             const CTipSystem& local,
                             const CTipSystem& remote);
        void NegotiateOption(CTipAudioMediaOption::AudioOption opt,
                             const CTipSystem& local,
                             const CTipSystem& remote);
        void NegotiateOption(MediaType type, uint16_t opt,
                             CTipMediaOption::OptionState lState,
                             CTipMediaOption::OptionState rState);
    };

};
#endif
