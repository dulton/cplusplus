/// @file
/// @brief SIP core "engine" implementation.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <fstream>
#include "SipEngine.h"
#include "SipCallSMBasic.h"
#include "SipCallSMTip.h"


#include "RvSipAka.h"

USING_RV_SIP_ENGINE_NAMESPACE;

struct StackLogCfgDesc{
    std::string desc;
    int          value;
};

static struct StackLogCfgDesc StackLogMaskDesc[] = {
    {"CALL",RVSIP_CALL},
    {"TRANSACTION",RVSIP_TRANSACTION},
    {"MESSAGE",RVSIP_MESSAGE},
    {"TRANSPORT",RVSIP_TRANSPORT},
    {"PARSER",RVSIP_PARSER},
    {"STACK",RVSIP_STACK},
    {"MSGBUILDER",RVSIP_MSGBUILDER},
    {"AUTHENTICATOR",RVSIP_AUTHENTICATOR},
    {"TRANSCCLIENT",RVSIP_TRANSCCLIENT},
    {"REGCLIENT",RVSIP_REGCLIENT},
    {"PUBCLIENT",RVSIP_PUBCLIENT},
    {"SUBSCRIPTION",RVSIP_SUBSCRIPTION},
    {"COMPARTMENT",RVSIP_COMPARTMENT},
    {"RESOLVER",RVSIP_RESOLVER},
    {"TRANSMITTER",RVSIP_TRANSMITTER},
    {"SECURITY",RVSIP_SECURITY},
    {"SEC_AGREE",RVSIP_SEC_AGREE},
    {"ADS_RLIST",RVSIP_ADS_RLIST},
    {"ADS_RA",RVSIP_ADS_RA},
    {"ADS_RPOOL",RVSIP_ADS_RPOOL},
    {"ADS_HASH",RVSIP_ADS_HASH},
    {"ADS_PQUEUE",RVSIP_ADS_PQUEUE},
    {"CORE_SEMAPHORE",RVSIP_CORE_SEMAPHORE},
    {"CORE_MUTEX",RVSIP_CORE_MUTEX},
    {"CORE_LOCK",RVSIP_CORE_LOCK},
    {"CORE_MEMORY",RVSIP_CORE_MEMORY},
    {"CORE_THREAD",RVSIP_CORE_THREAD},
    {"CORE_QUEUE",RVSIP_CORE_QUEUE},
    {"CORE_TIMER",RVSIP_CORE_TIMER},
    {"CORE_TIMESTAMP",RVSIP_CORE_TIMESTAMP},
    {"CORE_CLOCK",RVSIP_CORE_CLOCK},
    {"CORE_TM",RVSIP_CORE_TM},
    {"CORE_SOCKET",RVSIP_CORE_SOCKET},
    {"CORE_PORTRANGE",RVSIP_CORE_PORTRANGE},
    {"CORE_SELECT",RVSIP_CORE_SELECT},
    {"CORE_HOST",RVSIP_CORE_HOST},
    {"CORE_TLS",RVSIP_CORE_TLS},
    {"CORE_ARES",RVSIP_CORE_ARES},
    {"CORE_RCACHE",RVSIP_CORE_RCACHE},
    {"CORE_EHD",RVSIP_CORE_EHD},
    {"CORE_IMSIPSEC",RVSIP_CORE_IMSIPSEC},
    {"CORE",RVSIP_CORE},           
    {"ADS",RVSIP_ADS},
};
static struct StackLogCfgDesc StackLogFilterDesc[] = {
    {"DEBUG",RVSIP_LOG_DEBUG_FILTER},
    {"INFO" ,RVSIP_LOG_INFO_FILTER },
    {"WARN" ,RVSIP_LOG_WARN_FILTER},
    {"ERROR" ,RVSIP_LOG_ERROR_FILTER},
    {"EXCEP" ,RVSIP_LOG_EXCEP_FILTER},
    {"LOCKDBG" ,RVSIP_LOG_LOCKDBG_FILTER},
    {"ENTER" ,RVSIP_LOG_ENTER_FILTER},
    {"LEAVE" ,RVSIP_LOG_LEAVE_FILTER},
    {"ALL" ,RVSIP_LOG_DEBUG_FILTER|RVSIP_LOG_INFO_FILTER|RVSIP_LOG_WARN_FILTER|RVSIP_LOG_ERROR_FILTER|RVSIP_LOG_EXCEP_FILTER},
};

//=============================================================================
// static member defines
//=============================================================================

const char* SipEngine::SIP_DEFAULT_ADDRESS = "127.0.0.1";
const char* SipEngine::SIP_DEFAULT_LOOPBACK_ADDRESS = "127.0.0.1";
const char* SipEngine::SIP_DEFAULT_LOOPBACK_INTERFACE = "lo";
const char* SipEngine::SIP_DEFAULT_INTERFACE = "eth0";
const char* SipEngine::SIP_UDP_SUPPORTED = "100rel,timer";
const char* SipEngine::SIP_TCP_SUPPORTED = "timer";
const char* SipEngine::SIP_REG_SUPPORTED = "path";
const int SipEngine::MAX_NUM_OF_DNS_SERVERS = 128;
const int SipEngine::MAX_NUM_OF_DNS_DOMAINS = 128;
#ifdef UNIT_TEST
	bool SipEngine::reliableFlag = false;
#endif

static int _LINGER_SEC() {
#if defined(__OCTEON__) || defined(__MIPS__)
  return 32; //RFC3261
#else 
  return 6; //Too many CPS projected
#endif
}

#define LINGER_SEC _LINGER_SEC()

static int _LINGER_MAX_CPS(int channels) {
#if defined(__OCTEON__)
  return 800;
#elif  defined(__MIPS__) || defined(UNIT_TEST) || defined(RVSIPTRIAL)
  return 200;
#else
  if(channels>16000) return 8000;
  if(channels>8000) return 4000;
  if(channels>4000) return 2000;
  return 1000;
#endif
}

#define LINGER_MAX_CPS(channels) _LINGER_MAX_CPS(channels)

static int _LINGER_EXTRA(int channels) {
  return (LINGER_SEC * LINGER_MAX_CPS(channels));
}

#define LINGER_EXTRA(channels) _LINGER_EXTRA(channels)

SipEngine  * SipEngine::mInstance = 0;

//=============================================================================

int32_t SipEngine::getMaxPendingChannels() {
#if defined(__OCTEON__) || defined(__MIPS__)
  return 200;
#else 
  return 400;
#endif
}

//=============================================================================

