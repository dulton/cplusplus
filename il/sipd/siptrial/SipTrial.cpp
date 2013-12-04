/// @file
/// @brief SIP core package functionality testing application.
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

/*
 * SIP Trial application.
 * 
 * DESCRIPTION:
 * 
 * In the trial application we are just measuring CPS: SIP calls per seconds.
 * One call is a sequence of:
 *    INVITE --->
 *           <--- 200 OK
 *       ACK --->
 *       BYE --->
 *           <--- OK
 * 
 * The following options can be specified:
 * 
 * 1.  Number of sessions
 * 2.  How to build the address per session:
 *     3.1 - Single IP address & Single port
 *
 * the command line interface is:
 * 
 * # siptrial -n channels -A address -P port -R remhost -r remIP -t time -T [TCP|UDP|UNDEF] -s dns-ip
 * 
 *   where:
 * 
 * 		-n: - number of simultaneous SIP channels. Default is 2.
 * 		-N: - number of calls.
 *              -t: - call time.
 * 		-A: - base IP address. Default is SipEngine::SIP_DEFAULT_ADDRESS.
 * 		-I: - base interface name. Default is eth0.
 * 		-R: - remote host. Default none.
 * 		-r: - remote IP. Default none.
 * 		-T: - transport. Default UNDEF.
 * 		-P: - base port. Default SipEngine::SIP_DEFAULT_PORT.
 *              -c: - call termination type: o - originator, t - terminator, g - OTOT.
 *              -d: - SIP remote domain.
 *              -j: - SIP remote domain port.
 *              -l: - SIP local domain.
 *              -g: - SIP registration domain.
 *              -X: - SIP Registrar port.
 *              -m: - SIP auth realm.
 *              -p: - SIP Proxy ip.
 *              -x: - SIP Proxy port.
 *              -s: - DNS Server IP.
 *              -a: - AKA auth.
 *              -b: - Mobile.
 *              -u: - local user name.
 *              -U: - remote user name.
 *              -w: - password
 *              -f: - configure file.
 *              -j: - proxy name.
 *              -k: - proxy domain.
 *              -q: - registrar name.
 *              -S: - registrar IP.
 *              -C: - enable subscripton on reg.
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>

#include <sched.h>

#include "md5.h"

#include "SipEngine.h"

static volatile int flag_exit = 0;

USING_RV_SIP_ENGINE_NAMESPACE;

class MD5ProcessorSipTrial : public MD5Processor {
 public:
  MD5ProcessorSipTrial() {}
  virtual ~MD5ProcessorSipTrial() {}

  virtual void init(state_t *pms) const {
    md5_init((md5_state_t*)pms);
  }
  virtual void update(state_t *pms, const unsigned char *data, int nbytes) const {
    md5_append((md5_state_t*)pms,(const md5_byte_t*)data,nbytes);
  }

  virtual void finish(state_t *pms, unsigned char digest[16]) const {
    md5_finish((md5_state_t*)pms,(md5_byte_t*)digest);
  }
};

class SipTrialMediaInterface : public MediaInterface {
 public:
  SipTrialMediaInterface() {}
  virtual ~SipTrialMediaInterface() {}

  virtual RvStatus media_onConnected(IN  RvSipCallLegHandle            hCallLeg, RvUint16 &code) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_onCallExpired(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
    hold=false;
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_onDisconnecting(IN  RvSipCallLegHandle            hCallLeg) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }
      
  virtual RvStatus media_onDisconnected(IN  RvSipCallLegHandle            hCallLeg) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_stop() {
    return 0;
  }

  virtual RvStatus media_beforeConnect(IN  RvSipCallLegHandle            hCallLeg) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_onOffering(IN  RvSipCallLegHandle            hCallLeg, RvUint16& code, bool bMandatory=true) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_beforeAccept(IN  RvSipCallLegHandle            hCallLeg) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }


  virtual RvStatus media_beforeDisconnect(IN  RvSipCallLegHandle            hCallLeg) {
    printf("%s\n",__FUNCTION__);
    return 0;
  }

  virtual RvStatus media_onByeReceived(IN  RvSipCallLegHandle            hCallLeg, bool &hold) {
    hold=false;
    printf("%s\n",__FUNCTION__);
    return 0;
  }
};

//=============================================================================
// forward function declarations
//=============================================================================

/////////////////////// ex refs/////////////////////////
extern "C" void rtpprintf(const char* format,...) {
  ;
}

//=============================================================================
// local types
//=============================================================================

typedef struct __TestOptions
{
public:
  /* get property */
  int       getNumChannels()  { return m_numChannels; }
  int       getCallTime() { return m_callTime; }
  int       getNumCalls()     { return m_numCalls; }
  const char* getBaseIpAddr()   { return m_baseIpAddr; }
  const char* getBaseInterfaceName() {
    return m_ifName;
  }
  const char* getRemoteHostName() {
    return mRemoteHostName;
  }
  const char* getRemoteIP() {
    return mRemoteIP;
  }
  const char* getDNSIP() {
    return mDNSIP;
  }
  const char* getProxy() {
    return mProxyIP;
  }
  const char* getProxyIP() {
    return mProxyIP;
  }
  const char* getProxyName() {
    return mProxyName;
  }
  const char* getProxyDomain() {
    return mProxyDomain;
  }
  const char* getRemoteDomain() {
    return mRemoteDomain;
  }
  const char* getLocalDomain() {
    return mLocalDomain;
  }
  const char* getRealm() {
    return mRealm;
  }
  bool getAka() {
    return mAka;
  }
  bool getSubscription() {
    return mSubOnReg;
  }
  bool getMobile() {
    return mMobile;
  }
  std::string getLocalUserName() {
    return mLocalUserName;
  }
  std::string getRemoteUserName() {
    return mRemoteUserName;
  }
  std::string getPassword() {
    return mPassword;
  }
  void setLocalUserName(const char *n) {
    mLocalUserName = n;
  }
  void setRemoteUserName(const char *n) {
    mRemoteUserName = n;
  }
  void setPassword(const char *n) {
    mPassword = n;
  }
  RvSipTransport getSipTransport() const {
    return mTransport;
  }
  bool getRegistrationOnly() {
      return mregistration_only;
  }
  bool getDebug(){
      return mDebug;
  }
  unsigned short     getBaseIpPort()   { return m_baseIpPort; }
  unsigned short     getProxyPort()   { return m_ProxyPort; }
  unsigned short     getRegistrarPort()   { return mRegistrarPort; }
  const char*        getRegName()   { return mRegServerName; }
  const char*        getRegDomain() { return mRegDomain; }
  const char*        getRegIP()   { return mRegServerIP; }



  bool isAllOriginate() { return mOriginate;}
  bool isAllTerminate() { return mTerminate;}
  bool UseIMSI() {return mUseIMSI;}
  
  /* set property */
  void setNumChannels(int numChannels)   { m_numChannels = numChannels; }
  void setCallTime(int callTime)   { m_callTime = callTime; }
  void setNumCalls(int numCalls)         { m_numCalls = numCalls; }
  void setBaseIpAddr(const char* baseIpAddr) { strcpy(m_baseIpAddr,baseIpAddr); }
  void setBaseInterfaceName(const char* ifName) { if(ifName) strncpy(m_ifName,ifName,sizeof(m_ifName)-1); }
  void setRemoteHostName(const char* hostName) { if(hostName) strncpy(mRemoteHostName,hostName,sizeof(mRemoteHostName)-1); }
  void setRemoteIP(const char* ip) { if(ip) strncpy(mRemoteIP,ip,sizeof(mRemoteIP)-1); }
  void setDNSIP(const char* ip) { if(ip) strncpy(mDNSIP,ip,sizeof(mDNSIP)-1); }
  void setProxy(const char* proxy) { if(proxy) strncpy(mProxyIP,proxy,sizeof(mProxyIP)-1); }
  void setProxyName(const char* proxyName) { if(proxyName) strncpy(mProxyName,proxyName,sizeof(mProxyName)-1); }
  void setProxyDomain(const char* proxyDomain) { if(proxyDomain) strncpy(mProxyDomain,proxyDomain,sizeof(mProxyDomain)-1); }
  void setRemoteDomain(const char* domain) { if(domain) strncpy(mRemoteDomain,domain,sizeof(mRemoteDomain)-1); }
  void setRemoteDomainPort(unsigned short port) { mRemoteDomainPort = port; }
  unsigned short getRemoteDomainPort() { return mRemoteDomainPort; }
  void setLocalDomain(const char* domain) { if(domain) strncpy(mLocalDomain,domain,sizeof(mLocalDomain)-1); }
  void setRegDomain(const char* domain) { if(domain) strncpy(mRegDomain,domain,sizeof(mRegDomain)-1); }
  void setRegName(const char* domain) { if(domain) strncpy(mRegServerName,domain,sizeof(mRegServerName)-1); }
  void setRegIP(const char* domain) { if(domain) strncpy(mRegServerIP,domain,sizeof(mRegServerIP)-1); }
  void setRealm(const char* realm) { if(realm) strncpy(mRealm,realm,sizeof(mRealm)-1); }
  void setAka(bool value) { mAka=value; }
  void setSubscription(bool value) { mSubOnReg=value; }
  void setSubscriptionStr(const char *value){if(!strcmp(value,"true") || !strcmp(value,"TRUE")) setSubscription(true);else setSubscription(false);}
  void setAkaStr(const char *value){if(!strcmp(value,"true") || !strcmp(value,"TRUE")) setAka(true);else setAka(false);}
  void setMobile(bool value) { mMobile=value; }
  void setMobileStr(const char *value){if(!strcmp(value,"true") || !strcmp(value,"TRUE")) setMobile(true);else setMobile(false);}
  void setSipTransport(RvSipTransport tr) { mTransport=tr; }
  void setBaseIpPort(unsigned short baseIpPort)   { m_baseIpPort = baseIpPort; }
  void setBaseIpPortStr(const char *baseIpPortStr)   { setBaseIpPort(atoi(baseIpPortStr)); }
  void setProxyPort(unsigned short pPort)   { m_ProxyPort = pPort; }
  void setProxyPortStr(const char *pPortStr)   { setProxyPort(atoi(pPortStr)); }
  void setRegistrarPort(unsigned short pPort)   { mRegistrarPort = pPort; }
  void setRegistrarPortStr(const char *pPortStr)   { setRegistrarPort(atoi(pPortStr)); }

  void setAllOriginate() { mOriginate=true; }
  void setAllTerminate() { mTerminate=true;}
  void setRegistrationOnly(bool rr) { mregistration_only = rr;}
  void setDebug(bool d) { mDebug = d;}
  void setIMSI(bool i) {mUseIMSI = i;}
  void setIMSIStr(const char *value){if(!strcmp(value,"true") || !strcmp(value,"TRUE")) setIMSI(true);else setIMSI(false);}

