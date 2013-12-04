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

#ifndef DEBUG_TOOLS_H_
#define DEBUG_TOOLS_H_

#include <stdint.h>

#include <iostream>
#include <iomanip>

namespace LibTip {
    class HexDump {
    public:
        HexDump(const uint8_t* buffer, uint32_t size) :
            mpBuffer(buffer), mSize(size) {}
        ~HexDump() {}
    
        const uint8_t* mpBuffer;
        uint32_t mSize;
    };

    std::ostream& operator<<(std::ostream& o, const HexDump& hd) {
        const uint32_t numCols = 16;

        o << '<' << std::dec << hd.mSize << " bytes" << '>' << std::setfill('0');
        for (uint32_t i = 0; i < hd.mSize; i++) {
            if ((i % numCols) == 0) {
                o << '\n' << std::dec << std::setw(4) << i << ": ";
            }

            o << std::hex << std::setw(2) << (int) hd.mpBuffer[i] << ' ';
        }

        return o;
    }
};

#endif /* DEBUG_TOOLS_H_ */
