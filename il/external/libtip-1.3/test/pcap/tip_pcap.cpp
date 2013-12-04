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

#include <net/ethernet.h>
#include <netinet/ip.h>
#define __FAVOR_BSD // use BSD names for UDP structure
#include <netinet/udp.h>
#undef __FAVOR_BSD
#include <getopt.h>
#include <stdint.h>
#include <pcap.h>

#include "rtcp_packet.h"
#include "rtcp_packet_factory.h"
#include "rtcp_tip_feedback_packet.h"

#include "tip.h"
#include "tip_profile.h"
#include "tip_relay.h"

uint32_t calc_ip_offset(const uint8_t* packet)
{
    // ethernet header, no VLANs yet
    return sizeof(struct ether_header);
}

uint32_t calc_udp_offset(const uint8_t* packet)
{
    uint32_t offset = calc_ip_offset(packet);

    // IP header
    struct ip* ip  = (struct ip*) (packet + sizeof(struct ether_header));
    offset += (ip->ip_hl * 4);

    return offset;
}

uint32_t calc_rtcp_offset(const uint8_t* packet)
{
    uint32_t offset = calc_udp_offset(packet);

    // UDP header
    offset += sizeof(struct udphdr);

    return offset;
}

void get_ip_string(char* str, struct in_addr addr)
{
    strcpy(str, inet_ntoa(addr));
}

void parse_packet(uint8_t* packet, struct pcap_pkthdr& header, bool verbose,
                  bool doAllRtcp, LibTip::MediaType mType)
{
    struct ip* ip   = (struct ip*) (packet + calc_ip_offset(packet));
    struct udphdr* udp = (struct udphdr*) (packet + calc_udp_offset(packet));
    uint32_t offset  = calc_rtcp_offset(packet);

    char srcIP[20];
    char dstIP[20];

    get_ip_string(srcIP, ip->ip_src);
    get_ip_string(dstIP, ip->ip_dst);
    
    LibTip::CPacketBuffer buffer((packet + offset), (header.len - offset));

    // handle compound packets
    while (buffer.GetBufferSize()) {
        std::string type = "";
        LibTip::CRtcpPacket* rtcp_packet = LibTip::CRtcpPacketFactory::CreatePacketFromBuffer(buffer);

        if (rtcp_packet == NULL) {
            break;
        }

        if (rtcp_packet->GetType() == LibTip::CRtcpPacket::RTPFB) {
            LibTip::CRtcpAppFeedbackPacket* fb =
                dynamic_cast<LibTip::CRtcpAppFeedbackPacket*>(rtcp_packet);

            if (fb != NULL) {
                type = "FEEDBACK";
            }

        } else if (rtcp_packet->GetType() == LibTip::CRtcpPacket::APP) {

            LibTip::CRtcpTipPacket* tip_packet =
                dynamic_cast<LibTip::CRtcpTipPacket*>(rtcp_packet);

            if (tip_packet != NULL) {
                type = tip_packet->GetTipPacketTypeString();
            }
            
        } else if (rtcp_packet->GetType() == LibTip::CRtcpPacket::RR) {
            if (doAllRtcp) {
                type = "RR";
            }
        } else if (rtcp_packet->GetType() == LibTip::CRtcpPacket::SDES) {
            if (doAllRtcp) {
                type = "SDES";
            }
        }

        if (type != "") {
            printf("%08lu.%06lu:  %s:%05hu tx %s to %s:%05hu",
                   header.ts.tv_sec, (unsigned long) header.ts.tv_usec,
                   srcIP, ntohs(udp->uh_sport),
                   type.c_str(), dstIP, ntohs(udp->uh_dport));
            
            if (verbose) {
                std::ostringstream o;
                rtcp_packet->ToStream(o, mType);
                printf("%s\n\n", o.str().c_str());
            } else {
                printf("\n");
            }
        }
        
        delete rtcp_packet;
    }
}

