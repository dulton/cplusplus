/// @file
/// @brief Endpoint Pair Enumerator implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <sstream>

#include <phxexception/PHXException.h>
#include <phxlog/phxlog.h>

#include "EndpointPairEnumerator.h"
#include <ildaemon/LocalInterfaceEnumerator.h>
#include <ildaemon/RemoteInterfaceEnumerator.h>

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

EndpointPairEnumerator::EndpointPairEnumerator(uint16_t port, uint32_t srcIfHandle, const IFSETUP_NS::NetworkInterfaceStack_t& dstIfStack)
    : mPort(port),
      mSrcEnum(new IL_DAEMON_LIB_NS::LocalInterfaceEnumerator(mPort, srcIfHandle)),
      mDstEnum(new IL_DAEMON_LIB_NS::RemoteInterfaceEnumerator(mPort, dstIfStack)),
      mPattern(EndpointConnectionPatternOptions_PAIR),
      mSrcIndex(0),
      mDstIndex(0)
{
}

///////////////////////////////////////////////////////////////////////////////

EndpointPairEnumerator::~EndpointPairEnumerator()
{
}

///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::SetPattern(pattern_t pattern)
{
    switch (mPattern)
    {
      case EndpointConnectionPatternOptions_PAIR:
      case EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST:
      case EndpointConnectionPatternOptions_BACKBONE_DST_FIRST:
      case EndpointConnectionPatternOptions_BACKBONE_INTERLEAVED:
        mPattern = pattern;
        break;
        
      default:
      {
          std::ostringstream oss;
          oss << "[EndpointPairEnumerator::SetPattern] Unsupported endpoint connection pattern option ("<< tms_enum_to_string(pattern, em_EndpointConnectionPatternOptions) << ")";
          const std::string err(oss.str());
          TC_LOG_ERR(mPort, err);
          throw EPHXBadConfig(err);
      }
    }
}

///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::SetSrcPortNum(uint16_t srcPortNum)
{
    mSrcEnum->SetPortNum(srcPortNum);
}

///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::SetDstPortNum(uint16_t dstPortNum)
{
    mDstEnum->SetPortNum(dstPortNum);
}

///////////////////////////////////////////////////////////////////////////////

size_t EndpointPairEnumerator::GetSrcTotalCount(void) const
{
    return mSrcEnum->TotalCount();
}

///////////////////////////////////////////////////////////////////////////////

size_t EndpointPairEnumerator::GetDstTotalCount(void) const
{
    return mDstEnum->TotalCount();
}
    
///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::Reset(void)
{
    mSrcEnum->Reset();
    mDstEnum->Reset();
    mSrcIndex = mDstIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::Next(void)
{
    switch (mPattern)
    {
      case EndpointConnectionPatternOptions_PAIR:
          if (++mSrcIndex >= mSrcEnum->TotalCount() || ++mDstIndex >= mDstEnum->TotalCount())
          {
              mSrcEnum->Reset();
              mDstEnum->Reset();
              mSrcIndex = mDstIndex = 0;
          }
          else
          {
              mSrcEnum->Next();
              mDstEnum->Next();
          }
          break;

      case EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST:
          if (++mSrcIndex >= mSrcEnum->TotalCount())
          {
              mSrcEnum->Reset();
              mSrcIndex = 0;
              mDstEnum->Next();
          }
          else
              mSrcEnum->Next();
          break;
          
      case EndpointConnectionPatternOptions_BACKBONE_DST_FIRST:
          if (++mDstIndex >= mDstEnum->TotalCount())
          {
              mDstEnum->Reset();
              mDstIndex = 0;
              mSrcEnum->Next();
          }
          else
              mDstEnum->Next();
          break;

      case EndpointConnectionPatternOptions_BACKBONE_INTERLEAVED:
          mSrcEnum->Next();
          mDstEnum->Next();
          break;
          
      default:
          throw EPHXInternal("EndpointPairEnumerator::Next");
    }
}

///////////////////////////////////////////////////////////////////////////////

void EndpointPairEnumerator::GetEndpointPair(std::string& srcIfName, ACE_INET_Addr& srcAddr, ACE_INET_Addr& dstAddr)
{
    mSrcEnum->GetInterface(srcIfName, srcAddr);
    mDstEnum->GetInterface(dstAddr);
}

///////////////////////////////////////////////////////////////////////////////
#ifdef UNIT_TEST
void EndpointPairEnumerator::SetLocalInterfaces(const std::vector<ACE_INET_Addr>& addrs)
{
    mSrcEnum->SetInterfaces(addrs);
}
#endif
///////////////////////////////////////////////////////////////////////////////
