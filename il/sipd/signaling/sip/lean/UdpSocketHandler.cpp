/// @file
/// @brief UDP socket handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <ace/Reactor.h>
#include <ace/OS_NS_sys_socket.h>

#include <boost/bind.hpp>

#include <phxexception/PHXExceptionFactory.h>

#include <vif/Packet.h>

#include "VoIPLog.h"
#include "UdpSocketHandler.h"

#ifndef IPV6_TCLASS
# define IPV6_TCLASS 67         // ref include/linux/in6.h in kernel 2.6 sources
#endif

USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

const int UdpSocketHandler::RCVBUF_SIZE = 256 * 1024;

///////////////////////////////////////////////////////////////////////////////

UdpSocketHandler::UdpSocketHandler(ACE_Reactor& reactor, uint16_t portNumber)
    : mPortNumber(portNumber),
      mTxPacketDump(true),
      mRxPacketDump(true)
{
    // Open AF_INET6 UDP socket
    if ((mSockFd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler ctor] socket(AF_INET6, SOCK_DGRAM, 0) failed: " << strerror(errno));
        PHXExceptionFactory::ThrowException(errno);
    }

    // Bind to wildcard address and specific port number
    struct sockaddr_in6 sin6;
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(mPortNumber);
    sin6.sin6_flowinfo = 0;
    sin6.sin6_addr = in6addr_any;
    sin6.sin6_scope_id = 0;

    if (bind(mSockFd, reinterpret_cast<const struct sockaddr *>(&sin6), sizeof(sin6)) < 0)
    {
        const int errnoSave(errno);
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler ctor] bind(*:" << mPortNumber << ") failed: " << strerror(errnoSave));
        PHXExceptionFactory::ThrowException(errnoSave);
    }

    const int one = 1;
    if (setsockopt(mSockFd, SOL_IP, IP_PKTINFO, &one, sizeof(one)) < 0)
    {
        const int errnoSave(errno);
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler ctor] setsockopt(SOL_IP, IP_PKTINFO) failed: " << strerror(errnoSave));
        close(mSockFd);
        PHXExceptionFactory::ThrowException(errnoSave);
    }
    
    if (setsockopt(mSockFd, SOL_IPV6, IPV6_PKTINFO, &one, sizeof(one)) < 0)
    {
        const int errnoSave(errno);
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler ctor] setsockopt(SOL_IPV6, IPV6_PKTINFO) failed: " << strerror(errnoSave));
        close(mSockFd);
        PHXExceptionFactory::ThrowException(errnoSave);
    }
    
    if (setsockopt(mSockFd, SOL_SOCKET, SO_RCVBUF, &RCVBUF_SIZE, sizeof(RCVBUF_SIZE)) < 0)
    {
        const int errnoSave(errno);
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler ctor] setsockopt(SOL_SOCKET, SO_RCVBUF) failed: " << strerror(errnoSave));
        close(mSockFd);
        PHXExceptionFactory::ThrowException(errnoSave);
    }

    // Register ACE event handler for socket read events
    reactor.register_handler(this, READ_MASK);
}

///////////////////////////////////////////////////////////////////////////////

UdpSocketHandler::~UdpSocketHandler()
{
    ACE_Reactor *reactor(this->reactor());

    if (reactor)
        reactor->remove_handler(this, READ_MASK);

    close(mSockFd);
}

///////////////////////////////////////////////////////////////////////////////

void UdpSocketHandler::AttachReceiver(const ACE_INET_Addr& addr, unsigned int ifIndex, const receiverDelegate_t& delegate)
{
  const bool inserted = mReceivers.insert(std::make_pair(VoIPUtils::AddrKey(addr,ifIndex), delegate)).second;
  if (!inserted)
    throw EAddrInUse("UdpSocketHandler::AttachReceiver");
}

///////////////////////////////////////////////////////////////////////////////

void UdpSocketHandler::DetachReceiver(const ACE_INET_Addr& addr, unsigned int ifIndex)
{
  mReceivers.erase(VoIPUtils::AddrKey(addr,ifIndex));
}

///////////////////////////////////////////////////////////////////////////////

