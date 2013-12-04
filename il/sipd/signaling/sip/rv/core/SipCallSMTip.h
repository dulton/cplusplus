/// @file
/// @brief SIP core "engine" TIP state machine header.
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPCSMT_H_
#define SIPCSMT_H_

#include "SipCallSM.h"

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

//TIP implementation:
class SipCallSMTip : public SipCallSM {
  
 public:
  
 SipCallSMTip(SipCallSM* parent) : SipCallSM(parent) {}
  virtual ~SipCallSMTip() {}
  
 protected:
  
  virtual bool stateChangedEvHandler(
				     IN  RvSipCallLegHandle            hCallLeg,
				     IN  RvSipAppCallLegHandle         hAppCallLeg,
				     IN  RvSipCallLegState             eState,
				     IN  RvSipCallLegStateChangeReason eReason,
				     IN  SipChannel * sc);
  
  virtual bool byeStateChangedEvHandler(
					IN  RvSipCallLegHandle                hCallLeg,
					IN  RvSipAppCallLegHandle             hAppCallLeg,
					IN  RvSipTranscHandle                 hTransc,
					IN  RvSipAppTranscHandle              hAppTransc,
					IN  RvSipCallLegByeState              eByeState,
					IN  RvSipTransactionStateChangeReason eReason,
					IN  SipChannel* sc);

  virtual bool prackStateChangedEvHandler(
					  IN  RvSipCallLegHandle            hCallLeg,
					  IN  RvSipAppCallLegHandle         hAppCallLeg,
					  IN  RvSipCallLegPrackState        eState,
					  IN  RvSipCallLegStateChangeReason eReason,
					  IN  RvInt16                       prackResponseCode,
					  IN  SipChannel* sc);
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*SIPCSMT_H_*/
