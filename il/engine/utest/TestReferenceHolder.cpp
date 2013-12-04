#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "ReferenceHolder.h"

USING_L4L7_ENGINE_NS;

///////////////////////////////////////////////////////////////////////////////

class TestReferenceHolder : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestReferenceHolder);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST_SUITE_END();

public:
    TestReferenceHolder()
    {
    }

    ~TestReferenceHolder()
    {
    }

    void setUp() 
    { 
    }

    void tearDown() 
    { 
    }
    
protected:
    void testBasic();

private:
};

///////////////////////////////////////////////////////////////////////////////

class MockReferenceCounted
{
  public:
    MockReferenceCounted() : mRefCount(1)    
    {
    }

    void AddReference()
    {
        ++mRefCount;
    }

    void RemoveReference()
    {
        --mRefCount; // don't delete
    }

    uint32_t GetRefCount()
    {
        return mRefCount;
    }

  private:
    uint32_t mRefCount;
};

///////////////////////////////////////////////////////////////////////////////

void TestReferenceHolder::testBasic()
{
    MockReferenceCounted mock;

    CPPUNIT_ASSERT_EQUAL(uint32_t(1), mock.GetRefCount());

    {
        ReferenceHolder<MockReferenceCounted> holder(mock);
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), mock.GetRefCount());
    }

    CPPUNIT_ASSERT_EQUAL(uint32_t(1), mock.GetRefCount());
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestReferenceHolder);
