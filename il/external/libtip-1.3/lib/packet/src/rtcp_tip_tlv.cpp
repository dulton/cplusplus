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

#include <string.h>

#include "rtcp_tip_tlv.h"
using namespace LibTip;

CTipTlv::CTipTlv()
{
	mTag = 0;
	mLength = 0;
}

CTipTlv::~CTipTlv()
{
}

uint32_t CTipTlv::Pack(CPacketBuffer& buffer) const
{
    buffer.Add(mTag);
    buffer.Add(mLength);
    buffer.Add(mData, mLength);

    return buffer.GetBufferSize();
}

void CTipTlv::SetData(const uint8_t* data, uint32_t len)
{
	if (data == NULL && len != 0) {
		/* this is no good */
        return;
	}

    if (len > MAX_TLV_DATA_LEN) {
        /* reduce to max size */
        len = MAX_TLV_DATA_LEN;
    }
    
	memcpy(mData, data, len);
	mLength = len;
}

int CTipTlv::Unpack(CPacketBuffer& buffer)
{
    // verify we have at least the minimum number of bytes
    if (buffer.GetBufferSize() < MIN_TLV_PACK_LEN) {
        return -1;
    }

    buffer.Rem(mTag);
    buffer.Rem(mLength);

    // verify that we can get at least length bytes
    if (buffer.GetBufferSize() < mLength) {
        return -1;
    }

    buffer.Rem(mData, mLength);
    return 0;
}

