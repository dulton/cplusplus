/// @file
/// @brief SIP core package common header.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPSTACK_H_
#define SIPSTACK_H_

#define INADDRESS_LEN  16

#if !defined(UPDATED_BY_SPIRENT)
#define UPDATED_BY_SPIRENT
#endif

#include "RvSipMid.h"
#include "RvSipStackTypes.h"
#include "RvSipStack.h"
#include "RvSipResolver.h"
#include "RvSipCallLegTypes.h"
#include "RvSipCallLeg.h"
#include "RvSipAddress.h"
#include "RvSipBody.h"
#include "RvSipBodyPart.h"
#include "RvSipContentTypeHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipSPIRENTEncoder.h"
#include "RvSipMsg.h"
#include "RvSipRegClient.h"
#include "RvSipPubClient.h"
#include "RvSipSubscription.h"
#include "SELI_API.h"
#include "rvsdp.h"
#include "RvSipPUriHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipPAccessNetworkInfoHeader.h"

#define BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE namespace voip { namespace signaling { namespace sip { namespace rv { namespace core {
#define END_DECL_RV_SIP_ENGINE_NAMESPACE }}}}}
#define USING_RV_SIP_ENGINE_NAMESPACE using namespace voip::signaling::sip::rv::core
#define RV_SIP_ENGINE_NAMESPACE voip::signaling::sip::rv::core

#endif /*SIPSTACK_H_*/