extern "C" {

  static void MD5toString(unsigned char *digest,char* buff) {
    unsigned int i;
    char *ptr = buff;
    for (i = 0; i < 16; i++) {
      sprintf (ptr,"%02x", digest[i]);
      ptr+=2;
    }
  }

  //Radvision stack callbacks

  static RvStatus AppTransactionOpenCallLegEv(IN  RvSipTranscMgrHandle    hTranscMgr,
					      IN  void                   *pAppMgr,
					      IN  RvSipTranscHandle       hTransc,
					      OUT RvBool                 *bOpenCallLeg)
  {
    RvStatus       rv    = RV_OK; 

    if(bOpenCallLeg) {

      *bOpenCallLeg = RV_TRUE;  // default.
      
      // except the following situation cases:
      RvSipMsgHandle hMsg  = NULL;
   
      rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
      if(rv == RV_OK && hMsg != NULL) {

	RvSipMsgType msgType = RvSipMsgGetMsgType(hMsg);

	if(msgType == RVSIP_MSG_REQUEST) {

	  RvSipMethodType methodType = RvSipMsgGetRequestMethod(hMsg);

	  if(methodType == RVSIP_METHOD_REGISTER) {

	    *bOpenCallLeg = RV_FALSE;

	  } else if(methodType == RVSIP_METHOD_OTHER) {

	    RvChar strMethod[MAX_LENGTH_OF_METHOD]="\0";

	    RvSipTransactionGetMethodStr(hTransc,sizeof(strMethod)-1,strMethod);
	    
	    if(strcmp(strMethod,"OPTIONS") == 0) {
	      *bOpenCallLeg = RV_FALSE;    
	    }
	  }
	}
      } 
    }

    return rv;
  }

  static void RVCALLCONV AppTransactionCreatedEvHandler(
							IN RvSipTranscHandle hTransc,
							IN void *context,
							OUT RvSipTranscOwnerHandle *phAppTransc,
							OUT RvBool *b_handleTransc)
  {
    RvStatus       rv    = RV_OK; 

    RvSipMsgHandle hMsg  = NULL;
    
    rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
    if(rv == RV_OK && hMsg != NULL) {

      RvSipMsgType msgType = RvSipMsgGetMsgType(hMsg);
      
      if(msgType == RVSIP_MSG_REQUEST) {
	
	RvSipMethodType methodType = RvSipMsgGetRequestMethod(hMsg);
	
	if(methodType == RVSIP_METHOD_REGISTER) {
	  
	  *b_handleTransc = RV_TRUE;
	  
	} else if(methodType == RVSIP_METHOD_OTHER) {
	  
	  RvChar strMethod[MAX_LENGTH_OF_METHOD]="\0";
	  
	  RvSipTransactionGetMethodStr(hTransc,sizeof(strMethod)-1,strMethod);
	  if(strcmp(strMethod,"OPTIONS") == 0) {
	    *b_handleTransc = RV_TRUE;
	  }
	}
      }
    }
  }

  static void RVCALLCONV AppTransactionStateChangedEvHandler(
							     IN RvSipTranscHandle hTransc,
							     IN RvSipTranscOwnerHandle hAppTransc,
							     IN RvSipTransactionState eState,
							     IN RvSipTransactionStateChangeReason eReason)
  {
    RvStatus rv=RV_OK;

    switch(eState) {

    case RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD: {

      RvSipMsgHandle hMsg  = NULL;
      
      rv = RvSipTransactionGetReceivedMsg(hTransc, &hMsg);
      if(rv == RV_OK && hMsg != NULL) {

	RvSipMsgType msgType = RvSipMsgGetMsgType(hMsg);
	
	if(msgType == RVSIP_MSG_REQUEST) {
	  
	  RvSipMethodType methodType = RvSipMsgGetRequestMethod(hMsg);
	  
	  if(methodType == RVSIP_METHOD_REGISTER) {
	    
	    rv = RvSipTransactionRespond(hTransc,200,NULL);
	    if(rv!= RV_OK) {
	      RVSIPPRINTF("Failed to respond to the request");
	    }
	    
	  } else if(methodType == RVSIP_METHOD_OTHER) {
	    
	    RvChar strMethod[MAX_LENGTH_OF_METHOD]="\0";
	    RvSipTransactionGetMethodStr(hTransc,sizeof(strMethod)-1,strMethod);
	    if(strcmp(strMethod,"OPTIONS") == 0) {
	      rv = RvSipTransactionRespond(hTransc,200,NULL);
	      if(rv!= RV_OK) {
		RVSIPPRINTF("Failed to respond to the request");
	      }
	    }
	  }
	}
      }
      break;
    }
    default:
      break;
    }
  }
  
  static void AppCallLegCreatedEvHandler(
				  IN  RvSipCallLegHandle			hCallLeg,
				  OUT RvSipAppCallLegHandle 	  * phAppCallLeg)
  {
    ENTER();
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appCallLegCreatedEvHandler(hCallLeg, phAppCallLeg);
    RET;
    
  } 
  
  static void AppStateChangedEvHandler(
				IN  RvSipCallLegHandle            hCallLeg,
				IN  RvSipAppCallLegHandle         hAppCallLeg,
				IN  RvSipCallLegState             eState,
				IN  RvSipCallLegStateChangeReason eReason)
  {
    ENTER();
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appStateChangedEvHandler(hCallLeg, hAppCallLeg, eState, eReason);
    RET;
  }

  static void AppByeCreatedEvHandler(
				     IN  RvSipCallLegHandle            hCallLeg,
				     IN  RvSipAppCallLegHandle         hAppCallLeg,
				     IN  RvSipTranscHandle             hTransc,
				     OUT RvSipAppTranscHandle          *hAppTransc,
				     OUT RvBool                       *bAppHandleTransc) {
    ENTER();
    if (SipEngine::getInstance()) {
      SipEngine::getInstance()->appByeCreatedEvHandler(
						       hCallLeg,
						       hAppCallLeg,
						       hTransc,
						       hAppTransc,
						       bAppHandleTransc);
    }
    RET;
  }

  static void AppByeStateChangedEvHandler(
					  IN  RvSipCallLegHandle                hCallLeg,
					  IN  RvSipAppCallLegHandle             hAppCallLeg,
					  IN  RvSipTranscHandle                 hTransc,
					  IN  RvSipAppTranscHandle              hAppTransc,
					  IN  RvSipCallLegByeState              eByeState,
					  IN  RvSipTransactionStateChangeReason eReason) {
    ENTER();
    if (SipEngine::getInstance()) {
      SipEngine::getInstance()->appByeStateChangedEvHandler(
							    hCallLeg,
							    hAppCallLeg,
							    hTransc,
							    hAppTransc,
							    eByeState,
							    eReason);
    }
  }
  
  static void AppPrackStateChangedEvHandler(
				     IN  RvSipCallLegHandle              hCallLeg,
				     IN  RvSipAppCallLegHandle           hAppCallLeg,
				     IN  RvSipCallLegPrackState          eState,
				     IN  RvSipCallLegStateChangeReason   eReason,
				     IN  RvInt16                         prackResponseCode) 
  {
    ENTER();
    if (SipEngine::getInstance()) SipEngine::getInstance()->appPrackStateChangedEvHandler(hCallLeg, hAppCallLeg, eState, eReason, prackResponseCode);
    RET;
  }
  
  static RV_Status AppCallLegMsgToSendEvHandler(
					 IN RvSipCallLegHandle            hCallLeg,
					 IN RvSipAppCallLegHandle         hAppCallLeg,
					 IN RvSipMsgHandle                hMsg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ?
      SipEngine::getInstance()->appCallLegMsgToSendEvHandler(hCallLeg, hAppCallLeg, hMsg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }
  static void AppCallLegReInviteCreatedEvHandler(
                    IN RvSipCallLegHandle               hCallLeg,
                    IN RvSipAppCallLegHandle            hAppCallLeg, 
                    IN RvSipCallLegInviteHandle         hReInvite,
                    OUT RvSipAppCallLegInviteHandle     *phAppReInvite,
                    OUT RvUint16                        *pResponseCode)
  {
    ENTER();
    
    if(SipEngine::getInstance()) SipEngine::getInstance()->appCallLegReInviteCreatedEvHandler(hCallLeg, hAppCallLeg,hReInvite,phAppReInvite,pResponseCode);
    
    RET;
  }

  static void AppCallLegReInviteStateChangedEvHandler(
                    IN RvSipCallLegHandle               hCallLeg,
                    IN RvSipAppCallLegHandle            hAppCallLeg, 
                    IN RvSipCallLegInviteHandle         hReInvite,
                    IN RvSipAppCallLegInviteHandle      hAppReInvite,
                    IN RvSipCallLegModifyState          eModifyState,
                    IN RvSipCallLegStateChangeReason    eReason )
  {
    ENTER();
    
    if(SipEngine::getInstance()) SipEngine::getInstance()->appCallLegReInviteStateChangedEvHandler(hCallLeg, hAppCallLeg,hReInvite,hAppReInvite,eModifyState,eReason);
    
    RET;
  }

  static void AppRegClientStateChangedEvHandler(
						IN  RvSipRegClientHandle          hRegClient,
						IN  RvSipAppRegClientHandle       hAppRegClient,
						IN  RvSipRegClientState             eNewState,
						IN  RvSipRegClientStateChangeReason eReason)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ?
      SipEngine::getInstance()->appRegClientStateChangedEvHandler(hRegClient, hAppRegClient, eNewState, eReason) :
      RV_ERROR_UNINITIALIZED;
    
    RET;
  }

  static RV_Status AppRegClientMsgToSendEvHandler(
						  IN  RvSipRegClientHandle          hRegClient,
						  IN  RvSipAppRegClientHandle       hAppRegClient,
						  IN  RvSipMsgHandle                hMsg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ?
      SipEngine::getInstance()->appRegClientMsgToSendEvHandler(hRegClient, hAppRegClient, hMsg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }
  
  static RV_Status AppRegClientMsgReceivedEvHandler(
						  IN  RvSipRegClientHandle          hRegClient,
						  IN  RvSipAppRegClientHandle       hAppRegClient,
						  IN  RvSipMsgHandle                hMsg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ?
      SipEngine::getInstance()->appRegClientMsgReceivedEvHandler(hRegClient, hAppRegClient, hMsg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }
  
  static RV_Status AppCallLegMsgReceivedEvHandler(
					   IN RvSipCallLegHandle            hCallLeg,
					   IN RvSipAppCallLegHandle         hAppCallLeg,
					   IN RvSipMsgHandle                hMsg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ? SipEngine::getInstance()->appCallLegMsgReceivedEvHandler(hCallLeg, hAppCallLeg, hMsg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }

  static RvStatus AppCallLegTimerRefreshAlertEvHandler(IN    RvSipCallLegHandle       hCallLeg,
						       IN    RvSipAppCallLegHandle    hAppCallLeg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ? SipEngine::getInstance()->appCallLegTimerRefreshAlertEvHandler(hCallLeg, hAppCallLeg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }

  static RvStatus AppCallLegSessionTimerNotificationEv(
						       IN RvSipCallLegHandle                         hCallLeg,
						       IN RvSipAppCallLegHandle                      hAppCallLeg,
						       IN RvSipCallLegSessionTimerNotificationReason eReason) {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ? SipEngine::getInstance()->appCallLegSessionTimerNotificationEv(hCallLeg, hAppCallLeg, eReason) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }

  /***************************************************************************
   * SIP_AuthenticatorGetSharedSecretEv
   * ------------------------------------------------------------------------
   * General:  Notifies that there is a need for the user-name and password.
   *           This callback function may be called in 2 cases:
   *        1. Client authentication - when the stack builds the Authorization
   *           header, using the received WWW-Authenticate header.
   *           In this case, the client should supply the user-name and password.
   *        2. Server authentication - when application uses the stack to build
   *           an Authentication-info header. in this case the stack already has
   *           the user-name, from the Authorization header, and application
   *           should only supply the password.
   * Return Value: (-)
   * ------------------------------------------------------------------------
   * Arguments:
   * Input:   hAuthenticator    - Handle to the authenticator object.
   *          hAppAuthenticator - Handle to the application authenticator.
   *          hObject           - Handle to the Object, that is served
   *                              by the Authenticator (e.g. CallLeg, RegClient)
   *          peObjectType      - pointer to the variable, that stores the
   *                              type of the hObject.
   *                              Use following code to get the type:
   *                              RvSipCommonStackObjectType eObjType = *peObjectType;
   *          pRpoolRealm       - the realm string in RPool_ptr format
   * Output:  pRpoolUserName    - the user-name string in RPool_ptr format.
   *                              (this is an OUT parameter for client, and IN
   *                              parameter for server).
   *          pRpoolPassword    - the password string in RPool_ptr format
   ***************************************************************************/
  static void AppGetSharedSecretAuthenticationHandler(
						      IN    RvSipAuthenticatorHandle       hAuthenticator,
						      IN    RvSipAppAuthenticatorHandle    hAppAuthenticator,
						      IN    void*                          hObject,
						      IN    void*                          peObjectType,
						      IN    RPOOL_Ptr                     *pRpoolRealm,
						      INOUT RPOOL_Ptr                     *pRpoolUserName,
						      OUT   RPOOL_Ptr                     *pRpoolPassword)
  {
    ENTER();

    if(SipEngine::getInstance()) {
      SipEngine::getInstance()->appGetSharedSecretAuthenticationHandler(hAuthenticator,hAppAuthenticator,hObject,
									peObjectType,pRpoolRealm,
									pRpoolUserName,pRpoolPassword);
    }

    RET;
  }
  
  /******************************************************************************
   * RvSipAuthenticatorNonceCountUsageEv
   * ----------------------------------------------------------------------------
   * General: This callback function notifies the application about the value
   *          of the nonceCount parameter, which the Stack is going to use,
   *          while calculating credentials.
   *          The Application can change this value in order to fit more
   *          precious management of the nonceCount. The Stack doesn't check
   *          the uniquity of the used NONCE in the realm or by the different
   *          object.
   * Return Value: none.
   * ------------------------------------------------------------------------
   * Arguments:
   * Input:   hAuthenticator        - Handle to the Authenticator object
   *          hAppAuthenticator     - Application handle stored in Authenticator.
   *          hObject               - Handle to the Object, that is served
   *                                  by the Authenticator(e.g.CallLeg,RegClient)
   *          peObjectType          - Pointer to the variable that stores
   *                                  the type of the hObject.
   *                                  Use following code to get the type:
   *                                  RvSipCommonStackObjectType eObjType = *peObjectType;
   *          hAuthenticationHeader - Handle to the Authentication header,
   *                                  containing challenge, credentials for which
   *                                  are being prepared.
   *          pNonceCount           - pointer to the nonceCount value, managed by
   *                                  the Stack per Challenge.
   * Output:  pNonceCount           - pointer to the nonceCount value, set by
   *                                  the Application in order to be used by
   *                                  the Stack for Credentials calculation.
   ***************************************************************************/
  static void AppAuthenticatorNonceCountUsageEv(
						IN    RvSipAuthenticatorHandle        hAuthenticator,
						IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
						IN    void*                           hObject,
						IN    void*                           peObjectType,
						IN    RvSipAuthenticationHeaderHandle hAuthenticationHeader,
						INOUT RvInt32*                        pNonceCount)
  {
    ENTER();

    if(SipEngine::getInstance()) {
      SipEngine::getInstance()->appAuthenticatorNonceCountUsageEv(hAuthenticator,hAppAuthenticator,hObject,
						peObjectType,hAuthenticationHeader,pNonceCount);
    }

    RET;
  }

  /***************************************************************************
   * RvSipAuthenticatorMD5Ev
   * ------------------------------------------------------------------------
   * General:  Notifies that there is a need to use the MD5 algorithm.
   * Return Value: (-)
   * ------------------------------------------------------------------------
   * Arguments:
   * Input: pRpoolMD5Input - Rpool pointer to the MD5 input
   *          length         - length of the string inside the page
   * Output: strMd5Output   - The output of the hash algorithm
   ***************************************************************************/
  static void AppMD5AuthenticationExHandler (
					     IN    RvSipAuthenticatorHandle        hAuthenticator,
					     IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
					     IN  RPOOL_Ptr                     *pRpoolMD5Input,
					     IN  RV_UINT32                     length,
					     OUT RPOOL_Ptr                     *pRpoolMD5Output)
  {
    ENTER();

    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppAuthenticator);

    if (SipEngine::getInstance()) 
      SipEngine::getInstance()->appMD5AuthenticationExHandler (hAuthenticator, hAppAuthenticator, pRpoolMD5Input, length, pRpoolMD5Output);

    RET;
  }

  /***************************************************************************
   * RvSipAuthenticatorMD5EntityBodyEv
   * ------------------------------------------------------------------------
   * General:  This callback function notifies the application that
   *           it should supply the hashing result performed on the message body
   *           MD5(entity-body). Message body hash value is needed when
   *           the required quality of protection (qop) is 'auth-int'.
   *
   *           Two notes regarding the message parameter supplied in this callback:
   *           1) On client side: Note that this callback is called before the 
   *           msgToSend callback of the stack object is called. Therefore, if 
   *           your code adds the message body in the msgToSend callback, the body 
   *           will not be available on this callback.
   *           In order to use the message body at this stage, you must use the
   *           outbound message mechanism to add the body.
   *           2) On server side: Note that the message is supplied only if the
   *           authentication procedure was started synchronic (within the
   *           change-state callback). in a-synchronic mode, the parameter is NULL.
   *           In order to perform auth-int authentication in an a-synchronic 
   *           mode, the application should save the incoming request in the msg-rcvd
   *           callback, and use it in this function.
   *
   * Return Value: (-)
   * ------------------------------------------------------------------------
   * Arguments:
   * Input:   hAuthenticator    - handle to the Authenticator object
   *          hAppAuthenticator - Handle to the application authenticator.
   *          hObject           - Handle to the Object, that is served
   *                              by the Authenticator (e.g. CallLeg, RegClient)
   *          peObjectType      - Pointer to the variable, that stores the
   *                              type of the hObject.
   *                              Use following code to get the type:
   *                              RvSipCommonStackObjectType eObjType = *peObjectType;
   *          hMsg              - Handle to the message that is now being sent
   *                              and that will include the user credentials.
   * Output:  pRpoolMD5Output   - The MD5 of the message body in RPOOL_Ptr format.
   *          pLength           - length of the string after MD5 result
   *                              concatenation.
   ***************************************************************************/
  
  static void AppAuthenticatorMD5EntityBodyEv(
					      IN     RvSipAuthenticatorHandle      hAuthenticator,
					      IN     RvSipAppAuthenticatorHandle   hAppAuthenticator,
					      IN     void*                         hObject,
					      IN     void*                         peObjectType,
					      IN     RvSipMsgHandle                hMsg,
					      OUT    RPOOL_Ptr                     *pRpoolMD5Output,
					      OUT    RvUint32                      *pLength) {
    
    ENTER();
    
    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppAuthenticator);
    
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appAuthenticatorMD5EntityBodyEv(hAuthenticator,hAppAuthenticator,hObject,peObjectType,hMsg,
								pRpoolMD5Output,pLength);
    
    RET;
  }
  
  static void AppSubsStateChangedEvHandler(
					   IN  RvSipSubsHandle            hSubs,
					   IN  RvSipAppSubsHandle         hAppSubs,
					   IN  RvSipSubsState             eState,
					   IN  RvSipSubsStateChangeReason eReason) {
    ENTER();
    
    RVSIPPRINTF("%s: app=0x%x, state =%d, reason=%d\n",__FUNCTION__,(unsigned int)hAppSubs,(int)eState,(int)eReason);
    
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appSubsStateChangedEvHandler(hSubs,hAppSubs,eState,eReason);
    
    RET;
  }
  static void AppSubsExpirationAlertEvHandler(RvSipSubsHandle            hSubs,
					      RvSipAppSubsHandle         hAppSubs){
    ENTER();
    
    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppSubs);
    
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appSubsExpirationAlertEv(hSubs,hAppSubs);
    
    RET;
  }
  
  static void AppSubsNotifyEvHandler( 
                                     IN  RvSipSubsHandle        hSubs,
                                     IN  RvSipAppSubsHandle     hAppSubs,
                                     IN  RvSipNotifyHandle      hNotification,
                                     IN  RvSipAppNotifyHandle   hAppNotification,
                                     IN  RvSipSubsNotifyStatus  eNotifyStatus,
                                     IN  RvSipSubsNotifyReason  eNotifyReason,
                                     IN  RvSipMsgHandle         hNotifyMsg){
    ENTER();
    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppSubs);
    if (SipEngine::getInstance())
      SipEngine::getInstance()->appSubsNotifyEvHandler(hSubs,hAppSubs,hNotification,hAppNotification,eNotifyStatus,eNotifyReason,hNotifyMsg);
    
    
    RET;
  }
  
  static RvStatus AppTransportBadSyntaxMsgEvHandler(IN    RvSipTransportMgrHandle   hTransportMgr,
						    IN    RvSipAppTransportMgrHandle hAppTransportMgr,
						    IN    RvSipMsgHandle            hMsgReceived,
						    OUT   RvSipTransportBsAction    *peAction){
    ENTER();
    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppTransportMgr);
    
    *peAction = RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS;
    //currently we just continue process..
    
    RETCODE(RV_OK);
  }

  static RV_Status AppSubsMsgToSendEvHandler(
					     IN  RvSipSubsHandle          hSubsClient,
					     IN  RvSipAppSubsHandle       hAppSubsClient,
					     IN  RvSipNotifyHandle        hNotify,
					     IN  RvSipAppNotifyHandle     hAppNotify,
					     IN  RvSipMsgHandle           hMsg)
  {
    RvStatus rv;
    ENTER();
    
    rv = (SipEngine::getInstance()) ?
      SipEngine::getInstance()->appSubsMsgToSendEvHandler(hSubsClient, hAppSubsClient, hNotify, hAppNotify, hMsg) :
      RV_ERROR_UNINITIALIZED;
    
    RETCODE(rv);
  }

  static void AppRetransmitEvHandler(
                         IN  RvSipMethodType          method,
                         IN  void                     *owner)
  {
     ENTER();

     if (SipEngine::getInstance())
        SipEngine::getInstance()->appRetransmitEvHandler(method, owner);

     RET;
  }

   static void AppRetransmitRcvdEvHandler(
                         IN  RvSipMsgHandle           hMsg,
                         IN  void                     *owner)
  {
     ENTER();

     if (SipEngine::getInstance())
        SipEngine::getInstance()->appRetransmitRcvdEvHandler(hMsg, owner);

     RET;
  }
  
  
}; //extern "C"

