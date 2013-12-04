#include <iostream>
#include <memory>
#include <string>

#include <ace/Message_Block.h>
#include <boost/tokenizer.hpp>
#include <Tr1Adapter.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "MessageIterator.h"
#include "MessageUtils.h"

USING_L4L7_UTILS_NS;

///////////////////////////////////////////////////////////////////////////////

class TestMessageIterator : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestMessageIterator);
    CPPUNIT_TEST(testIterator);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    typedef boost::tokenizer<boost::char_separator<char>, MessageConstIterator, std::string> tokenizer_t;
    
    void testIterator(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestMessageIterator::testIterator(void)
{
    std::tr1::shared_ptr<ACE_Message_Block> head;
    ACE_Message_Block *tail = 0;
    size_t i;
    
    // Build a chain of three message blocks
    for (i = 0; i < 3; i++)
    {
        std::auto_ptr<ACE_Message_Block> mb(new ACE_Message_Block(8));

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

        switch (i)
        {
          case 0:
              memcpy(tail->wr_ptr(), "xxx\nDave", 8);
              break;

          case 1:
              memcpy(tail->wr_ptr(), "\nAbhites", 8);
              break;

          case 2:
              memcpy(tail->wr_ptr(), "h\nKate\n\n", 8);
              break;

          default:
              CPPUNIT_ASSERT(false);
              break;
        }

        tail->wr_ptr(8);
    }

    CPPUNIT_ASSERT(head->total_length() == 24);

    // Validate mutable iterator functionality
    MessageIterator iter(head.get());

    CPPUNIT_ASSERT(iter.block() == head.get());
    CPPUNIT_ASSERT(*iter == 'x');
    
    *iter++ = 'T';
    *iter++ = 'o';
    *iter++ = 'm';

    // Validate mutable iterator to const iterator conversion
    CPPUNIT_ASSERT(std::string(MessageConstIterator(head.get()), MessageConstIterator(iter)) == "Tom");

    // Validate iterator's ability to span message blocks
    boost::char_separator<char> sep("\n", "", boost::keep_empty_tokens);
    tokenizer_t tokens(MessageConstIterator(head.get()), MessageConstIterator(tail, tail->wr_ptr()), sep);

    i = 0;
    for (tokenizer_t::iterator tok = tokens.begin(); tok != tokens.end(); ++tok)
    {
        switch (i++)
        {
          case 0:
              CPPUNIT_ASSERT(*tok == "Tom");
              break;
              
          case 1:
              CPPUNIT_ASSERT(*tok == "Dave");
              break;
              
          case 2:
              CPPUNIT_ASSERT(*tok == "Abhitesh");
              break;
              
          case 3:
              CPPUNIT_ASSERT(*tok == "Kate");
              break;

          case 4:
              CPPUNIT_ASSERT(*tok == "");
              break;

          case 5:
              CPPUNIT_ASSERT(*tok == "");
              break;

          default:
              CPPUNIT_ASSERT(false);
              break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestMessageIterator);
