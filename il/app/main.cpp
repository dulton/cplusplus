///
/// @file
/// @brief Main source code for the L4-L7 application daemons
///
/// Copyright (c) 2007 by Spirent Communications Inc.
/// All Rights Reserved.
///
/// This software is confidential and proprietary to Spirent Communications Inc.
/// No part of this software may be reproduced, transmitted, disclosed or used
/// in violation of the Software License Agreement without the expressed
/// written consent of Spirent Communications Inc.
///
/// $File: //TestCenter/integration/content/traffic/l4l7/il/app/main.cpp $
/// $Revision: #2 $
/// \n<b>Last submission by:</b>
/// <ul>
/// <li>$Author: byoshino $</li>
/// <li>$DateTime: 2011/09/02 19:36:56 $</li>
/// <li>$Change: 577743 $</li>
/// </ul>
///

///////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <sys/resource.h>

#include <ace/Signal.h>
#include <hal/HwInfo.h>
#include <mps/mps.h>
#include <PHXCrashHandler.h>
#include <phxlog/phxlog.h>
#include <sysmgrlib.h>
#include <vif/VifConfigFactory.h>
#include <ildaemon/ilDaemonCommon.h>

#include <ildaemon/ApplicationRegistry.h>
#include <ildaemon/CommandLineOpts.h>
#include <ildaemon/ThreadSpecificStorage.h>

///////////////////////////////////////////////////////////////////////////////

const static rlim_t MAXIMUM_OPEN_FILE_HARD_LIMIT = (32 * 1024);

///////////////////////////////////////////////////////////////////////////////

#ifdef GPROF
# include <sys/time.h>
struct itimerval GprofItimer;
#endif

//#ifdef UNIT_TEST
std::string ModuleName;
std::string LogMaskDescription;
//#endif

///////////////////////////////////////////////////////////////////////////////

class SigTermHandler : public ACE_Event_Handler
{
  public:
    SigTermHandler(void) : mCaught(false) { }

    bool Caught(void) const { return mCaught; }

  private:
    int handle_signal (int sig, siginfo_t *, ucontext_t *)
    { 
        TC_LOG_INFO(0, "Caught signal " << sig);
        mCaught = true;
        return 0;
    }

    bool mCaught;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
#ifdef GPROF
    getitimer(ITIMER_PROF, &GprofItimer);
#endif

    // Parse command line arguments
    IL_DAEMON_LIB_NS::CommandLineOpts opts;
    if (!opts.Parse(argc, argv))
        exit(-1);

    if (opts.NewMessageSetName())
    {
        // if we are running an alternate message set, 
        // we should have another module name as well
        std::string exeName = argv[0];
        size_t lastSlash = exeName.find_last_of('/');
        if (lastSlash != std::string::npos)
            exeName = exeName.substr(lastSlash + 1, std::string::npos);

        ModuleName = exeName;
    }
    
    // Setup logging
    if (opts.Debug())
    {
        PHX_LOG_REDIRECTLOG_INIT(opts.TraceFileName().c_str(), LogMaskDescription.c_str(), LOG_UPTO(opts.LogLevel()), opts.LogMask());
    }
    else
    {
        if (PHX_LOG_INIT(const_cast<char *>(ModuleName.c_str()), LogMaskDescription.c_str(), PHX_LOG_FWAM, LOG_UPTO(opts.LogLevel()), opts.LogMask()) != 0)
            exit(-1);
    }

    TC_LOG_INFO(0, "Starting up " << ModuleName << " daemon (" << argv[0] << ")");

    // Initialize system manager daemon interface
    if (opts.SysmgrDaemon())
    {
        if (sysmgr_init() < 0)
        {
            TC_LOG_ERR(0, "Failed to initialize system manager interface");
            exit(-1);
        }
    }

    // Try a bunch of eager resource initializations while we're still a single-threaded application...
    // It is better for these to fail earlier rather than later!
    TC_LOG_INFO(0, "Performing early resource initializations");

    struct rlimit fileLimits;
    getrlimit(RLIMIT_NOFILE, &fileLimits);
    if (fileLimits.rlim_cur < MAXIMUM_OPEN_FILE_HARD_LIMIT || fileLimits.rlim_max < MAXIMUM_OPEN_FILE_HARD_LIMIT)
    {
        fileLimits.rlim_cur = std::max(fileLimits.rlim_cur, MAXIMUM_OPEN_FILE_HARD_LIMIT);
        fileLimits.rlim_max = std::max(fileLimits.rlim_max, MAXIMUM_OPEN_FILE_HARD_LIMIT);

        if (setrlimit(RLIMIT_NOFILE, &fileLimits) < 0)
        {
            TC_LOG_ERR(0, "Failed to increase open file limit (RLIMIT_NOFILE) to " << MAXIMUM_OPEN_FILE_HARD_LIMIT << ": " << strerror(errno));
            exit(-1);
        }

        TC_LOG_INFO(0, "Increased open file limit (RLIMIT_NOFILE) to " << MAXIMUM_OPEN_FILE_HARD_LIMIT);
    }