private:
  int 			m_numChannels;
  int 			m_callTime;
  int			m_numCalls;
  char		        m_baseIpAddr[129];
  char                  m_ifName[65];
  char                  mRemoteHostName[65];
  char                  mRemoteIP[65];
  char                  mDNSIP[65];
  char                  mRemoteDomain[129];
  unsigned short        mRemoteDomainPort;
  char                  mLocalDomain[129];

  //registrar server 
  char                  mRegServerName[129];
  char                  mRegServerIP[65];
  char                  mRegDomain[129];
  unsigned short        mRegistrarPort;


  //proxy
  char                  mProxyName[129];
  char                  mProxyIP[65];
  char                  mProxyDomain[129];
  unsigned short        m_ProxyPort;

  char                  mRealm[129];
  unsigned short        m_baseIpPort;
  RvSipTransport        mTransport;
  bool                  mOriginate;
  bool                  mTerminate;
  bool                  mregistration_only;
  bool                  mDebug;
  bool                  mAka;
  bool                  mSubOnReg;
  bool                  mMobile;
  bool                  mUseIMSI;
  std::string           mLocalUserName;
  std::string           mRemoteUserName;
  std::string           mPassword;
  
} TestOptions;

//=============================================================================
// local data
//=============================================================================

static TestOptions topt;
typedef void (TestOptions::*opt_set_func)(const char *);
typedef std::map<std::string,opt_set_func> confParser;
confParser ConfParser;

