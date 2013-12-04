/// @file
/// @brief SIP core "engine" state machine header.
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPCSM_H_
#define SIPCSM_H_

#include "RvSipHeaders.h"
#include "RvSipUtils.h"
#include "SipChannel.h"

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

//Interface class:
class SipCallSM {
  
 public:

#ifdef UNIT_TEST
  static bool reliableFlag;
#endif
  
  SipCallSM(SipCallSM *parent) : mParent(parent) {}
  virtual ~SipCallSM() {}
  
  void appStateChangedEvHandler(
				IN  RvSipCallLegHandle            hCallLeg,
				IN  RvSipAppCallLegHandle         hAppCallLeg,
				IN  RvSipCallLegState             eState,
				IN  RvSipCallLegStateChangeReason eReason,
				IN  SipChannel * sc) {
    SipCallSM *handler=this;
    while(handler && !(handler->stateChangedEvHandler(hCallLeg,hAppCallLeg,eState,eReason,sc))) {
      handler=handler->mParent;
    }
  }
  void appByeStateChangedEvHandler(
				   IN  RvSipCallLegHandle                hCallLeg,
				   IN  RvSipAppCallLegHandle             hAppCallLeg,
				   IN  RvSipTranscHandle                 hTransc,
				   IN  RvSipAppTranscHandle              hAppTransc,
				   IN  RvSipCallLegByeState              eByeState,
				   IN  RvSipTransactionStateChangeReason eReason,
				   IN  SipChannel* sc) {
    SipCallSM *handler=this;
    while(handler && !(handler->byeStateChangedEvHandler(hCallLeg,hAppCallLeg,hTransc,hAppTransc,eByeState,eReason,sc))) {
      handler=handler->mParent;
    }
  }

  void appPrackStateChangedEvHandler(
				     IN  RvSipCallLegHandle            hCallLeg,
				     IN  RvSipAppCallLegHandle         hAppCallLeg,
				     IN  RvSipCallLegPrackState        eState,
				     IN  RvSipCallLegStateChangeReason eReason,
				     IN  RvInt16                       prackResponseCode,
				     IN  SipChannel* sc) {
    SipCallSM *handler=this;
    while(handler && !(handler->prackStateChangedEvHandler(hCallLeg,hAppCallLeg,eState,eReason,prackResponseCode,sc))) {
      handler=handler->mParent;
    }
  }

 protected:
  virtual bool stateChangedEvHandler(
				     IN  RvSipCallLegHandle            hCallLeg,
				     IN  RvSipAppCallLegHandle         hAppCallLeg,
				     IN  RvSipCallLegState             eState,
				     IN  RvSipCallLegStateChangeReason eReason,
				     IN  SipChannel * sc) = 0;

  virtual bool byeStateChangedEvHandler(
					IN  RvSipCallLegHandle                hCallLeg,
					IN  RvSipAppCallLegHandle             hAppCallLeg,
					IN  RvSipTranscHandle                 hTransc,
					IN  RvSipAppTranscHandle              hAppTransc,
					IN  RvSipCallLegByeState              eByeState,
					IN  RvSipTransactionStateChangeReason eReason,
					IN  SipChannel* sc) = 0;

  virtual bool prackStateChangedEvHandler(
					  IN  RvSipCallLegHandle            hCallLeg,
					  IN  RvSipAppCallLegHandle         hAppCallLeg,
					  IN  RvSipCallLegPrackState        eState,
					  IN  RvSipCallLegStateChangeReason eReason,
					  IN  RvInt16                       prackResponseCode,
					  IN  SipChannel* sc) = 0;

  SipCallSM* mParent;
};

END_DECL_RV_SIP_ENGINE_NAMESPACE;

#endif /*SIPCSM_H_*/
