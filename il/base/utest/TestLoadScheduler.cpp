#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include <ace/Reactor.h>
#include <ace/Time_Value.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "LoadPhase.h"
#include "LoadProfile.h"
#include "LoadScheduler.h"
#include "LoadStrategy.h"

USING_L4L7_BASE_NS;

#undef HIGH_DEFINITION_OUTPUT

///////////////////////////////////////////////////////////////////////////////

class TestLoadScheduler : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestLoadScheduler);
    CPPUNIT_TEST(testLoadSchedulerWithStairProfile);
    CPPUNIT_TEST(testLoadSchedulerWithBurstProfile);
    CPPUNIT_TEST(testLoadSchedulerWithSinusoidProfile);
    CPPUNIT_TEST(testLoadSchedulerWithRandomProfile);
    CPPUNIT_TEST(testLoadSchedulerWithSawToothProfile);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testLoadSchedulerWithStairProfile(void);
    void testLoadSchedulerWithBurstProfile(void);
    void testLoadSchedulerWithSinusoidProfile(void);
    void testLoadSchedulerWithRandomProfile(void);
    void testLoadSchedulerWithSawToothProfile(void);

private:
    class MockStrategy;
    
    static std::auto_ptr<LoadPhase> MakeFlatPhase(int32_t height, uint32_t rampTime, uint32_t steadyTime);
    static std::auto_ptr<LoadPhase> MakeStairPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime);
    static std::auto_ptr<LoadPhase> MakeBurstPhase(int32_t height, uint32_t repetitions, uint32_t burstTime, uint32_t pauseTime);
    static std::auto_ptr<LoadPhase> MakeSinusoidPhase(int32_t height, uint32_t repetitions, uint32_t period);
    static std::auto_ptr<LoadPhase> MakeRandomPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime);
    static std::auto_ptr<LoadPhase> MakeSawToothPhase(int32_t height, uint32_t repetitions, uint32_t pauseTime, uint32_t steadyTime);
    
#ifdef HIGH_DEFINITION_OUTPUT
    static const ClientLoadPhaseTimeUnitOptions TIME_UNITS = ClientLoadPhaseTimeUnitOptions_SECONDS;
#else    
    static const ClientLoadPhaseTimeUnitOptions TIME_UNITS = ClientLoadPhaseTimeUnitOptions_MILLISECONDS;
#endif    
};

///////////////////////////////////////////////////////////////////////////////

class TestLoadScheduler::MockStrategy : public LoadStrategy
{
public:
    MockStrategy(const std::string& fileName)
        : mFileName(fileName)
    {
    }

    virtual ~MockStrategy() { }
    
    void Start(ACE_Reactor *reactor, uint32_t hz)
    {
    }
    
    void Stop(void)
    {
    }
    
    void SetLoad(int32_t load)
    {
        const ACE_Time_Value now(ACE_OS::gettimeofday());

        if (mStartTime == ACE_Time_Value::zero)
            mStartTime = now;

        const ACE_Time_Value delta(now - mStartTime);

        if (!mFileStr.is_open())
        {
            const std::string csvOutputPath(OUTPUT_PATH + mFileName + ".csv");
            mFileStr.open(csvOutputPath.c_str(), std::fstream::out);
        }

        if (mFileStr.is_open())
            mFileStr << delta.sec() << "." << std::setfill('0') << std::setw(6) << delta.usec() << ", " << load << "\n";
    }

    void Plot(void)
    {
        mFileStr.close();

#ifndef QUICK_TEST
        std::stringstream ss;
        ss << "echo 'set term png small;"
           << "set output \"" << OUTPUT_PATH << mFileName << ".png\";"
           << "plot \"" << OUTPUT_PATH << mFileName << ".csv\" with steps'"
           << "| gnuplot - > /dev/null 2>&1";

        int result = system(ss.str().c_str());
        CPPUNIT_ASSERT_MESSAGE("Failed to run gnuplot - perhaps it's not available on this system?", result == 0);
#endif
    }
    
private:
    static const std::string OUTPUT_PATH;
    
    std::string mFileName;
    std::fstream mFileStr;
    ACE_Time_Value mStartTime;
};

///////////////////////////////////////////////////////////////////////////////

