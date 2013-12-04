/// @file
/// @brief SIP core "engine" state machine basic header.
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef SIPCSMB_H_
#define SIPCSMB_H_

#include "SipCallSM.h"

BEGIN_DECL_RV_SIP_ENGINE_NAMESPACE

//Base concrete implementation:
class SipCallSMBasic : public SipCallSM {
  
 public:
  
 SipCallSMBasic() : SipCallSM(0) {}
  virtual ~SipCallSMBasic() {}
  
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

#endif /*SIPCSMB_H_*/