DnsServerCfg::DnsServerCfg(){
    m_tries = -1;
    m_timeout = -1;
    m_numservers = 0;
}
DnsServerCfg::DnsServerCfg(int timeout,int tries,int numofservers){
    m_timeout = timeout;
    m_tries = tries;
    m_numservers = numofservers;
}
DnsServerCfg::DnsServerCfg(const DnsServerCfg &cfg){
    m_timeout = cfg.timeout();
    m_tries = cfg.tries();
    m_numservers = cfg.numofservers();
}
const DnsServerCfg & DnsServerCfg::operator =(const DnsServerCfg &cfg){
    m_timeout = cfg.timeout();
    m_tries = cfg.tries();
    m_numservers = cfg.numofservers();
    return *this;
}

SipEngine::SipEngine(int numOfThreads,int numChannels, 
		     SipChannelFinder& scFinder,
		     const MD5Processor &md5Processor) : mNumChannels(numChannels), mAllocatedChannels(0),
							   mScFinder(scFinder), mMD5Processor(md5Processor)
{
  // 2.1 Initialize SIP Stack
  TRACE("SipEngine");

  mStateMachines[CFT_BASIC]=new SipCallSMBasic();
  mStateMachines[CFT_TIP]=new SipCallSMTip(mStateMachines[CFT_BASIC]);

  {
    unsigned int i=0;
    for(i=0;i<sizeof(StackLogMaskDesc)/sizeof(StackLogMaskDesc[0]);i++){
      mLogModuleTypes.insert(std::make_pair(StackLogMaskDesc[i].desc,StackLogMaskDesc[i].value));
    }
    for(i=0;i<sizeof(StackLogFilterDesc)/sizeof(StackLogFilterDesc[0]);i++){
      mLogFilterTypes.insert(std::make_pair(StackLogFilterDesc[i].desc,StackLogFilterDesc[i].value));
    }
  }

  mDnsAltered=false;
  mDnsCfgs.clear();

  if(initSipStack(numOfThreads)<0 || !mHMidMgr || !mHSipStack) {
    RVSIPPRINTF ("?initSipStack?\n");
    return;
  }
  
  // 2.2 Initialize SDP Stack
  TRACE("initSdpStack()");
  if(initSdpStack() < 0) {
    RVSIPPRINTF ("?initSdpStack?\n");
    return;
  }

  mAppPool = RPOOL_Construct(2048, 10+(4*mNumChannels), NULL, RV_FALSE, "ApplicationPool");

  setEvHandlers();
}

SipChannel* SipEngine::newSipChannel() {
  RvSipCallLegMgrHandle hMgr = (RvSipCallLegMgrHandle)0;	// CallLeg Manager handle
  RvStatus rv = RvSipStackGetCallLegMgrHandle (mHSipStack, &hMgr);
  if(rv>=0 && hMgr) {
    RvSipTransportMgrHandle hTrMgr = (RvSipTransportMgrHandle)0;  
    rv = RvSipStackGetTransportMgrHandle (mHSipStack, &hTrMgr);
    if(rv>=0 && hTrMgr) {
      RvSipRegClientMgrHandle hRegMgr = (RvSipRegClientMgrHandle)0;
      RvSipAuthenticatorHandle hAuthMgr=(RvSipAuthenticatorHandle)0;
      RvSipMsgMgrHandle hMsgMgr=(RvSipMsgMgrHandle)0;
      RvSipSubsMgrHandle hSubsMgr=(RvSipSubsMgrHandle)0;

      rv = RvSipStackGetRegClientMgrHandle(mHSipStack,&hRegMgr);
      rv |= RvSipStackGetAuthenticatorHandle(mHSipStack, &hAuthMgr);
      rv |= RvSipStackGetMsgMgrHandle(mHSipStack,&hMsgMgr); 
      rv |= RvSipStackGetSubsMgrHandle(mHSipStack,&hSubsMgr); 
      if(rv>=0) {
	SipChannelHandler epi=NULL;
	{
	  RVSIPLOCK(mLocker);
	  if(mSipChannelsAvailable.empty()) {
	    if(mAllocatedChannels>=mNumChannels) return NULL;
	    epi=new SipChannel(); 
	    mAllocatedChannels++;
	    mSipChannelsAll.push_back(epi);
	  } else {
	    FreeContainerType::iterator iter = mSipChannelsAvailable.begin();
	    if(iter != mSipChannelsAvailable.end()) {
	      epi=*iter;
	      mSipChannelsAvailable.erase(iter);
	    }
	  }
	}
	RVSIPLOCK(epi->mLocker);
	epi->setFree(false);
	epi->set(hMgr, hMsgMgr, mHMidMgr, hRegMgr, hTrMgr, hAuthMgr, mAppPool,hSubsMgr); 
	return epi;
      }
    }
  }
  return 0;
}

SipEngine::~SipEngine()
{
  destroySdpStack();
  destroySipStack();
  if(mAppPool) {
    RPOOL_Destruct(mAppPool);
    mAppPool=NULL;
  }
  {
    ContainerType::iterator iter = mSipChannelsAll.begin();
    while(iter != mSipChannelsAll.end()) {
      SipChannel* sc = *iter;
      delete sc;
      ++iter;
    }
  }

  mDnsCfgs.clear();
  mSipChannelsAvailable.clear();
  mSipChannelsAll.clear();
  {
    for(int cft=0;cft<CFT_TOTAL;++cft) {
      if(mStateMachines[cft]) {
	delete mStateMachines[cft];
	mStateMachines[cft]=0;
      }
    }
  }
}

void SipEngine::releaseChannel(SipChannel* sc) {
  if(sc) {
    RVSIPLOCK(mLocker);
    RVSIPLOCK(sc->mLocker);
    sc->setFree(true);
    mSipChannelsAvailable.insert(sc);
  }
}

size_t SipEngine::getNumberOfBusyChannels() const {
  size_t all = mSipChannelsAll.size();
  size_t av = mSipChannelsAvailable.size();
  if(av>=all) return 0;
  return (all-av);
} 

int SipEngine::init(int numOfThreads, int numChannels, 
		    SipChannelFinder& scFinder,
		    const MD5Processor &md5Processor) {

  mInstance = new SipEngine(numOfThreads, numChannels, scFinder, md5Processor);

  return 0;
}

SipEngine * SipEngine::getInstance() {
  return mInstance;
}

int SipEngine::run_immediate_events(uint32_t delay) {
  return (int)RvSipStackSelectUntil(delay);
}