void initConfParser(){
      ConfParser.clear();
      ConfParser.insert(std::make_pair("INTERFACE",reinterpret_cast<opt_set_func>(&TestOptions::setBaseInterfaceName)));
      ConfParser.insert(std::make_pair("LOCALIP",reinterpret_cast<opt_set_func>(&TestOptions::setBaseIpAddr)));
      ConfParser.insert(std::make_pair("PROXYNAME",reinterpret_cast<opt_set_func>(&TestOptions::setProxyName)));
      ConfParser.insert(std::make_pair("PROXYIP",reinterpret_cast<opt_set_func>(&TestOptions::setProxy)));
      ConfParser.insert(std::make_pair("PROXYDOMAIN",reinterpret_cast<opt_set_func>(&TestOptions::setProxyDomain)));
      ConfParser.insert(std::make_pair("PROXYPORT",reinterpret_cast<opt_set_func>(&TestOptions::setProxyPortStr)));

      ConfParser.insert(std::make_pair("REGNAME",reinterpret_cast<opt_set_func>(&TestOptions::setRegName)));
      ConfParser.insert(std::make_pair("REGIP",reinterpret_cast<opt_set_func>(&TestOptions::setRegIP)));
      ConfParser.insert(std::make_pair("REGDOMAIN",reinterpret_cast<opt_set_func>(&TestOptions::setRegDomain)));
      ConfParser.insert(std::make_pair("REGPORT",reinterpret_cast<opt_set_func>(&TestOptions::setRegistrarPortStr)));

      ConfParser.insert(std::make_pair("DNSSERVER",reinterpret_cast<opt_set_func>(&TestOptions::setDNSIP)));

      ConfParser.insert(std::make_pair("AKA",reinterpret_cast<opt_set_func>(&TestOptions::setAkaStr)));
      ConfParser.insert(std::make_pair("USERNAME",reinterpret_cast<opt_set_func>(&TestOptions::setLocalUserName)));
      ConfParser.insert(std::make_pair("PASSWORD",reinterpret_cast<opt_set_func>(&TestOptions::setPassword)));
      ConfParser.insert(std::make_pair("REALM",reinterpret_cast<opt_set_func>(&TestOptions::setRealm)));
      ConfParser.insert(std::make_pair("MOBILE",reinterpret_cast<opt_set_func>(&TestOptions::setMobileStr)));
      ConfParser.insert(std::make_pair("IMSI",reinterpret_cast<opt_set_func>(&TestOptions::setIMSIStr)));
      ConfParser.insert(std::make_pair("REMOTE_DOMAIN",reinterpret_cast<opt_set_func>(&TestOptions::setRemoteDomain)));
      ConfParser.insert(std::make_pair("REMOTE_USERNAME",reinterpret_cast<opt_set_func>(&TestOptions::setRemoteUserName)));
      ConfParser.insert(std::make_pair("SUBSCRIPTION",reinterpret_cast<opt_set_func>(&TestOptions::setSubscriptionStr)));


}
static opt_set_func findConfPaser(char *param){
    if(ConfParser.find(param) != ConfParser.end()) 
        return ConfParser.find(param)->second; 
    return NULL;
}

static void parseConfFile(const char *conf_file){
    std::ifstream fin;
    char conf[256];

    fin.open(conf_file);

    if(fin.is_open()){
        while (!fin.eof()) {
            fin.getline(conf,256);
            if (conf[0] == '#' || strlen(conf) == 0)
                continue;
            else{
                char *p = NULL;
                char *q = NULL;

                p = conf;
                while(*p == ' ' || *p == '\t'){
                    p++;
                }
                q = strchr(p,'=');
                if(q){
                    char *d= NULL;
                    opt_set_func cb = NULL;
                    *q = '\0';
                    q++;
                    d = q;
                    while((*d != ' ') && (*d != '\n') && (*d != '\r') && (*d != '\0')) d++;
                    *d = '\0';
                    if(strlen(q)){
                        cb = findConfPaser(p);
                        if(cb){
                            (topt.*cb)(q);
                        }
                    }
                }
            }
        }
        fin.close();
    }

}

//=============================================================================

