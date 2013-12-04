/// @file
/// @brief SIP core "engine" state machine implementation.
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "SipCallSMTip.h"

USING_RV_SIP_ENGINE_NAMESPACE;

// A TIP event has come ... 
bool SipCallSMTip::stateChangedEvHandler(
        IN  RvSipCallLegHandle            hCallLeg,
        IN  RvSipAppCallLegHandle         hAppCallLeg,
        IN  RvSipCallLegState             eState,
        IN  RvSipCallLegStateChangeReason eReason,
        IN  SipChannel* sc)
{
    ENTER();

    RVSIPPRINTF("%s: hCallLeg=0x%x, eState=%lu, eReason=%lu\n",__FUNCTION__,(int)hCallLeg,(unsigned long)eState,(unsigned long)eReason);

    if(!sc) return false;

    if (eState == RVSIP_CALL_LEG_STATE_OFFERING) {  
        if(sc->mHCallLeg) {
            RvSipCallLegState    cleState = RVSIP_CALL_LEG_STATE_UNDEFINED;
            RvSipCallLegGetCurrentState (sc->mHCallLeg,&cleState);
            if(cleState != RVSIP_CALL_LEG_STATE_UNDEFINED  &&
                    cleState != RVSIP_CALL_LEG_STATE_IDLE &&
                    cleState != RVSIP_CALL_LEG_STATE_OFFERING &&
                    cleState != RVSIP_CALL_LEG_STATE_DISCONNECTED &&
                    cleState != RVSIP_CALL_LEG_STATE_DISCONNECTING &&
                    cleState != RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT &&
                    cleState != RVSIP_CALL_LEG_STATE_CANCELLING &&
                    cleState != RVSIP_CALL_LEG_STATE_CANCELLED &&
                    cleState != RVSIP_CALL_LEG_STATE_TERMINATED) {
                RvUint16 code=486;
                RvSipCallLegReject ( hCallLeg, code );
                return true;
            }
        }
        sc->mRole=TERMINATE;
        sc->setCallLeg(hCallLeg);
    }

    if (sc->mHCallLeg != hCallLeg) {
        if(eState != RVSIP_CALL_LEG_STATE_UNDEFINED  &&
                eState != RVSIP_CALL_LEG_STATE_IDLE &&
                eState != RVSIP_CALL_LEG_STATE_TERMINATED) {
            if(eState != RVSIP_CALL_LEG_STATE_DISCONNECTED &&
                    eState != RVSIP_CALL_LEG_STATE_DISCONNECTING &&
                    eState != RVSIP_CALL_LEG_STATE_CANCELLING &&
                    eState != RVSIP_CALL_LEG_STATE_CANCELLED) {
                RvSipCallLegDisconnect(hCallLeg);
            } else {
                RvSipCallLegTerminate(hCallLeg);
            }
        }
        return true;
    }

    if (sc->isTerminatePrivate()) {
        // TERMINATE state sequence: 
        // IDLE -> OFFERING -> ACCEPTED -> CONNECTED -> DISCONNECTED

        switch(eState) {

            case RVSIP_CALL_LEG_STATE_OFFERING: {

                                                    TRACE("(TIP)T:OFFERING\n");

                                                    sc->stopAllTimers();
                                                    sc->clearBasicData();

                                                    if(sc->mMediaHandler) {
                                                        RvUint16 code=0;
                                                        RvStatus rv = sc->mMediaHandler->media_onOffering(hCallLeg,code,false);
                                                        if(rv<0) {
                                                            if(code<1) code=415;
                                                            RvSipCallLegReject ( hCallLeg, code );
                                                            return true;
                                                        }
                                                    }

                                                    sc->setRefreshTimer(false);

                                                    sc->setLocalCallLegData();

                                                    RvSipTransaction100RelStatus relStatus=RVSIP_TRANSC_100_REL_UNDEFINED;
                                                    RvSipCallLegGet100RelStatus(hCallLeg,&relStatus);
                                                    RVSIPPRINTF("%s: relStatus=%d\n",__FUNCTION__,(int)relStatus);
                                                    if((sc->isRel100() && relStatus == RVSIP_TRANSC_100_REL_SUPPORTED) ||
                                                            relStatus == RVSIP_TRANSC_100_REL_REQUIRED) {
                                                        RvSipCallLegProvisionalResponseReliable(hCallLeg, 180);
                                                        sc->setReadyToAccept(false);
                                                        sc->setRingTimer();
                                                        sc->setRel100(true);
                                                        sc->setPrackTimer();
                                                    } else {
                                                        RvSipCallLegProvisionalResponse(hCallLeg, 180);
                                                        sc->setReadyToAccept(true);
                                                        sc->setRingTimer();
                                                    }

                                                    break;
                                                }

            case RVSIP_CALL_LEG_STATE_ACCEPTED: {
                                                    TRACE("(TIP)T:ACCEPTED\n");
                                                    sc->prackTimerStop();
                                                    if(sc->mMediaHandler) {
                                                        if(sc->RemoteSDPOffered()){
                                                            sc->mMediaHandler->media_startTip(sc->generate_mux_csrc());
                                                            sc->TipStarted(true);
                                                        }
                                                    }
                                                    
                                                    sc->markInvite200TxStartTime();
                                                    break;
                                                }

            case RVSIP_CALL_LEG_STATE_CONNECTED: {
                                                     //TODO start TIP negotiation.
                                                     TRACE("(TIP)T:CONNECTED\n");
                                                     if(!sc->TipStarted()){
                                                         if(sc->mMediaHandler) {
                                                             RvUint16 code=0;
                                                             RvStatus rv = sc->mMediaHandler->media_prepareConnect(hCallLeg,code,NULL);
                                                             if(rv<0) {
                                                                 RvSipCallLegDisconnect(hCallLeg);
                                                                 return true;
                                                             }
                                                             sc->mMediaHandler->media_startTip(sc->generate_mux_csrc());
                                                             sc->TipStarted(true);
                                                         }
                                                     }

                                                     CallStateNotifier* notifier = sc->mCallStateNotifier;
                                                     if(notifier)
                                                         notifier->inviteAckReceived(sc->getInvite200TxStartTime());

                                                     break;
                                                 }

            case RVSIP_CALL_LEG_STATE_DISCONNECTING: {
                                                         TRACE("(TIP)T:DISCONNECTING\n");
                                                         if(sc->mMediaHandler) {
                                                             sc->mMediaHandler->media_onDisconnecting(hCallLeg);
                                                         }
                                                         break;
                                                     } 

            case RVSIP_CALL_LEG_STATE_DISCONNECTED: {
                                                        TRACE("(TIP)T:DISCONNECTED\n");
                                                        if(sc->mMediaHandler) {
                                                            sc->mMediaHandler->media_onDisconnected(hCallLeg);
                                                        }
                                                        if(sc->isConnected()) {
                                                            sc->setConnected(false);
                                                        }
                                                        break;
                                                    }

            case RVSIP_CALL_LEG_STATE_TERMINATED: {
                                                      TRACE("(TIP)T:TERMINATED");
                                                      if(sc->isConnected()) {
                                                          sc->setConnected(false);
                                                      }
                                                      sc->stopCallSession();
                                                      break;
                                                  } 

            case RVSIP_CALL_LEG_STATE_UNAUTHENTICATED: {
                                                           TRACE("(TIP)T:AUTHENTICATE");
                                                           RvSipUtils::SIP_RemoveAuthHeader(hCallLeg);

                                                           RvSipMsgHandle hSipMsg=(RvSipMsgHandle)0;
                                                           RvSipCallLegGetReceivedMsg(hCallLeg,&hSipMsg);
                                                           bool aka=RvSipUtils::RvSipAka::SIP_isAkaMsg(hSipMsg);
                                                           RVSIPPRINTF("%s: aka=%d\n",__FUNCTION__,(int)aka);
                                                           if(aka != sc->useAka()){
                                                               RvSipCallLegTerminate(hCallLeg);
                                                               break;
                                                           }

                                                           if(aka) {
                                                               RvSipAuthObjHandle   hAuthObj=(RvSipAuthObjHandle)0;
                                                               RvStatus rv= RV_Success;

                                                               // get the authentication object from the 407-response message.
                                                               rv = RvSipCallLegAuthObjGet (hCallLeg, RVSIP_FIRST_ELEMENT, NULL, &hAuthObj);
                                                               if(rv != RV_Success) {
                                                                   rv = RvSipCallLegTerminate(hCallLeg);
                                                                   break;
                                                               }

                                                               // Generate the AKA Authentication vector
                                                               char pwd[MAX_LENGTH_OF_PWD];
                                                               int pwd_len;
                                                               strncpy(pwd,sc->mAuthPasswd.c_str(),sizeof(pwd)-1);
                                                               pwd_len=strlen(pwd);
                                                               rv = SIP_AkaClientGenerateClientAV (sc->mOP,
                                                                       pwd,pwd_len+1,
                                                                       hAuthObj, &(sc->mAuthVector));
                                                               if(rv != RV_Success) {
                                                                   rv = RvSipCallLegTerminate(hCallLeg);
                                                                   break;
                                                               }
                                                           }
                                                           sc->addDynamicRoutes();
                                                           sc->setOutboundCallLegMsg();
                                                           sc->addSdp();
                                                           RvSipCallLegAuthenticate(hCallLeg);
                                                           break;

                                                       }

            case RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT: {
                                                              TRACE("(TIP)T:PROCEEDING_TIMEOUT");
                                                              RvSipCallLegCancel(hCallLeg);
                                                              break;
                                                          }

            case RVSIP_CALL_LEG_STATE_CANCELLING: {
                                                      TRACE("(TIP)T:CANCELLING");
                                                      break;
                                                  }

            case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE: {
                                                            TRACE("(TIP)T:MSG_SEND_FAILURE");
                                                            RvStatus rv = RvSipCallLegDNSContinue(hCallLeg,NULL,NULL);
                                                            if(rv>=0) {
                                                                rv = RvSipCallLegDNSReSendRequest(hCallLeg,NULL);
                                                            }
                                                            if(rv<0) {
                                                                RvSipCallLegTerminate(hCallLeg);
                                                            }
                                                            break;
                                                        }

            default: {
                         RVSIPPRINTF("T:UNKNOWN (%d)\n", eState);
                     }
        }; //switch

    } else {
        // ORIGINATE state sequence:
        // IDLE ->INVITING ->CONNECTED -> DISCONNECTING -> DISCONNECTED

        switch(eState) {
            case RVSIP_CALL_LEG_STATE_INVITING: {
                                                    TRACE("(TIP)O:INVITING\n");
                                                    break;
                                                } 

            case RVSIP_CALL_LEG_STATE_REDIRECTED: {
                                                      TRACE("(TIP)O:REDIRECTED\n");
                                                      RvSipCallLegConnect(hCallLeg);
                                                      break;
                                                  }

            case RVSIP_CALL_LEG_STATE_REMOTE_ACCEPTED: {
                                                           TRACE("(TIP)O:REMOTE_ACCEPTED\n");
                                                           sc->prackTimerStop();
                                                           sc->markAckStartTime();
                                                           if(sc->mMediaHandler) {
                                                               RvUint16 code=0;
                                                               //FIXME to check return value.
                                                               if(!sc->TipStarted()){
                                                                   sc->mMediaHandler->media_prepareConnect(hCallLeg,code,NULL);
                                                                   if(sc->RemoteSDPOffered()){
                                                                       sc->mMediaHandler->media_startTip(sc->generate_mux_csrc());
                                                                       sc->TipStarted(true);
                                                                   }
                                                               }
                                                           }else{
                                                               RvSipCallLegAck(hCallLeg);
                                                           }
                                                           break;
                                                       } 

            case RVSIP_CALL_LEG_STATE_PROCEEDING: {
                                                      TRACE("(TIP)O:PROCEEDING\n");
                                                      CallStateNotifier* notifier = sc->mCallStateNotifier;

                                                      // Received 1xx message
                                                      if(notifier)
                                                          notifier->inviteResponseReceived(false);

                                                      break;
                                                  } 

            case RVSIP_CALL_LEG_STATE_CONNECTED: {
                                                     TRACE("(TIP)O:CONNECTED\n");
                                                     sc->setConnected(true);
                                                     sc->setInviting(false);
                                                     CallStateNotifier* notifier = sc->mCallStateNotifier;
                                                     if(notifier) {
                                                         notifier->inviteCompleted(true);
                                                     }
                                                     sc->setCallTimer();
                                                     break;
                                                 } 

            case RVSIP_CALL_LEG_STATE_DISCONNECTING: {
                                                         TRACE("(TIP)O:DISCONNECTING\n");
                                                         if(sc->mMediaHandler) {
                                                             sc->mMediaHandler->media_onDisconnecting(hCallLeg);
                                                         }
                                                         sc->setDisconnecting(true);
                                                         sc->markByeStartTime();

                                                         CallStateNotifier* notifier = sc->mCallStateNotifier;
                                                         if(notifier) {
                                                              notifier->nonInviteSessionInitiated();
                                                         }

                                                         break;
                                                     } 

            case RVSIP_CALL_LEG_STATE_DISCONNECTED: {
                                                        TRACE("(TIP)O:DISCONNECTED\n");
                                                        if(sc->mMediaHandler) {
                                                            sc->mMediaHandler->media_onDisconnected(hCallLeg);
                                                        }
                                                        if(sc->isConnected()) {
                                                            sc->callCompletionNotification();
                                                            sc->setConnected(false);
                                                        }

                                                        CallStateNotifier* notifier = sc->mCallStateNotifier;

                                                        if(sc->isDisconnecting()) {
                                                            sc->markByeEndTime();
                                                            
                                                            if(notifier) 
                                                                notifier->byeStateDisconnected(sc->getByeStartTime(), sc->getByeEndTime());

                                                            sc->setDisconnecting(false);
                                                        }

                                                        if(notifier) {
                                                              switch(eReason)
                                                              {
                                                                  case RVSIP_CALL_LEG_REASON_DISCONNECTED: {
                                                                      RvSipMsgHandle hSipMsg=(RvSipMsgHandle)0;
                                                                      RvSipCallLegGetReceivedMsg(hCallLeg,&hSipMsg);

                                                                      int statusCode = (int)RvSipMsgGetStatusCode(hSipMsg);

                                                                      if(statusCode == 200)
                                                                          notifier->nonInviteSessionSuccessful();
                                                                      else if(statusCode == 408)  // Request Timeout
                                                                          notifier->nonInviteSessionFailed(false, true);  // transport err=false, timeout=true
                                                                      else 
                                                                          notifier->nonInviteSessionFailed(false, false); 

                                                                      break;
                                                                  }
                                                                  default:
                                                                      ;
                                                              }
                                                        }
                                                        break;
                                                    } 

            case RVSIP_CALL_LEG_STATE_TERMINATED: {
                                                      TRACE("(TIP)O:TERMINATED");
                                                      CallStateNotifier* notifier = sc->mCallStateNotifier;
                                                      if(sc->isInviting()) {
                                                          RVSIPPRINTF("%s: reason=%d, uname=%s\n",__FUNCTION__,(int)eReason,sc->getLocalUserNamePrivate(false).c_str());
                                                          
                                                          if(notifier)
                                                            notifier->inviteCompleted(false);
                                                          sc->setInviting(false);
                                                      } else if(sc->isDisconnecting()) {
                                                          sc->markByeEndTime();
                                                            
                                                          if(notifier)
                                                              notifier->byeStateDisconnected(sc->getByeStartTime(), sc->getByeEndTime());

                                                          sc->setDisconnecting(false);
                                                      }
                                                      if(sc->isConnected()) {
                                                          sc->callCompletionNotification();
                                                          sc->setConnected(false);
                                                      }
                                                      sc->stopCallSession();
                                                      sc->resetInitialInviteSentStatus();
                                                      sc->resetInitialByeSentStatus();
                                                      break;
                                                  } 

            case RVSIP_CALL_LEG_STATE_UNAUTHENTICATED: {
                                                           TRACE("(TIP)O:AUTHENTICATE");
                                                           RvSipUtils::SIP_RemoveAuthHeader(hCallLeg);
                                                           RvSipMsgHandle hSipMsg=(RvSipMsgHandle)0;
                                                           RvSipCallLegGetReceivedMsg(hCallLeg,&hSipMsg);
                                                           bool aka=RvSipUtils::RvSipAka::SIP_isAkaMsg(hSipMsg);
                                                           RVSIPPRINTF("%s: aka=%d\n",__FUNCTION__,(int)aka);
                                                           if(aka != sc->useAka()){
                                                               RvSipCallLegTerminate(hCallLeg);
                                                               break;
                                                           }

                                                           if(aka) {
                                                               RvSipAuthObjHandle   hAuthObj=(RvSipAuthObjHandle)0;
                                                               // get the authentication object from the 407-response message.
                                                               RvStatus rv= RV_Success;

                                                               rv = RvSipCallLegAuthObjGet (hCallLeg, RVSIP_FIRST_ELEMENT, NULL, &hAuthObj);
                                                               if(rv != RV_Success) {
                                                                   rv = RvSipCallLegTerminate(hCallLeg);
                                                                   break;
                                                               }

                                                               // Generate the AKA Authentication vector
                                                               char pwd[MAX_LENGTH_OF_PWD];
                                                               int pwd_len;
                                                               strncpy(pwd,sc->mAuthPasswd.c_str(),sizeof(pwd)-1);
                                                               pwd_len=strlen(pwd);
                                                               rv = SIP_AkaClientGenerateClientAV (sc->mOP,
                                                                       pwd,pwd_len+1,
                                                                       hAuthObj, &(sc->mAuthVector));
                                                               if(rv != RV_Success) {
                                                                   rv = RvSipCallLegTerminate(hCallLeg);
                                                                   break;
                                                               }
                                                           }
                                                           sc->addDynamicRoutes();
                                                           sc->setOutboundCallLegMsg();
                                                           sc->addSdp();
                                                           RvSipCallLegAuthenticate(hCallLeg);
                                                           break;
                                                       }

            case RVSIP_CALL_LEG_STATE_PROCEEDING_TIMEOUT: {
                                                              TRACE("(TIP)O:PROCEEDING_TIMEOUT");
                                                              RvSipCallLegCancel(hCallLeg);
                                                              break;
                                                          }

            case RVSIP_CALL_LEG_STATE_CANCELLING: {
                                                      TRACE("(TIP)O:CANCELLING");
                                                      break;
                                                  }

            case RVSIP_CALL_LEG_STATE_MSG_SEND_FAILURE: {
                                                            TRACE("(TIP)T:MSG_SEND_FAILURE");
                                                            RvStatus rv = RvSipCallLegDNSContinue(hCallLeg,NULL,NULL);
                                                            if(rv>=0) {
                                                                rv = RvSipCallLegDNSReSendRequest(hCallLeg,NULL);
                                                            }
                                                            if(rv<0) {
                                                                RVSIPPRINTF("%s: reason=%d, uname=%s\n",__FUNCTION__,(int)eReason,sc->getLocalUserNamePrivate(false).c_str());

                                                                CallStateNotifier* notifier = sc->mCallStateNotifier;

                                                                if(notifier) { 
                                                                    bool transportError = false;
                                                                    bool timeoutError = false;
                                                                    bool serverError = false;

                                                                    switch(eReason) {
                                                                        case RVSIP_CALL_LEG_REASON_NETWORK_ERROR: {
                                                                            transportError = true;
                                                                            break;
                                                                        }
                                                                        case RVSIP_CALL_LEG_REASON_LOCAL_TIME_OUT: {
                                                                            timeoutError = true; 
                                                                            break;
                                                                        }
                                                                        case RVSIP_CALL_LEG_REASON_503_RECEIVED:
                                                                        case RVSIP_CALL_LEG_REASON_SERVER_FAILURE: {
                                                                            serverError = true;
                                                                            break;
                                                                        }
                                                                        default:
                                                                            ;
                                                                    }

                                                                    if(sc->isInviting()) {                                                                   
                                                                        if(transportError || timeoutError || serverError)
                                                                            notifier->callFailed(transportError, timeoutError, serverError);
                                                                    } else {
                                                                        if (sc->isDisconnecting())
                                                                            notifier->byeFailed();

                                                                        notifier->nonInviteSessionFailed(transportError, timeoutError);
                                                                    }
                                                                }

                                                                RvSipCallLegTerminate(hCallLeg);
                                                            }
                                                            break;
                                                        }

            default: {
                         RVSIPPRINTF("O:UNKNOWN (%d)\n", eState);
                     }
        }; //switch

    }

    RETCODE(true);
}
bool SipCallSMTip::byeStateChangedEvHandler(
        IN  RvSipCallLegHandle                hCallLeg,
        IN  RvSipAppCallLegHandle             hAppCallLeg,
        IN  RvSipTranscHandle                 hTransc,
        IN  RvSipAppTranscHandle              hAppTransc,
        IN  RvSipCallLegByeState              eByeState,
        IN  RvSipTransactionStateChangeReason eReason,
        IN  SipChannel* sc) {

    ENTER();

    if(!sc) return false;

    //TODO:TIP

    RETCODE(false);
}