void SipEngine::setEvHandlers()
{
  {//Call Leg
    /*step 1*/
    RvSipCallLegEvHandlers appEvHandlers;
    /*step 2*/
    memset(&appEvHandlers, 0, sizeof(RvSipCallLegEvHandlers));
    /*step 3*/
    appEvHandlers.pfnCallLegCreatedEvHandler = AppCallLegCreatedEvHandler;
    appEvHandlers.pfnStateChangedEvHandler   = AppStateChangedEvHandler;
    appEvHandlers.pfnPrackStateChangedEvEvHandler   = AppPrackStateChangedEvHandler;
    appEvHandlers.pfnMsgToSendEvHandler  = AppCallLegMsgToSendEvHandler;
    appEvHandlers.pfnMsgReceivedEvHandler= AppCallLegMsgReceivedEvHandler;
    appEvHandlers.pfnSessionTimerRefreshAlertEvHandler = AppCallLegTimerRefreshAlertEvHandler;
    appEvHandlers.pfnSessionTimerNotificationEvHandler = AppCallLegSessionTimerNotificationEv;
    appEvHandlers.pfnByeCreatedEvHandler = AppByeCreatedEvHandler;
    appEvHandlers.pfnByeStateChangedEvHandler = AppByeStateChangedEvHandler;
    appEvHandlers.pfnReInviteCreatedEvHandler = AppCallLegReInviteCreatedEvHandler;
    appEvHandlers.pfnReInviteStateChangedEvHandler = AppCallLegReInviteStateChangedEvHandler;
    
    /*step 4*/
    RvSipCallLegMgrHandle hMgr;	// CallLeg Manager handle
    RvStatus rv = RvSipStackGetCallLegMgrHandle (mHSipStack, &hMgr);
    if(rv>=0 && hMgr) {
      RvSipCallLegMgrSetEvHandlers(hMgr, &appEvHandlers, sizeof(appEvHandlers));
    }
  }

  {//Reg Client
    /*Step 1*/
    RvSipRegClientEvHandlers appEvHandlers;
    /*Step 2*/
    memset(&appEvHandlers, 0, sizeof(RvSipRegClientEvHandlers));
    /*Step 3*/
    appEvHandlers.pfnMsgToSendEvHandler      = AppRegClientMsgToSendEvHandler;
    appEvHandlers.pfnMsgReceivedEvHandler = AppRegClientMsgReceivedEvHandler;
    appEvHandlers.pfnStateChangedEvHandler = AppRegClientStateChangedEvHandler;
    /*Step 4*/
    RvSipRegClientMgrHandle hRegMgr = (RvSipRegClientMgrHandle)0;
    RvStatus rv = RvSipStackGetRegClientMgrHandle(mHSipStack,&hRegMgr);
    if(rv>=0 && hRegMgr) {
      RvSipRegClientMgrSetEvHandlers(hRegMgr, &appEvHandlers, sizeof(appEvHandlers));
    }
  }

  {//Auth
    /*Step 1*/
    RvSipAuthenticatorEvHandlers authEvHandlers;
    /*Step 2*/
    memset(&authEvHandlers,0,sizeof(authEvHandlers));
    /*Step 3*/
    authEvHandlers.pfnGetSharedSecretAuthenticationHandler = AppGetSharedSecretAuthenticationHandler;
    authEvHandlers.pfnNonceCountUsageAuthenticationHandler = AppAuthenticatorNonceCountUsageEv;
    authEvHandlers.pfnMD5AuthenticationExHandler = AppMD5AuthenticationExHandler;
    authEvHandlers.pfnMD5EntityBodyAuthenticationHandler = AppAuthenticatorMD5EntityBodyEv;
    /*Step 4*/
    RvSipAuthenticatorHandle hAuthMgr=(RvSipAuthenticatorHandle)0;
    RvStatus rv = RvSipStackGetAuthenticatorHandle(mHSipStack, &hAuthMgr);
    if(rv>=0 && hAuthMgr) {
      RvSipAuthenticatorSetEvHandlers(hAuthMgr,&authEvHandlers,sizeof(authEvHandlers));
    }
  }

  {//Transactions
    RvSipTranscMgrHandle  hTranscMgr=(RvSipTranscMgrHandle)0;
    RvStatus rv = RvSipStackGetTransactionMgrHandle(mHSipStack,&hTranscMgr);
    if(rv>=0 && hTranscMgr) {
      RvSipTransactionEvHandlers appEvHandlers;
      memset(&appEvHandlers,0,sizeof (RvSipTransactionEvHandlers));
      appEvHandlers.pfnEvTransactionCreated = AppTransactionCreatedEvHandler;
      appEvHandlers.pfnEvStateChanged = AppTransactionStateChangedEvHandler;
      appEvHandlers.pfnEvOpenCallLeg = AppTransactionOpenCallLegEv;
      RvSipTransactionMgrSetEvHandlers(hTranscMgr,NULL,&appEvHandlers,
				       sizeof(RvSipTransactionEvHandlers));
    }
  }
  {//Subscriptions
    RvSipSubsMgrHandle  hSubsMgr=(RvSipSubsMgrHandle)0;
    RvStatus rv = RvSipStackGetSubsMgrHandle(mHSipStack,&hSubsMgr);
    if(rv>=0 && hSubsMgr){
        RvSipSubsEvHandlers appEvHandlers;
        memset(&appEvHandlers,0,sizeof(RvSipSubsEvHandlers));

        appEvHandlers.pfnSubsCreatedEvHandler       = NULL;
        appEvHandlers.pfnStateChangedEvHandler      = AppSubsStateChangedEvHandler;
        appEvHandlers.pfnExpirationAlertEvHandler   = AppSubsExpirationAlertEvHandler;
        appEvHandlers.pfnNotifyEvHandler            = AppSubsNotifyEvHandler;
	appEvHandlers.pfnMsgToSendEvHandler         = AppSubsMsgToSendEvHandler;
        RvSipSubsMgrSetEvHandlers(hSubsMgr, &appEvHandlers, sizeof(RvSipSubsEvHandlers));
    }

  }
  {//Transport
    RvSipTransportMgrHandle hTransportMgr =(RvSipTransportMgrHandle)0;
    RvStatus rv = RvSipStackGetTransportMgrHandle(mHSipStack,&hTransportMgr);
    if(rv>=0 && hTransportMgr){
        RvSipTransportMgrEvHandlers appEvHandlers;
        memset(&appEvHandlers,0,sizeof(RvSipTransportMgrEvHandlers));

        appEvHandlers.pfnEvBadSyntaxMsg = AppTransportBadSyntaxMsgEvHandler;
        RvSipTransportMgrSetEvHandlers(hTransportMgr,NULL,&appEvHandlers,sizeof(appEvHandlers));
    }

  }
}

// Call Leg has created ... 
void SipEngine::appCallLegCreatedEvHandler(
					   IN  RvSipCallLegHandle			hCallLeg,
					   OUT RvSipAppCallLegHandle 	  * phAppCallLeg)
{
  ENTER();

  *phAppCallLeg = 0;

  char strUserName[128];

  RvSipCallLegDirection direction;  
  RvSipCallLegGetDirection(hCallLeg, &direction);
  
  if(direction == RVSIP_CALL_LEG_DIRECTION_INCOMING) {

    RvSipMsgHandle       hMsg;
    RvSipCallLegGetReceivedMsg(hCallLeg,&hMsg);
    RvSipUtils::SIP_ToPhoneNumberExtract(hMsg, strUserName, sizeof(strUserName));
    
    RVSIPPRINTF("%s: cl=0x%x, direction=%d, uname=%s\n",__FUNCTION__,(unsigned int)hCallLeg,(int)direction,strUserName);
    
    SipChannel* sc=NULL;
    {
      RvSipTranscHandle hTransc = (RvSipTranscHandle)0;
      RvSipCallLegGetActiveTransaction(hCallLeg,&hTransc);
      if(hTransc) {
	RvSipTransportLocalAddrHandle handle = RvSipTransactionGetCurrentLocalAddressHandle(hTransc);
	if(handle) {
	  sc = mScFinder.findSipChannel(strUserName,(int)handle);	
	}
      }
    }
    
    if(sc) {
      if(!sc->isFree()) {
	*phAppCallLeg = reinterpret_cast<RvSipAppCallLegHandle>(sc);
      }
    }
  }
  
  RET;
}

// An event has come ... 
void SipEngine::appStateChangedEvHandler(
					 IN  RvSipCallLegHandle            hCallLeg,
					 IN  RvSipAppCallLegHandle         hAppCallLeg,
					 IN  RvSipCallLegState             eState,
					 IN  RvSipCallLegStateChangeReason eReason)
{
  ENTER();
  
  SipChannel * sc = (SipChannel * )hAppCallLeg;

  if(!sc) {

    if (eState == RVSIP_CALL_LEG_STATE_OFFERING) {
      RvUint16 code=404;
      RvSipCallLegReject ( hCallLeg, code );
    }
    return;

  } else {

    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree()) {
      
      RVSIPPRINTF("%s: hCallLeg=0x%x, eState=%lu, eReason=%lu\n",__FUNCTION__,(int)hCallLeg,(unsigned long)eState,(unsigned long)eReason);

      getCSM(sc).appStateChangedEvHandler(hCallLeg,hAppCallLeg,eState,eReason,sc);
    }
  }
  
  RET;
}

void SipEngine::appByeCreatedEvHandler(
				       IN  RvSipCallLegHandle            hCallLeg,
				       IN  RvSipAppCallLegHandle         hAppCallLeg,
				       IN  RvSipTranscHandle             hTransc,
				       OUT RvSipAppTranscHandle          *hAppTransc,
				       OUT RvBool                       *bAppHandleTransc) {
  if(hAppCallLeg) {
    *hAppTransc=(RvSipAppTranscHandle)hAppCallLeg;
    *bAppHandleTransc=RV_TRUE;
  }
}

void SipEngine::appByeStateChangedEvHandler(
					    IN  RvSipCallLegHandle                hCallLeg,
					    IN  RvSipAppCallLegHandle             hAppCallLeg,
					    IN  RvSipTranscHandle                 hTransc,
					    IN  RvSipAppTranscHandle              hAppTransc,
					    IN  RvSipCallLegByeState              eByeState,
					    IN  RvSipTransactionStateChangeReason eReason) {
  
  if(hAppCallLeg) {
    
    SipChannel * sc = (SipChannel * )hAppCallLeg;
    
    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree()) {
      getCSM(sc).appByeStateChangedEvHandler(hCallLeg,hAppCallLeg,hTransc,hAppTransc,eByeState,eReason,sc);
    }
  }
}

// An event has come ... 
void SipEngine::appPrackStateChangedEvHandler(
					      IN  RvSipCallLegHandle            hCallLeg,
					      IN  RvSipAppCallLegHandle         hAppCallLeg,
					      IN  RvSipCallLegPrackState        eState,
					      IN  RvSipCallLegStateChangeReason eReason,
					      IN  RvInt16                       prackResponseCode)
{
  ENTER();
  
  SipChannel * sc = (SipChannel * )hAppCallLeg;
  
  if(sc) {

    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree()) {
      getCSM(sc).appPrackStateChangedEvHandler(hCallLeg,hAppCallLeg,eState,eReason,prackResponseCode,sc);
    }
  }
  
  RET;
}

RV_Status SipEngine::appCallLegMsgToSendEvHandler(
						  IN RvSipCallLegHandle            hCallLeg,
						  IN RvSipAppCallLegHandle         hAppCallLeg,
						  IN RvSipMsgHandle                hMsg)
{
    ENTER();

    SipChannel * sc = (SipChannel * )hAppCallLeg;

    if(sc && !sc->isFree()) {

        RVSIPLOCK(sc->mLocker);

        if(sc->mHCallLeg == hCallLeg) {
            CallStateNotifier* notifier = NULL;

            RvSipMsgForceCompactForm(hMsg,sc->isShortForm());

            if(sc->isTCP()) {
                setSupportedTCP(hMsg);
            }

            if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST) {

                RVSIPPRINTF("%s: cl=0x%x, msg=%d[%d]\n",__FUNCTION__,(unsigned int)hCallLeg,(int)RvSipMsgGetRequestMethod(hMsg),(int)RvSipMsgGetMsgType(hMsg)); 

                RvSipMethodType msgType = RvSipMsgGetRequestMethod(hMsg);
                switch(msgType){
                    case RVSIP_METHOD_ACK:
                        RvSipPartyHeaderHandle hToHandle;

                        hToHandle = RvSipMsgGetToHeader(hMsg);
                        RvSipPartyHeaderSetCompactForm(hToHandle,sc->isShortForm());

                        removeContactHeader(hMsg);
                        sc->addUserAgentInfo((void *)hMsg);

                        notifier = sc->mCallStateNotifier;
                        if(notifier)
                            notifier->ackSent();
                        break;
                    case RVSIP_METHOD_CANCEL:
                        sc->addDynamicRoutes(hMsg);
                        break;
                    case RVSIP_METHOD_INVITE:
                        sc->addUserAgentInfo((void *)hMsg);
                        notifier = sc->mCallStateNotifier;
                        if(notifier)
                            notifier->inviteSent(sc->getInitialInviteSent());
                        sc->setInitialInviteSent(true);
                        if(sc->useMobilePrivate())
                            sc->addNetworkAccessInfoToMsg((void *)hMsg);
                        break;
                    case RVSIP_METHOD_BYE:
                        sc->addUserAgentInfo((void *)hMsg);
                        notifier = sc->mCallStateNotifier;
                        if(notifier)
                            notifier->byeSent(sc->getInitialByeSent(), sc->getAckStartTime());
                        sc->setInitialByeSent(true);
                        if(sc->useMobilePrivate())
                            sc->addNetworkAccessInfoToMsg((void *)hMsg);
                        break;
                    default:
                        if(sc->useMobilePrivate())
                            sc->addNetworkAccessInfoToMsg((void *)hMsg);
                        break;

                }

                /* OPTIONS TESTING
                   if(sc->isOriginate()) {
                   RvSipMethodType method = RvSipMsgGetRequestMethod(hMsg);
                   if(method==RVSIP_METHOD_BYE) {
                   RvSipMsgSetMethodInRequestLine(hMsg,RVSIP_METHOD_OTHER,"OPTIONS");
                   RvSipCSeqHeaderHandle hcseq = RvSipMsgGetCSeqHeader(hMsg);
                   RvSipCSeqHeaderSetMethodType(hcseq,RVSIP_METHOD_OTHER,"OPTIONS");
                   RvSipMsgSetCallIdHeader(hMsg,"a84b4c76e66710");
                   }
                   }
                   */
            } else if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE) {
                RVSIPPRINTF("%s: cl=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hCallLeg,(int)RvSipMsgGetStatusCode(hMsg),
                        (int)RvSipMsgGetMsgType(hMsg)); 

                int statusCode = (int)RvSipMsgGetStatusCode(hMsg);

                if(statusCode != 401 && statusCode != 407) {
                    notifier = sc->mCallStateNotifier;
                    if(notifier)
                        notifier->responseSent(statusCode);
                }
                if(sc->useMobilePrivate()){
                    if((statusCode != 487) || (RvSipCSeqHeaderGetMethodType(RvSipMsgGetCSeqHeader(hMsg)) != RVSIP_METHOD_CANCEL))
                        sc->addNetworkAccessInfoToMsg((void *)hMsg);

                }
            }
        }
    }
    RETCODE(RV_OK);
}
void SipEngine::appCallLegReInviteCreatedEvHandler(
                    IN RvSipCallLegHandle               hCallLeg,
                    IN RvSipAppCallLegHandle            hAppCallLeg, 
                    IN RvSipCallLegInviteHandle         hReInvite,
                    OUT RvSipAppCallLegInviteHandle     *phAppReInvite,
                    OUT RvUint16                        *pResponseCode)
{
    ENTER();
    SipChannel * sc = (SipChannel * )hAppCallLeg;

    if(sc && !sc->isFree()) {

        RVSIPLOCK(sc->mLocker);

        if(sc->mHCallLeg == hCallLeg) {
            if(sc->getSMType() == CFT_TIP){
                sc->setReinviteHandle(hReInvite); 
            }
            *phAppReInvite = NULL;
            *pResponseCode = 0;
        }
    }

    RET;
}

