#include <memory>
#include <string>

#include <ace/Signal.h>
#include <ildaemon/ActiveScheduler.h>
#include <ildaemon/BoundMethodRequestImpl.tcc>
#include <base/BaseCommon.h>
#include <boost/bind.hpp>
#include <vif/IfSetupCommon.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "FtpApplication.h"

#undef EXTENSIVE_UNIT_TESTS

USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestFtpApplication : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestFtpApplication);
    CPPUNIT_TEST(testBasicClientServer);
#ifdef EXTENSIVE_UNIT_TESTS
    CPPUNIT_TEST(testClientServerWithSinusoid);
    CPPUNIT_TEST(testBasicClientServerWithSingleThread);
    CPPUNIT_TEST(testNoServerMultipleClients);
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) ;
    void tearDown(void) { }
    
protected:
    static const size_t NUM_IO_THREADS = 4;
    static const rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT;
    static const unsigned short SERVER_PORT_NUMBER_BASE = 10009;
    unsigned short mServerPortNumber;

    // utility methods
    void SetupBasicServer(FtpApplication::serverConfig_t &serverConfig, int maxSimultaneousClients, int maxRequestPerClient) const ;
    void SetupBasicClient(FtpApplication::clientConfig_t &clientConfig, int maxTransPerServer, bool port, bool retr) const;

    // test methods.
    void testBasicClientServer(void) ;
    void testClientServerWithSinusoid(void) ;
    void testBasicClientServerWithSingleThread(void) ;
    void testNoServerMultipleClients(void) ;
};

///////////////////////////////////////////////////////////////////////////////

const rlim_t TestFtpApplication::MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

void TestFtpApplication::setUp(void)
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
void TestFtpApplication::SetupBasicServer(FtpApplication::serverConfig_t &serverConfig, int maxSimultaneousClients, int maxRequestPerClient) const
{
    serverConfig.Common.Profile.L4L7Profile.ProfileName = "TestFtpApplication";
    serverConfig.Common.Profile.L4L7Profile.Ipv4Tos = 0;
    serverConfig.Common.Profile.L4L7Profile.Ipv6TrafficClass = 0;
    serverConfig.Common.Profile.L4L7Profile.RxWindowSizeLimit = 0;
    serverConfig.Common.Profile.L4L7Profile.EnableDelayedAck = true;
    serverConfig.Common.Endpoint.SrcIfHandle = 0;
    serverConfig.ProtocolProfile.BaseDirPath = std::string("");
    serverConfig.ProtocolProfile.ServerPortNum = mServerPortNumber;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.ResponseTimingType = L4L7_BASE_NS::ResponseTimingOptions_FIXED;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.FixedResponseLatency = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyMean = 0;
    serverConfig.ResponseConfig.L4L7ServerResponseConfig.RandomResponseLatencyStdDeviation = 0;
    serverConfig.ResponseConfig.BodySizeType = L4L7_BASE_NS::BodySizeOptions_FIXED;
    serverConfig.ResponseConfig.FixedBodySize = 1024;
    serverConfig.ResponseConfig.RandomBodySizeMean = 1024;
    serverConfig.ResponseConfig.RandomBodySizeStdDeviation = 128;
    serverConfig.ResponseConfig.BodyContentType = L4L7_BASE_NS::BodyContentOptions_BINARY;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle = 1;
    serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.ServerName = "TestFtpApplication";
    serverConfig.ProtocolConfig.MaxSimultaneousClients = maxSimultaneousClients;
    serverConfig.ProtocolConfig.MaxRequestsPerClient = maxRequestPerClient;
} 

///////////////////////////////////////////////////////////////////////////////

