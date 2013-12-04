#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <Tr1Adapter.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "AgingQueue.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestAgingQueue : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestAgingQueue);
    CPPUNIT_TEST(testAgingQueue);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    class MockObject
    {
    public:
        MockObject(size_t index)
            : mIndex(index)
        {
        }

        size_t Index(void) const { return mIndex; }
        
    private:
        const size_t mIndex;
    };
    
    void testAgingQueue(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestAgingQueue::testAgingQueue(void)
{
    typedef std::tr1::shared_ptr<MockObject> objectPtr_t;
    AgingQueue<objectPtr_t> queue;
    std::vector<objectPtr_t> objects;

    CPPUNIT_ASSERT(queue.empty());
    CPPUNIT_ASSERT(queue.size() == 0);
    
    for (size_t index = 0; index < 100; index++)
    {
        objectPtr_t mockObj(new MockObject(index));
        objects.push_back(mockObj);
        queue.push(mockObj);
    }

    CPPUNIT_ASSERT(!queue.empty());
    CPPUNIT_ASSERT(queue.size() == 100);
    CPPUNIT_ASSERT(queue.front()->Index() == 0);
    CPPUNIT_ASSERT(queue.back()->Index() == 99);

    queue.erase(objects[50]);
    CPPUNIT_ASSERT(!queue.empty());
    CPPUNIT_ASSERT(queue.size() == 99);
    CPPUNIT_ASSERT(queue.front()->Index() == 0);
    CPPUNIT_ASSERT(queue.back()->Index() == 99);

    for (size_t index = 0; !queue.empty(); index++)
    {
        objectPtr_t mockObj(queue.front());
        queue.pop();

        if (index < 50)
            CPPUNIT_ASSERT(index == mockObj->Index());
        else
            CPPUNIT_ASSERT(index == (mockObj->Index() - 1));
    }

    CPPUNIT_ASSERT(queue.empty());
    CPPUNIT_ASSERT(queue.size() == 0);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestAgingQueue);
