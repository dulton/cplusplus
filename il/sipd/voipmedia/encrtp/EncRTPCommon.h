/// @file
/// @brief Enc RTP Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _ENCRTP_COMMON_H_
#define _ENCRTP_COMMON_H_

/////////////////////////////////////////////////////////////////////////////

#include "VoIPCommon.h"
#include "EncTIP.h"


/////////////////////////////////////////////////////////////////////////////

#ifndef ENCRTP_NS
# define ENCRTP_NS MEDIA_NS::encrtp
#endif

#ifndef USING_ENCRTP_NS
# define USING_ENCRTP_NS using namespace MEDIA_NS::encrtp
#endif

#ifndef DECL_ENCRTP_NS
# define DECL_ENCRTP_NS DECL_MEDIA_NS namespace encrtp {
# define END_DECL_ENCRTP_NS END_DECL_MEDIA_NS }
#endif

#ifndef DECL_CLASS_FORWARD_ENCRTP_NS
# define DECL_CLASS_FORWARD_ENCRTP_NS(CLASSNAME) DECL_ENCRTP_NS class CLASSNAME; END_DECL_ENCRTP_NS
#endif

///////////////////////////////////////////////////////////////////////////////

DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

typedef uint32_t EncRTPIndex;

///////////////////////////////////////////////////////////////////////////////

typedef boost::function0<void> TrivialFunctionType;

///////////////////////////////////////////////////////////////////////////////

class EncRTPMediaEndpoint;
typedef EncRTPMediaEndpoint EndpointType;
typedef boost::shared_ptr<EndpointType> EndpointHandler;

///////////////////////////////////////////////////////////////////////////////

struct EncRTPProcessingParams {
  static const uint32_t MaxNumberOfPacketsScheduledOnce = 2000;
  static const uint32_t PacketSchedulerIntervalInMs = 200;
  static const uint32_t PacketSchedulerIntervalInMks = 200000;
  static const uint32_t InitialSessionDelay = 500;
  static const uint32_t CommandQueueSizeFactor = 10;
  static const uint32_t VQStatsCycles = 0x1F;
  static const uint32_t VQStatsCyclesInitial = 7;
  static const uint32_t MAX_IDS = 65536; //max number of enc rtp channels
  static const uint32_t MAX_TIME_OF_SILENT_REACTOR_CYCLES = 60000; //MS
  static const uint32_t REACTOR_EVENTS_TIMEOUT_MKS = 14000;
};

struct EncRTPOptimizationParams{
    static const int        CPU_ARCH_INTEL = 1;
    static const int        CPU_ARCH_OTHER = 0;
    static const uint32_t   VOICE_SOCK_BUFFER_SIZE = 32*1024;
    static const uint32_t   VIDEO_SOCK_BUFFER_SIZE = 64*1024;
    static const uint32_t   ReceiverProcessIdleDurationInMks= 5000;
    static const uint32_t   TIPPacketProcessIntervalInMs = 100;
    static const uint32_t   ProcessIdleDurationInMks= 5000;
    static const uint32_t   SenderProcessIdleDurationInMks= 5000;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_ENCRTP_NS

///////////////////////////////////////////////////////////////////////////////

#endif //_ENCRTP_COMMON_H_

