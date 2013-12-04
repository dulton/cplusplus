/// @file
/// @brief Generator client handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _GENERATOR_CLIENT_HANDLER_H_
#define _GENERATOR_CLIENT_HANDLER_H_

#include <ace/Condition_Thread_Mutex.h>
#include <ace/Event_Handler.h>
#include <ace/Message_Queue.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <app/AppCommon.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <FastDelegate.h>
#include <Tr1Adapter.h>
#include <ildaemon/ilDaemonCommon.h>
#include <ace/Recursive_Thread_Mutex.h>

#include "AsyncCompletionToken.h"
#include "generator_3_port_client.h"

// Forward declarations (global)
class ACE_Reactor;

namespace IL_DAEMON_LIB_NS
{
    class BoundMethodRequest;
}

#include "FPGARTPCommon.h"

DECL_FPGARTP_NS
using namespace Generator_3_port_client;

///////////////////////////////////////////////////////////////////////////////


class GeneratorClientHandler : public ACE_Event_Handler, boost::noncopyable
{
  public:
    
    explicit GeneratorClientHandler(ACE_Reactor& reactor);
    ~GeneratorClientHandler();

    void SetMethodCompletionDelegate(VOIP_NS::methodCompletionDelegate_t delegate);
    void ClearMethodCompletionDelegate(void);
    
    bool GetStreamIndexes(uint16_t port, uint32_t streamBlock, std::vector<uint32_t>& streamIndexes);

    void StartStreams(uint16_t port, const std::vector<uint32_t>& streamIndexes, VOIP_NS::asyncCompletionToken_t act = VOIP_NS::asyncCompletionToken_t());
    void StopStreams(uint16_t port, const std::vector<uint32_t>& streamIndexes, VOIP_NS::asyncCompletionToken_t act = VOIP_NS::asyncCompletionToken_t());
    void ModifyStreamsDestination(uint16_t port, std::vector<uint32_t>& streamIndexes, const Generator_3_port_client::Ipv4Address_t& addr, std::vector<uint16_t>& udpPorts,uint8_t ipServiceLevel);
    void ModifyStreamsDestination(uint16_t port, std::vector<uint32_t>& streamIndexes, const Generator_3_port_client::Ipv6Address_t& addr, std::vector<uint16_t>& udpPorts,uint8_t ipServiceLevel);

  private:
    /// Input handler services ACE message queue from within ACE reactor thread
    int handle_timeout(const ACE_Time_Value &current_time, const void *arg);

    /// Private implementation class forward declaration
    class MessageSetClient;
    class FpgaCommandSeq;

    //typedef ACE_Message_Queue_Ex<IL_DAEMON_LIB_NS::BoundMethodRequest, ACE_MT_SYNCH> methodQueue_t;
    typedef boost::shared_ptr<FpgaCommandSeq> fpga_method_seq_t;

    const ACE_thread_t mOwner;                          ///< thread id of owner
    boost::scoped_ptr<MessageSetClient> mClient;        ///< message set client implementation

    std::vector<uint32_t> fpga_streamIndexes;
    std::vector<asyncCompletionToken_t> fpga_acts;
    std::vector<streamModifierArray_t> fpga_modifiers;
    ACE_Recursive_Thread_Mutex mLock;
    ACE_Recursive_Thread_Mutex mLockFpgaCommandSeq;
    std::vector<fpga_method_seq_t> methodSeqQueue;
};

typedef boost::shared_ptr<GeneratorClientHandler> GeneratorClientHandlerSharedPtr;

///////////////////////////////////////////////////////////////////////////////

END_DECL_FPGARTP_NS

#endif