//=============================================================================
static void usage()
{
  printf("\n\n"
	 "Usage:  siptrial -n channels -A address -P port -R host -r remIP -t time -T [TCP|UDP|UNDEF] -d rdomain -l ldomain -g regdomain -m realm -s dnsserverip [-a] -c{o|t|g]\n\n"
	 "  Options:\n\n"
	 "-h    This help\n"
	 "-n    Number of simultaneous SIP channels. Default is 2\n"
	 "-N    Number of calls.\n "
	 "-t    Call time.\n "
	 "-A    Base IP address. Default is %s.\n"
	 "-I    Base interface name. Default is eth0.\n"
	 "-R    Remote host. Default none. \n"
	 "-r    Remote IP. Default none. \n"
	 "-T    transport: TCP or UDP or UNDEF. Default UNDEF. \n"
	 "-P    Base port. Default SipEngine::SIP_DEFAULT_PORT\n"
         "-c    call termination type: o - originator, t - terminator, g - OTOT\n"
	 "-d    SIP remote domain\n"
	 "-l    SIP local domain\n"
         "-g    SIP registration domain\n"
         "-X    SIP Registrar port\n"
         "-m    SIP realm\n"
	 "-p    SIP Proxy\n"
         "-x    SIP Proxy port\n"
         "-s    DNS Server IP\n"
         "-a    AKA auth\n"
         "-b    Mobile\n"
	 "-u    local user name\n"
	 "-U    remote user name\n"
	 "-w    reg password\n"
         "-O    registration only\n"
	 "-D    open stack debug\n" 
     "-f    use config file."   
	 "\n",
	 SipEngine::SIP_DEFAULT_ADDRESS
	 );
}

//=============================================================================
static void set_default_test_options(void)
{
  topt.setNumChannels(0);
  topt.setCallTime(1);
  topt.setNumCalls(-1);
  topt.setBaseIpAddr(SipEngine::SIP_DEFAULT_ADDRESS);
  topt.setBaseInterfaceName(SipEngine::SIP_DEFAULT_INTERFACE);
  topt.setRemoteHostName("");
  topt.setRemoteIP("");
  topt.setDNSIP("");
  topt.setProxy("");
  topt.setRemoteDomain("");
  topt.setRemoteDomainPort(0);
  topt.setLocalDomain("");
  topt.setRegDomain("");
  topt.setRegIP("");
  topt.setRegName("");
  topt.setProxyName("");
  topt.setProxyDomain("");
  topt.setRealm("");
  topt.setAka(false);
  topt.setSubscription(false);
  topt.setMobile(false);
  topt.setSipTransport(RVSIP_TRANSPORT_UNDEFINED);
  topt.setBaseIpPort(SipChannel::SIP_DEFAULT_PORT);
  topt.setProxyPort(SipChannel::SIP_DEFAULT_PORT);
  topt.setRegistrarPort(SipChannel::SIP_DEFAULT_PORT);
  topt.setRegistrationOnly(false);
  topt.setDebug(false);
}

//=============================================================================
static void print_test_options(void)
{
  const char* baseipaddr = topt.getBaseIpAddr();
  
  printf("=============== TEST Options ===================\n");	
  printf("%18s: %d\n", "numChannels", topt.getNumChannels());
  printf("%18s: %d\n", "callTime", topt.getCallTime());
  if (topt.getNumCalls() > 0) {
    printf("%18s: %d\n", "numCalls", topt.getNumCalls());
  }
  printf("%18s: transport=%d: %s@%s:%d/%s [%s]\n", "base url", (int)topt.getSipTransport(), "stc_000000", 
	 baseipaddr, topt.getBaseIpPort(), topt.getBaseInterfaceName(), topt.getRemoteHostName());
}

