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

#ifndef TEST_PACKET_DATA_H
#define TEST_PACKET_DATA_H

// these define the default bytes seen in an RTCP packet.
#define RTCP_HEADER_BYTES(subtype, type, len) (0x80 | (subtype)), (type), 0x00, (len)
#define RTCP_PACKET_BYTES(subtype, type, len) RTCP_HEADER_BYTES(subtype, type, len), 0x00, 0x00, 0x00, 0x00
#define RTCP_APP_PACKET_BYTES 0x00, 0x00, 0x00, 0x00
#define RTCP_APP_PACKET_BYTES_TP1 'x', 'c', 't', 's'
#define RTCP_TIP_PACKET_BYTES 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#endif
