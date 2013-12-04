#include <memory>
#include <string>
#include <sys/resource.h>

#include <ace/Signal.h>
#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <vif/IfSetupCommon.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "HttpApplication.h"
#include "HttpClientRawStats.h"

#undef EXTENSIVE_UNIT_TESTS

USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestHttpApplication : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestHttpApplication);
    CPPUNIT_TEST(testOneServerManyClients);
    CPPUNIT_TEST(testOneServerManyClientsWithMutipleClientBlocks);
#ifdef EXTENSIVE_UNIT_TESTS
    CPPUNIT_TEST(testNoServerManyClients);
    CPPUNIT_TEST(testOneServerManyClientsWithSingleThread);
    CPPUNIT_TEST(testNoServerManyClientsWithSingleThread);
    CPPUNIT_TEST(testOneServerManyClientsWithSinusoid);
    CPPUNIT_TEST(testNoServerManyClientsWithSinusoid);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void) { }
    
protected:
    static const size_t NUM_IO_THREADS = 4;
    static const rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT;
    static const unsigned short SERVER_PORT_NUMBER_BASE = 10009;
    unsigned short mServerPortNumber;
    
    void testOneServerManyClients(void);
    void testOneServerManyClientsWithMutipleClientBlocks(void);
    void testNoServerManyClients(void);
    void testOneServerManyClientsWithSingleThread(void);
    void testNoServerManyClientsWithSingleThread(void);
    void testOneServerManyClientsWithSinusoid(void);
    void testNoServerManyClientsWithSinusoid(void);
};

///////////////////////////////////////////////////////////////////////////////

const rlim_t TestHttpApplication::MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::setUp(void)
{
    ACE_OS::signal(SIGPIPE, SIG_IGN);

    struct rlimit fileLimits;
    getrlimit(RLIMIT_NOFILE, &fileLimits);
    if (fileLimits.rlim_cur < MAXIMUM_OPEN_FILE_HARD_LIMIT || fileLimits.rlim_max < MAXIMUM_OPEN_FILE_HARD_LIMIT)
    {
        fileLimits.rlim_cur = std::max(fileLimits.rlim_cur, MAXIMUM_OPEN_FILE_HARD_LIMIT);
        fileLimits.rlim_max = std::max(fileLimits.rlim_max, MAXIMUM_OPEN_FILE_HARD_LIMIT);
        setrlimit(RLIMIT_NOFILE, &fileLimits);
    }

    mServerPortNumber = SERVER_PORT_NUMBER_BASE + (getpid() % 1023); 
}