void SipEngine::appCallLegReInviteStateChangedEvHandler(
                            IN RvSipCallLegHandle               hCallLeg,
                            IN RvSipAppCallLegHandle            hAppCallLeg, 
                            IN RvSipCallLegInviteHandle         hReInvite,
                            IN RvSipAppCallLegInviteHandle      hAppReInvite,
                            IN RvSipCallLegModifyState          eModifyState,
                            IN RvSipCallLegStateChangeReason    eReason )
{
    ENTER();

    SipChannel * sc = (SipChannel * )hAppCallLeg;

    if(sc && !sc->isFree()) {

        RVSIPLOCK(sc->mLocker);

        if(sc->mHCallLeg == hCallLeg && hReInvite == sc->mHReInvite) {
            switch(eModifyState){
                case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_RCVD:
                    {
                        sc->mMediaHandler->media_beforeAccept(hCallLeg);
                        RvSipCallLegAccept(hCallLeg);
                    }
                    break;
                case RVSIP_CALL_LEG_MODIFY_STATE_REINVITE_REMOTE_ACCEPTED:
                    RvSipCallLegReInviteAck(hCallLeg,hReInvite);
                    break;
                case RVSIP_CALL_LEG_MODIFY_STATE_ACK_SENT:
                case RVSIP_CALL_LEG_MODIFY_STATE_ACK_RCVD:
                    {
                        RvUint16 code = 0;
                        RvStatus rv = sc->mMediaHandler->media_onConnected(hCallLeg,code,false);
                        if(rv < 0 ){
                            RvSipCallLegDisconnect(hCallLeg);
                        }
                    }
                    break;
                case RVSIP_CALL_LEG_MODIFY_STATE_TERMINATED:
                    sc->setReinviteHandle(NULL);
                    break;
                default:
                    break;

            }
        }
    }
    RET;
}

RV_Status SipEngine::appCallLegMsgReceivedEvHandler(
						    IN RvSipCallLegHandle            hCallLeg,
						    IN RvSipAppCallLegHandle         hAppCallLeg,
						    IN RvSipMsgHandle                hMsg)
{
  ENTER();
   
  SipChannel * sc = (SipChannel * )hAppCallLeg;
  CallStateNotifier* notifier = NULL;
  
  if(sc) {
      if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST) {
        
        RVSIPPRINTF("%s: cl=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hCallLeg,
        (int)RvSipMsgGetRequestMethod(hMsg),(int)RvSipMsgGetMsgType(hMsg));  
        
        RvSipMethodType msgType = RvSipMsgGetRequestMethod(hMsg);
        
        switch(msgType){
            case RVSIP_METHOD_ACK: {
                RVSIPLOCK(sc->mLocker);
                notifier = sc->mCallStateNotifier;
                if(notifier)
                    notifier->ackReceived();
                break;
            }
            case RVSIP_METHOD_INVITE: {
                RvSipOtherHeaderHandle hOtherHeader = NULL;
                RvSipHeaderListElemHandle hListElem = NULL;

                char buff[4];
                size_t length = 0;
                /* initialize as 70, since it is the default value.*/
                size_t maxForwards = 70;   

                if(NULL != (hOtherHeader = RvSipMsgGetHeaderByName(hMsg, "Max-Forwards", RVSIP_FIRST_HEADER, &hListElem)))
                { 
                    RvSipOtherHeaderGetValue(hOtherHeader, buff, sizeof(buff), &length);

                    RVSIPLOCK(sc->mLocker);
                    notifier = sc->mCallStateNotifier;
                    maxForwards = atoi(buff);    
                }
                if(notifier)
                    notifier->inviteReceived(maxForwards);

                break;
            }
            case RVSIP_METHOD_BYE: {
                RVSIPLOCK(sc->mLocker);
                notifier = sc->mCallStateNotifier;
                if(notifier)
                    notifier->byeReceived();
                break;
            }
            default: {
                break;
            }
        }

      } else if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE) {
          RVSIPPRINTF("%s: cl=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hCallLeg,
          (int)RvSipMsgGetStatusCode(hMsg),(int)RvSipMsgGetMsgType(hMsg));
        
          int statusCode = (int)RvSipMsgGetStatusCode(hMsg);

          if(statusCode > 100 && statusCode <200){
              RVSIPLOCK(sc->mLocker);
              if(sc->getSMType() == CFT_TIP){
                  RvUint16 code;
                  sc->mMediaHandler->media_prepareConnect(hCallLeg,code,hMsg);
                  if(sc->RemoteSDPOffered()){
                      sc->mMediaHandler->media_startTip(sc->generate_mux_csrc());
                      sc->TipStarted(true);
                  }
              }

              if(statusCode == 180) {
                  notifier = sc->mCallStateNotifier;
                  if(notifier)
                      notifier->inviteResponseReceived(true);
              }
          }

          if(statusCode != 401 && statusCode != 407) {
              RVSIPLOCK(sc->mLocker);
              notifier = sc->mCallStateNotifier;
              if(notifier)
                  notifier->responseReceived(statusCode);
          }
      }
  }
  RETCODE(RV_OK);
}

RV_Status SipEngine::setSupportedTCP(RvSipMsgHandle hMsg) {

  RV_Status rv=RV_OK;

  if(hMsg) {
    
    RvSipOtherHeaderHandle    hOther;
    RvSipHeaderListElemHandle    listElem;
    
    hOther = (RvSipOtherHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
								 RVSIP_HEADERTYPE_OTHER, 
								 RVSIP_FIRST_HEADER, 
								 RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
    while (hOther != NULL) {

      RvChar strBuffer[129] = {'\0'};
      RvUint len=0;

      RvSipOtherHeaderGetName(hOther,
			      strBuffer,
			      sizeof(strBuffer),
			      &len);
      if(len>0 && strBuffer[0]) {

	if(strcmp(strBuffer,"supported")==0 || strcmp(strBuffer,"Supported")==0 || strcmp(strBuffer,"SUPPORTED")==0) {
	  strcpy(strBuffer,SIP_TCP_SUPPORTED);
	  RvSipOtherHeaderSetValue(hOther, strBuffer);
	  break;
	}
      }
      // get the next
      hOther = (RvSipOtherHeaderHandle)RvSipMsgGetHeaderByTypeExt( hMsg,
								   RVSIP_HEADERTYPE_OTHER, 
								   RVSIP_NEXT_HEADER, 
								   RVSIP_MSG_HEADERS_OPTION_ALL, &listElem);
    }
  }

  return rv;
}

RV_Status SipEngine::setSupportedReg(RvSipMsgHandle hMsg) {

  RV_Status rv=RV_OK;

  if(hMsg) {

    RvSipOtherHeaderHandle hOtherHeader = NULL;
    rv = RvSipOtherHeaderConstructInMsg(hMsg, RV_TRUE, &hOtherHeader);
    if(rv>=0 && hOtherHeader) {
      RvChar name[11];
      strcpy(name,"Supported");
      rv = RvSipOtherHeaderSetName(hOtherHeader, name);
      if(rv >= 0) {
	RvChar value[129];
	strcpy(value,SIP_REG_SUPPORTED); 
	rv = RvSipOtherHeaderSetValue(hOtherHeader, value);
      }
    }
  }

  return rv;
}

RV_Status SipEngine::appRegClientMsgToSendEvHandler(
						    IN RvSipRegClientHandle            hRegClient,
						    IN RvSipAppRegClientHandle         hAppRegClient,
						    IN RvSipMsgHandle                hMsg)
{
  ENTER();

  SipChannel * sc = (SipChannel * )hAppRegClient;
  
  if(sc && !sc->isFree()) {
      RVSIPLOCK(sc->mLocker);

      if(sc->isTCP()) {
        setSupportedTCP(hMsg);
      }

      RvSipMsgForceCompactForm(hMsg,sc->isShortForm());

      if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST) {
          sc->addUserAgentInfo((void *)hMsg);
          if(sc->useMobilePrivate())
              sc->addNetworkAccessInfoToMsg((void *)hMsg);

          RVSIPPRINTF("%s: rc=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hRegClient,(int)RvSipMsgGetRequestMethod(hMsg),
            (int)RvSipMsgGetMsgType(hMsg));

      } else if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE) {

          RVSIPPRINTF("%s: rc=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hRegClient,(int)RvSipMsgGetStatusCode(hMsg),
            (int)RvSipMsgGetMsgType(hMsg)); 
        
          int statusCode = (int)RvSipMsgGetStatusCode(hMsg);
      
          if(statusCode != 401 && statusCode != 407) {
              if(sc->mHRegClient == hRegClient) {
                CallStateNotifier* notifier = sc->mCallStateNotifier;
                if(notifier)
                    notifier->responseSent(statusCode);
              }
          }
      }

      setSupportedReg(hMsg);
  }
  
  RETCODE(RV_OK);
}

RV_Status SipEngine::appRegClientMsgReceivedEvHandler(
						    IN RvSipRegClientHandle            hRegClient,
						    IN RvSipAppRegClientHandle         hAppRegClient,
						    IN RvSipMsgHandle                hMsg)
{
  ENTER();

  if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST) {
    RVSIPPRINTF("%s: rc=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hAppRegClient,
		(int)RvSipMsgGetRequestMethod(hMsg),(int)RvSipMsgGetMsgType(hMsg));   
  } else {
    RVSIPPRINTF("%s: rc=0x%x, msg=%d[%d]\n",__FUNCTION__,(int)hAppRegClient,
		(int)RvSipMsgGetStatusCode(hMsg),(int)RvSipMsgGetMsgType(hMsg));
    
    int statusCode = (int)RvSipMsgGetStatusCode(hMsg);

    if(statusCode != 401 && statusCode != 407) {
      SipChannel * sc = (SipChannel * )hAppRegClient;
      if(sc) {
        RVSIPLOCK(sc->mLocker);
        CallStateNotifier* notifier = sc->mCallStateNotifier;
        if(notifier)
            notifier->responseReceived(statusCode);
      }
    }
  }	
  RETCODE(RV_OK);
}

RV_Status SipEngine::appSubsMsgToSendEvHandler(IN  RvSipSubsHandle          hSubsClient,
					       IN  RvSipAppSubsHandle       hAppSubsClient,
					       IN  RvSipNotifyHandle        hNotify,
					       IN  RvSipAppNotifyHandle     hAppNotify,
					       IN  RvSipMsgHandle           hMsg)
{
    ENTER();

    SipChannel * sc = (SipChannel * )hAppSubsClient;

    if(sc) {

        RVSIPLOCK(sc->mLocker);

        if(!sc->isFree()) {

            if(sc->isTCP()) {
                setSupportedTCP(hMsg);
            }

            if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST) {
                if(sc->useMobilePrivate()){
                    sc->addPreferredIdentity((void *)hMsg);
                    sc->addNetworkAccessInfoToMsg((void *)hMsg);
                }
                sc->sessionSetCallLegPrivacy((void *)hMsg);
            }

            RvSipMsgForceCompactForm(hMsg,sc->isShortForm());
        }
    }

    RETCODE(RV_OK);
}

