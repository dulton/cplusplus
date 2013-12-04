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

#ifndef RTCP_TIP_TLV_H
#define RTCP_TIP_TLV_H

#include <stdint.h>

#include "packet_buffer.h"

namespace LibTip {

    class CTipTlv {
    public:
        CTipTlv();
        virtual ~CTipTlv();

        /* set/get tag */
        void SetTag(uint8_t tag) { mTag = tag; }
        uint8_t GetTag() const { return mTag; }

        enum {
            MIN_TLV_DATA_LEN = 0,
            MAX_TLV_DATA_LEN = 255,
        };
    
        /* get data length */
        uint8_t GetLength() const { return mLength; }

        /* set length and data, data is copied.  */
        void SetData(const uint8_t* data, uint32_t len);
        const uint8_t* GetData() const { return mData; }

        /* copy TLV into the given buffer, return the # of bytes copied */
        enum {
            MIN_TLV_PACK_LEN = (MIN_TLV_DATA_LEN + 2),
            MAX_TLV_PACK_LEN = (MAX_TLV_DATA_LEN + 2)
        };
        uint32_t Pack(CPacketBuffer& buffer) const;

        uint32_t GetPackSize() const { return mLength + 2; }

        int Unpack(CPacketBuffer& buffer);
    
    protected:
        uint8_t mTag;
        uint8_t mLength;
        uint8_t mData[MAX_TLV_DATA_LEN]; // 0 or more bytes of data
    };

};

#endif