///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testOneServerManyClientsWithMutipleClientBlocks(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t serverHandles;
    HttpApplication::handleList_t clientHandles;

	HttpApplication app(1);
    
    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);
    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

	// Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 10000;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 10000;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 10000;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 10000;
    clientConfig.Common.Load.LoadPhases.resize(1);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 100;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 110;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    HttpApplication::clientConfigList_t clients;
    {
        for(int i=0; i<4; i++)
        {
          clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = i;
          clientConfig.ServerPortInfo = mServerPortNumber+i;
          clients.push_back(clientConfig);         
        }       
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }
    CPPUNIT_ASSERT(clientHandles.size() == 4);
    for(unsigned int i=0; i<4; i++)
    {   
		CPPUNIT_ASSERT(clientHandles[i] == i);
		CPPUNIT_ASSERT(clients[i].ServerPortInfo == mServerPortNumber+i);
    }

	// Configure server block
    HttpApplication::serverConfig_t serverConfig;

    serverConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    serverConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    serverConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    serverConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    serverConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    serverConfig.ProtocolProfile.HttpServerTypes = http_1_port_server::HttpServerType_APACHE;
    serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType = L4L7_BASE_NS::ResponseTimingOptions_FIXED;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation = 0;
    serverConfig.ResponseConfig.BodySizeType = L4L7_BASE_NS::BodySizeOptions_FIXED;
    serverConfig.ResponseConfig.FixedBodySize = 64;
    serverConfig.ResponseConfig.RandomBodySizeMean = 0;
    serverConfig.ResponseConfig.RandomBodySizeStdDeviation = 0;
    serverConfig.ResponseConfig.BodyContentType = L4L7_BASE_NS::BodyContentOptions_ASCII;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestHttpApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 100000;
    serverConfig.ProtocolConfig.MaxRequestsPerClient = 100000;
    serverConfig.ProtocolProfile.EnableVideoServer = false;

    HttpApplication::serverConfigList_t servers;
    {
		for(unsigned int i=0; i<4;i++)
		{
		    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 10+i;
	        serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber+i;
		    servers.push_back(serverConfig);   	   
		}
		IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
	    scheduler.Enqueue(req);
	    req->Wait();
    }

    for(unsigned int i=0; i<4; i++)
    {     
        CPPUNIT_ASSERT(serverHandles[i] == 10+i);
	    CPPUNIT_ASSERT(servers[i].ProtocolProfile.ServerPortNum == mServerPortNumber+i);
    }

    HttpApplication::handleList_t handles;
    for (int i = 0; i<4; i++)
    {
	    handles.push_back(serverHandles[i]);
        handles.push_back(clientHandles[i]);
    }

    for (int run = 0; run < 1; run++)
    {
       // Start the server and client blocks
        {                 
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

	    sleep(80);
	    // Stop the server and client blocks
        {   
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            //sleep 5 seconds, Be sure that the req is received
	        sleep(5);
		
	        {
			    std::vector<HttpClientRawStats> stats;
		        std::vector<HttpClientRawStats> stats2;
			    uint64_t totalAttemptedConnections = 0;
		 	    uint64_t totalAttemptedConnections2 = 0;
	            app.GetHttpClientStats(0,clientHandles,stats);
			    
			    for(unsigned int i=0; i<stats.size(); i++)
	            {  
	               totalAttemptedConnections += stats[i].attemptedConnections;        
			    }
				
			    sleep(35);
			    
	            app.GetHttpClientStats(0,clientHandles,stats2);

		        for(unsigned int i=0; i<stats2.size(); i++)
	            {   
	                 totalAttemptedConnections2 += stats2[i].attemptedConnections;        
		        }

	            //printf(" totalAttemptedConnections2: %d", (unsigned int)totalAttemptedConnections2);
	            //printf(" totalAttemptedConnections: %d",  (unsigned int)totalAttemptedConnections);
			    CPPUNIT_ASSERT(totalAttemptedConnections2 == totalAttemptedConnections);
            }
		
            req->Wait();
        }  
    }
   	// Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);		
	
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testOneServerManyClients(void)
{

    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t serverHandles;
    HttpApplication::handleList_t clientHandles;
	HttpApplication app(1);
    
    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    HttpApplication::serverConfig_t serverConfig;

    serverConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    serverConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    serverConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    serverConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    serverConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    serverConfig.ProtocolProfile.HttpServerTypes = http_1_port_server::HttpServerType_APACHE;
    serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType = L4L7_BASE_NS::ResponseTimingOptions_FIXED;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation = 0;
    serverConfig.ResponseConfig.BodySizeType = L4L7_BASE_NS::BodySizeOptions_RANDOM;
    serverConfig.ResponseConfig.FixedBodySize = 0;
    serverConfig.ResponseConfig.RandomBodySizeMean = 1024;
    serverConfig.ResponseConfig.RandomBodySizeStdDeviation = 128;
    serverConfig.ResponseConfig.BodyContentType = L4L7_BASE_NS::BodyContentOptions_ASCII;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestHttpApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;
    serverConfig.ProtocolConfig.MaxRequestsPerClient = 0;
    serverConfig.ProtocolProfile.EnableVideoServer = false;
    
    {
        const HttpApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testNoServerManyClients(void)
{    
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t clientHandles;
	HttpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testOneServerManyClientsWithSingleThread(void)
{    
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t serverHandles;
    HttpApplication::handleList_t clientHandles;
    HttpApplication app(1);
	
    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    HttpApplication::serverConfig_t serverConfig;

    serverConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    serverConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    serverConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    serverConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    serverConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    serverConfig.ProtocolProfile.HttpServerTypes = http_1_port_server::HttpServerType_APACHE;
    serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType = L4L7_BASE_NS::ResponseTimingOptions_FIXED;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation = 0;
    serverConfig.ResponseConfig.BodySizeType = L4L7_BASE_NS::BodySizeOptions_RANDOM;
    serverConfig.ResponseConfig.FixedBodySize = 0;
    serverConfig.ResponseConfig.RandomBodySizeMean = 1024;
    serverConfig.ResponseConfig.RandomBodySizeStdDeviation = 128;
    serverConfig.ResponseConfig.BodyContentType = L4L7_BASE_NS::BodyContentOptions_ASCII;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestHttpApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;
    serverConfig.ProtocolConfig.MaxRequestsPerClient = 0;

    {
        const HttpApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testNoServerManyClientsWithSingleThread(void)
{   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t clientHandles;
	HttpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 10;
    clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 60;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(120);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testOneServerManyClientsWithSinusoid(void)
{
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t serverHandles;
    HttpApplication::handleList_t clientHandles;
	HttpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure server block
    HttpApplication::serverConfig_t serverConfig;

    serverConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    serverConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    serverConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    serverConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    serverConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    serverConfig.ProtocolProfile.HttpServerTypes = http_1_port_server::HttpServerType_APACHE;
    serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType = L4L7_BASE_NS::ResponseTimingOptions_FIXED;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation = 0;
    serverConfig.ResponseConfig.BodySizeType = L4L7_BASE_NS::BodySizeOptions_RANDOM;
    serverConfig.ResponseConfig.FixedBodySize = 0;
    serverConfig.ResponseConfig.RandomBodySizeMean = 1024;
    serverConfig.ResponseConfig.RandomBodySizeStdDeviation = 128;
    serverConfig.ResponseConfig.BodyContentType = L4L7_BASE_NS::BodyContentOptions_ASCII;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestHttpApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = 0;
    serverConfig.ProtocolConfig.MaxRequestsPerClient = 0;

    {
        const HttpApplication::serverConfigList_t servers(1, serverConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(serverHandles.size() == 1);
    CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_STAIR;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Repetitions = 1;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_SINUSOID;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Height = 100;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Repetitions = 6;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Period = 10;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 1;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 19;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(serverHandles[0]);
            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(115);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
            handles.push_back(serverHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpApplication::testNoServerManyClientsWithSinusoid(void)
{   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
    HttpApplication::handleList_t clientHandles;
	HttpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);

    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    // Configure client block
    HttpApplication::clientConfig_t clientConfig;

    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;
    clientConfig.Common.Load.LoadPhases.resize(4);
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 0;
    clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_STAIR;
    clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Height = 300;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.Repetitions = 1;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.RampTime = 30;
    clientConfig.Common.Load.LoadPhases[1].StairDescriptor.SteadyTime = 0;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_SINUSOID;
    clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Height = 100;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Repetitions = 6;
    clientConfig.Common.Load.LoadPhases[2].SinusoidDescriptor.Period = 10;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
    clientConfig.Common.Load.LoadPhases[3].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.Height = 0;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.RampTime = 1;
    clientConfig.Common.Load.LoadPhases[3].FlatDescriptor.SteadyTime = 19;
    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestHttpApplication";
    clientConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    clientConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    clientConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    clientConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    clientConfig.Common.Endpoint.SrcIfHandle = 0;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors.resize(1);
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
    clientConfig.Common.Endpoint.DstIf.ifDescriptors[0].indexInList = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList.resize(1);
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.IfRecycleCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.BllHandle = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].NetworkInterface.AffiliatedInterface = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[0] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[1] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[2] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrStep.address[3] = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].SkipReserved = false;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].AddrRepeatCount = 0;
    clientConfig.Common.Endpoint.DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    clientConfig.ProtocolProfile.HttpVersion = http_1_port_server::HttpVersionOptions_VERSION_1_1;
    clientConfig.ProtocolProfile.EnableKeepAlive = true;
    clientConfig.ProtocolProfile.UserAgentHeader = "Spirent Communications";
    clientConfig.ProtocolProfile.EnablePipelining = true;
    clientConfig.ProtocolProfile.MaxPipelineDepth = 8;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestHttpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = 16;
    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.ProtocolProfile.EnableVideoClient = false;

    {
        const HttpApplication::clientConfigList_t clients(1, clientConfig);
        
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
        scheduler.Enqueue(req);
        req->Wait();
    }

    CPPUNIT_ASSERT(clientHandles.size() == 1);
    CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);

    for (size_t run = 0; run < 1; run++)
    {
        // Start the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }

        // Let the blocks run for a bit
        sleep(115);

        // Stop the server and client blocks
        {
            HttpApplication::handleList_t handles;

            handles.push_back(clientHandles[0]);
        
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::StopProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    }
    
    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &HttpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestHttpApplication);
