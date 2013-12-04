#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "LoadPhase.h"
#include "LoadProfile.h"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestLoadProfile : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestLoadProfile);
    CPPUNIT_TEST(testLoadProfileWithRampDown);
    CPPUNIT_TEST(testLoadProfileNoRampDown);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testLoadProfileWithRampDown(void);
    void testLoadProfileNoRampDown(void);

private:
    static std::auto_ptr<LoadPhase> MakeFlatPhase(int32_t height, uint32_t rampTime, uint32_t steadyTime);
    static std::auto_ptr<LoadPhase> MakeStairPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime);
    
    static const ClientLoadPhaseTimeUnitOptions SECONDS = ClientLoadPhaseTimeUnitOptions_SECONDS;
};

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadProfile::MakeFlatPhase(int32_t height, uint32_t rampTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_FLAT;
    clientLoadPhase.LoadPhaseDurationUnits = SECONDS;

    FlatPatternDescriptor_t flatDesc;
    flatDesc.Height = height;
    flatDesc.RampTime = rampTime;
    flatDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &flatDesc));
}

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<LoadPhase> TestLoadProfile::MakeStairPhase(int32_t height, uint32_t repetitions, uint32_t rampTime, uint32_t steadyTime)
{
    ClientLoadPhase_t clientLoadPhase;
    clientLoadPhase.LoadPattern = LoadPatternTypes_STAIR;
    clientLoadPhase.LoadPhaseDurationUnits = SECONDS;

    StairPatternDescriptor_t stairDesc;
    stairDesc.Height = height;
    stairDesc.Repetitions = repetitions;
    stairDesc.RampTime = rampTime;
    stairDesc.SteadyTime = steadyTime;
    
    return std::auto_ptr<LoadPhase>(new LoadPhase(clientLoadPhase, &stairDesc));
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadProfile::testLoadProfileWithRampDown(void)
{
    const ACE_Time_Value FLAT_RAMP_QUANTUM(0, 300000);
    const ACE_Time_Value STAIR_RAMP_QUANTUM(0, 50000);
    
    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    CPPUNIT_ASSERT(!prof.Active());
    
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeStairPhase(90, 10, 1, 10));
    prof.AddLoadPhase(MakeStairPhase(0, 1, 0, 60));
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 60));

    CPPUNIT_ASSERT(prof.Duration() == ACE_Time_Value(270, 0));
    
    ACE_Time_Value next;

    // Start the load profile: normal first pass simulating robust scheduler
    prof.Start();
    CPPUNIT_ASSERT(prof.Active());

    // Validate that the load starts at zero
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::zero, next) == 0);
    CPPUNIT_ASSERT(next.sec() == 10);
    CPPUNIT_ASSERT(next.usec() == 0);
    
    // Validate load at end of the first phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(10, 0), next) == 0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(10, 0) + FLAT_RAMP_QUANTUM));
    
    // Validate load at end of the second phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(40, 0), next) == 100);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(40, 0) + STAIR_RAMP_QUANTUM));
    
    // Validate load at end of the third phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(150, 0), next) == 1000);
    CPPUNIT_ASSERT(next == ACE_Time_Value(210, 0));
    
    // Validate load at end of the fourth phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(210, 0), next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(270, 0));

    // Validate load at end of the fifth phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(270, 0), next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load at max time
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::max_time, next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    prof.Stop();
    CPPUNIT_ASSERT(!prof.Active());
    
    // Restart the load profile: abbreviated second pass simulating overloaded scheduler
    prof.Start();
    CPPUNIT_ASSERT(prof.Active());

    // Validate that the load starts at zero
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::zero, next) == 0);
    CPPUNIT_ASSERT(next.sec() == 10);
    CPPUNIT_ASSERT(next.usec() == 0);
    
    // Validate load at end of the second phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(40, 0), next) == 100);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(40, 0) + STAIR_RAMP_QUANTUM));

    // Validate load at end of the fourth phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(210, 0), next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(270, 0));

    // Validate load at max time
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::max_time, next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    prof.Stop();
    CPPUNIT_ASSERT(!prof.Active());
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadProfile::testLoadProfileNoRampDown(void)
{
    const ACE_Time_Value FLAT_RAMP_QUANTUM(0, 300000);
    const ACE_Time_Value STAIR_RAMP_QUANTUM(0, 50000);

    ClientLoadProfile_t clientLoadProf;

    LoadProfile prof(clientLoadProf);
    CPPUNIT_ASSERT(!prof.Active());
    
    prof.AddLoadPhase(MakeFlatPhase(0, 0, 10));
    prof.AddLoadPhase(MakeStairPhase(100, 1, 30, 0));
    prof.AddLoadPhase(MakeStairPhase(90, 10, 1, 10));
    prof.AddLoadPhase(MakeStairPhase(0, 1, 0, 60));
    
    CPPUNIT_ASSERT(prof.Duration() == ACE_Time_Value(210, 0));
    
    ACE_Time_Value next;

    // Start the load profile: normal first pass simulating robust scheduler
    prof.Start();
    CPPUNIT_ASSERT(prof.Active());
    
    // Validate that the load starts at zero
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::zero, next) == 0);
    CPPUNIT_ASSERT(next.sec() == 10);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load at end of the first phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(10, 0), next) == 0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(10, 0) + FLAT_RAMP_QUANTUM));
    
    // Validate load at end of the second phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(40, 0), next) == 100);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(40, 0) + STAIR_RAMP_QUANTUM));
    
    // Validate load at end of the third phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(150, 0), next) == 1000);
    CPPUNIT_ASSERT(next == ACE_Time_Value(210, 0));
    
    // Validate load at end of the fourth phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(210, 0), next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load at max time
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::max_time, next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    prof.Stop();
    CPPUNIT_ASSERT(!prof.Active());
    
    // Restart the load profile: abbreviated second pass simulating overloaded scheduler
    prof.Start();
    CPPUNIT_ASSERT(prof.Active());

    // Validate that the load starts at zero
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::zero, next) == 0);
    CPPUNIT_ASSERT(next.sec() == 10);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load at end of the second phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(40, 0), next) == 100);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(40, 0) + STAIR_RAMP_QUANTUM));
    
    // Validate load at end of the fourth phase
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value(210, 0), next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load at max time
    CPPUNIT_ASSERT(prof.Eval(ACE_Time_Value::max_time, next) == 0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    prof.Stop();
    CPPUNIT_ASSERT(!prof.Active());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestLoadProfile);
