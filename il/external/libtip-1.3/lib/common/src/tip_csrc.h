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

#ifndef TIP_CSRC_H
#define TIP_CSRC_H

#include <stdint.h>

namespace LibTip {

    /**
     * Helper class for manipulating CSRC values.
     */
    class CTipCSRC {
    public:
        CTipCSRC(uint32_t csrc = 0) : mCSRC(csrc) {}
        ~CTipCSRC() {}

        /**
         * Set the CSRC
         */
        void SetCSRC(uint32_t csrc) {
            mCSRC = csrc;
        }

        /**
         * Get the CSRC
         */
        uint32_t GetCSRC() const {
            return mCSRC;
        }
        
        /**
         * Set the clock ID portion of a CSRC
         */
        void SetClockID(uint32_t clockID) {
            mCSRC &= ~gCsrcClockIDMask;
            mCSRC |= (clockID << gCsrcClockIDShift);
        }

        /**
         * Get the clock ID portion of a CSRC
         */
        uint32_t GetClockID() const {
            return ((mCSRC & gCsrcClockIDMask) >> gCsrcClockIDShift);
        }

        /**
         * Set the output position of a CSRC
         */
        void SetOutputPos(uint8_t pos) {
            mCSRC &= ~gCsrcOutputPosMask;
            mCSRC |= (pos << gCsrcOutputPosShift);
        }

        /**
         * Get the output position of a CSRC
         */
        uint8_t GetOutputPos() const {
            return ((mCSRC & gCsrcOutputPosMask) >> gCsrcOutputPosShift);
        }
        
        /**
         * Set the source position of a CSRC
         */
        void SetSourcePos(uint8_t pos) {
            mCSRC &= ~gCsrcSourcePosMask;
            mCSRC |= (pos << gCsrcSourcePosShift);
        }

        /**
         * Get the source position of a CSRC
         */
        uint8_t GetSourcePos() const {
            return ((mCSRC & gCsrcSourcePosMask) >> gCsrcSourcePosShift);
        }
        
        /**
         * Set the sink position of a CSRC
         */
        void SetSinkPos(uint8_t pos) {
            mCSRC &= ~gCsrcSinkPosMask;
            mCSRC |= (pos << gCsrcSinkPosShift);
        }

        /**
         * Get the sink position of a CSRC
         */
        uint8_t GetSinkPos() const {
            return ((mCSRC & gCsrcSinkPosMask) >> gCsrcSinkPosShift);
        }
        
        /**
         * Masks used to insert/remove positions from an Tip CSRC.
         */
        static const uint32_t gCsrcClockIDMask    = 0xFFFFF000;
        static const uint32_t gCsrcClockIDShift   = 12;
        static const uint32_t gCsrcOutputPosMask  = 0x00000F00;
        static const uint32_t gCsrcOutputPosShift = 8;
        static const uint32_t gCsrcSourcePosMask  = 0x000000F0;
        static const uint32_t gCsrcSourcePosShift = 4;
        static const uint32_t gCsrcSinkPosMask    = 0x0000000F;
        static const uint32_t gCsrcSinkPosShift   = 0;
        
    protected:
        uint32_t mCSRC;
    };
};

#endif