void TestFtpApplication::SetupBasicClient(FtpApplication::clientConfig_t &clientConfig, int maxTransPerServer, bool port, bool retr) const
{
    clientConfig.Common.Load.LoadProfile.LoadType = L4L7_BASE_NS::LoadTypes_CONNECTIONS;  // L4L7_BASE_NS::LoadTypes_CONNECTIONS_PER_TIME_UNIT
    clientConfig.Common.Load.LoadProfile.RandomizationSeed = 0;
    clientConfig.Common.Load.LoadProfile.MaxConnectionsAttempted = 0;
    clientConfig.Common.Load.LoadProfile.MaxOpenConnections = 0;
    clientConfig.Common.Load.LoadProfile.MaxTransactionsAttempted = 0;    

    clientConfig.Common.Profile.L4L7Profile.ProfileName = "TestFtpApplication";
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
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle = 2;
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.ClientName = "TestFtpApplication";
    clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.EndpointConnectionPattern = L4L7_BASE_NS::EndpointConnectionPatternOptions_BACKBONE_SRC_FIRST;
    clientConfig.ProtocolConfig.MaxTransactionsPerServer = maxTransPerServer;

    clientConfig.ServerPortInfo = mServerPortNumber;
    clientConfig.StorBodySizeType = L4L7_BASE_NS::BodySizeOptions_FIXED;
    clientConfig.StorFixedBodySize = 1024;
    clientConfig.StorRandomBodySizeMean = 1024;
    clientConfig.StorRandomBodySizeStdDeviation = 128;
    clientConfig.StorBodyContentType = L4L7_BASE_NS::BodyContentOptions_BINARY;

    if (port)
        clientConfig.ProtocolConfig.EnablePassiveDataXfer = 0 ;
    else
        clientConfig.ProtocolConfig.EnablePassiveDataXfer = 1 ;

    if (retr)
        clientConfig.ProtocolConfig.DoGetOp = true ;
    else
        clientConfig.ProtocolConfig.DoGetOp = false ;

    
}

//////////////////////////////////////////////////////////////////////////////

void TestFtpApplication::testBasicClientServer(void) 
{
	std::cerr << "\n\nInit: testBasicClientServer ....\n" ;   
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
	FtpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);
    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    const int maxClients = 1;
    const int maxTransactions = 3 ;
    for (int cycles = 0; cycles < 2; cycles++)
    {
        for (int tests = 0 ; tests < 4; tests++)
        {        
            FtpApplication::handleList_t serverHandles;
            FtpApplication::handleList_t clientHandles;

    	    std::cerr << "\n\n Starting Cycle: " << cycles + 1 << " Test: " << tests + 1 << "\n" ;
            //std::cerr << "Press ENTER any key to continue: " ;
	        //std::cin.get() ;

            // configure server block
    	   std::cerr << "configuring server....\n" ;
           FtpApplication::serverConfig_t serverConfig ;
           {
                // Configure server block
                if (cycles == 0)
                    SetupBasicServer(serverConfig, maxClients, maxTransactions) ; // only 1 client, 3 requests per client.
                else
                    SetupBasicServer(serverConfig, 0, 1) ; // Any number of clients, but only 1 request per client
         
                const FtpApplication::serverConfigList_t servers(1, serverConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
                scheduler.Enqueue(req);
                req->Wait();
           }
    	   std::cerr << "done configuring server....\n" ;
    
           CPPUNIT_ASSERT(serverHandles.size() == 1);
           CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
           // Configure client block 
           std::cerr << "configuring clients....\n" ;
           FtpApplication::clientConfig_t clientConfig;
           {
                // Configure client block            
                switch (tests)
                {
                case 0:
                    // PORT command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, true, true) ;
                    break;
                case 1:
                    // PORT command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, true, false) ;
                    break;
                case 2:
                    // PASV command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, false, true) ;
                    break;
                case 3:
                    // PASV command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, false, false) ;
                    break;

                default:
                    CPPUNIT_ASSERT("Unit test internal error" && 0) ;
                }  
                //setup the load profile          
                clientConfig.Common.Load.LoadPhases.resize(2);
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 100 ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 5 ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 10 ;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 0 ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 5 ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 1 ;              

                // configure client.
                const FtpApplication::clientConfigList_t clients(1, clientConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
                scheduler.Enqueue(req);
                req->Wait();
            }
           std::cerr << "done configuring clients....\n" ;
           CPPUNIT_ASSERT(clientHandles.size() == 1);
    
           CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);
    
    	   std::cerr << "starting server and clients....\n" ;
           // Start the server and client blocks
           {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StartProtocol, &app, 0, boost::cref(handles));
                scheduler.Enqueue(req);
                req->Wait();
            }
    
            // Let the blocks run for a bit
    	    std::cerr << "going to sleep...\n" ;    
            sleep(20); // long enough for basic load profile to complete.
    	    std::cerr << "woke up...exitting\n" ;
    
            // Stop the server and client blocks
            {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
				IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StopProtocol, &app, 0, boost::cref(handles));
				scheduler.Enqueue(req);
				req->Wait();
			}
			std::cerr << "Stopped the server...\n" ;

			// now delete the server
			std::cerr << "Deleting server...\n" ;
			{
				IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteServers, &app, 0, boost::cref(serverHandles));
				scheduler.Enqueue(req);
				req->Wait();
			}
			std::cerr << "Done deleting server...\n" ;
	
			// delete the client
			std::cerr << "Deleting client...\n" ;
			{
				IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteClients, &app, 0, boost::cref(clientHandles));
                scheduler.Enqueue(req);
				req->Wait();
			}
			std::cerr << "Done deleting client ...\n" ;
            
		}
    }

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    std::cerr << "Shutting down app...\n" ;
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