class PcapExecPacketTransmit : public LibTip::CTipPacketTransmit {
public:
    virtual LibTip::Status Transmit(const uint8_t*, uint32_t, LibTip::MediaType)
    {
        return LibTip::TIP_OK;
    }
};        

void exec_packet(uint8_t* packet, struct pcap_pkthdr& header,
                 LibTip::MediaType mType, LibTip::CTip& tip)
{
    uint32_t offset  = calc_rtcp_offset(packet);
    tip.ReceivePacket((packet + offset), (header.len - offset), mType);
}

void classify_packet(uint8_t* packet, struct pcap_pkthdr& header)
{
    struct ip* ip   = (struct ip*) (packet + calc_ip_offset(packet));
    struct udphdr* udp = (struct udphdr*) (packet + calc_udp_offset(packet));
    uint32_t offset  = calc_rtcp_offset(packet);

    uint16_t pos = 0;
    LibTip::CTipRelay relay;
    LibTip::CTipRelay::Classification c =
        relay.Classify((packet + offset), (header.len - offset), pos);

    char srcIP[20];
    char dstIP[20];

    get_ip_string(srcIP, ip->ip_src);
    get_ip_string(dstIP, ip->ip_dst);
    
    printf("%08lu.%06lu:  %s:%05hu tx ",
           header.ts.tv_sec, (unsigned long) header.ts.tv_usec,
           srcIP, ntohs(udp->uh_sport));
           
    switch (c) {
    case LibTip::CTipRelay::SYSTEM:
        printf("SYSTEM ");
        break;
        
    case LibTip::CTipRelay::MEDIA_SOURCE:
        printf("MEDIA SOURCE (pos = 0x%hx) ", pos);
        break;
        
    case LibTip::CTipRelay::MEDIA_SINK:
        printf("MEDIA SINK (pos = 0x%hx) ", pos);
        break;

    case LibTip::CTipRelay::NOT_TIP:
    default:
        printf("NOT_TIP ");
        break;
    }

    printf("to %s:%05hu\n", dstIP, ntohs(udp->uh_dport));
}


