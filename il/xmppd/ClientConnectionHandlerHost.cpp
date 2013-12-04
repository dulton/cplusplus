
/// @file
/// @brief Client Connection Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <string.h>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <unistd.h>

#include <ace/Message_Block.h>
#include <utils/MessageUtils.h>
#include <utils/MessageIterator.h>
#include <xmppvj_1_port_server.h>
#include <hal/TimeStamp.h>
#include <pubsubitem.h>
#include <boost/lexical_cast.hpp>

#include "ClientConnectionHandlerHost.h"
#include "CapabilitiesParser.h"
#include "XmppdLog.h"

#define LOG_CCHH_LOGGER logger_local_id_5
#define LOG_CCHH_FLAG PHX_LOCAL_ID_5_FLAG
#define LOG_CCHH PHX_LOCAL_ID_5
#define LOG_ALL_CCHH (LOG_ALL | LOG_CCHH)


#define RUNNING_PERIOD 100
#define SESSION_DURATION_TIME 10
#define PUBINTERVAL 2
static std::string debug_level = "DEBUG";
static std::string serverDomain = "openfire";
static int nbrOfClients=1;
static int Retires =1;
static int nbrOfRuns = 1;
static int __total = 1;
static uint32_t g_nbrOfCCH=0;
static int delay = 15;
static int running_periold = RUNNING_PERIOD;
static bool running_done = false;

USING_XMPP_NS;
using namespace XmppvJ_1_port_server;
using namespace xmppvj_1_port_server;

void Cfg::importFile(std::string fileName){
    std::ifstream fin;
    char conf[256];

    fin.open(fileName.c_str());

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
                    *q = '\0';
                    q++;
                    d = q;
                    while((*d != ' ') && (*d != '\n') && (*d != '\r') && (*d != '\0')) d++;
                    *d = '\0';
                    if(strlen(q)){
                        std::string var(p);
                        std::string val(q);
                        mCfg[var] = val;
                    }
                }
            }
        }
        fin.close();
    }
}
int Cfg::getCfgInt(std::string var){
    if(mCfg[var].empty())
        return 0;
    else
        return atoi(mCfg[var].c_str());
}
bool Cfg::getCfgBool(std::string var){
    if(mCfg[var].empty())
        return false;
    else{
        if(mCfg[var] == "Y")
            return true;
        else
            return false;
    }
}

ClientConnectionHandlerHost::ClientConnectionHandlerHost(uint32_t serial) : ClientConnectionHandler(serial) {
    g_nbrOfCCH++;
}

ClientConnectionHandlerHost::~ClientConnectionHandlerHost(){
    if(--g_nbrOfCCH == 0){
        if(mTR)
            mTR->report();
        if(mTester){
            mTester->onRoundDone();
        }
    }
}
void ClientConnectionHandlerHost::onConnect()
{
    if(GetClientType() == CLIENT_LOGIN && glClientConnected()){
        if (mTR) 
            mTR->inc("LoginSuccess");
    }
    ClientConnectionHandler::onConnect();
}

void ClientConnectionHandlerHost::setTestResults(testReporter *tr){
    mTR = tr;
    if(mTR){
        mTR->inc("Clients");
        PHX_LOG_LOCAL(PHX_LOG_ERR,LOG_CCHH, "[ %4d / %4d ]",mTR->nbrof("Clients"),__total);
    }
}
bool ClientConnectionHandlerHost::InitiateLogout(const ACE_Time_Value& tv){
    ClientConnectionHandler::InitiateLogout(tv);
    aging();
    return true;
}
bool ClientConnectionHandlerHost::SendPublish(void) {

    if(mTR)
        mTR->inc("PubAttempt");
    return ClientConnectionHandler::SendPublish();
}