//////////////////////////////////////////////////////////////////////////////
void TestFtpApplication::testClientServerWithSinusoid(void)
{
	std::cerr << "\n\nInit: testClientServerWithSinusoid ....\n" ;  
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
	FtpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);
    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    const int maxClients = 1;
    const int maxTransactions = 3 ;
    for (int cycles = 0; cycles < 2; cycles++)
    {
        for (int tests = 0 ; tests < 4; tests++)
        {        
            FtpApplication::handleList_t serverHandles;
            FtpApplication::handleList_t clientHandles;

    	    std::cerr << "\n\n Starting Cycle: " << cycles + 1 << " Test: " << tests + 1 << "\n" ;
 
            // configure server block
    	   std::cerr << "configuring server....\n" ;
           FtpApplication::serverConfig_t serverConfig ;
           {
                // Configure server block
                if (cycles == 0)
                    SetupBasicServer(serverConfig, maxClients, maxTransactions) ; // only 1 client, 3 requests per client.
                else 
                    SetupBasicServer(serverConfig, 0, 1) ; // Any number of clients, but only 1 request per client

                const FtpApplication::serverConfigList_t servers(1, serverConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
                scheduler.Enqueue(req);
                req->Wait();
           }
    	   std::cerr << "done configuring server....\n" ;
    
           CPPUNIT_ASSERT(serverHandles.size() == 1);
           CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
           // Configure client block 
           std::cerr << "configuring clients....\n" ;
           FtpApplication::clientConfig_t clientConfig;
           {
                // Configure client block            
                switch (tests)
                {
                case 0:
                    // PORT command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, true, true) ;
                    break;
                case 1:
                    // PORT command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, true, false) ;
                    break;
                case 2:
                    // PASV command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, false, true) ;
                    break;
                case 3:
                    // PASV command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, false, false) ;
                    break;

                default:
                    CPPUNIT_ASSERT("Unit test internal error" && 0) ;
                }  
                //setup the load profile   
                clientConfig.Common.Load.LoadPhases.resize(3);
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_STAIR;
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
                clientConfig.Common.Load.LoadPhases[0].StairDescriptor.Height = 20;
                clientConfig.Common.Load.LoadPhases[0].StairDescriptor.Repetitions = 1;
                clientConfig.Common.Load.LoadPhases[0].StairDescriptor.RampTime = 5;
                clientConfig.Common.Load.LoadPhases[0].StairDescriptor.SteadyTime = 0;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_SINUSOID;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
                clientConfig.Common.Load.LoadPhases[1].SinusoidDescriptor.Height = 30;
                clientConfig.Common.Load.LoadPhases[1].SinusoidDescriptor.Repetitions = 3;
                clientConfig.Common.Load.LoadPhases[1].SinusoidDescriptor.Period = 10;
                clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPattern = L4L7_BASE_NS::LoadPatternTypes_FLAT;
                clientConfig.Common.Load.LoadPhases[2].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS;
                clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.Height = 0;
                clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.RampTime = 1;
                clientConfig.Common.Load.LoadPhases[2].FlatDescriptor.SteadyTime = 5;                                    

                // configure client.
                const FtpApplication::clientConfigList_t clients(1, clientConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
                scheduler.Enqueue(req);
                req->Wait();
            }
           std::cerr << "done configuring clients....\n" ;
           CPPUNIT_ASSERT(clientHandles.size() == 1);
    
           CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);
    
    	   std::cerr << "starting server and clients....\n" ;
           // Start the server and client blocks
           {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StartProtocol, &app, 0, boost::cref(handles));
                scheduler.Enqueue(req);
                req->Wait();
            }
    
            // Let the blocks run for a bit
    	    std::cerr << "going to sleep...\n" ;    
            if ( ((cycles + 1) % (tests + 1)) == 0 )
                sleep(30) ; // abort prematurely (i.e. w.r.t completion of load profile)
            else
                sleep(40); // long enough for basic load profile to complete.
    	    std::cerr << "woke up...exitting\n" ;
    
            // Stop the server and client blocks
            {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
		IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StopProtocol, &app, 0, boost::cref(handles));
		scheduler.Enqueue(req);
		req->Wait();
            }
            std::cerr << "Stopped the server...\n" ;

            // now delete the server
            std::cerr << "Deleting server...\n" ;
            {
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteServers, &app, 0, boost::cref(serverHandles));
		scheduler.Enqueue(req);
		req->Wait();
            }
            std::cerr << "Done deleting server...\n" ;
	
            // delete the client
            std::cerr << "Deleting client...\n" ;
            {
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteClients, &app, 0, boost::cref(clientHandles));
                scheduler.Enqueue(req);
                req->Wait();
            }
            std::cerr << "Done deleting client ...\n" ;
	}
    }

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    std::cerr << "Shutting down app...\n" ;
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}