    ACE_Reactor *reactor(ACE_Reactor::instance());
    TSS_Reactor.ts_object(reactor);
    TC_LOG_DEBUG(0, "Main thread is using ACE_Reactor instance " << reactor);
    
    IL_DAEMON_LIB_NS::ApplicationRegistry *appRegistry(IL_DAEMON_LIB_NS::ApplicationRegistry::Instance());
    Vif::ConfigFactory::Instance();

    TC_LOG_INFO(0, "Early resource initializations complete");
    
    // Install signal handlers to catch exits
    SigTermHandler sigHandler;
    PHXCrashHandler crashHandler(ModuleName.c_str());

    if ((reactor->register_handler(SIGTERM, &sigHandler) < 0) ||
        (reactor->register_handler(SIGHUP, &sigHandler) < 0) ||
        (reactor->register_handler(SIGINT, &sigHandler) < 0) ||
        (reactor->register_handler(SIGQUIT, &sigHandler) < 0) ||
        (PHXCrash::InstallSignalHandlers(reactor, &crashHandler) < 0))
    {   
        TC_LOG_ERR(0, "Failed to initialize signal handlers");
        exit(-1);
    }

    // Ignore SIGPIPE
    ACE_OS::signal(SIGPIPE, SIG_IGN);
    
    // Initialize MPS and register message sets
    TC_LOG_INFO(0, "Initializing MPS");

    MPS *mps(MPS::instance(ModuleName));
    if (mps->open(reactor, opts.MPSPortNum()) < 0)
    {
        TC_LOG_ERR(0, "Failed to initialize MPS");
        exit(-1);
    }

    if (reactor->register_handler(mps, ACE_Event_Handler::ACCEPT_MASK) < 0)
    {
        TC_LOG_ERR(0, "Failed to register MPS with ACE reactor");
        exit(-1);
    }

    PHX_LOG_REGISTER(MODULE_NAME, MPS::instance());

    // Initialize application objects
    const uint16_t ports = static_cast<uint16_t>(Hal::HwInfo::getPortGroupPortCount());
    if (!appRegistry->InitializeApplications(ports, opts))
    {
        TC_LOG_ERR(0, "Failed to initialize application(s)");
        exit(-1);
    }

    if (!appRegistry->RegisterApplicationsWithMPS(mps))
    {
        TC_LOG_ERR(0, "Failed to register application(s) with MPS");
        exit(-1);
    }

    // Activate application objects
    if (!appRegistry->ActivateApplications())
    {
        TC_LOG_ERR(0, "Failed to activate application(s)");
        exit(-1);
    }
    
    // Report to system manager that we're up
    if (opts.SysmgrDaemon())
    {
        if (sysmgr_report(MODULE_UP) < 0)
        {
            TC_LOG_ERR(0, "Could not report MODULE_UP status to system manager daemon");
            exit(-1);
        }
    }

    if (opts.StartUpScript())
    {
        fstream fifo(opts.StartUpFifoName().c_str());
        fifo << "OK";
    }
    
    // Run in an infinite loop
    while (!sigHandler.Caught())
    {
	try
	{
	    reactor->handle_events();
	}
        catch (const PHXException& e)
        {
            TC_LOG_ERR(0, "Caught a PHXException (" << e.Type() << ", " << e.GetErrorString() << ") in the main loop, backtrace follows:");

            for (unsigned int i = 0; i < e.Depth(); ++i)
                TC_LOG_ERR(0, "    " << e.Trace(i));
        }
        catch (const std::exception& e)
        {
            TC_LOG_ERR(0, "Caught a std::exception (" << ExceptionType(typeid(e)) << ", " << e.what() << ") in the main loop");
        }
        catch (...)
        {
            TC_LOG_ERR(0, "Caught an unknown exception in the main loop");
        }
    }

    // Report to system manager that we're down
    if (opts.SysmgrDaemon())
    {
        if (sysmgr_report(MODULE_DOWN) < 0)
            TC_LOG_ERR(0, "Could not report MODULE_DOWN status to system manager daemon");
    }

    // Deactivate application objects
    appRegistry->DeactivateApplications();

    // Unregister applications with MPS
    appRegistry->UnregisterApplicationsWithMPS(mps);

    // Destroy applicaitons
    appRegistry->DestroyApplications();

    reactor->close();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