HostRunner::HostRunner(std::string &serverIP,uint16_t port,std::string &local,uint16_t lport,ACE_Reactor *reactor,int testCounts,testReporter *tr)
    : mServer(port,serverIP.c_str()),
      mLocal(lport,local.c_str()),
      mReactor(reactor),
      mTries(testCounts),
      mtR(tr)
{
    mCchs_del.clear();
    this->reactor(mReactor);
    mConnector.reset(new Connector_t(mReactor));
    mConnector->SetConnectNotificationDelegate(fastdelegate::MakeDelegate(this, &HostRunner::HandleConnectionNotification));
    mConnector->SetCloseNotificationDelegate(  fastdelegate::MakeDelegate(this, &HostRunner::HandleCloseNotification));

    std::string file_path("capabilities.xml");

    CapabilitiesParser parser(mPubCapabilities);

    parser.Parse(file_path);
    for(int i=0;i<(int)mPubCapabilities.size();i++)
        PHX_LOG_LOCAL(PHX_LOG_DEBUG,LOG_CCHH,"tag number %d is %s.\n",i,mPubCapabilities[i]->xml().c_str());
}

HostRunner::~HostRunner(){
    if(mTimerId != -1){
          this->reactor()->cancel_timer(mTimerId);
          mTimerId = -1;
          this->reactor()->remove_handler(this,ACE_Event_Handler::TIMER_MASK | ACE_Event_Handler::DONT_CALL);
    }
    this->reactor()->close();
}

int HostRunner::handle_timeout(const ACE_Time_Value &current_time, const void *arg) {
    //printf("HostRunner::%s Retires:%d running_done:%d running_periold:%d.\n",__func__,Retires,running_done,running_periold);
    if(Retires-- > 0){
        cchGenerator();
    }else{
        if(!running_done && (--running_periold == 0))
            running_done = true;
        if(running_done){
            std::list<cch_ptr_t>::iterator _it = mCchs_del.begin();
            std::list<cch_ptr_t>::iterator it;

            while( _it != mCchs_del.end()){
                it = _it++;
                delete (*it);
                mCchs_del.erase(it);
            }
        }
    }
    
    return 0;
}

bool HostRunner::HandleConnectionNotification(ClientConnectionHandlerHost& connHandler)
{
    PHX_LOG_LOCAL(PHX_LOG_DEBUG,LOG_CCHH, "HostRunner::%s.",__func__);
    static size_t index = 0;
    std::string un("");
    std::string pw("");

    connHandler.SetServerDomain(serverDomain);
    connHandler.SetClientType(ClientConnectionHandlerHost::CLIENT_LOGIN);
    //connHandler.SetSessionType(EnumSessionType_LOGIN_ONLY);
    //connHandler.SetSessionType(EnumSessionType_FULL_PUB_SUB);
    connHandler.SetSessionType(EnumSessionType_TIMED_PUB_SUB);
    connHandler.SetPubInterval(PUBINTERVAL);
    //connHandler.SetClientType(ClientConnectionHandlerHost::CLIENT_REGISTER);
    connHandler.SetSessionDuration(SESSION_DURATION_TIME);
    connHandler.SetClientIndex(index);
    connHandler.SetPubCapabilities(mPubCapabilities);

    un = "spirent_t"+boost::lexical_cast<std::string>(index/100+1)+boost::lexical_cast<std::string>(510000+(index++%100));
    //un = "spirent_t5510"+boost::lexical_cast<std::string>(300+index++);
    pw = "spirent1";

    connHandler.SetUsername(un);
    connHandler.SetPassword(pw);
      
    return true;
}

void HostRunner::HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler)
{
    PHX_LOG_LOCAL(PHX_LOG_DEBUG,LOG_CCHH, "HostRunner::%s.",__func__);
    cch_ptr_t connHandler = reinterpret_cast<cch_ptr_t>(&socketHandler);

    if (connHandler->IsPending())
    {
        if (mConnector)
            mConnector->cancel(connHandler);
    }
    aging(connHandler);
}

void HostRunner::runTest(){
    ACE_Time_Value initialDelay(0,delay*1000000);
    ACE_Time_Value interval(0,100000);
    mTimerId = this->reactor()->schedule_timer(this,this->reactor(),initialDelay,interval);
    mReactor->run_reactor_event_loop();
}

