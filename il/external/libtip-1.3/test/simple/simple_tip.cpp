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

#include <stdlib.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>

#include "tip.h"
#include "tip_profile.h"
#include "tip_debug_print.h"

class SimplePacketXmit : public LibTip::CTipPacketTransmit {
public:
    SimplePacketXmit(int as, int vs, const char* rHost, uint16_t rPortA, uint16_t rPortV) :
        mAudioSocket(as), mVideoSocket(vs)
    {
        mRemoteAddrA.sin_family = AF_INET;
        mRemoteAddrA.sin_port   = htons(rPortA);

        mRemoteAddrV.sin_family = AF_INET;
        mRemoteAddrV.sin_port   = htons(rPortV);

        struct hostent* pHost = gethostbyname(rHost);
        if (!pHost || !pHost->h_addr_list[0]) {
            printf("ERROR could not resolve hostname %s\n", rHost);
            exit(1);
        }
        mRemoteAddrA.sin_addr.s_addr = *(uint32_t*)pHost->h_addr_list[0];
        mRemoteAddrV.sin_addr.s_addr = *(uint32_t*)pHost->h_addr_list[0];
    }

    ~SimplePacketXmit() {}

    LibTip::Status Transmit(const uint8_t* pktBuffer, uint32_t pktSize,
                            LibTip::MediaType mType)
    {
        int ret = -1;

        if (mType == LibTip::AUDIO) {
            ret = sendto(mAudioSocket, pktBuffer, pktSize, 0,
                         (struct sockaddr*) &mRemoteAddrA, sizeof(mRemoteAddrA));
        } else {
            ret = sendto(mVideoSocket, pktBuffer, pktSize, 0,
                         (struct sockaddr*) &mRemoteAddrV, sizeof(mRemoteAddrV));
        }            
        if (ret != (int) pktSize) {
            return LibTip::TIP_ERROR;
        }

        return LibTip::TIP_OK;
    }

    int                mAudioSocket;
    int                mVideoSocket;
    struct sockaddr_in mRemoteAddrA;
    struct sockaddr_in mRemoteAddrV;
};

class SimpleCallback : public LibTip::CTipCallback {
public:
    SimpleCallback() {
        for (LibTip::MediaType mType = LibTip::VIDEO; mType < LibTip::MT_MAX; mType++) {
            mLocalDone[mType] = false;
            mRemoteDone[mType] = false;
        }
    }
    
    virtual void TipNegotiationEarly(LibTip::MediaType mType) {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
    }
    virtual void TipNegotiationFailed(LibTip::MediaType mType) {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
    }

    virtual bool TipNegotiationMismatch(LibTip::MediaType mType, uint8_t version) {
        printf("#### %s %s %d\n", __FUNCTION__, LibTip::GetMediaString(mType),
               version);
        return true;
    }

    virtual void TipNegotiationIncompatible(LibTip::MediaType mType, uint8_t version) {
        printf("#### %s %s %d\n", __FUNCTION__, LibTip::GetMediaString(mType),
               version);
    }

    virtual void TipNegotiationLastAckReceived(LibTip::MediaType mType) {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
        mLocalDone[mType] = true;
    }

    virtual bool TipNegotiationLastAckTransmit(LibTip::MediaType mType, void* id) {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
        mRemoteDone[mType] = true;
        return true;
    }

    virtual bool TipNegotiationUpdate(LibTip::MediaType mType, void* id) {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
        return true;
    }

    virtual void
    LocalPresentationStart(uint8_t position,
                           LibTip::CTipSystem::PresentationStreamFrameRate fps) {
        printf("#### %s %u %d\n", __FUNCTION__, position, fps);
    }
    
    virtual void LocalPresentationStop() {
        printf("#### %s\n", __FUNCTION__);
    }
    
    virtual bool
    RemotePresentationStart(uint8_t position,
                            LibTip::CTipSystem::PresentationStreamFrameRate fps,
                            void* id)
    {
        printf("#### %s %u %d\n", __FUNCTION__, position, fps);
        return true;
    }