RV_Status SipEngine::appRegClientStateChangedEvHandler(
						       IN RvSipRegClientHandle            hRegClient,
						       IN RvSipAppRegClientHandle         hAppRegClient,
						       IN  RvSipRegClientState             eNewState,
						       IN  RvSipRegClientStateChangeReason eReason)
{
  ENTER();

  RV_Status rv = RV_OK;

  SipChannel * sc = (SipChannel * )hAppRegClient;

  RVSIPPRINTF("%s: sc=0x%x, rc=0x%x, apprc=0x%x, new state=%d, reason=%d\n",__FUNCTION__, (int)sc,(int)hRegClient,(int)hAppRegClient,eNewState,eReason);
  
  if(eNewState == RVSIP_REG_CLIENT_STATE_REGISTERED &&
     eReason == RVSIP_REG_CLIENT_REASON_RESPONSE_UNAUTHENTICATED_RECVD){
      eNewState = RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED;
  }
  
  if(sc) {

    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree() && sc->mHRegClient==hRegClient) {

      switch (eNewState) {
      case RVSIP_REG_CLIENT_STATE_REGISTERING: {
          sc->mIsRegistering=true;
          break;
      }
      case RVSIP_REG_CLIENT_STATE_REGISTERED: {
          CallStateNotifier* notifier = sc->mCallStateNotifier;

          sc->mIsRegistering=false;
          sc->mAuthTry = 0;
          sc->regSuccessNotify();
          if(notifier)
              notifier->nonInviteSessionSuccessful();
          break;
      }
      case RVSIP_REG_CLIENT_STATE_FAILED:
          sc->mIsRegistering=false;
          sc->regFailureNotify();
          break;
      case RVSIP_REG_CLIENT_STATE_MSG_SEND_FAILURE:
          rv = RvSipRegClientDNSContinue(hRegClient);
          if(rv<0) RVSIPPRINTF("%s: 111.111: rv=%d\n",__FUNCTION__,rv);
          if(rv>=0) {
              sc->sessionSetRegClientAllow();
              rv = RvSipRegClientDNSReSendRequest(hRegClient);

              if(rv<0) RVSIPPRINTF("%s: 111.222: rv=%d\n",__FUNCTION__,rv);
          }
          if(rv < 0) {
              sc->mHRegClient=0;
              rv = RvSipRegClientTerminate(hRegClient);
              sc->mIsRegistering=false;
              uint32_t regRetriesInitial = sc->getRegRetries();
              sc->regFailureNotify();   
              CallStateNotifier* notifier = sc->mCallStateNotifier;
              if(notifier){
                  bool transport = false;
                  bool timeout = false;

                  switch(eReason) {
                      case RVSIP_REG_CLIENT_REASON_TRANSACTION_TIMEOUT:
                          timeout = true;
                          break;
                      case RVSIP_REG_CLIENT_REASON_NETWORK_ERROR:
                          transport = true;
                          break;
                      default:
                          ;
                          break;
                  }

                  if((transport || timeout) && (regRetriesInitial == 0))
                      notifier->nonInviteSessionFailed(transport, timeout);
              }
                
          }
          break;
      case RVSIP_REG_CLIENT_STATE_REDIRECTED:
          sc->sessionSetRegClientAllow();
          RvSipRegClientRegister(hRegClient);
          break;
      case RVSIP_REG_CLIENT_STATE_UNAUTHENTICATED:
      {
          //Tmp:
          //if(sc->mAuthTry>0) {
          //  rv = RvSipRegClientTerminate(hRegClient);
          //  sc->mIsRegistering=false;
          //  sc->regFailureNotify();
          //  break;
          //}
          sc->mAuthTry++;
                sc->sessionSetRegClientAllow();
          RvSipUtils::SIP_RemoveAuthHeader(hRegClient);
          RvSipMsgHandle hSipMsg=(RvSipMsgHandle)0;
          RvSipRegClientGetReceivedMsg(hRegClient,&hSipMsg);
          bool aka=RvSipUtils::RvSipAka::SIP_isAkaMsg(hSipMsg);
          RVSIPPRINTF("%s: aka=%d\n",__FUNCTION__,(int)aka);
            if(aka != sc->useAka()){
              RvSipRegClientTerminate(hRegClient);
              sc->mIsRegistering=false;
              sc->regFailureNotify();
                break;
            }

          if(aka) {
            RvSipAuthObjHandle   hAuthObj=(RvSipAuthObjHandle)aka;
            // get the authentication object from the 401-response message.
            rv = RvSipRegClientAuthObjGet (hRegClient, RVSIP_FIRST_ELEMENT, NULL, &hAuthObj);
            if(rv != RV_Success) {
              rv = RvSipRegClientTerminate(hRegClient);
              sc->mIsRegistering=false;
              sc->regFailureNotify();
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
              rv = RvSipRegClientTerminate(hRegClient);
              sc->mIsRegistering=false;
              sc->regFailureNotify();
              break;
            }
          }

          rv = RvSipRegClientAuthenticate(hRegClient);
      }
      break;
      case RVSIP_REG_CLIENT_STATE_TERMINATED:
          if(sc->mIsRegistering) {
            sc->regFailureNotify();
            sc->mIsRegistering=false;
          }
          sc->stopRegistration();
          break;
      default:
               ;
      }
    }
  }
  
  RETCODE(rv);
}

RvStatus SipEngine::appCallLegTimerRefreshAlertEvHandler(IN    RvSipCallLegHandle       hCallLeg,
							 IN    RvSipAppCallLegHandle    hAppCallLeg) {
  
  ENTER();

  RvStatus rv=RV_OK;
  
  SipChannel * sc = (SipChannel * )hAppCallLeg;

  if(sc) {

    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree() && (sc->mHCallLeg == hCallLeg)) {
      rv = sc->callLegSessionTimerRefreshAlertEv(hCallLeg);
    }
  }

  RETCODE(rv);
}

RvStatus SipEngine::appCallLegSessionTimerNotificationEv(
							 IN RvSipCallLegHandle                         hCallLeg,
							 IN RvSipAppCallLegHandle                      hAppCallLeg,
							 IN RvSipCallLegSessionTimerNotificationReason eReason) {
  ENTER();
  
  RvStatus rv=RV_OK;
  
  SipChannel * sc = (SipChannel * )hAppCallLeg;

  if(sc) {

    RVSIPLOCK(sc->mLocker);

    if(!sc->isFree() && (sc->mHCallLeg == hCallLeg)) {
      rv = sc->callLegSessionTimerNotificationEv(hCallLeg,eReason);
    }
  }
  
  RETCODE(rv);
}

void SipEngine::appMD5AuthenticationExHandler (
					       IN    RvSipAuthenticatorHandle        hAuthenticator,
					       IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
					       IN  RPOOL_Ptr                     *pRpoolMD5Input,
					       IN  RV_UINT32                     length,
					       OUT RPOOL_Ptr                     *pRpoolMD5Output)
{
  ENTER();
  
  unsigned char *strInput;
  unsigned char strResponse[MAX_LENGTH_OF_DIGEST_RESPONSE];
  unsigned char digest[MAX_LENGTH_OF_DIGEST];
  MD5Processor::state_t mdc;
  /*Allocates the consecutive buffer.*/
  strInput = (unsigned char*)malloc(length);
  if(!strInput)
      RET;
  /*Gets the string out of the page.*/
  RPOOL_CopyToExternal(pRpoolMD5Input->hPool,
		       pRpoolMD5Input->hPage,
		       pRpoolMD5Input->offset,
		       (void*) strInput,
		       length);
  /*Implements the MD5 algorithm.*/
  mMD5Processor.init(&mdc);
  mMD5Processor.update(&mdc, strInput,strlen((char*)strInput));
  mMD5Processor.finish(&mdc,digest);
  /*Changes the digest into a string format.*/
  MD5toString(digest, (char*)strResponse);
  RVSIPPRINTF("%s: digest=%s\n",__FUNCTION__,strResponse);
  /*Inserts the MD5 output to the page that is supplied in pRpoolMD5Output.*/
  RPOOL_AppendFromExternalToPage(pRpoolMD5Output->hPool,
				 pRpoolMD5Output->hPage,
				 (void*)strResponse,
				 strlen((char*)strResponse) + 1,
				 &(pRpoolMD5Output->offset));
  free(strInput);
  RET;
}