void HostRunner::cchGenerator(){
    ClientConnectionHandlerHost *cch = 0;
    int err;
    //static uint32_t g_serial = 0;

    std::string localInf("eth0");
    mConnector->BindToIfName(&localInf);
    mConnector->MakeHandler(cch);
    if(cch){
        cch->setTestResults(mtR);
        cch->setTester(this);
        err = mConnector->connect(cch,mServer,ACE_Synch_Options(ACE_Synch_Options::USE_REACTOR),mLocal,1);
    }else{
        PHX_LOG_LOCAL(PHX_LOG_ERR,LOG_CCHH, "create CCHH instance failed.");
    }

}

void HostRunner::aging(cch_ptr_t connHandler){
    std::list<cch_ptr_t>::iterator it;
    it = find(mCchs_del.begin(),mCchs_del.end(),connHandler);
    if(it == mCchs_del.end())
        mCchs_del.push_back(connHandler);
}
void HostRunner::onRoundDone(){
    if(--nbrOfRuns > 0){
        Retires = nbrOfClients;
        running_periold = RUNNING_PERIOD;
        running_done = false;
    }
}

int main(int argc,char  **argv){
    ACE_Reactor *reactor = ACE_Reactor::instance();
    Cfg cfg = Cfg();
    std::string server(cfg.getCfg("SERVERIP"));
    uint16_t rport=cfg.getCfgInt("SERVERPORT");
    std::string local(cfg.getCfg("LOCALIP"));
    uint16_t lport=cfg.getCfgInt("LOCALPORT");
    nbrOfClients = cfg.getCfgInt("NBR_OF_CLIENTS");
    nbrOfRuns = cfg.getCfgInt("NBR_OF_LOOP");
    delay = cfg.getCfgInt("DELAY");
    debug_level = cfg.getCfg("DEBUGLEVEL");
    serverDomain = cfg.getCfg("SERVERDOMAIN");

    std::string opt_string = "c:l:d:v:n:";
    int c;

    while((c=getopt(argc, argv, opt_string.c_str())) != -1) {
        switch(c){
            case 'l'://loop
                {
                    nbrOfRuns = atoi(optarg);
                    break;
                }
            case 'd'://delay
                {
                    delay = atoi(optarg);
                    break;
                }
            case 'v'://debug level
                {
                    debug_level = optarg;
                    break;

                }
            case 'c'://debug level
                {
                    nbrOfClients = atoi(optarg);
                    break;

                }
            case 'n':
                {
                    serverDomain = optarg;
                }
        }

    }
    int _log_level =  PHX_LOG_DEBUG;
    if(debug_level == "ERROR")
        _log_level = PHX_LOG_ERR;
    else if(debug_level == "INFO")
        _log_level = PHX_LOG_INFO;
    else if(debug_level == "DEBUG")
        _log_level = PHX_LOG_DEBUG;
    else
        _log_level = PHX_LOG_DEBUG;


    PHX_LOG_REDIRECTLOG_INIT("/dev/stdout","0x1-MPS,0x2-Client,0x4-Server,0x8-SOCKET,0x10-ClientHost",LOG_UPTO(_log_level),LOG_ALL_CCHH);

    Retires = nbrOfClients;
    __total = nbrOfClients*nbrOfRuns;
    printf("nbrOfClients:%d nbrOfRuns:%d delay:%d debug:%s.\n",nbrOfClients,nbrOfRuns,delay,debug_level.c_str());
    testReporter *tr = new testReporter();
    PHX_LOG_LOCAL(PHX_LOG_DEBUG,LOG_CCHH, "testReporter created.\n");

    HostRunner *hr = new HostRunner(server,rport,local,lport,reactor,nbrOfClients,tr);
    hr->runTest();
    delete hr;
    delete tr;
    PHX_LOG_REDIRECTLOG_CLOSE("/dev/stdout");

    printf("test done.\n");
    return 0;
}

