#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "LoadPatternImpl.tcc"

USING_L4L7_BASE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestLoadPatternImpl : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestLoadPatternImpl);
    CPPUNIT_TEST(testFlatLoadPattern);
    CPPUNIT_TEST(testStairLoadPattern);
    CPPUNIT_TEST(testBurstLoadPattern);
    CPPUNIT_TEST(testSinusoidLoadPattern);
    CPPUNIT_TEST(testRandomLoadPattern);
    CPPUNIT_TEST(testSawToothLoadPattern);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testFlatLoadPattern(void);
    void testStairLoadPattern(void);
    void testBurstLoadPattern(void);
    void testSinusoidLoadPattern(void);
    void testRandomLoadPattern(void);
    void testSawToothLoadPattern(void);

private:
    static const ClientLoadPhaseTimeUnitOptions SECONDS = ClientLoadPhaseTimeUnitOptions_SECONDS;
};

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testFlatLoadPattern(void)
{
    static const ACE_Time_Value RAMP_QUANTUM(0, 50000);
    static const int32_t Y0 = 1000;
    
    FlatPatternDescriptor_t desc;
    desc.Height = 100;
    desc.RampTime = 10;
    desc.SteadyTime = 50;
    
    FlatLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(60, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);
    CPPUNIT_ASSERT(next == RAMP_QUANTUM);
    
    // Validate load mid-way through ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(5, 0), next) == 550);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(5, 0) + RAMP_QUANTUM));

    // Validate load at the end of ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(10, 0), next) == 100);
    CPPUNIT_ASSERT(next.sec() == 60);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load mid-way through steady time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(35, 0), next) == 100);
    CPPUNIT_ASSERT(next.sec() == 60);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load at the end of steady time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(60, 0), next) == 100);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of steady time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(60, 1), next) == 100);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testStairLoadPattern(void)
{
    static const ACE_Time_Value RAMP_QUANTUM(0, 100000);
    static const int32_t HEIGHT = 100;
    static const int32_t Y0 = 1000;
    
    StairPatternDescriptor_t desc;
    desc.Height = HEIGHT;
    desc.Repetitions = 4;
    desc.RampTime = 10;
    desc.SteadyTime = 50;

    StairLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(240, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);
    CPPUNIT_ASSERT(next == RAMP_QUANTUM);
    
    // Validate load mid-way through the first step's ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(5, 0), next) == (Y0 + (HEIGHT / 2)));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(5, 0) + RAMP_QUANTUM));
    
    // Validate load immediately at the first step
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(10, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next.sec() == 60);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load mid-way through the first step's steady time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(35, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next.sec() == 60);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load mid-way through the second step's ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(65, 0), next) == (Y0 + (HEIGHT * 3 / 2)));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(65, 0) + RAMP_QUANTUM));
    
    // Validate load immediately at the second step
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(70, 0), next) == (Y0 + (HEIGHT * 2)));
    CPPUNIT_ASSERT(next.sec() == 120);
    CPPUNIT_ASSERT(next.usec() == 0);

    // Validate load mid-way through the second step's steady time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(95, 0), next) == (Y0 + (HEIGHT * 2)));
    CPPUNIT_ASSERT(next.sec() == 120);
    CPPUNIT_ASSERT(next.usec() == 0);
    
    // Validate load at the end of the staircase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 0), next) == (Y0 + (HEIGHT * 4)));
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of the staircase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 1), next) == (Y0 + (HEIGHT * 4)));
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testBurstLoadPattern(void)
{
    static const ACE_Time_Value RAMP_QUANTUM(0, 200000);
    static const int32_t HEIGHT = 100;
    static const int32_t Y0 = 1000;
    
    BurstPatternDescriptor_t desc;
    desc.Height = HEIGHT;
    desc.Repetitions = 4;
    desc.BurstTime = 40;
    desc.PauseTime = 20;

    BurstLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(240, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);
    CPPUNIT_ASSERT(next == RAMP_QUANTUM);
    
    // Validate load mid-way through the first burst's up-ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(10, 0), next) == (Y0 + (HEIGHT / 2)));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(10, 0) + RAMP_QUANTUM));
    
    // Validate load at the peak of the first burst
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(20, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(20, 0) + RAMP_QUANTUM));

    // Validate load mid-way through the first burst's down-ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(30, 0), next) == (Y0 + (HEIGHT / 2)));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(30, 0) + RAMP_QUANTUM));

    // Validate load mid-way through the first burst's pause time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(50, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(60, 0));
    
    // Validate load mid-way through the second burst's up-ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(70, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(70, 0) + (RAMP_QUANTUM * 0.5)));
    
    // Validate load at the peak of the second burst
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(80, 0), next) == (Y0 + (HEIGHT * 2)));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(80, 0) + (RAMP_QUANTUM * 0.5)));

    // Validate load mid-way through the second burst's down-ramp time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(90, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(90, 0) + (RAMP_QUANTUM * 0.5)));
    
    // Validate load mid-way through the second burst's pause time
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(110, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(120, 0));
    
    // Validate load at the end of the bursts
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of the bursts
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 1), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testSinusoidLoadPattern(void)
{
    static const ACE_Time_Value WAVE_QUANTUM(0, 50000);
    static const int32_t HEIGHT = 100;
    static const int32_t Y0 = 1000;
    
    SinusoidPatternDescriptor_t desc;
    desc.Height = HEIGHT;
    desc.Repetitions = 4;
    desc.Period = 60;

    SinusoidLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(240, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);
    CPPUNIT_ASSERT(next == WAVE_QUANTUM);
    
    // Validate load at the peak of the first wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(15, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(15, 0) + WAVE_QUANTUM));

    // Validate load at the mid-point of the first wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(30, 0), next) == Y0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(30, 0) + WAVE_QUANTUM));
    
    // Validate load at the trough of the first wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(45, 0), next) == (Y0 - HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(45, 0) + WAVE_QUANTUM));
    
    // Validate load at the end-point of the first wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(60, 0), next) == Y0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(60, 0) + WAVE_QUANTUM));
    
    // Validate load at the peak of the second wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(75, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(75, 0) + WAVE_QUANTUM));

    // Validate load at the mid-point of the second wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(90, 0), next) == Y0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(90, 0) + WAVE_QUANTUM));
    
    // Validate load at the trough of the second wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(105, 0), next) == (Y0 - HEIGHT));
    CPPUNIT_ASSERT(next == (ACE_Time_Value(105, 0) + WAVE_QUANTUM));
    
    // Validate load at the end-point of the second wave
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(120, 0), next) == Y0);
    CPPUNIT_ASSERT(next == (ACE_Time_Value(120, 0) + WAVE_QUANTUM));
    
    // Validate load at the end of the waveform
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of the waveform
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 1), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testRandomLoadPattern(void)
{
    static const int32_t HEIGHT = 100;
    static const int32_t Y0 = 1000;
    
    RandomPatternDescriptor_t desc;
    desc.Height = HEIGHT;
    desc.Repetitions = 100;
    desc.RampTime = 30;
    desc.SteadyTime = 30;

    RandomLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(6000, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);

    // Validate load at the end of each repetition
    for (uint32_t rep = 0; rep < desc.Repetitions; rep++)
    {
        const int32_t load = patt.Eval(ACE_Time_Value(60, 0) * rep, next);
        CPPUNIT_ASSERT(load >= Y0);
        CPPUNIT_ASSERT(load <= (Y0 + HEIGHT));
    }
    
    // Validate load at the end of the pattern
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(6000, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of the pattern
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(6000, 1), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

void TestLoadPatternImpl::testSawToothLoadPattern(void)
{
    static const int32_t HEIGHT = 100;
    static const int32_t Y0 = 1000;
    
    SawToothPatternDescriptor_t desc;
    desc.Height = HEIGHT;
    desc.Repetitions = 4;
    desc.PauseTime = 30;
    desc.SteadyTime = 30;

    SawToothLoadPattern patt(desc, SECONDS);
    patt.SetYIntercept(Y0);

    CPPUNIT_ASSERT(patt.Duration() == ACE_Time_Value(240, 0));
    
    ACE_Time_Value next;

    // Validate Y-intercept
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value::zero, next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(30, 0));
    
    // Validate load at the mid-point of the first tooth's pause phase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(15, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(30, 0));

    // Validate load at the mid-point of the first tooth's steady phase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(45, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == ACE_Time_Value(60, 0));
    
    // Validate load at the mid-point of the second tooth's pause phase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(75, 0), next) == Y0);
    CPPUNIT_ASSERT(next == ACE_Time_Value(90, 0));

    // Validate load at the mid-point of the second tooth's steady phase
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(105, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == ACE_Time_Value(120, 0));
    
    // Validate load at the end of the pattern
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 0), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);

    // Validate load immediately after the end of the pattern
    CPPUNIT_ASSERT(patt.Eval(ACE_Time_Value(240, 1), next) == (Y0 + HEIGHT));
    CPPUNIT_ASSERT(next == ACE_Time_Value::zero);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestLoadPatternImpl);