int UdpSocketHandler::handle_input(ACE_HANDLE)
{
    Packet packet;

    struct msghdr msg = { 0 };
    struct iovec iov;
    uint8_t cmsgBuffer[CMSG_SPACE(sizeof(struct in_pktinfo)) + CMSG_SPACE(sizeof(struct in6_pktinfo))];
    
    msg.msg_name = &packet.SrcAddr.sin6;
    msg.msg_namelen = sizeof(struct sockaddr_in6);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgBuffer;
    msg.msg_controllen = sizeof(cmsgBuffer);
    msg.msg_flags = 0;

    iov.iov_base = packet.Data();
    iov.iov_len = packet.Tailroom();

    // Receive packet and extended info using recvmsg()
    ssize_t bytesRecv;
    if ((bytesRecv = recvmsg(mSockFd, &msg, 0)) < 0)
    {
        if (errno == EINTR)
            return 0;

        const int errnoSave(errno);
        TC_LOG_ERR_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::handle_input] recvmsg() failed: " << strerror(errnoSave));
        PHXExceptionFactory::ThrowException(errnoSave);
    }

    if (bytesRecv == 0)
    {
        TC_LOG_WARN_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::handle_input] recvmsg() returned 0 bytes");
        return 0;
    }
        
    packet.Put(bytesRecv);

    const struct in_pktinfo* ipi = 0;
    const struct in6_pktinfo* ipi6 = 0;

    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != 0; cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_PKTINFO)
            ipi = reinterpret_cast<const struct in_pktinfo *>(CMSG_DATA(cmsg));

        if (cmsg->cmsg_level == SOL_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO)
            ipi6 = reinterpret_cast<const struct in6_pktinfo *>(CMSG_DATA(cmsg));
    }
    
    // AF_INET6 socket will always return source address in AF_INET6 format
    int addrFamily;

    if (IN6_IS_ADDR_V4MAPPED(&packet.SrcAddr.sin6.sin6_addr))
    {
        if (!ipi)
        {
            TC_LOG_WARN_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::handle_input] recvmsg() returned IPv4 packet but IP_PKTINFO is missing");
            return 0;
        }
        
        // Map IPv4 source address back into native format
        const in_addr_t s_addr = packet.SrcAddr.sin6.sin6_addr.s6_addr32[3];
        const uint16_t sin_port = packet.SrcAddr.sin6.sin6_port;

        packet.IfIndex = ipi->ipi_ifindex;
        packet.SrcAddr.sin.sin_family = AF_INET;
        packet.SrcAddr.sin.sin_addr.s_addr = s_addr;
        packet.SrcAddr.sin.sin_port = sin_port;
        packet.DestAddr.sin.sin_family = AF_INET;
        packet.DestAddr.sin.sin_addr = ipi->ipi_spec_dst;
        packet.DestAddr.sin.sin_port = htons(mPortNumber);

        addrFamily = AF_INET;
    }
    else
    {
        if (!ipi6)
        {
            TC_LOG_WARN_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::handle_input] recvmsg() returned IPv6 packet but IPV6_PKTINFO is missing");
            return 0;
        }
        
        packet.IfIndex = ipi6->ipi6_ifindex;
        packet.DestAddr.sin6.sin6_family = AF_INET6;
        packet.DestAddr.sin6.sin6_addr = ipi6->ipi6_addr;
        packet.DestAddr.sin6.sin6_port = htons(mPortNumber);

        addrFamily = AF_INET6;
    }

    // Dump packet for debugging
    if (mRxPacketDump && (log4cplus_table[0].LevelMask & PHX_LOG_DEBUG_FLAG) == PHX_LOG_DEBUG_FLAG)
    {
        char fromAddrStr[INET6_ADDRSTRLEN];
        char toAddrStr[INET6_ADDRSTRLEN];
        uint16_t fromPort;
        
        if (addrFamily == AF_INET)
        {
            inet_ntop(AF_INET, &packet.SrcAddr.sin.sin_addr, fromAddrStr, INET6_ADDRSTRLEN);
            fromPort = ntohs(packet.SrcAddr.sin.sin_port);
            inet_ntop(AF_INET, &packet.DestAddr.sin.sin_addr, toAddrStr, INET6_ADDRSTRLEN);
        }
        else
        {
            inet_ntop(AF_INET6, &packet.SrcAddr.sin6.sin6_addr, fromAddrStr, INET6_ADDRSTRLEN);
            fromPort = ntohs(packet.SrcAddr.sin6.sin6_port);
            inet_ntop(AF_INET6, &packet.DestAddr.sin6.sin6_addr, toAddrStr, INET6_ADDRSTRLEN);
        }
        
        TC_LOG_DEBUG_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::Receive] Received " << packet.Length() << "-byte packet from "
                           << fromAddrStr << ":" << fromPort <<" to " << toAddrStr << ":" << mPortNumber << " on ifIndex " << packet.IfIndex);

        DumpPacket(packet);
    }

    // Dispatch packet
    receiverMap_t::const_iterator iter = mReceivers.find(VoIPUtils::AddrKey(ACE_INET_Addr(&packet.DestAddr.sin, (addrFamily == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)),packet.IfIndex));
    if (iter != mReceivers.end()) {
        iter->second(packet);
    } else {
        TC_LOG_DEBUG_LOCAL(0, LOG_SOCKET, "[UdpSocketHandler::Receive] Packet was not dispatched");
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool UdpSocketHandler::SendImmediate(const Packet& packet)
{
    const int addrFamily = packet.SrcAddr.storage.ss_family;

    if (packet.DestAddr.storage.ss_family != addrFamily)
        throw EPHXInternal("UdpSocketHandler::SendImmediate");
    
    // Dump packet for debugging
    if (mTxPacketDump && (log4cplus_table[0].LevelMask & PHX_LOG_DEBUG_FLAG) == PHX_LOG_DEBUG_FLAG)
    {
        char fromAddrStr[INET6_ADDRSTRLEN];
        char toAddrStr[INET6_ADDRSTRLEN];
        uint16_t toPort;
        
        if (addrFamily == AF_INET)
        {
            inet_ntop(AF_INET, &packet.SrcAddr.sin.sin_addr, fromAddrStr, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET, &packet.DestAddr.sin.sin_addr, toAddrStr, INET6_ADDRSTRLEN);
            toPort = ntohs(packet.DestAddr.sin.sin_port);
        }
        else
        {
            inet_ntop(AF_INET6, &packet.SrcAddr.sin6.sin6_addr, fromAddrStr, INET6_ADDRSTRLEN);
            inet_ntop(AF_INET6, &packet.DestAddr.sin6.sin6_addr, toAddrStr, INET6_ADDRSTRLEN);
            toPort = ntohs(packet.DestAddr.sin6.sin6_port);
        }
        
        TC_LOG_DEBUG_LOCAL(packet.Port, LOG_SOCKET, "[UdpSocketHandler::SendImmediate] Transmitting " << packet.Length() << "-byte packet from "
                           << fromAddrStr << ":" << mPortNumber << " to " << toAddrStr << ":" << toPort << " on ifIndex " << packet.IfIndex);
        DumpPacket(packet);
    }

    // Send packet with sendmsg()
    struct msghdr msg = { 0 };
    struct sockaddr_in6 destAddr;
    struct iovec iov;
    uint8_t cmsgBuffer[CMSG_SPACE(sizeof(struct in_pktinfo)) + CMSG_SPACE(sizeof(struct in6_pktinfo))];

    msg.msg_name = &destAddr;
    msg.msg_namelen = sizeof(struct sockaddr_in6);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgBuffer;
    msg.msg_flags = 0;
    
    destAddr.sin6_family = AF_INET6;
    destAddr.sin6_flowinfo = 0;
    destAddr.sin6_scope_id = 0;

    if (addrFamily == AF_INET)
    {
        // Map IPv4 destination address into IPv6 format for AF_INET6 socket
        destAddr.sin6_addr.s6_addr32[0] = 0;
        destAddr.sin6_addr.s6_addr32[1] = 0;
        destAddr.sin6_addr.s6_addr32[2] = htonl(0xffff);
        destAddr.sin6_addr.s6_addr32[3] = packet.DestAddr.sin.sin_addr.s_addr;
        destAddr.sin6_port = packet.DestAddr.sin.sin_port;
        
        // We need to use IP_PKTINFO to tell the IPv4 stack which interface index to use
        msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
        
        struct in_pktinfo* ipi = reinterpret_cast<struct in_pktinfo *>(CMSG_DATA(cmsg));
        ipi->ipi_ifindex = packet.IfIndex;
        ipi->ipi_spec_dst.s_addr = packet.SrcAddr.sin.sin_addr.s_addr;
        ipi->ipi_addr.s_addr = packet.DestAddr.sin.sin_addr.s_addr;

        const int tos = static_cast<int>(packet.Flags & 0xff);
        if (setsockopt(mSockFd, SOL_IP, IP_TOS, &tos, sizeof(tos)) < 0)
        {
            TC_LOG_ERR_LOCAL(packet.Port, LOG_SOCKET, "[UdpSocketHandler::SendImmediate] setsockopt(SOL_IP, IP_TOS) failed: " << strerror(errno));
            return false;
        }
    }
    else
    {
        // IPv6 destination address is usable as-is
        destAddr.sin6_addr = packet.DestAddr.sin6.sin6_addr;
        destAddr.sin6_port = packet.DestAddr.sin6.sin6_port;

        // Stick with IPV6_PKTINFO
        msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));

        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_IPV6;
        cmsg->cmsg_type = IPV6_PKTINFO;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
            
        struct in6_pktinfo* ipi6 = reinterpret_cast<struct in6_pktinfo *>(CMSG_DATA(cmsg));
        ipi6->ipi6_addr = packet.SrcAddr.sin6.sin6_addr;
        ipi6->ipi6_ifindex = packet.IfIndex;

        const int tclass = static_cast<int>(packet.Flags & 0xff);
        if (setsockopt(mSockFd, SOL_IPV6, IPV6_TCLASS, &tclass, sizeof(tclass)) < 0)
        {
            TC_LOG_ERR_LOCAL(packet.Port, LOG_SOCKET, "[UdpSocketHandler::SendImmediate] setsockopt(SOL_IPV6, IPV6_TCLASS) failed: " << strerror(errno));
            return false;
        }
    }

    iov.iov_base = const_cast<uint8_t *>(packet.Data());
    iov.iov_len = packet.Length();
    
    // Send packet
    ssize_t bytesSent;
    if ((bytesSent = sendmsg(mSockFd, &msg, 0)) < 0)
    {
        TC_LOG_ERR_LOCAL(packet.Port, LOG_SOCKET, "[UdpSocketHandler::SendImmediate] sendmsg() failed on ifIndex " << packet.IfIndex << ": " << strerror(errno));
        return false;
    }
    
    if (static_cast<unsigned int>(bytesSent) != packet.Length())
    {
        TC_LOG_ERR_LOCAL(packet.Port, LOG_SOCKET, "[UdpSocketHandler::SendImmediate] sendmsg() failed to send complete packet on ifIndex " << packet.IfIndex);
        return false;
    }
    else
        return true;
}

///////////////////////////////////////////////////////////////////////////////

void UdpSocketHandler::DumpPacket(const Packet& packet) const
{
    const uint8_t *data = packet.Data();
    unsigned int length = packet.Length();
    unsigned int offset, dlen;

    for (offset = 0; length > 0; length -= dlen)
    {
        char buffer[80];
        char *bufPos = buffer;
        uint8_t ch;
        
	bufPos += sprintf(bufPos, "%08x: ", offset);
	dlen = (length >= 16) ? 16 : length;

	for (unsigned int i = 0; i < 16; i++)
	{
	    if (i < dlen)
		bufPos += sprintf(bufPos, "%02x ", data[offset+i]);
	    else
		bufPos += sprintf(bufPos, "   ");
	}
	    
	bufPos += sprintf(bufPos, " ");
	for (unsigned int i = 0; i < dlen; i++)
	{
	    ch = data[offset+i];
	    if (ch < 0x20 || ch > 0x7f)
		ch = '.';
		
	    bufPos += sprintf(bufPos, "%c", ch);
	}

        TC_LOG_DEBUG_LOCAL(packet.Port, LOG_SOCKET, buffer);
	offset += dlen;
    }
}

///////////////////////////////////////////////////////////////////////////////