int
main(int argc, char** argv)
{
    const char* inFile = "-";  // default to stdin
    const char* hostIP = NULL; // default to no filter on IP
    const char* port   = NULL; // default to not filter on port

    bool doRX       = true;  // default to showing pkts RX by ip and port
    bool doParse    = false; // default to no parsing
    bool doExecute  = false; // default to no execute
    bool doClassify = false; // default to no classify
    bool verbose    = false; // default to not verbose
    bool doAllRtcp  = false; // default to only MUX/TIP

    LibTip::MediaType mType = LibTip::VIDEO; // default to video

    char* filterString = NULL; // default to no user filter string

    uint8_t  profile = 1; // default to CTS-1000 profile
    
    const char* usageString =
        "\n"
        "[--file file]\n"
        "[--ip IP]\n"
        "[--port port]\n"
        "[--xmit]\n"
        "[--parse]\n"
        "[--exec]\n"
        "[--classify]\n"
        "[--verbose]\n"
        "[--allrtcp]\n"
        "[--audio]\n"
        "[--filter pcap_filter_string]\n"
        "[--profile 1 (CTS1000) | 2 (T1-CTS1000) | 3 (CTS3000)]\n";

    char* progName = argv[0];
    while (true) {
        int c = -1;

        static struct option long_options[] = {
            { "file",    1, 0, 'a' },
            { "ip",      1, 0, 'b' },
            { "port",    1, 0, 'c' },
            { "parse",   0, 0, 'd' },
            { "exec",    0, 0, 'e' },
            { "verbose", 0, 0, 'f' },
            { "audio",   0, 0, 'g' },
            { "xmit",    0, 0, 'h' },
            { "filter",  1, 0, 'i' },
            { "profile", 1, 0, 'j' },
            { "classify",0, 0, 'k' },
            { "allrtcp", 0, 0, 'l' },
            { NULL,      0, 0, 0 }
        };

        c = getopt_long_only(argc, argv, "", long_options, NULL);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'a':
            inFile = optarg;
            break;

        case 'b':
            hostIP = optarg;
            break;

        case 'c':
            port = optarg;
            break;
            
        case 'd':
            doParse = true;
            break;

        case 'e':
            doExecute = true;
            break;

        case 'f':
            verbose = true;
            break;

        case 'g':
            mType = LibTip::AUDIO;
            break;

        case 'h':
            doRX = false;
            break;

        case 'i':
            filterString = optarg;
            break;

        case 'j':
            if (sscanf(optarg, "%hhu", &profile) != 1) {
                printf("ERROR:  invalid profile '%s'\n", optarg);
            }
            break;

        case 'k':
            doClassify = true;
            break;
            
        case 'l':
            doAllRtcp = true;
            break;

        case '?':
            printf("usage:  %s %s", progName, usageString);
            return 0;
        }
    }

    // must give us something to do
    if (!doParse && !doExecute && !doClassify) {
        printf("ERROR:  must specify either --parse or --execute or --classify\n");
        printf("usage:  %s %s", progName, usageString);
        return 1;
    }
    
    // create an tip instance for executing the packet trace
    PcapExecPacketTransmit xmit;
    LibTip::CTip tip(xmit);
    
    // configure with requested profile
    if (profile == 1) {
        LibTip::CSingleScreenProfile::Configure(tip.GetTipSystem(), false, false);
    } else if (profile == 2) {
        LibTip::CSingleScreenExtendedReachProfile::Configure(tip.GetTipSystem(), false);
    } else {
        LibTip::CTripleScreenProfile::Configure(tip.GetTipSystem(), false, false);
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcap = pcap_open_offline(inFile, errbuf);
    if (pcap == NULL) {
        printf("ERROR opening pcap file:  '%s'\n", errbuf);
        return 1;
    }

    char filter_exp[2048] = { '\0' };

    if (filterString == NULL) {
        // default filter, only look at UDP
        sprintf(filter_exp, "ip proto \\udp");
        if (hostIP != NULL) {
            if (doRX) {
                sprintf(filter_exp, "%s && dst host %s", filter_exp, hostIP);
            } else {
                sprintf(filter_exp, "%s && src host %s", filter_exp, hostIP);
            }
        }
        if (port != NULL) {
            if (doRX) {
                sprintf(filter_exp, "%s && dst port %s", filter_exp, port);
            } else {
                sprintf(filter_exp, "%s && src port %s", filter_exp, port);
            }            
        }
        
    } else {
        strncpy(filter_exp, filterString, sizeof(filter_exp));
    }

    struct bpf_program fp;
    if (pcap_compile(pcap, &fp, filter_exp, 0, 0) == -1) {
        printf("ERROR:  couldn't compile filter '%s': '%s'\n",
               filter_exp, pcap_geterr(pcap));
        return 1;
    }
    if (pcap_setfilter(pcap, &fp) == -1) {
        printf("ERROR:  Couldn't set filter '%s': '%s'\n",
               filter_exp, pcap_geterr(pcap));
        return 1;
    }

    if (doExecute) {
        tip.StartTipNegotiate(mType);
    }
    
    uint32_t pid = 0;
    
    while (1) {
        struct pcap_pkthdr header;
        uint8_t* packet = (uint8_t*) pcap_next(pcap, &header);
        if (packet == NULL) {
            break;
        }

        pid++;
        
        if (header.len > header.caplen) {
            printf("WARNING:  ignoring packet # %u b/c it is truncated\n", pid);
            continue;
        }
        
        if (doParse) {
            parse_packet(packet, header, verbose, doAllRtcp, mType);
        }

        if (doExecute) {
            exec_packet(packet, header, mType, tip);
        }

        if (doClassify) {
            classify_packet(packet, header);
        }
    }
    
    pcap_close(pcap);
    return 0;
}
