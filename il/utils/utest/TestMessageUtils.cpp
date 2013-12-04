#include <iostream>
#include <memory>
#include <string>

#include <ace/Message_Block.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "MessageUtils.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestMessageUtils : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestMessageUtils);
    CPPUNIT_TEST(testAppend);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testAppend(void);
    void testTrim(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestMessageUtils::testAppend(void)
{
    std::tr1::shared_ptr<ACE_Message_Block> head;
    std::tr1::shared_ptr<ACE_Message_Block> tail;

    // Append to an empty head
    tail = MessageAlloc(new ACE_Message_Block(8));
    head = MessageAppend(head, tail);

    CPPUNIT_ASSERT(head);
    CPPUNIT_ASSERT(head->total_size() == 8);

    // Append to a non-empty head
    tail = MessageAlloc(new ACE_Message_Block(8));
    head = MessageAppend(head, tail);

    CPPUNIT_ASSERT(head->total_size() == 16);
}

///////////////////////////////////////////////////////////////////////////////

void TestMessageUtils::testTrim(void)
{
    std::tr1::shared_ptr<ACE_Message_Block> head;
    ACE_Message_Block *tail = 0;
    size_t i;
    
    // Build a chain of three message blocks
    for (i = 0; i < 3; i++)
    {
        std::auto_ptr<ACE_Message_Block> mb(new ACE_Message_Block(8));
        mb->wr_ptr(8);
        
        if (tail)
        {
            tail->cont(mb.release());
            tail = tail->cont();
        }
        else
        {
            head = MessageAlloc(mb.release());
            tail = head.get();
        }
    }

    CPPUNIT_ASSERT(head->total_size() == 24);

    // Trim chain so that only the tail message block remains
    head = MessageTrim(head, tail);

    CPPUNIT_ASSERT(head.get() == tail);
    CPPUNIT_ASSERT(head->total_size() == 8);

    // Trim final message block
    head = MessageTrim(head, 0);

    CPPUNIT_ASSERT(!head);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestMessageUtils);