//=============================================================================
static int parse_test_options(int argc, char ** argv, const char * opt_string)
{
  while (1) {
    switch(getopt(argc, argv, opt_string)) {
      
    case 'n':
      {
	int numChannels = atoi(optarg);
	if (topt.getNumChannels() < numChannels)
	  topt.setNumChannels((numChannels>>1)<<1); // even number
	break;
      }
    
    case 't':
      {
	int callTime = atoi(optarg);
	topt.setCallTime(callTime);
	break;
      }
      
    case 'N':
      {
	topt.setNumCalls(atoi(optarg));
	break;
      }
      
    case 'A':
      {
	printf("%s: optarg=%s\n",__FUNCTION__,optarg);
	topt.setBaseIpAddr(optarg);
	if(strcmp(optarg,"127.0.0.1")==0) topt.setBaseInterfaceName(SipEngine::SIP_DEFAULT_LOOPBACK_INTERFACE);
	if(strcmp(optarg,"::1")==0) topt.setBaseInterfaceName(SipEngine::SIP_DEFAULT_LOOPBACK_INTERFACE);
	break;
      }
      
    case 'I':
      {
	printf("%s: optarg=%s\n",__FUNCTION__,optarg);
	topt.setBaseInterfaceName(optarg);
	break;
      }
      
    case 'R':
      {
	printf("%s: optarg=%s\n",__FUNCTION__,optarg);
	topt.setRemoteHostName(optarg);
	break;
      }
      
    case 'r':
      {
	printf("%s: optarg=%s\n",__FUNCTION__,optarg);
	topt.setRemoteIP(optarg);
	break;
      }

    case 's':
      {
	printf("%s: optarg=%s\n",__FUNCTION__,optarg);
	topt.setDNSIP(optarg);
	break;
      }
      
    case 'P':
      {
	unsigned short baseIpPort = (unsigned short) atoi(optarg);
	topt.setBaseIpPort(baseIpPort);
	break;
      }
      
    case 'x':
      {
	unsigned short pPort = (unsigned short) atoi(optarg);
	topt.setProxyPort(pPort);
	break;
      }
    case 'X':
      {
	unsigned short pPort = (unsigned short) atoi(optarg);
	topt.setRegistrarPort(pPort);
	break;
      }
    case 'O':
      topt.setRegistrationOnly(true);
      break;

    case 'D':
      topt.setDebug(true);
      break;
      
    case 'T':
      {
	RvSipTransport value=RVSIP_TRANSPORT_UNDEFINED;
	if(!strcmp(optarg,"TCP")) value=RVSIP_TRANSPORT_TCP;
	else if(!strcmp(optarg,"UDP")) value=RVSIP_TRANSPORT_UDP;
	topt.setSipTransport(value);
	break;
      }
      
    case 'c':
      if(optarg[0]=='t') topt.setAllTerminate();
      else if(optarg[0]=='o') topt.setAllOriginate();
      break;

    case 'd':
      topt.setRemoteDomain(optarg);
      break;

    case 'j':
      topt.setRemoteDomainPort(atoi(optarg));
      break;

    case 'l':
      topt.setLocalDomain(optarg);
      break;

    case 'g':
      topt.setRegDomain(optarg);
      break;

    case 'm':
      topt.setRealm(optarg);
      break;

    case 'a':
      topt.setAka(true);
      break;
    case 'C':
      topt.setSubscription(true);
      break;

    case 'b':
      topt.setMobile(true);
      break;

    case 'u':
      topt.setLocalUserName(optarg);
      break;

    case 'U':
      topt.setRemoteUserName(optarg);
      break;

    case 'w':
      topt.setPassword(optarg);
      break;

    case 'p':
      topt.setProxy(optarg);
      break;
    case 'e':
      topt.setIMSI(true);
      break;

    case 'f':
      initConfParser();
      parseConfFile(optarg);
      break;

    case 'k':
      topt.setProxyDomain(optarg);
      break;
    case 'q':
      topt.setRegName(optarg);
      break;
    case 'S':
      topt.setRegIP(optarg);
      break;
      
    case 'h':
      return +1;
      
    case -1:
      return 0;
    }
  }

  return 0;
}

//=============================================================================

class TrialCallStateNotifier : public CallStateNotifier {
public:
  mutable unsigned int counter;
  TrialCallStateNotifier() {
    counter=0;
  }
  virtual ~TrialCallStateNotifier() {}
  virtual void regSuccess() {
    printf("NOTIFIER: %s\n",__FUNCTION__);
    counter++;
  }
  virtual void regFailure(uint32_t regExpires, int regRetries) {
    printf("NOTIFIER: %s: re=%d, rr=%d\n",__FUNCTION__,(int)regExpires,regRetries);
    if(regRetries<1) counter++;
  }
  virtual void inviteCompleted(bool success) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
  virtual void callAnswered(void) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
  virtual void callCompleted(void) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
  virtual void refreshCompleted(void) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
  virtual void responseSent(int msgNum) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
  virtual void responseReceived(int msgNum) {
    printf("NOTIFIER: %s\n",__FUNCTION__);
  }
};

static   TrialCallStateNotifier rn;

static int reg(SipEngine * se, SipChannel **scv, int numChannels)
{
  ENTER();

  int cct = 0;

  if(topt.getProxy()[0]==0 && topt.getRegDomain()[0] == 0) 
      return 0;

  rn.counter=0;

  for (cct = 0; cct < numChannels; cct++) {
    if (scv[cct]->startRegistration() < 0)
      RETCODE(-1);
  }
  
  int numSessions = 0;

  int regTimeLeft = 300000;
  int sleepInterval=10;

  do {

    se->run_immediate_events(1);

    usleep(sleepInterval * 1000);
    regTimeLeft-=sleepInterval;

    if(regTimeLeft % 1000 == 0) {
      printf("Reg Time left %d\n",(int)(regTimeLeft/1000));
    }

  } while (!flag_exit && regTimeLeft > 0 && rn.counter<numChannels);
  
  RETCODE(0);
}

static int unreg(SipEngine * se, SipChannel **scv, int numChannels)
{
  ENTER();

  int cct = 0;

  if(topt.getProxy()[0]==0 && topt.getRegDomain()[0] == 0) 
      return 0;

  rn.counter=0;

  for (cct = 0; cct < numChannels; cct++) {
    if (scv[cct]->startDeregistration() < 0)
      RETCODE(-1);
  }

  int numSessions = 0;

  int regTimeLeft = 30000;
  int sleepInterval=10;

  do {

    se->run_immediate_events(1);

    usleep(sleepInterval * 1000);
    regTimeLeft-=sleepInterval;

    if(regTimeLeft % 1000 == 0) {
      printf("Unreg Time left %d: c=%d\n",(int)(regTimeLeft/1000),(int)rn.counter);
    }

  } while (regTimeLeft > 0 && rn.counter<numChannels);
  
  RETCODE(0);
}