//////////////////////////////////////////////////////////////////////////////
void TestFtpApplication::testBasicClientServerWithSingleThread(void) 
{
	std::cerr << "\n\nInit: testBasicClientServerWithSingleThread ....\n" ;    
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
	FtpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, 1) == 0);
    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    const int maxClients = 1;
    const int maxTransactions = 3 ;
    for (int cycles = 0; cycles < 2; cycles++)
    {
        for (int tests = 0 ; tests < 4; tests++)
        {        
            FtpApplication::handleList_t serverHandles;
            FtpApplication::handleList_t clientHandles;

    	    std::cerr << "\n\n Starting Cycle: " << cycles + 1 << " Test: " << tests + 1 << "\n" ;
 
            // configure server block
    	   std::cerr << "configuring server....\n" ;
           FtpApplication::serverConfig_t serverConfig ;
           {
                // Configure server block
                if (cycles == 0)
                    SetupBasicServer(serverConfig, maxClients, maxTransactions) ; // only 1 client, 3 requests per client.
                else 
                    SetupBasicServer(serverConfig, 0, 1) ; // Any number of clients, but only 1 request per client

                const FtpApplication::serverConfigList_t servers(1, serverConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigServers, &app, 0, boost::cref(servers), boost::ref(serverHandles));
                scheduler.Enqueue(req);
                req->Wait();
           }
    	   std::cerr << "done configuring server....\n" ;
    
           CPPUNIT_ASSERT(serverHandles.size() == 1);
           CPPUNIT_ASSERT(serverHandles[0] == serverConfig.ProtocolConfig.L4L7ProtocolConfigServerBase.L4L7ProtocolConfigBase.BllHandle);
    
           // Configure client block 
           std::cerr << "configuring clients....\n" ;
           FtpApplication::clientConfig_t clientConfig;
           {
                // Configure client block            
                switch (tests)
                {
                case 0:
                    // PORT command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, true, true) ;
                    break;
                case 1:
                    // PORT command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, true, false) ;
                    break;
                case 2:
                    // PASV command with RETR command
                    SetupBasicClient(clientConfig, maxTransactions, false, true) ;
                    break;
                case 3:
                    // PASV command with STOR command
                    SetupBasicClient(clientConfig, maxTransactions, false, false) ;
                    break;

                default:
                    CPPUNIT_ASSERT("Unit test internal error" && 0) ;
                }  
                //setup the load profile          
                clientConfig.Common.Load.LoadPhases.resize(2);
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
                clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 100 ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 5 ;
                clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 5 ;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
                clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 0 ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 5 ;
                clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 1 ;              

                // configure client.
                const FtpApplication::clientConfigList_t clients(1, clientConfig);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
                scheduler.Enqueue(req);
                req->Wait();
            }
           std::cerr << "done configuring clients....\n" ;
           CPPUNIT_ASSERT(clientHandles.size() == 1);
           CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);
    
    	   std::cerr << "starting server and clients....\n" ;
           // Start the server and client blocks
           {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StartProtocol, &app, 0, boost::cref(handles));
                scheduler.Enqueue(req);
                req->Wait();
            }
    
            // Let the blocks run for a bit
    	    std::cerr << "going to sleep...\n" ;    
            sleep(20); // long enough for basic load profile to complete.
    	    std::cerr << "woke up...exitting\n" ;
    
            // Stop the server and client blocks
            {
                FtpApplication::handleList_t handles;
    
                handles.push_back(serverHandles[0]);
                handles.push_back(clientHandles[0]);
            
		IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StopProtocol, &app, 0, boost::cref(handles));
		scheduler.Enqueue(req);
		req->Wait();
            }
            std::cerr << "Stopped the server...\n" ;

            // now delete the server
            std::cerr << "Deleting server...\n" ;
            {
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteServers, &app, 0, boost::cref(serverHandles));
                scheduler.Enqueue(req);
                req->Wait();
	    }
            std::cerr << "Done deleting server...\n" ;
	
            // delete the client
            std::cerr << "Deleting client...\n" ;
            {
                IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteClients, &app, 0, boost::cref(clientHandles));
                scheduler.Enqueue(req);
                req->Wait();
            }
            std::cerr << "Done deleting client ...\n" ;
        }
    }

    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    std::cerr << "Shutting down app...\n" ;
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}


