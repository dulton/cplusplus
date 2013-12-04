#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "SharedMap.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSharedMap : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestSharedMap);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testForeach);
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

        void Tweak() { mIndex += 100; }
        
    private:
        size_t mIndex;
    };

    void testBasic(void);
    void testForeach(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestSharedMap::testBasic(void)
{
    typedef SharedMap<int, MockObject> map_t;
    typedef map_t::shared_ptr_t shared_ptr_t;
    map_t map;
    std::vector<shared_ptr_t> objects;

    CPPUNIT_ASSERT(map.empty());
    CPPUNIT_ASSERT(map.size() == 0);
    
    for (size_t index = 0; index < 100; index++)
    {
        shared_ptr_t mockObj(new MockObject(index));
        objects.push_back(mockObj);
        map.insert(index, mockObj);
    }

    CPPUNIT_ASSERT(!map.empty());
    CPPUNIT_ASSERT(map.size() == 100);
    CPPUNIT_ASSERT(map.find(0).get() == objects[0].get());
    CPPUNIT_ASSERT(map.find(99).get() == objects[99].get());

    CPPUNIT_ASSERT(map.erase(50) == 1);
    CPPUNIT_ASSERT(map.erase(50) == 0);
    CPPUNIT_ASSERT(!map.empty());
    CPPUNIT_ASSERT(map.size() == 99);
    CPPUNIT_ASSERT(map.find(0).get() == objects[0].get());
    CPPUNIT_ASSERT(map.find(50).get() == 0);
    CPPUNIT_ASSERT(map.find(99).get() == objects[99].get());
}

///////////////////////////////////////////////////////////////////////////////

void TestSharedMap::testForeach(void)
{
    typedef SharedMap<int, MockObject> map_t;
    typedef map_t::shared_ptr_t shared_ptr_t;
    map_t map;

    for (size_t index = 0; index < 100; index++)
    {
        shared_ptr_t mockObj(new MockObject(index));
        map.insert(index, mockObj);
    }

    // verify initial values
    for (size_t index = 0; index < 100; index++)
    {
        CPPUNIT_ASSERT_EQUAL(index, map.find(index)->Index());
    }

    // tweak all (adds 100)
    map_t::delegate_t delegate(boost::bind(&MockObject::Tweak, _1));
    map.foreach(delegate);
   
    // verify final values
    for (size_t index = 0; index < 100; index++)
    {
        CPPUNIT_ASSERT_EQUAL(index + 100, map.find(index)->Index());
    }
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSharedMap);