static int run(SipEngine * se, SipChannel **scv, int numChannels, int callTime)
{
  ENTER();

  int cct = 0;

  for (cct = 0; cct < numChannels; cct++) {
    eRole role = (cct%2==0) ? ORIGINATE : TERMINATE;
    if(topt.isAllOriginate()) role=ORIGINATE;
    else if(topt.isAllTerminate()) role=TERMINATE;
    if(role==ORIGINATE) {
      scv[cct]->setPostCallTime(100);
      scv[cct]->setCallTime(callTime);
      if (scv[cct]->connectSession() < 0) {
	RETCODE(-1);
      }
    }
  }
  
  int numSessions = 0;

  int callTimeLeft = callTime * 1000;
  int sleepInterval=10;

  do {

    se->run_immediate_events(0);
    
    numSessions = 0;

    for (cct = 0; cct < numChannels; cct++) {
      RvSipCallLegState eState = scv[cct]->getImmediateCallLegState();
      //printf("eState=%d\n",(int)eState);
      if(eState != RVSIP_CALL_LEG_STATE_TERMINATED && eState !=RVSIP_CALL_LEG_STATE_UNDEFINED) {
	numSessions++;
      } else {
	//printf("Terminate: eState=%d\n",(int)eState);
	scv[cct]->stopCallSession();
      }
    }

    usleep(sleepInterval * 1000);
    callTimeLeft-=sleepInterval;

    if(callTimeLeft % 1000 == 0) {
      printf("Time left %d, ns=%d\n",(int)(callTimeLeft/1000),(int)numSessions);
    }

    if(topt.isAllTerminate()) break;

    if(callTimeLeft<-130000) {
      printf("%s: Call timeout\n",__FUNCTION__);
      for (cct = 0; cct < numChannels; cct++) {
	RvSipCallLegState eState = scv[cct]->getImmediateCallLegState();
	//printf("eState=%d\n",(int)eState);
	if(eState == RVSIP_CALL_LEG_STATE_CONNECTED && scv[cct]->isOriginate()) {
	  printf("%s: 111.111: DISCONNECT FROM APP: callTimeLeft=%d: eState=%d\n",__FUNCTION__,(int)callTimeLeft,(int)eState);
	  scv[cct]->disconnectSession();
	}
      }
      numSessions=0;
    }
  } while (!flag_exit && numSessions > 0);

  {
    int i=0;
    do {
      se->run_immediate_events(0);
      usleep(sleepInterval*1000);
    } while(!flag_exit && i++<100);
  }
  
  RETCODE(0);
}

// start the channels ...
static int start(SipEngine * se, SipChannel **scv, int numCalls, int numChannels, int callTime)
{
  ENTER();
  if (scv == NULL)
    RETCODE(-1);
  
  int leftCalls = numCalls;
  int i=0;
  
  do {

    if (run(se,scv,numChannels,callTime) == -1) RETCODE(-1);

    if(flag_exit) break;
    
    if(topt.isAllTerminate()) continue;

    printf("\n============ call %d out of %d done ===============\n",++i,numCalls);

    if (numCalls > 0) {
      if (--leftCalls <= 0) {
	break;
      }
    }
    
  } while(true);
  
  RETCODE(0);
}

//=============================================================================

class SipChannelFinderTrial : public SipChannelFinder {
 public:
  SipChannelFinderTrial(SipChannel **s, int n) : scv(s), numChannels(n) {}
  virtual ~SipChannelFinderTrial() {}
  virtual SipChannel* findSipChannel(const char* strUserName, int handle) const {

    int cct=0;

    if (strstr(strUserName, "stc_") == strUserName) {
      cct = atoi(strUserName+strlen("stc_"));
    } else if (strstr(strUserName, "stco_") == strUserName) {
      cct = atoi(strUserName+strlen("stco_"));
    } else if (strstr(strUserName, "stct_") == strUserName) {
      cct = atoi(strUserName+strlen("stct_"));
    } else {
      cct = 0;
      if(topt.isAllTerminate() && !topt.getMobile()) cct+=1000;
    }

    if(topt.isAllTerminate() && !topt.getMobile()) cct-=1000;

    if (cct<0 || cct >= numChannels) return NULL;

    return scv[cct];
  }
private:
  SipChannel **scv;
  int numChannels;
};

//=============================================================================
static int NbrOfChannels = 0;
static SipChannel **Scv = NULL;

static void sig_exit(int sig) {
  flag_exit++;
}

