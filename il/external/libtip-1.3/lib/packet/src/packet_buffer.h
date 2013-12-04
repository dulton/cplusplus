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

#ifndef PACKET_BUFFER_H
#define PACKET_BUFFER_H

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

namespace LibTip {

    // packet buffer implementation which requires a user buffer
    class CPacketBuffer {
    public:
        // construct based on given buffer, data is not copied
        CPacketBuffer(uint8_t* data, uint32_t length) {
            mpBuffer = data;
            mLength  = length;
            mHead    = 0;
            mTail    = length;
        }

        // destructor, data is not freed
        ~CPacketBuffer() {}

        // get a pointer to the buffer data
        uint8_t* GetBuffer() {
            return (mpBuffer + mHead);
        }

        // get the size of the used portion of the buffer
        uint32_t GetBufferSize() const {
            return (mTail - mHead);
        }

        // get our current offset into the buffer
        uint32_t GetBufferOffset() {
            return mHead;
        }
    
        // add a uint8_t buffer of given size to the end of the buffer, data
        // is copied
        void Add(const uint8_t* data, uint32_t length) {
            if ((mTail + length) > mLength) {
                length = (mLength - mTail);
            }
        
            memcpy((mpBuffer + mTail), data, length);
            mTail += length;
        }

        // add a 16 bit value to the end of the buffer.  byte swapping is
        // performed.
        void Add(const uint16_t& data) {
            uint16_t tmp = htons(data);
            Add((uint8_t*) &tmp, sizeof(data));
        }
    
        // add a 32 bit value to the end of the buffer.  byte swapping is
        // performed.
        void Add(const uint32_t& data) {
            uint32_t tmp = htonl(data);
            Add((uint8_t*) &tmp, sizeof(data));
        }

        // add a 64 bit value to the end of the buffer.  value is added as
        // two byte swapped 32 bit chunks (high 32 bits added "first")
        void Add(const uint64_t& data) {
            uint32_t tmp = htonl((uint32_t) (data >> 32));
            Add((uint8_t*) &tmp, sizeof(tmp));

            tmp = htonl((uint32_t) (data & 0xFFFFFFFF));
            Add((uint8_t*) &tmp, sizeof(tmp));
        }
    
        // add arbitrary data to the end of the packet buffer
        template< class T > void Add(const T& data) {
            Add((const uint8_t*) &data, sizeof(T));
        }

        // remove a uint8_t buffer of given size from the front of the
        // buffer.
        void Rem(uint8_t* buf, uint32_t length) {
            if ((mHead + length ) > mTail) {
                length = (mTail - mHead);
            }
        
            memcpy(buf, (mpBuffer + mHead), length);
            mHead += length;
        }

        // remove a 16 bit value from the front of the buffer, byte
        // swapping is performed.
        void Rem(uint16_t& data) {
            Rem((uint8_t*) &data, sizeof(data));
            data = ntohs(data);
        }
    
        // remove a 32 bit value from the front of the buffer, byte
        // swapping is performed.
        void Rem(uint32_t& data) {
            Rem((uint8_t*) &data, sizeof(data));
            data = ntohl(data);
        }

        // remove a 64 bit value from the front of the buffer.  data is
        // read out as two byte swapped 32 bit values.
        void Rem(uint64_t& data) {
            uint32_t tmp[2];
            Rem((uint8_t*) tmp, sizeof(data));
        
            data  = ((uint64_t) ntohl(tmp[0])) << 32;
            data |= ntohl(tmp[1]);
        }
    
        // remove arbitrary data from the front of the buffer
        template< class T > void Rem(T& data) {
            Rem((uint8_t*) &data, sizeof(data));
        }
    
        // consume all remaining data, data is not returned
        void RemAll() {
            mHead = mTail;
        }

        // reset a packet buffer to contain no data
        void Reset() {
            mHead = 0;
            mTail = 0;
        }

        // reset a packet buffer head so data can be removed again
        void ResetHead(uint32_t offset = 0) {
            if (offset > mTail) {
                offset = mTail;
            }
        
            mHead = offset;
        }

        // reset a packet buffer tail so data can be added again
        void ResetTail(uint32_t offset = 0) {
            if (offset < mHead) {
                offset = mHead;
            }

            mTail = offset;
        }
        
    protected:
        uint8_t* mpBuffer;
        uint32_t mLength;
        uint32_t mHead;
        uint32_t mTail;

    private:
        // no access to default constructor
        CPacketBuffer();
    };

    // packet buffer implementation that provides its own buffer
    class CPacketBufferData : public CPacketBuffer {
    public:
        CPacketBufferData() : CPacketBuffer(mBuffer, kMaxBufferSize) {
            // reset tail as we want to start off empty
            mTail = 0;
        }
            
        ~CPacketBufferData() {
            
        }
            
    protected:
        static const uint32_t kMaxBufferSize = 2048;
        uint8_t mBuffer[kMaxBufferSize];
    };

};

#endif