///////////////////////////////////////////////////////////////////////////////

void TestFtpApplication::testNoServerMultipleClients(void) 
{
	std::cerr << "\n\nInit: testNoServerMultipleClients ....\n" ;  
    IL_DAEMON_LIB_NS::ActiveScheduler scheduler;
	FtpApplication app(1);

    // Start application
    CPPUNIT_ASSERT(scheduler.Start(1, NUM_IO_THREADS) == 0);
    {
        ACE_Reactor *appReactor = scheduler.AppReactor();
        std::vector<ACE_Reactor *> ioReactorVec(1, scheduler.IOReactor(0));
        std::vector<ACE_Lock *> ioBarrierVec(1, scheduler.IOBarrier(0));
    
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Activate, &app, appReactor, boost::cref(ioReactorVec), boost::cref(ioBarrierVec));
        scheduler.Enqueue(req);
        req->Wait();
    }

    const int maxTransactions = 3 ;

    for (int tests = 0 ; tests < 4; tests++)
    {        
        FtpApplication::handleList_t clientHandles;

  	    std::cerr << "\n\n Starting Test: " << tests + 1 << "\n" ;
 
       // Skip configuration of server block
  	   std::cerr << "configuring server....\n" ;
    
       // Configure client block 
       std::cerr << "configuring clients....\n" ;
       FtpApplication::clientConfig_t clientConfig;
       {
            // Configure client block            
            switch (tests)
            {
            case 0:
                // PORT command with RETR command
                SetupBasicClient(clientConfig, maxTransactions, true, true) ;
                break;
            case 1:
               // PORT command with STOR command
               SetupBasicClient(clientConfig, maxTransactions, true, false) ;
               break;
            case 2:
                // PASV command with RETR command
                SetupBasicClient(clientConfig, maxTransactions, false, true) ;
                break;
            case 3:
                // PASV command with STOR command
                SetupBasicClient(clientConfig, maxTransactions, false, false) ;
                break;

            default:
                CPPUNIT_ASSERT("Unit test internal error" && 0) ;
            }  
            //setup the load profile          
            clientConfig.Common.Load.LoadPhases.resize(2);
            clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
            clientConfig.Common.Load.LoadPhases[0].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
            clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.Height = 100 ;
            clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.RampTime = 5 ;
            clientConfig.Common.Load.LoadPhases[0].FlatDescriptor.SteadyTime = 5 ;
            clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPattern =  L4L7_BASE_NS::LoadPatternTypes_FLAT ;
            clientConfig.Common.Load.LoadPhases[1].LoadPhase.LoadPhaseDurationUnits = L4L7_BASE_NS::ClientLoadPhaseTimeUnitOptions_SECONDS ;
            clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.Height = 0 ;
            clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.RampTime = 5 ;
            clientConfig.Common.Load.LoadPhases[1].FlatDescriptor.SteadyTime = 0 ;              

            // configure client.
            const FtpApplication::clientConfigList_t clients(1, clientConfig);
            
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::ConfigClients, &app, 0, boost::cref(clients), boost::ref(clientHandles));
            scheduler.Enqueue(req);
            req->Wait();
       }

       std::cerr << "done configuring clients....\n" ;
       CPPUNIT_ASSERT(clientHandles.size() == 1);
    
       CPPUNIT_ASSERT(clientHandles[0] == clientConfig.ProtocolConfig.L4L7ProtocolConfigClientBase.L4L7ProtocolConfigBase.BllHandle);
    
  	   std::cerr << "starting clients (no server)....\n" ;
       // Start the server and client blocks
       {
            FtpApplication::handleList_t handles;
    
            handles.push_back(clientHandles[0]);
            
            IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StartProtocol, &app, 0, boost::cref(handles));
            scheduler.Enqueue(req);
            req->Wait();
        }
    
        // Let the blocks run for a bit
  	    std::cerr << "going to sleep...\n" ;    
        sleep(20); // long enough for basic load profile to complete.
  	    std::cerr << "woke up...exitting\n" ;
    
        // Stop the client blocks (no server)
        {
            FtpApplication::handleList_t handles;
    
            handles.push_back(clientHandles[0]);
          
			IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::StopProtocol, &app, 0, boost::cref(handles));
			scheduler.Enqueue(req);
			req->Wait();
		}
		std::cerr << "Stopped the clients...\n" ;

		// delete the client
		std::cerr << "Deleting client...\n" ;
		{
			IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::DeleteClients, &app, 0, boost::cref(clientHandles));
            scheduler.Enqueue(req);
			req->Wait();
		}
		std::cerr << "Done deleting client ...\n" ;
	}


    // Shutdown the application
    {
        IL_DAEMON_LIB_MAKE_SYNC_REQUEST(req, void, &FtpApplication::Deactivate, &app);
        scheduler.Enqueue(req);
        req->Wait();
    }
    std::cerr << "Shutting down app...\n" ;
    CPPUNIT_ASSERT(scheduler.Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestFtpApplication);

