#ifndef _CLIENT_CONNECTION_HANDLER_HOST_H_
#define _CLIENT_CONNECTION_HANDLER_HOST_H_

#include <map>
#include "ClientConnectionHandler.h"
#include <app/StreamSocketConnector.tcc>
DECL_XMPP_NS
class ClientConnectionHandlerHost;
typedef L4L7_APP_NS::StreamSocketConnector<ClientConnectionHandlerHost> Connector_t;
typedef boost::shared_ptr<ClientConnectionHandlerHost> cch_t;
typedef ClientConnectionHandlerHost* cch_ptr_t;

class Cfg{
    public:
        Cfg(std::string cfgFile="xmppd_host.cfg"){mCfg.clear();importFile(cfgFile);}
        ~Cfg(){}
        void importFile(std::string fileName);
        std::string& getCfg(std::string var) {return mCfg[var];}
        int getCfgInt(std::string var);
        bool getCfgBool(std::string var);
    private:
        std::map<std::string,std::string> mCfg;
};

class testReporter{
    public:
        testReporter();
        ~testReporter(){}
        void inc(std::string var){uint32_t val=mDB[var];mDB[var]=++val;}
        uint32_t nbrof(std::string var){return mDB[var];}
        void report();
    private:
        std::map<std::string,uint32_t> mDB;

};

testReporter::testReporter(){
    mDB.clear();
}
void testReporter::report(){
    std::map<std::string,uint32_t>::iterator it;
    printf("==================================test results =======================================\n");
    for(it=mDB.begin();it!=mDB.end();++it){
        printf("%-12s:        %d\n",(*it).first.c_str(),(*it).second);

    }
    printf("======================================================================================\n");
}

class HostRunner : public ACE_Event_Handler{
    public:
        HostRunner(std::string &serverIP,uint16_t port,std::string &local,uint16_t lport,ACE_Reactor *reactor,int testCounts,testReporter *tr); 
        ~HostRunner();
        void cchGenerator();
        bool HandleConnectionNotification(ClientConnectionHandlerHost& connHandler);
        void HandleCloseNotification(L4L7_APP_NS::StreamSocketHandler& socketHandler);
        int handle_timeout(const ACE_Time_Value &current_time, const void *arg) ;
        void runTest();
        void aging(cch_ptr_t cch);
        void onRoundDone();
    private:
        boost::scoped_ptr<Connector_t> mConnector;
        ACE_INET_Addr mServer;
        ACE_INET_Addr mLocal;
        ACE_Reactor *mReactor;
        long mTimerId;
        int mTries;
        testReporter *mtR;
        std::list<cch_ptr_t> mCchs_del;
        std::vector<gloox::Tag*> mPubCapabilities; 
};

class ClientConnectionHandlerHost : public ClientConnectionHandler{
    friend class HostRunner;
    public:
        ClientConnectionHandlerHost(uint32_t serial = 0);
        ~ClientConnectionHandlerHost();
        void onConnect();
        bool InitiateLogout(const ACE_Time_Value& tv);
        void setTester(HostRunner *hr) {mTester = hr;}
        void setTestResults(testReporter *tr); 
        void aging(){if(mTester) mTester->aging(this);}
        bool SendPublish(void);
    private:
        HostRunner *mTester;
        testReporter *mTR;

};

END_DECL_XMPP_NS

#endif
