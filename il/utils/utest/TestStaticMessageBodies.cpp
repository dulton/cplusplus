#include <iostream>
#include <memory>
#include <string>

#include <Tr1Adapter.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "AppOctetStreamMessageBody.h"
#include "MessageIterator.h"
#include "MessageUtils.h"
#include "TextHtmlMessageBody.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestStaticMessageBodies : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestStaticMessageBodies);
    CPPUNIT_TEST(testAppOctetStreamMessageBody);
    CPPUNIT_TEST(testTextHtmlMessageBody);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testAppOctetStreamMessageBody(void);
    void testTextHtmlMessageBody(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestStaticMessageBodies::testAppOctetStreamMessageBody(void)
{
    // Test empty message body
    std::tr1::shared_ptr<ACE_Message_Block> body(MessageAlloc(new AppOctetStreamMessageBody(0)));
    CPPUNIT_ASSERT(body->total_length() == 0);

    // Test single block message body
    body = MessageAlloc(new AppOctetStreamMessageBody(16));
    CPPUNIT_ASSERT(body->total_length() == 16);

    {
        MessageConstIterator pos = MessageBegin(body.get());
        MessageConstIterator end = MessageEnd(body.get());

        while (pos != end)
        {
            CPPUNIT_ASSERT(*pos == '*');
            ++pos;
        }
    }

    // Test multiple block message body
    body = MessageAlloc(new AppOctetStreamMessageBody(16000));
    CPPUNIT_ASSERT(body->total_length() == 16000);

    {
        MessageConstIterator pos = MessageBegin(body.get());
        MessageConstIterator end = MessageEnd(body.get());

        while (pos != end)
        {
            CPPUNIT_ASSERT(*pos == '*');
            ++pos;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void TestStaticMessageBodies::testTextHtmlMessageBody(void)
{
    // Test empty message body
    std::tr1::shared_ptr<ACE_Message_Block> body(MessageAlloc(new TextHtmlMessageBody(0)));
    CPPUNIT_ASSERT(body->total_length() == 0);

    // Test single block message body
    body = MessageAlloc(new TextHtmlMessageBody(16));
    CPPUNIT_ASSERT(body->total_length() == 16);

    {
        MessageConstIterator pos = MessageBegin(body.get());
        MessageConstIterator end = MessageEnd(body.get());

        while (pos != end)
        {
            CPPUNIT_ASSERT(*pos == '*');
            ++pos;
        }
    }

    // Test multiple block message body
    body = MessageAlloc(new TextHtmlMessageBody(16000));
    CPPUNIT_ASSERT(body->total_length() == 16000);

    {
        MessageConstIterator begin = MessageBegin(body.get());
        MessageConstIterator end = MessageEnd(body.get());
        MessageConstIterator pos = begin;
        size_t starCount = 0;

        while (*pos != '*')
            ++pos;

        CPPUNIT_ASSERT(std::string(begin, pos) == "<html><pre>");

        while (*pos == '*')
        {
            ++pos;
            ++starCount;
        }

        CPPUNIT_ASSERT(starCount == (16000 - 26));
        CPPUNIT_ASSERT(std::string(pos, end) == "</pre></html>\r\n");
    }
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestStaticMessageBodies);