const std::string TestLoadScheduler::MockStrategy::OUTPUT_PATH("content/traffic/l4l7/il/base/utest/");

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeFlatPhase(int32_t height, uint32_t rampTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_FLAT;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    FlatPatternDescriptor_t flatDesc;
    flatDesc.Height = height;
    flatDesc.RampTime = rampTime;
    flatDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &flatDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeStairPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_STAIR;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    StairPatternDescriptor_t stairDesc;
    stairDesc.Height = height;
    stairDesc.Repetitions = repetitions;
    stairDesc.RampTime = rampTime;
    stairDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &stairDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeBurstPhase(int32_t height, uint32_t repetitions, uint32_t burstTime, uint32_t pauseTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_BURST;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    BurstPatternDescriptor_t burstDesc;
    burstDesc.Height = height;
    burstDesc.Repetitions = repetitions;
    burstDesc.BurstTime = burstTime;
    burstDesc.PauseTime = pauseTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &burstDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeSinusoidPhase(int32_t height, uint32_t repetitions, uint32_t period)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_SINUSOID;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    SinusoidPatternDescriptor_t sinDesc;
    sinDesc.Height = height;
    sinDesc.Repetitions = repetitions;
    sinDesc.Period = period;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &sinDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeRandomPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_RANDOM;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    RandomPatternDescriptor_t rndDesc;
    rndDesc.Height = height;
    rndDesc.Repetitions = repetitions;
    rndDesc.RampTime = rampTime;
    rndDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &rndDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadScheduler::MakeSawToothPhase(int32_t height, uint32_t repetitions, uint32_t pauseTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_SAWTOOTH;
    clientLoadPhase.LoadPhaseDurationUnits = TIME_UNITS;

    SawToothPatternDescriptor_t sawDesc;
    sawDesc.Height = height;
    sawDesc.Repetitions = repetitions;
    sawDesc.PauseTime = pauseTime;
    sawDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &sawDesc));
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadScheduler::testLoadSchedulerWithStairProfile(void)
{
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeStairPhase(90, 10, 1, 10));
    prof.AddLoadPhase(MakeStairPhase(0, 1, 0, 60));
    prof.AddLoadPhase(MakeFlatPhase(0, 1, 59));

    MockStrategy strat("TestLoadSchedulerWithStairProfile");
    LoadScheduler sched(prof, strat);

    ACE_Reactor reactor;
    sched.Start(&reactor);

    ACE_Time_Value runTime(prof.Duration());
    while (runTime != ACE_Time_Value::zero)
        reactor.handle_events(runTime);
    
    sched.Stop();
    strat.Plot();
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadScheduler::testLoadSchedulerWithBurstProfile(void)
{
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeBurstPhase(90, 5, 110, 10));
    prof.AddLoadPhase(MakeFlatPhase(0, 1, 59));

    MockStrategy strat("TestLoadSchedulerWithBurstProfile");
    LoadScheduler sched(prof, strat);

    ACE_Reactor reactor;
    sched.Start(&reactor);

    ACE_Time_Value runTime(prof.Duration());
    while (runTime != ACE_Time_Value::zero)
        reactor.handle_events(runTime);
    
    sched.Stop();
    strat.Plot();
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadScheduler::testLoadSchedulerWithSinusoidProfile(void)
{
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeSinusoidPhase(50, 6, 100));
    prof.AddLoadPhase(MakeFlatPhase(0, 1, 59));

    MockStrategy strat("TestLoadSchedulerWithSinusoidProfile");
    LoadScheduler sched(prof, strat);

    ACE_Reactor reactor;
    sched.Start(&reactor);

    ACE_Time_Value runTime(prof.Duration());
    while (runTime != ACE_Time_Value::zero)
        reactor.handle_events(runTime);
    
    sched.Stop();
    strat.Plot();
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadScheduler::testLoadSchedulerWithRandomProfile(void)
{
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeRandomPhase(50, 30, 10, 10));
    prof.AddLoadPhase(MakeFlatPhase(0, 1, 59));

    MockStrategy strat("TestLoadSchedulerWithRandomProfile");
    LoadScheduler sched(prof, strat);

    ACE_Reactor reactor;
    sched.Start(&reactor);

    ACE_Time_Value runTime(prof.Duration());
    while (runTime != ACE_Time_Value::zero)
        reactor.handle_events(runTime);
    
    sched.Stop();
    strat.Plot();
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadScheduler::testLoadSchedulerWithSawToothProfile(void)
{
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeSawToothPhase(50, 4, 30, 30));
    prof.AddLoadPhase(MakeFlatPhase(0, 1, 59));

    MockStrategy strat("TestLoadSchedulerWithSawToothProfile");
    LoadScheduler sched(prof, strat);

    ACE_Reactor reactor;
    sched.Start(&reactor);

    ACE_Time_Value runTime(prof.Duration());
    while (runTime != ACE_Time_Value::zero)
        reactor.handle_events(runTime);
    
    sched.Stop();
    strat.Plot();
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestLoadScheduler);