    virtual void RemotePresentationStop() {
        printf("#### %s\n", __FUNCTION__);
    }
    
    virtual bool TipSecurityKeyUpdate(LibTip::MediaType mType, uint16_t spi, 
                                      const uint8_t* salt, const uint8_t* kek, 
                                      void* id)
    {
        printf("#### %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType));
        return true;
    }

    virtual void TipSecurityStateUpdate(LibTip::MediaType mType, bool secure)
    {
        printf("#### %s %s %s\n", __FUNCTION__, LibTip::GetMediaString(mType),
               (secure ? "SECURE" : "NONSECURE"));
    }

    // variables set when certain callbacks occur
    bool mLocalDone[LibTip::MT_MAX];
    bool mRemoteDone[LibTip::MT_MAX];
};

int create_socket(uint16_t localPort)
{
    int s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        return s;
    }

	struct sockaddr_in localSin;
	localSin.sin_family = AF_INET;
	localSin.sin_port = htons(localPort);
	localSin.sin_addr.s_addr = INADDR_ANY;
    
	int ret = bind(s, (struct sockaddr*)&localSin, sizeof(localSin));
	if (ret) {
        printf("ERROR bind() failures\n");
        exit(1);
    }

    return s;
}

int
main(int argc, char** argv)
{
    // default values
    uint16_t localPort     = 40000;
    uint16_t remotePort    = 50000;
    const char* remoteHost = "127.0.0.1";
    bool     doAudio       = true;
    bool     doVideo       = true;
    uint32_t version       = LibTip::SUPPORTED_VERSION_MAX;
    uint8_t  profile       = 1;
    bool     isSecure      = false;
    bool     doAux30       = false;
    bool     doAuxOn       = false;

    // turn off logging by default
    // LibTip::gDebugFlags = 0;
    
    const char* usageString =
        "\n"
        "[--lport port]\n"
        "[--rport port]\n"
        "[--rhost host]\n"
        "[--noaudio]\n"
        "[--novideo]\n"
        "[--version version]\n"
        "[--profile 1 (CTS1000) | 2 (T1-CTS1000) | 3 (CTS3000)]\n"
        "[--aux30]\n"
        "[--auxon]\n"
        "[--secure]\n"
        "[--debug flags]\n";

    char* progName = argv[0];
    while (true) {
        int c = -1;

        static struct option long_options[] = {
            { "lport",   1, 0, 'a' },
            { "rport",   1, 0, 'b' },
            { "rhost",   1, 0, 'c' },
            { "noaudio", 0, 0, 'd' },
            { "novideo", 0, 0, 'e' },
            { "version", 1, 0, 'f' },
            { "profile", 1, 0, 'g' },
            { "aux30",   0, 0, 'h' },
            { "auxon",   0, 0, 'i' },
            { "secure",  0, 0, 'j' },
            { "debug",   1, 0, 'k' },
            { NULL,      0, 0, 0 }
        };

        c = getopt_long_only(argc, argv, "", long_options, NULL);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'a':
            if (sscanf(optarg, "%hu", &localPort) != 1) {
                printf("ERROR:  invalid local port '%s'\n", optarg);
            }
            break;

        case 'b':
            if (sscanf(optarg, "%hu", &remotePort) != 1) {
                printf("ERROR:  invalid remote port '%s'\n", optarg);
            }
            break;

        case 'c':
            remoteHost = optarg;
            break;

        case 'd':
            doAudio = false;
            break;

        case 'e':
            doVideo = false;
            break;

        case 'f':
            if (sscanf(optarg, "%u", &version) != 1) {
                printf("ERROR:  invalid version '%s'\n", optarg);
            }
            break;
            
        case 'g':
            if (sscanf(optarg, "%hhu", &profile) != 1) {
                printf("ERROR:  invalid profile '%s'\n", optarg);
            }
            break;

        case 'h':
            doAux30 = true;
            break;

        case 'i':
            doAuxOn = true;
            break;

        case 'j':
            isSecure = true;
            break;

        case 'k':
            if (sscanf(optarg, "%u", &LibTip::gDebugFlags) != 1) {
                printf("ERROR:  invalid debug flags '%s'\n", optarg);
            }
            break;
            
        case '?':
        default:
            printf("usage:  %s %s", progName, usageString);
            exit(0);
            
        }
    }

    int as = create_socket(localPort);
    if (as < 0) {
        printf("ERROR creating audio socket\n");
        return 1;
    }

    int vs = create_socket(localPort + 2);
    if (vs < 0) {
        printf("ERROR creating video socket\n");
        return 1;
    }
    
    // instantiate packet relay
    SimplePacketXmit xmit(as, vs, remoteHost, remotePort, (remotePort+2));
    
    // instantiate tip object
    LibTip::CTip tip(xmit);

    // configure with requested profile
    if (profile == 1) {
        LibTip::CSingleScreenProfile::Configure(tip.GetTipSystem(), doAux30, isSecure);
    } else if (profile == 2) {
        LibTip::CSingleScreenExtendedReachProfile::Configure(tip.GetTipSystem(), isSecure);
    } else {
        LibTip::CTripleScreenProfile::Configure(tip.GetTipSystem(), doAux30, isSecure);
    }

    // configure with a callback that just prints each invoked function
    SimpleCallback* sc = new SimpleCallback();
    tip.SetCallback(sc);

    // set the tip version to use
    if (tip.GetTipSystem().SetTipVersion((LibTip::ProtocolVersion) version) != LibTip::TIP_OK) {
        printf("ERROR setting TIP version to %u, using version %u instead\n",
               version, tip.GetTipSystem().GetTipVersion());
    }
    
    // good to go, start tip negotiation for audio and/or video
    if (doAudio) {
        tip.StartTipNegotiate(LibTip::AUDIO);
    } else {
        sc->mLocalDone[LibTip::AUDIO] = true;
        sc->mRemoteDone[LibTip::AUDIO] = true;
    }
    if (doVideo) {
        tip.StartTipNegotiate(LibTip::VIDEO);
    } else {
        sc->mLocalDone[LibTip::VIDEO] = true;
        sc->mRemoteDone[LibTip::VIDEO] = true;
    }

    // now we loop forever processing packets and handling tip events
    char recv_buffer[2048];
    
    while (1) {
        uint64_t delay = tip.GetIdleTime();
        struct timeval timeout;
        struct timeval* ptimeout;

        if (delay != (uint64_t) -1) {
            timeout.tv_sec = (delay / 1000);
            timeout.tv_usec = ((delay % 1000) * 1000);
            ptimeout = &timeout;
        } else {
            ptimeout = NULL;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(as, &fds);
        FD_SET(vs, &fds);

        int ret = select((std::max(as, vs) + 1), &fds, NULL, NULL, ptimeout);
        if (ret < 0) {
            printf("ERROR select failure\n");
            return 1;
        }

        if (ret == 0) {
            tip.DoPeriodicActivity();
        }

        if (FD_ISSET(as, &fds)) {
            ret = recv(as, recv_buffer, sizeof(recv_buffer), 0);
            if (ret < 0) {
                printf("ERROR recv() failure\n");
                return 1;
            }

            tip.ReceivePacket((uint8_t*) recv_buffer, ret, LibTip::AUDIO);
        }

        if (FD_ISSET(vs, &fds)) {
            ret = recv(vs, recv_buffer, sizeof(recv_buffer), 0);
            if (ret < 0) {
                printf("ERROR recv() failure\n");
                return 1;
            }

            tip.ReceivePacket((uint8_t*) recv_buffer, ret, LibTip::VIDEO);
        }

        bool negDone = (sc->mLocalDone[LibTip::AUDIO] && sc->mRemoteDone[LibTip::AUDIO] &&
                        sc->mLocalDone[LibTip::VIDEO] && sc->mRemoteDone[LibTip::VIDEO]);

        // assume control of aux if asked to do so and tip negotiation
        // is complete.
        if (doAuxOn && negDone) {
            tip.StartPresentation();
            doAuxOn = false;
        }
    }
}
