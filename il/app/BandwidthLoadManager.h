/// @file
/// @brief Bandwidth Load Manager header file 
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _BANDWIDTH_LOAD_MANAGER_H_
#define _BANDWIDTH_LOAD_MANAGER_H_

#include <string>

#include <app/AppCommon.h>
#include <app/StreamSocketHandler.h>

DECL_L4L7_APP_NS

///////////////////////////////////////////////////////////////////////////////

class BandwidthLoadManager
{
  public:
    BandwidthLoadManager();
    ~BandwidthLoadManager();

  protected:
    void InitDebugInfo(std::string protoName, uint32_t hnd) { mProtoName = protoName; mBllHnd = hnd; }
    void InitBwTimeStorage() { mLastInput = mLastOutput = ACE_OS::gettimeofday(); }

    void SetEnableBwCtrl(bool enable) { mEnableBwCtrl = enable; }
    bool GetEnableBwCtrl() { return mEnableBwCtrl; }

    void SetEnableDynamicLoad(bool enable) { mEnableDynamicLoad = enable; }
    bool GetEnableDynamicLoad() { return mEnableDynamicLoad; }

    int32_t GetDynamicLoadTotal() { return mDynamicLoadTotal; }
    int32_t GetDynamicLoadPerConnBps() { return mDynamicLoadPerConnBps; }

    /// Dynamic Load Delegate Methods
    void RegisterDynamicLoadDelegates(StreamSocketHandler *ssh);
    void UnregisterDynamicLoadDelegates(StreamSocketHandler *ssh);
    size_t GetDynamicLoad(bool isInput);
    size_t ProduceDynamicLoad(bool isInput, const ACE_Time_Value& currTime);
    size_t ConsumeDynamicLoad(bool isInput, size_t consumed);

    int32_t ConfigDynamicLoad(size_t conn, int32_t dynamicLoad);   

  private:
    std::string mProtoName;                                     ///< Used to debug instance
    uint32_t mBllHnd;                                           ///< Used to debug instance

    bool mEnableDynamicLoad;                                    ///< Dynamic load
    bool mEnableBwCtrl;                                         ///< Dynamic load
    int32_t mDynamicLoadTotal;                                  ///< Dynamic load
    int32_t mDynamicLoadPerConnBps;                             ///< Dynamic load
    int32_t mPrevLoadPerConnBps;                                ///< Dynamic load
    ACE_Time_Value mLastInput;                                  ///< Dynamic load time storage
    ACE_Time_Value mLastOutput;                                 ///< Dynamic load time storage

    /// Need a map of these if going to BW load per connection
    size_t mDynamicLoadInput;
    size_t mDynamicLoadOutput;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_APP_NS

#endif