int main(int argc, char **argv)
{
  const char opt_string[] = "hn:t:N:A:I:P:R:r:s:T:c:d:l:g:m:p:x:X:j:u:U:w:f:abSCODe";

  uint16_t vdevblock=17;

  // 1. Parse options
  set_default_test_options();
  if (parse_test_options(argc, argv, opt_string)!=0) {
    usage();
    return -1;
  }
  printf ("\n");
  
  int numChannels = topt.getNumChannels();
  if(numChannels<1) {
    if(!topt.isAllTerminate() && !topt.isAllOriginate()) topt.setNumChannels(2);
    else topt.setNumChannels(1);
    numChannels = topt.getNumChannels();
  }
  NbrOfChannels = numChannels;

  print_test_options();
  
  int callTime = topt.getCallTime();

  // 3. Create SIP Channel objects
  TRACE("Create SIP channels...");
  char userName[65];
  char userNumber[65];
  const char *userNumberContext="";

  SipChannel * scv[numChannels];
  Scv = scv;
  int port=topt.getBaseIpPort();

  SipChannelFinderTrial scFinder(scv,numChannels);
  MD5ProcessorSipTrial md5Processor;

  TRACE("Init SIP Engine");
  SipEngine::init(10,1024,scFinder,md5Processor);
  
  // 4. Create/prepare/start SIP engine...
  TRACE("Create SIP Engine...");
  SipEngine * se = SipEngine::getInstance();
  
  if (se == NULL) {
    printf("?SipNewEngine?\n");
    return -1;
  }
  if(topt.getDebug()){
      se->setlogmask();
  }


  std::string dnsIP=topt.getDNSIP();
  if(!dnsIP.empty()) {
    std::vector<std::string> dnsServers;
    dnsServers.push_back(dnsIP);
    int ret = se->setDNSServers(dnsServers,vdevblock,false);
    if(ret<0) {
      printf("?Cannot add DNS server %s: ret=%d\n",dnsIP.c_str(),ret);
      return -1;
    }
    ret = se->setDNSTimeout(5,2,vdevblock);
    if(ret<0) {
      printf("?Cannot set DNS timeout ret=%d\n",ret);
      return -1;
    }
  }

  SipTrialMediaInterface mediaHandler;

  bool telUriRemote=false;
  bool telUriLocal=false;

  bool useLocalDisplayName=false;

  int cct = 0;

  for (cct = numChannels-1; cct>=0; cct--) {

    std::string displayName;

    sprintf(userNumber,"0%u",cct);

    if(topt.isAllOriginate()) { sprintf (userName, "stco_%u", cct); if(useLocalDisplayName) displayName="originate"; userNumberContext="stc.org"; }
    else if(topt.isAllTerminate()) { sprintf (userName, "stct_%u", cct+1000); if(useLocalDisplayName) displayName="terminate"; sprintf(userNumber,"0%u",cct+1000); userNumberContext="spirent.com"; }
    else { sprintf (userName, "stc_%u", cct); if(useLocalDisplayName) displayName = std::string(userName)+std::string("_phone"); userNumberContext="sunnyvale.uk"; }

    if(!topt.getLocalUserName().empty()) sprintf (userName, "%s", topt.getLocalUserName().c_str());

    printf("username: (%s), port: %lu, cct=%lu, numChannels=%lu\n", userName, 
	   (unsigned long)port,(unsigned long)cct, (unsigned long)numChannels);
    
    const char* baseipaddr = topt.getBaseIpAddr();

    RvSipTransportAddr addrDetails;
    const char* ip = baseipaddr;
    addrDetails.eAddrType = (strstr(ip,":") ? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 : RVSIP_TRANSPORT_ADDRESS_TYPE_IP);
    addrDetails.eTransportType = topt.getSipTransport();
    addrDetails.port = port;
    strcpy(addrDetails.strIP,ip);
    strcpy((char*)addrDetails.if_name,(const char*)(topt.getBaseInterfaceName()));
    addrDetails.vdevblock=vdevblock;
    
    scv[cct] = se->newSipChannel();
 
    if (scv[cct] == NULL) {
      printf("?new SipChannel(cct=%d)?\n", cct);
      return -1;
    }

    int privacy=RVSIP_PRIVACY_HEADER_OPTION_HEADER | RVSIP_PRIVACY_HEADER_OPTION_SESSION | RVSIP_PRIVACY_HEADER_OPTION_CRITICAL;
    bool anon=true;
    bool secure=false;

    if(topt.getAka()||topt.getMobile()) {
      privacy=0;
      anon=false;
    }

    scv[cct]->setSMType(CFT_BASIC);

    scv[cct]->setProcessingType(true, false, false, secure, true, anon, false, privacy);

    scv[cct]->setIdentity(userName, userNumber, userNumberContext, displayName, telUriLocal);

    scv[cct]->setContactDomain(topt.getLocalDomain(), addrDetails.port);

    int sessexpired=90;
    if(topt.getAka()||topt.getMobile()) {
      sessexpired=900;
    }
    if(topt.getAka()||topt.getMobile()){
        scv[cct]->setSubOnReg(topt.getSubscription());
    }

    scv[cct]->setSessionExpiration(sessexpired, RVSIP_SESSION_EXPIRES_REFRESHER_UAS);
   
    scv[cct]->setMediaInterface(&mediaHandler);

    scv[cct]->setCallStateNotifier(&rn);

    scv[cct]->setRingTime(3);

    {
      //addrDetails.eTransportType = (RvSipTransport)0;
      //addrDetails.Ipv6Scope = 64;
      int rv = (int)(scv[cct]->setLocal(addrDetails, 192, numChannels));
      if(rv < 0) {
	printf("? set local %d, rv=%d, port=%d, strip=%s, tt=%d, at=%d?\n", cct, rv,
	       addrDetails.port,addrDetails.strIP,(int)addrDetails.eTransportType,(int)addrDetails.eAddrType);
	return -1;
      }
    }
  }
  
  // 3.1 For each Local party set the Remote
  TRACE("Set Remotes...");

  for (cct = 0; cct < numChannels; cct++) {
    int cctRemote = -1;
    char remoteName[33];
    std::string remoteContext;
    printf("host: %s\n",topt.getRemoteHostName());
    RvSipTransportAddr addrDetails;
    addrDetails.eTransportType = topt.getSipTransport();
    addrDetails.if_name[0]=0;
    addrDetails.vdevblock=23;

    if(topt.isAllOriginate()) { sprintf (userName, "stco_%u", cct); remoteContext="spirent.com"; }
    else if(topt.isAllTerminate()) { sprintf (userName, "stct_%u", cct+1000); remoteContext="stc.org"; }
    else { sprintf (userName, "stc_%u", cct); remoteContext="sunnyvale.uk"; }

    if(!topt.getLocalUserName().empty()) sprintf (userName, "%s", topt.getLocalUserName().c_str());

    std::string displayNameRemote;
    
    if(topt.isAllOriginate()) {
      displayNameRemote="terminate";
      cctRemote = cct+1000;
      sprintf(remoteName,"stct_%u",cctRemote);
      const char* ip = topt.getRemoteIP();
      addrDetails.eAddrType = (strstr(ip,":") ? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 : RVSIP_TRANSPORT_ADDRESS_TYPE_IP);
      addrDetails.port = scv[cct]->getLocalAddress().port;
      strcpy(addrDetails.strIP,ip);
    } else if(topt.isAllTerminate()) {
      displayNameRemote="originate";
      cctRemote = cct;
      sprintf(remoteName,"stco_%u",cctRemote);
      const char* ip = topt.getRemoteIP();
      addrDetails.eAddrType = (strstr(ip,":") ? RVSIP_TRANSPORT_ADDRESS_TYPE_IP6 : RVSIP_TRANSPORT_ADDRESS_TYPE_IP);
      addrDetails.port = scv[cct]->getLocalAddress().port;
      strcpy(addrDetails.strIP,ip);
    } else {
      cctRemote = (cct&1) ? cct-1 : cct+1;
      sprintf(remoteName,"stc_%d",cctRemote);
      displayNameRemote=scv[cctRemote]->getLocalDisplayName();
      memcpy(&addrDetails,&scv[cctRemote]->getLocalAddress(),sizeof(addrDetails));
      addrDetails.eTransportType = topt.getSipTransport();
    }

    if(telUriRemote) sprintf(remoteName,"0%u",cctRemote);

    if(!topt.getRemoteUserName().empty()) sprintf (remoteName, "%s", topt.getRemoteUserName().c_str());
    if(!topt.getRemoteUserName().empty()) displayNameRemote = topt.getRemoteUserName();

    char authName[33];
    char authPwd[33];
    sprintf(authName,"User%d",cct+1);
    if(!topt.getLocalUserName().empty()) sprintf (authName, "%s", topt.getLocalUserName().c_str());
    sprintf(authPwd,"User%d",cct+1);
    if(!topt.getPassword().empty()) sprintf (authPwd, "%s", topt.getPassword().c_str());

    scv[cct]->setAka(topt.getAka());

    if(topt.getMobile())
        scv[cct]->setMobile(true);
    if(topt.UseIMSI()){
        scv[cct]->setImsi(authName);
        scv[cct]->setAuth(authName,topt.getRealm(), authPwd);
        
    }else{
        scv[cct]->setMobile(false);
        scv[cct]->setAuth(authName,topt.getRealm(), authPwd);
    }

    if(!topt.isAllTerminate() && (topt.isAllOriginate() || (cct%2==0))) {
 
        bool usingProxyDiscovery=false;
        if(strlen(topt.getProxyDomain()) && !strlen(topt.getProxyName()) && !strlen(topt.getProxyIP()))
          bool usingProxyDiscovery=true;

        scv[cct]->setRemote(remoteName,
                remoteContext,
                displayNameRemote,
                addrDetails,
                topt.getProxyIP(),
                topt.getProxyName(),
                topt.getProxyDomain(),
                topt.getProxyPort(),
                topt.getRemoteHostName(),
                topt.getRemoteDomain(),
                topt.getRemoteDomainPort(),
                telUriRemote,
                usingProxyDiscovery);
    }

    int expired=120;
    if(topt.getAka()||topt.getMobile()) {
      expired=1200;
    }

    bool usingRegistrarDiscovery = false;
    if(strlen(topt.getRegDomain()) && !strlen(topt.getRegName()) && !strlen(topt.getRegIP()))
        usingRegistrarDiscovery = true;

    if(dnsIP.empty()) {
        scv[cct]->setRegistration(userName,"",topt.getProxy(),topt.getRegDomain(),topt.getRegistrarPort(),expired,77,3,true,false);
    } else {
        scv[cct]->setRegistration(userName,topt.getRegName(),topt.getRegIP(),topt.getRegDomain(),topt.getRegistrarPort(),expired,77,3,true,usingRegistrarDiscovery);
    }
    std::vector<std::string> ecs;
    //ecs.push_back(std::string(userName)+std::string(" <sip:")+std::string(userName)+std::string("@1.2.3.4:5678;transport=UDP>"));
    //ecs.push_back(std::string(userName)+std::string(" <sip:")+std::string(userName)+std::string("@11.21.31.41:56781;transport=TCP>"));
    scv[cct]->setExtraContacts(ecs);
    
    if(topt.getAka()|| topt.getMobile()) {
        if(!topt.UseIMSI()){
            std::string authName = scv[cct]->getAuthName();
            if(authName.find("@")==std::string::npos) {
                authName=authName+"@"+scv[cct]->getAuthRealm();
                std::string impi=authName;
                std::string impu="sip:"+impi;
                scv[cct]->setAuth(authName,scv[cct]->getAuthRealm(), authPwd);
                scv[cct]->setIMSIdentity(impu,impi);
            }
        }
    }
  }

  TRACE("Start SIP Engine...");
  struct sigaction sa;
  memset(&sa,0,sizeof(sa));

  sa.sa_handler = sig_exit;
  sigaction(SIGTERM,&sa,NULL);
  sigaction(SIGINT,&sa,NULL);

  reg(se,scv,numChannels);
  printf("reg done...\n");

  {
    int i=0;
    do {
      se->run_immediate_events(0);
      usleep(1000);
    } while(!flag_exit && i++<1000);
  }

  if(topt.getRegistrationOnly() == false){

      //===============================================================
      if(start(se,scv,topt.getNumCalls(),numChannels,callTime) == -1)
          printf("?se->start()?\n");
      //===============================================================
  }
  
  unreg(se,scv,numChannels);

  printf("================ siptrial done ==============\n");
  
  for (cct = 0; cct < numChannels; cct++) if (scv[cct]) se->releaseChannel(scv[cct]);

  SipEngine::destroy();
  
  return 0;
}