bool SipCallSMTip::prackStateChangedEvHandler(
        IN  RvSipCallLegHandle            hCallLeg,
        IN  RvSipAppCallLegHandle         hAppCallLeg,
        IN  RvSipCallLegPrackState        eState,
        IN  RvSipCallLegStateChangeReason eReason,
        IN  RvInt16                       prackResponseCode,
        IN  SipChannel* sc) {

    ENTER();

    if(!sc) return false;

    //TODO:TIP
    if(sc->mHCallLeg == hCallLeg) {

        RVSIPPRINTF("%s: hCallLeg=0x%x, eState=%lu, eReason=%lu\n",__FUNCTION__,(int)hCallLeg,
                (unsigned long)eState,(unsigned long)eReason);

        if (!sc->isTerminatePrivate()) {
            CallStateNotifier* notifier = NULL;
            // ORIGINATE state sequence:
            switch(eState) {
                case RVSIP_CALL_LEG_PRACK_STATE_PRACK_FINAL_RESPONSE_RCVD: {
                    notifier = sc->mCallStateNotifier;
                    
                    if(notifier) {
                        RvSipMsgHandle hSipMsg=(RvSipMsgHandle)0;
                        RvSipCallLegGetReceivedMsg(hCallLeg,&hSipMsg);
                                                                          
                        int statusCode = (int)RvSipMsgGetStatusCode(hSipMsg);

                        if(statusCode == 200)
                            notifier->nonInviteSessionSuccessful();
                    }
                }
                break;
                case RVSIP_CALL_LEG_PRACK_STATE_PRACK_SENT:  {
                    notifier = sc->mCallStateNotifier;
                    
                    if(notifier)
                        notifier->nonInviteSessionInitiated();
                }
                break;
                default:
                    ;
            }
        }
    }

    RETCODE(false);
}