void SipEngine::appGetSharedSecretAuthenticationHandler(
							IN    RvSipAuthenticatorHandle       hAuthenticator,
							IN    RvSipAppAuthenticatorHandle    hAppAuthenticator,
							IN    void*                          hObject,
							IN    void*                          peObjectType,
							IN    RPOOL_Ptr                     *pRpoolRealm,
							INOUT RPOOL_Ptr                     *pRpoolUserName,
							OUT   RPOOL_Ptr                     *pRpoolPassword)
{
  ENTER();

  char  realm[MAX_LENGTH_OF_REALM_NAME];
  char* unquotedRealm = NULL;
  int   realmLength = 0;
  RvStatus status=RV_OK;

  RVSIPPRINTF("%s: obj=0x%x, app=0x%x, peObjectType=0x%x\n",__FUNCTION__,(int)hObject,(int)hAppAuthenticator,(int)peObjectType);
  if(hObject && peObjectType) {
    RvSipCommonStackObjectType eObjectType = *(RvSipCommonStackObjectType *)peObjectType; 
    RVSIPPRINTF("%s: oType=%d\n",__FUNCTION__,(int)eObjectType);
    SipChannel *sc=NULL;
    if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT) {
      RvSipRegClientGetAppHandle((RvSipRegClientHandle)hObject,(RvSipAppRegClientHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT) {
      RvSipPubClientGetAppHandle((RvSipPubClientHandle)hObject,(RvSipAppPubClientHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG) {
      RvSipCallLegGetAppHandle((RvSipCallLegHandle)hObject,(RvSipAppCallLegHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_APP_OBJECT) {
      sc=(SipChannel*)hObject;
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION) {
      RvSipSubsGetAppSubsHandle((RvSipSubsHandle)hObject,(RvSipAppSubsHandle*)&sc);
    }
    RVSIPPRINTF("%s: sc=0x%x\n",__FUNCTION__,(int)sc);
    if(sc) {
      RVSIPLOCK(sc->mLocker);
      if(!sc->isFree()) {
	
	std::string authName;
	std::string authPasswd;
	std::string authRealm;
	
	sc->getAuth(authName,authRealm,authPasswd);

	if(!authRealm.empty()) {
	  // Get realm value of the proxy-authentication header of the message.
	  realmLength = RPOOL_Strlen(pRpoolRealm->hPool, pRpoolRealm->hPage, pRpoolRealm->offset);
	  status = RPOOL_CopyToExternal(pRpoolRealm->hPool, pRpoolRealm->hPage, pRpoolRealm->offset,
					realm, realmLength);
	  if(status != RV_OK) {
	    RET;
	  } 
	  
	  // remove the left and right quote character from the realm.
	  if(realmLength >= 2) {
        if(realmLength > sizeof(realm))
          realmLength = sizeof(realm);
        unquotedRealm = realm + 1;  
        realm[realmLength-1] = '\0';
	  } else {
	    unquotedRealm = realm;
	  }
	  
	  if(strcmp(unquotedRealm,authRealm.c_str())) {
	    RET;
	  }
	}
	
	char strAuthUserName[MAX_LENGTH_OF_USER_NAME];
	strncpy(strAuthUserName,authName.c_str(),sizeof(strAuthUserName)-1);

	/*Appends the username to the given page.*/
	RPOOL_AppendFromExternalToPage(
				       pRpoolUserName->hPool,
				       pRpoolUserName->hPage,
				       (void*)strAuthUserName,
				       strlen(strAuthUserName) + 1,
				       &(pRpoolUserName->offset));
	
	char strAuthPassword[MAX_LENGTH_OF_PWD];

	if(!sc->mUseAka) {
	  strncpy(strAuthPassword,authPasswd.c_str(),sizeof(strAuthPassword)-1);
	} else {
	  strncpy(strAuthPassword,(char*)(sc->mAuthVector.XRES),sizeof(strAuthPassword)-1);
	}

	
	/*Appends the password to the given page.*/
	RPOOL_AppendFromExternalToPage(
				       pRpoolPassword->hPool,
				       pRpoolPassword->hPage,
				       (void*) strAuthPassword,
				       strlen(strAuthPassword) + 1,
				       &(pRpoolPassword->offset));
	
      }
    }
  }
  RET;
}

void SipEngine::appAuthenticatorNonceCountUsageEv(
						 IN    RvSipAuthenticatorHandle        hAuthenticator,
						 IN    RvSipAppAuthenticatorHandle     hAppAuthenticator,
						 IN    void*                           hObject,
						 IN    void*                           peObjectType,
						 IN    RvSipAuthenticationHeaderHandle hAuthenticationHeader,
						 INOUT RvInt32*                        pNonceCount)
{
  ENTER();
  RVSIPPRINTF("%s: 111.111: obj=0x%x, app=0x%x, peObjectType=0x%x, nc=%d\n",__FUNCTION__,
	 (int)hObject,(int)hAppAuthenticator,(int)peObjectType,(int)(*pNonceCount));
  if(hObject && peObjectType) {
    RvSipCommonStackObjectType eObjectType = *(RvSipCommonStackObjectType *)peObjectType; 
    RVSIPPRINTF("%s: oType=%d\n",__FUNCTION__,(int)eObjectType);
    SipChannel *sc=NULL;
    if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT) {
      RvSipRegClientGetAppHandle((RvSipRegClientHandle)hObject,(RvSipAppRegClientHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT) {
      RvSipPubClientGetAppHandle((RvSipPubClientHandle)hObject,(RvSipAppPubClientHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG) {
      RvSipCallLegGetAppHandle((RvSipCallLegHandle)hObject,(RvSipAppCallLegHandle*)&sc);
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_APP_OBJECT) {
      sc=(SipChannel*)hObject;
    } else if(eObjectType == RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION) {
      RvSipSubsGetAppSubsHandle((RvSipSubsHandle)hObject,(RvSipAppSubsHandle*)&sc);
    }
    RVSIPPRINTF("%s: sc=0x%x\n",__FUNCTION__,(int)sc);
    if(sc) {
      RVSIPLOCK(sc->mLocker);
      if(!sc->isFree()) {
	
	RvSipAuthQopOption         eQopOption  = (RvSipAuthQopOption)0;
	
	// Especially for endpoint/manual, we are only required to provide a nonce-count value in an authorization 
	// header if the authentication header contains a QOP option.   So, let's check to see whether the authentication
	// header contain a QOP option. if no such option found, then we don't need to continue (default behavior)
	eQopOption = RvSipAuthenticationHeaderGetQopOption(hAuthenticationHeader);
	if(eQopOption == RVSIP_AUTH_QOP_AUTH_ONLY ||
	   eQopOption == RVSIP_AUTH_QOP_AUTHINT_ONLY ||
	   eQopOption == RVSIP_AUTH_QOP_AUTH_AND_AUTHINT) {
	  
	  char                       nonce[MAX_LENGTH_OF_NONCE];
	  RvUint32                   nonceLength = 0;
	  char                       realm[MAX_LENGTH_OF_REALM_NAME];
	  RvUint32                   realmLength = 0;
	  
	  
	  // get the given realm and nonce value.
	  if(RvSipAuthenticationHeaderGetRealm(hAuthenticationHeader, realm, sizeof(realm)-1, &realmLength) >= 0 &&
	     realmLength > 0 &&
	     RvSipAuthenticationHeaderGetNonce(hAuthenticationHeader, nonce, sizeof(nonce)-1, &nonceLength) >= 0 &&
	     nonceLength > 0) {
	    
	    realm[realmLength] = '\0';
	    nonce[nonceLength] = '\0';
	    
	    *pNonceCount = sc->setNonceCount(realm, nonce, *pNonceCount);
	  }
	}
      }
    }
  }
  
  RVSIPPRINTF("%s: 111.222: nc=%d\n",__FUNCTION__,(int)(*pNonceCount));
  
  RET;
}

void SipEngine::appAuthenticatorMD5EntityBodyEv(
						IN     RvSipAuthenticatorHandle      hAuthenticator,
						IN     RvSipAppAuthenticatorHandle   hAppAuthenticator,
						IN     void*                         hObject,
						IN     void*                         peObjectType,
						IN     RvSipMsgHandle                hMsg,
						OUT    RPOOL_Ptr                     *pRpoolMD5Output,
						OUT    RvUint32                      *pLength) {

    ENTER();
  
    RVSIPPRINTF("%s: app=0x%x\n",__FUNCTION__,(unsigned int)hAppAuthenticator);

    *pLength=0;
    
    unsigned char strBody[65537];
    uint32_t length = 0;
    strBody[0]=0;
    
    {
      RvSipBodyHandle                   hBody;
      
      hBody = RvSipMsgGetBodyObject (hMsg);
      if (NULL != hBody) {
	
	length = RvSipBodyGetBodyStrLength(hBody);
	
	if(length>0 && (length+1)<=sizeof(strBody)) {
	  RvSipBodyGetBodyStr(hBody, (RvChar*)strBody, length, &length);
	}
      }
    }
    
    strBody[length]=0;
    
    unsigned char strResponse[MAX_LENGTH_OF_DIGEST_RESPONSE];
    unsigned char digest[MAX_LENGTH_OF_DIGEST];
    MD5Processor::state_t mdc;
    
    /*Implements the MD5 algorithm.*/
    mMD5Processor.init(&mdc);
    mMD5Processor.update(&mdc, strBody,length);
    mMD5Processor.finish(&mdc,digest);
    /*Changes the digest into a string format.*/
    MD5toString(digest, (char*)strResponse);
    RVSIPPRINTF("%s: digest=%s\n",__FUNCTION__,strResponse);
    /*Inserts the MD5 output to the page that is supplied in pRpoolMD5Output.*/
    length = strlen((char*)strResponse);
    RPOOL_AppendFromExternalToPage(pRpoolMD5Output->hPool,
				   pRpoolMD5Output->hPage,
				   (void*)strResponse,
				   length + 1,
				   &(pRpoolMD5Output->offset));
    
    *pLength=length;
    
    RET;
}

void SipEngine::appSubsStateChangedEvHandler(
                                              IN  RvSipSubsHandle            hSubs,
                                              IN  RvSipAppSubsHandle         hAppSubs,
                                              IN  RvSipSubsState             eState,
                                              IN  RvSipSubsStateChangeReason eReason) {
  SipChannel * sc = (SipChannel * )hAppSubs;
  if(sc) {
    CallStateNotifier* notifier = NULL;
    RVSIPLOCK(sc->mLocker);
    switch(eState){
    case RVSIP_SUBS_STATE_SUBS_SENT:
      notifier = sc->mCallStateNotifier;
      if(notifier)
          notifier->nonInviteSessionInitiated();
      break;
    case RVSIP_SUBS_STATE_UNAUTHENTICATED:
      RvSipSubsSubscribe(hSubs);
      break;
    case RVSIP_SUBS_STATE_UNSUBSCRIBING:
      notifier = sc->mCallStateNotifier;
      if(notifier)
          notifier->nonInviteSessionInitiated();
      break;
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_RCVD:
      sc->subsUnsubscribed(hSubs);
      break;
    case RVSIP_SUBS_STATE_UNSUBSCRIBE_2XX_RCVD:
      notifier = sc->mCallStateNotifier;
      if(notifier)
          notifier->nonInviteSessionSuccessful();
      sc->subsUnsubscribed(hSubs);
      break;
    case RVSIP_SUBS_STATE_TERMINATED:
      sc->subsTerminated(hSubs);
      break;
    case RVSIP_SUBS_STATE_2XX_RCVD:
      notifier = sc->mCallStateNotifier;
      if(notifier)
          notifier->nonInviteSessionSuccessful();
      //TODO
      break;
    case RVSIP_SUBS_STATE_ACTIVE:
    case RVSIP_SUBS_STATE_PENDING:
      //TODO
      break;
    case RVSIP_SUBS_STATE_MSG_SEND_FAILURE:
    {
      notifier = sc->mCallStateNotifier;

      if(notifier){
          bool transport = false;
          bool timeout = false;

          switch(eReason) {
              case RVSIP_SUBS_REASON_TRANSC_TIME_OUT:
                  timeout = true;
                  break;
              case RVSIP_SUBS_REASON_NETWORK_ERROR:
                  transport = true;
                  break;
              default:
                  ;
                  break;
          }

          if(transport || timeout)
              notifier->nonInviteSessionFailed(transport, timeout);
      }
      
      break;
    }
    default:
      break;
    } // switch
  }
}

void SipEngine::appSubsExpirationAlertEv(IN  RvSipSubsHandle            hSubs,
                                         IN  RvSipAppSubsHandle         hAppSubs){


  SipChannel * sc = (SipChannel * )hAppSubs;
  if(sc) {
    RVSIPLOCK(sc->mLocker);
    RvSipSubsRefresh(hSubs,sc->getImsSubsExpires());
  }
}

void SipEngine::appSubsNotifyEvHandler( 
                                     IN  RvSipSubsHandle        hSubs,
                                     IN  RvSipAppSubsHandle     hAppSubs,
                                     IN  RvSipNotifyHandle      hNotification,
                                     IN  RvSipAppNotifyHandle   hAppNotification,
                                     IN  RvSipSubsNotifyStatus  eNotifyStatus,
                                     IN  RvSipSubsNotifyReason  eNotifyReason,
                                     IN  RvSipMsgHandle         hNotifyMsg){
  SipChannel * sc = (SipChannel * )hAppSubs;
  if(sc) {
    RVSIPLOCK(sc->mLocker);
    switch(eNotifyStatus){
    case RVSIP_SUBS_NOTIFY_STATUS_REQUEST_RCVD:
      if(sc->isRegSubs(hSubs)) {
	//TODO update reg status.
      }
      RvSipNotifyAccept(hNotification);
      break;
    default:
      break;
    }  
  } else {
    RvSipNotifyReject(hNotification,404);
  }
}

void SipEngine::appRetransmitEvHandler(IN RvSipMethodType method,
                                       IN void *owner)
{
   SipChannel *sc = NULL;

   RvSipCallLegHandle hCallLeg = (RvSipCallLegHandle)owner;
   RvSipCallLegGetAppHandle(hCallLeg, (RvSipAppCallLegHandle*)&sc);

   if(sc && !sc->isFree()) {

      RVSIPLOCK(sc->mLocker);

      if(sc->mHCallLeg == hCallLeg) {
         CallStateNotifier* notifier = NULL;

         switch(method) {
            case RVSIP_METHOD_INVITE:
               notifier = sc->mCallStateNotifier;
               if(notifier)
                   notifier->inviteSent(sc->getInitialInviteSent());
               break;
            case RVSIP_METHOD_BYE:
               notifier = sc->mCallStateNotifier;
               if(notifier)
                   notifier->byeSent(sc->getInitialByeSent(), sc->getAckStartTime());
               break;
            default:
               break;
         }
      }
   }
}

void SipEngine::appRetransmitRcvdEvHandler(IN RvSipMsgHandle hMsg,
                                           IN void *owner)
{
   SipChannel *sc = NULL;

   RvSipCallLegHandle hCallLeg = (RvSipCallLegHandle)owner;
   RvSipCallLegGetAppHandle(hCallLeg, (RvSipAppCallLegHandle*)&sc);

   if(sc && !sc->isFree()) {

      RVSIPLOCK(sc->mLocker);

      if(sc->mHCallLeg == hCallLeg) {
         CallStateNotifier* notifier = NULL;
         RvSipMethodType msgType = RvSipMsgGetRequestMethod(hMsg);

         switch(msgType) {
            case RVSIP_METHOD_INVITE: {
               RvSipOtherHeaderHandle hOtherHeader = NULL;
               RvSipHeaderListElemHandle hListElem = NULL;
               char buff[4];
               size_t length = 0;
               /* initialize as 70, since it is the default value.*/
               size_t maxForwards = 70;   
               
               if(NULL != (hOtherHeader = RvSipMsgGetHeaderByName(hMsg, "Max-Forwards", RVSIP_FIRST_HEADER, &hListElem)))
               { 
                  RvSipOtherHeaderGetValue(hOtherHeader, buff, sizeof(buff), &length);
                  notifier = sc->mCallStateNotifier;
                  if(notifier){
                      maxForwards = atoi(buff);    
                      notifier->inviteReceived(maxForwards);
                  }
               }
               break;
            }
            case RVSIP_METHOD_BYE: {
               notifier = sc->mCallStateNotifier;
               if(notifier)
                   notifier->byeReceived();
               break;
            }
            default:
               break;
         }
      }
   }
}




void RVCALLCONV SipStackLogPrintFunction(IN void* context,
					 IN RvSipLogFilters filter,
					 IN const RvChar *formattedText)
{
  RVSIPPRINTF("[SIP STACK] %s\n", formattedText);
}

//=============================================================================

//return number of really added servers, or -1 in the case of error
int SipEngine::setDNSServers(const std::vector<std::string> &dnsServers,uint16_t vdevblock,bool add) {

  RVSIPLOCK(mLocker);

  if(!mDnsAltered) add=false;

  RvSipResolverMgrHandle  hResolverMgr=0;

  RvStatus rv = RvSipStackGetResolverMgrHandle(mHSipStack,&hResolverMgr);

  if(rv<0 || !hResolverMgr) {
    return -1;
  }

  RvSipTransportAddr         dnsServersAddrs[MAX_NUM_OF_DNS_SERVERS];
  RvSipTransportAddr         newDnsServersAddrs[MAX_NUM_OF_DNS_SERVERS];
  RvInt32 listSize=MAX_NUM_OF_DNS_SERVERS;
  memset(&dnsServersAddrs,0,sizeof(dnsServersAddrs));

  rv = RvSipResolverMgrGetDnsServers(hResolverMgr,dnsServersAddrs,&listSize,vdevblock);

  if(rv<0) {
    return -1;
  }

  size_t i=0;

  RvInt32 newListSize=0;

  RvInt32 newListMaxSize=MAX_NUM_OF_DNS_SERVERS;
  if(add) newListMaxSize-=listSize;

  while(i<dnsServers.size() && newListSize<newListMaxSize) {

    std::string ip = dnsServers[i];

    SipChannel::fixIpv6Address(ip);

    if(!ip.empty()) {

      bool isNew=true;
      
      if(add) {
	int j=0;
	for(j=0;j<listSize;j++) {
	  RVSIPPRINTF("%s: server[%d]=%s, tt=%d, at=%d, port=%d\n",__FUNCTION__,j,dnsServersAddrs[j].strIP,
		      (int)dnsServersAddrs[j].eTransportType,
		      (int)dnsServersAddrs[j].eAddrType, 
		      (int)dnsServersAddrs[j].port);
	  if(ip == std::string(dnsServersAddrs[j].strIP)) {
	    isNew=false;
	    break;
	  }
	}
      }
      
      if(isNew) {
	
	newDnsServersAddrs[newListSize].eTransportType = RVSIP_TRANSPORT_UNDEFINED; 
	if(!strstr(ip.c_str(),":")) newDnsServersAddrs[newListSize].eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP;
	else newDnsServersAddrs[newListSize].eAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_IP6;
	newDnsServersAddrs[newListSize].port=0; //default port is used
	strncpy(newDnsServersAddrs[newListSize].strIP,ip.c_str(),sizeof(newDnsServersAddrs[newListSize].strIP)-1);
	newDnsServersAddrs[newListSize].Ipv6Scope=0;
	
	newListSize++;
      }
    }

    i++;
  }

  if(newListSize>0) {

    if(add) {
      int i=0;
      for(i=0; i<listSize; i++) {
	void * p1 = &(newDnsServersAddrs[i+newListSize]);
	void * p2 = &(dnsServersAddrs[i]);
	memcpy(p1,p2,sizeof(RvSipTransportAddr));
      }
      newListSize++;
    }

    rv = RvSipResolverMgrSetDnsServers(hResolverMgr,newDnsServersAddrs,newListSize,vdevblock);

    if(rv<0) {
      return -1;
    }
    rv = RvSipResolverMgrClearDnsEngineCache(hResolverMgr,vdevblock);

    if(rv<0) {
      return -1;
    }

    mDnsAltered=true;
    
  }
  DnsCfgIterator it = mDnsCfgs.find(vdevblock);

  if(it != mDnsCfgs.end()){
      it->second.numofservers(newListSize);
      if(add){
          if(it->second.timeout() != -1 || it->second.tries() != -1)
              setDNSTimeout(it->second.timeout(),it->second.tries()*newListSize,vdevblock);
      }
  }else{
      DnsServerCfg cfg(-1,-1,newListSize);
      mDnsCfgs.insert(std::make_pair(vdevblock,cfg));
  }

  return newListSize;
}

RvStatus SipEngine::setDNSTimeout(int timeout, int tries, uint16_t vdevblock) {

  RVSIPLOCK(mLocker);

  RvSipResolverMgrHandle  hResolverMgr=0;

  RvStatus rv = RvSipStackGetResolverMgrHandle(mHSipStack,&hResolverMgr);

  if(rv<0 || !hResolverMgr) {
    return -1;
  }

  int numservers=-1;
  DnsCfgIterator it = mDnsCfgs.find(vdevblock);
  if(it != mDnsCfgs.end()){
      numservers = it->second.numofservers();
      it->second.tries(tries);
      it->second.timeout(timeout);
  }else{
      DnsServerCfg cfg(timeout,tries,-1);
      mDnsCfgs.insert(std::make_pair(vdevblock,cfg));
  }

  return RvSipResolverMgrSetTimeout(hResolverMgr,(RvInt)timeout,(RvInt)tries*numservers,(RvUint16)vdevblock);
}

//=============================================================================

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

RvStatus SipEngine::initSipStack(int numOfThreads)
{
  RvSipMidCfg midCfg;

  {
    struct rlimit r;
    unsigned int maxFDs = mNumChannels*5;
  
    if(getrlimit(RLIMIT_NOFILE,&r) < 0) {
      RVSIPPRINTF("%s: cannot set rlimit\n",__FUNCTION__);
    } else {
      if(r.rlim_cur < maxFDs)
	r.rlim_cur = maxFDs;
      if(r.rlim_max < maxFDs)
	r.rlim_max = maxFDs;
      if(setrlimit(RLIMIT_NOFILE,&r) < 0) {
	RVSIPPRINTF("%s: cannot set rlimit\n",__FUNCTION__);
      } else {
	RVSIPPRINTF("%s: rlimit set to %d / %d\n",__FUNCTION__,(int)(mNumChannels*5),(int)(mNumChannels*5));
      }
    }
  }

  RvStatus rv = RvSipMidInit();
  if (rv<0) {
    RVSIPPRINTF("%s: cannot init mid\n",__FUNCTION__);
    return rv;
  }
  
  /* initiate the mid layer before initiating the stack */
  midCfg.hLog = NULL;
  midCfg.maxUserFd = 4096;
  midCfg.maxUserTimers = mNumChannels * 9 + LINGER_EXTRA(mNumChannels) * 9 + 10; // Each SIP alias needs a timer. Each timer may take 64 bytes.
  rv = RvSipMidConstruct(sizeof(midCfg),&midCfg,&mHMidMgr);
  if (rv<0) {
    RVSIPPRINTF("%s: cannot construct mid\n",__FUNCTION__);
    return rv;
  }

  RvUint32 maxFds=0;
  RvSipMidSelectGetMaxDesc(&maxFds);
  RVSIPPRINTF("%s: maxFds=%lu, FD_SETSIZE=%d\n",__FUNCTION__,(long unsigned int)maxFds,(int)FD_SETSIZE);

  RvSipStackCfg stackCfg;
  
  /*Initialize the configuration structure with default values*/
  RvSipStackInitCfg(sizeof(stackCfg), &stackCfg);

  stackCfg.numberOfProcessingThreads = numOfThreads;

  stackCfg.localUdpAddress[0]=0;
  stackCfg.localTcpAddress[0]=0;
  stackCfg.bIgnoreLocalAddresses = RV_TRUE;
  
  stackCfg.tcpEnabled = RV_TRUE;
  
  stackCfg.maxCallLegs           = mNumChannels + LINGER_EXTRA(mNumChannels) + 128;
  stackCfg.maxTransactions       = mNumChannels + LINGER_EXTRA(mNumChannels) + 128;
  stackCfg.maxTransmitters       = mNumChannels + LINGER_EXTRA(mNumChannels) + 128;

  stackCfg.maxRegClients        = mNumChannels + 2048;
  stackCfg.maxPubClients        = 128; //we do not support publish, yet; mNumChannels + LINGER_EXTRA(mNumChannels) + 128;
  
  //stackCfg.messagePoolNumofPages = mNumChannels * 3 * 3 * 2 + LINGER_EXTRA(mNumChannels) * 2;
  //stackCfg.messagePoolPageSize   = 1536;
  //stackCfg.generalPoolNumofPages = mNumChannels * 12 + 20 + LINGER_EXTRA(mNumChannels) * 2;
  //stackCfg.generalPoolPageSize   = 1024;

  stackCfg.maxNumOfLocalAddresses  = mNumChannels * 2 + 1024;
  stackCfg.bDLAEnabled = RV_TRUE;

  stackCfg.maxSubscriptions      = mNumChannels + LINGER_EXTRA(mNumChannels) + 128;

  //dfb: comment the following settings to use RV default setting.

  stackCfg.sendReceiveBufferSize = 4096;
#if 0
  stackCfg.numOfReadBuffers = numOfThreads+4;
  stackCfg.processingQueueSize = mNumChannels * 8 + 10;
#endif
  
  stackCfg.bOldInviteHandling    = RV_FALSE;
  stackCfg.manualSessionTimer    = RV_FALSE;
  stackCfg.bDynamicInviteHandling = RV_TRUE;
  stackCfg.bDisableMerging = RV_FALSE;
  stackCfg.manualPrack = RV_FALSE;
  stackCfg.bEnableForking = RV_FALSE;
  stackCfg.manualAckOn2xx = RV_TRUE;
  stackCfg.manualBehavior = RV_FALSE;
  stackCfg.enableInviteProceedingTimeoutState = RV_TRUE;
  stackCfg.bUseRportParamInVia = RV_TRUE;
  stackCfg.maxDnsServers = MAX_NUM_OF_DNS_SERVERS;
  stackCfg.maxDnsDomains = MAX_NUM_OF_DNS_DOMAINS;

  stackCfg.bResolveTelUrls = RV_FALSE;
  
  stackCfg.maxSecAgrees       = 0;
  stackCfg.maxSecurityObjects = 0;
  stackCfg.maxIpsecSessions   = 0;
  
  stackCfg.eOutboundProxyCompression = RVSIP_COMP_UNDEFINED;
  stackCfg.maxCompartments           = 1;
  
  /* Set timers high to avoid retransimissions */
  stackCfg.retransmissionT1  = 500; //RFC3261
  stackCfg.retransmissionT2  = 4000; //RFC3261
  stackCfg.retransmissionT4  = 5000; //RFC3261
  stackCfg.provisionalTimer = 180000; // 0 means no time limit on pending invite when provisional response received
  
  stackCfg.generalLingerTimer    = LINGER_SEC * 1000;

  stackCfg.supportedExtensionList = const_cast<RvChar*>(SIP_UDP_SUPPORTED);
  
  stackCfg.pfnPrintLogEntryEvHandler = SipStackLogPrintFunction;
  stackCfg.defaultLogFilters = RVSIP_LOG_EXCEP_FILTER | RVSIP_LOG_ERROR_FILTER;
  stackCfg.ePersistencyLevel = RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER;
  stackCfg.connectionCapacityPercent = 100;

  stackCfg.pfnRetransmitEvHandler     = AppRetransmitEvHandler;
  stackCfg.pfnRetransmitRcvdEvHandler = AppRetransmitRcvdEvHandler;

  
  /* Call the stack initialization function */
  rv = RvSipStackConstruct(sizeof(stackCfg), &stackCfg, &mHSipStack);
  if(rv<0) {
    RVSIPPRINTF("?RvSipStackConstruct?\n");
  }
  
  return rv;
}

//=============================================================================
void SipEngine::destroySipStack()
{
  if(mHSipStack) {
    RvSipStackDestruct(mHSipStack);
    mHSipStack=0;
  }
  if(mHMidMgr) {
    RvSipMidDestruct(mHMidMgr);
    mHMidMgr=0;
  }
}

//=============================================================================
RvStatus SipEngine::initSdpStack()
{
  return RvSdpMgrConstruct();
}

//=============================================================================
void SipEngine::destroySdpStack()
{
  RvSdpMgrDestruct();
}

//=============================================================================

void SipEngine::destroy() {
  if(mInstance) {
    delete mInstance;
    mInstance=0; 
  }
}

void SipEngine::setlogmask( char *src,char *filter){

    SipEngine::LogSrcType::iterator it_src;
    SipEngine::LogFilterType::iterator it_filter;

    it_src = mLogModuleTypes.find(std::string(src));
    it_filter = mLogFilterTypes.find(std::string(filter));

    if(it_src != mLogModuleTypes.end() && it_filter != mLogFilterTypes.end()){
        RvSipStackSetNewLogFilters(mHSipStack,static_cast<RvSipStackModule>(it_src->second),it_filter->second);
    }
}

void SipEngine::setlogmask( void) {
    std::ifstream fin;
    char conf[256];

    fin.open("log.conf");

    if(fin.is_open()){
        while (!fin.eof()) {
            fin.getline(conf,256); 
            if (conf[0] == '#' || strlen(conf) == 0) 
                continue;
            else{
                //printf("conf:%s\n",conf);
                char *p = NULL;
                char *q = NULL;

                p = conf;
                while(*p == ' ' || *p == '\t'){
                    p++;
                }
                q = strchr(p,'=');
                if(q){
                    *q = '\0';
                    q++;
                    setlogmask(p,q);
                }
            }
        }
    }
    fin.close(); 
}

RvStatus SipEngine::removeContactHeader(RvSipMsgHandle hMsg) {

  RvStatus rv=RV_OK;

  while(true) {

    RvSipHeaderListElemHandle   hListElem = (RvSipHeaderListElemHandle)0;
    RvSipContactHeaderHandle hContactHeader = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
												RVSIP_HEADERTYPE_CONTACT,
												RVSIP_FIRST_HEADER,
												&hListElem);

    if(!hContactHeader) break;

    rv = RvSipMsgRemoveHeaderAt(hMsg,hListElem);

    if(rv<0) break;
  }

  return rv;
}

SipCallSM& SipEngine::getCSM(SipChannel* sc) const {
  ChannelFunctionalType cft=CFT_BASIC;
  if(sc) {
    cft=sc->getSMType();
    if(cft<0 || cft>=CFT_TOTAL) cft=CFT_BASIC;
  }
  return *(mStateMachines[cft]);
}
