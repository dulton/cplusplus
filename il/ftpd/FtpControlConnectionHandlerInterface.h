/// @file
/// @brief Control Connection Handler common interface file.
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _FTP_CONTROL_CONNECTION_HANDLER_INTERFACE_H_
#define _FTP_CONTROL_CONNECTION_HANDLER_INTERFACE_H_

#include <string>
#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class FtpControlConnectionHandlerInterface 
{
public:
    FtpControlConnectionHandlerInterface() {}
    virtual ~FtpControlConnectionHandlerInterface() {};

    /// @brief Get the INET address for the local source of data connection
    virtual const ACE_INET_Addr& GetLocalDataInetAddr() const = 0 ;

    /// @brief Get the INET address for the remote destination of data connection
    virtual const ACE_INET_Addr& GetRemoteDataInetAddr() const = 0 ;

    /// @brief Get the interface name used by the control connection
    virtual const std::string& GetLocalIfName() const = 0 ;

    /// @brief Get the Tx body configuration.
    virtual void GetTxBodyConfig(uint8_t &szType, 
                         uint32_t &fixedSz, 
                         uint32_t &randomSzMean, 
                         uint32_t &randomStdDev, 
                         uint8_t &contentType) const = 0 ;    

    /// @brief Get the IPv4 TOS socket option
    virtual uint8_t GetIpv4Tos() const = 0;

    /// @brief Get the IPv6 traffic class socket option
    virtual uint8_t GetIpv6TrafficClass() const = 0;

    /// @brief Get the TCP window size limit socket option
    virtual uint32_t GetTcpWindowSizeLimit() const = 0;

    /// @brief Get the TCP delayed ack socket option
    virtual bool GetTcpDelayedAck() const= 0 ;

    /// @brief Notification that Data was established
    virtual void NotifyDataConnectionOpen(const ACE_Time_Value& connectTime) = 0 ;

    /// @brief Notification that Data connection traffic was sent
    virtual void NotifyIncrementalTxBytes(uint32_t bytes) = 0 ;

    /// @brief Notification that Data connection traffic was received
    virtual void NotifyIncrementalRxBytes(uint32_t bytes) = 0 ;

    /// @brief Notification that Data was closed
    virtual void NotifyDataConnectionClose(const ACE_Time_Value& closeTime) = 0 ;

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS


#endif


