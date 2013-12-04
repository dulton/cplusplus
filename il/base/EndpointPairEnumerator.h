/// @file
/// @brief Endpoint Pair Enumerator header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_ENDPOINT_PAIR_ENUMERATOR_H_
#define _L4L7_ENDPOINT_PAIR_ENUMERATOR_H_

#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <base/BaseCommon.h>
#include <vif/IfSetupCommon.h>
#include <ildaemon/ilDaemonCommon.h>

// Forward declarations (global)
class ACE_INET_Addr;

namespace IL_DAEMON_LIB_NS
{
    class LocalInterfaceEnumerator;
    class RemoteInterfaceEnumerator;
}

DECL_L4L7_BASE_NS

///////////////////////////////////////////////////////////////////////////////

class EndpointPairEnumerator : boost::noncopyable
{
  public:
    typedef EndpointConnectionPatternOptions pattern_t;
    
    EndpointPairEnumerator(uint16_t port, uint32_t srcIfHandle, const IFSETUP_NS::NetworkInterfaceStack_t& dstIfStack);
    ~EndpointPairEnumerator();

    /// @brief Set the connection pattern
    /// @param pattern One of the supported pattern options (PAIR, BACKBONE, etc.)
    void SetPattern(pattern_t pattern);

    /// @brief Set the source port number
    void SetSrcPortNum(uint16_t srcPortNum);

    /// @brief Set the destination port number
    void SetDstPortNum(uint16_t dstPortNum);

    /// @brief Return the total number of source endpoints
    size_t GetSrcTotalCount(void) const;

    /// @brief Return the total number of destination endpoints
    size_t GetDstTotalCount(void) const;
    
    /// @brief Reset the endpoint pair to its starting value
    void Reset(void);

    /// @brief Advance the endpoint pair to its next value
    void Next(void);
    
    /// @brief Return the current endpoint pair value
    /// @param srcIfName Source interface to bind
    /// @param srcAddr Source address
    /// @param dstAddr Destination address
    void GetEndpointPair(std::string& srcIfName, ACE_INET_Addr& srcAddr, ACE_INET_Addr& dstAddr);
    
#ifdef UNIT_TEST
    void SetLocalInterfaces(const std::vector<ACE_INET_Addr>& addrs);
#endif

  private:
    const uint16_t mPort;                                       ///< physical port number
    
    boost::scoped_ptr<IL_DAEMON_LIB_NS::LocalInterfaceEnumerator> mSrcEnum;  ///< endpoint source enumerator
    boost::scoped_ptr<IL_DAEMON_LIB_NS::RemoteInterfaceEnumerator> mDstEnum; ///< endpoint destination enumerator

    pattern_t mPattern;                                         ///< pattern selector
    size_t mSrcIndex;                                           ///< source index
    size_t mDstIndex;                                           ///< destination index
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_BASE_NS

#endif
