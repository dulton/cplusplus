#include <memory>
#include <string>

#include <ace/Message_Block.h>
#include <utils/MessageIterator.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "RawTcpProtocol.h"

USING_L4L7_UTILS_NS;
USING_RAWTCP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestRawTcpProtocol : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestRawTcpProtocol);
    CPPUNIT_TEST(testBuildRequestLine);
    CPPUNIT_TEST(testParseRequestLine);
    CPPUNIT_TEST(testBuildStatusLine);
    CPPUNIT_TEST(testParseStatusLine);
    CPPUNIT_TEST(testBuildHeaderLine);
    CPPUNIT_TEST(testParseHeaderLine);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testBuildRequestLine(void);
    void testParseRequestLine(void);
    void testBuildStatusLine(void);
    void testParseStatusLine(void);
    void testBuildHeaderLine(void);
    void testParseHeaderLine(void);

private:
    std::auto_ptr<ACE_Message_Block> makeMessage(const std::string& str);
};

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<ACE_Message_Block> TestRawTcpProtocol::makeMessage(const std::string& str)
{
    std::auto_ptr<ACE_Message_Block> mb(new ACE_Message_Block(str.size()));
    mb->copy(str.data(), str.size());
    return mb;
}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testBuildRequestLine(void)
{
    // Test for "Test1 5"
    CPPUNIT_ASSERT(RawTcpProtocol::BuildRequestLine(5) == "SPIRENT_RAW_TCP_CLIENT_REQ 5\r\n");

    // Test for "SPIRENT_RAW_TCP 100"
    CPPUNIT_ASSERT(RawTcpProtocol::BuildRequestLine(100) == "SPIRENT_RAW_TCP_CLIENT_REQ 100\r\n");

    // Test "SPIRENT_RAW_TCP 9999"
    CPPUNIT_ASSERT(RawTcpProtocol::BuildRequestLine(9999) == "SPIRENT_RAW_TCP_CLIENT_REQ 9999\r\n");;
}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testParseRequestLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    RawTcpProtocol::MethodType method;
    uint32_t requestNum;


    // Test blank lines
    line = makeMessage("");
    CPPUNIT_ASSERT(RawTcpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, requestNum));
    CPPUNIT_ASSERT(method == RawTcpProtocol::RAWTCP_METHOD_BLANK_LINE);

    line = makeMessage("\r\n");
    CPPUNIT_ASSERT(RawTcpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, requestNum));
    CPPUNIT_ASSERT(method == RawTcpProtocol::RAWTCP_METHOD_BLANK_LINE);
    
    // Test SPIRENT_RAW_TCP_CLIENT_REQ 0
    line = makeMessage("SPIRENT_RAW_TCP_CLIENT_REQ 0\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, requestNum));
    CPPUNIT_ASSERT(method == RawTcpProtocol::RAWTCP_METHOD_REQUEST);
    CPPUNIT_ASSERT(requestNum == 0);
                   
    // Test SPIRENT_RAW_TCP_CLIENT_REQ 199
    line = makeMessage("SPIRENT_RAW_TCP_CLIENT_REQ 199\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, requestNum));
    CPPUNIT_ASSERT(method == RawTcpProtocol::RAWTCP_METHOD_REQUEST);
    CPPUNIT_ASSERT(requestNum == 199);
    
    // Test garbage
    line = makeMessage("sdfsdfas asfasfas");
    CPPUNIT_ASSERT(!RawTcpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, requestNum));
}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testBuildStatusLine(void)
{
    // Test SPIRENT_RAW_TCP_CLIENT_REQ
    CPPUNIT_ASSERT(RawTcpProtocol::BuildStatusLine(RawTcpProtocol::RAWTCP_METHOD_REQUEST, 1) == "SPIRENT_RAW_TCP_CLIENT_REQ 1\r\n");
    CPPUNIT_ASSERT(RawTcpProtocol::BuildStatusLine(RawTcpProtocol::RAWTCP_METHOD_REQUEST, 201) == "SPIRENT_RAW_TCP_CLIENT_REQ 201\r\n");

    // Test SPIRENT_RAW_TCP_SERVER_RESP
    CPPUNIT_ASSERT(RawTcpProtocol::BuildStatusLine(RawTcpProtocol::RAWTCP_METHOD_RESPONSE, 6) == "SPIRENT_RAW_TCP_SERVER_RESP 6\r\n");
    CPPUNIT_ASSERT(RawTcpProtocol::BuildStatusLine(RawTcpProtocol::RAWTCP_METHOD_RESPONSE, 26) == "SPIRENT_RAW_TCP_SERVER_RESP 26\r\n");
}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testParseStatusLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    uint32_t requestNum;
  
    // Test SPIRENT_RAW_TCP_CLIENT_REQ 1999
    line = makeMessage("SPIRENT_RAW_TCP_SERVER_RESP 1999\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), requestNum));
    CPPUNIT_ASSERT(requestNum == 1999);
                   
    // Test SPIRENT_RAW_TCP_CLIENT_REQ 21999
    line = makeMessage("SPIRENT_RAW_TCP_SERVER_RESP 21999\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), requestNum));
    CPPUNIT_ASSERT(requestNum == 21999);

    // Test garbage
    line = makeMessage("SPIRENT_RAW_TCP_CLIENT_REQ 26");
    CPPUNIT_ASSERT(!RawTcpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), requestNum));

    line = makeMessage("sdfsdfsd 1");
    CPPUNIT_ASSERT(!RawTcpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), requestNum));

}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testBuildHeaderLine(void)
{
    // Test various header lines
    CPPUNIT_ASSERT(RawTcpProtocol::BuildHeaderLine(RawTcpProtocol::RAWTCP_HEADER_LENGTH, 64) == "Length: 64\r\n");
    CPPUNIT_ASSERT(RawTcpProtocol::BuildHeaderLine(RawTcpProtocol::RAWTCP_HEADER_LENGTH, 2048) == "Length: 2048\r\n");

}

///////////////////////////////////////////////////////////////////////////////

void TestRawTcpProtocol::testParseHeaderLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    RawTcpProtocol::HeaderType header;
    std::string value;

    // Test blank lines
    line = makeMessage("");
    CPPUNIT_ASSERT(RawTcpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == RawTcpProtocol::RAWTCP_HEADER_BLANK_LINE);
    CPPUNIT_ASSERT(value.empty());
    
    line = makeMessage("\r\n");
    CPPUNIT_ASSERT(RawTcpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == RawTcpProtocol::RAWTCP_HEADER_BLANK_LINE);
    CPPUNIT_ASSERT(value.empty());
    
    // Test headers with various spacing and capitalizations
    line = makeMessage("LeNgTh: 1\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == RawTcpProtocol::RAWTCP_HEADER_LENGTH);
    CPPUNIT_ASSERT(value == "1");
                   
    line = makeMessage("Length:\t64\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == RawTcpProtocol::RAWTCP_HEADER_LENGTH);
    CPPUNIT_ASSERT(value == "64");

    line = makeMessage("Length: 128\r\n");

    CPPUNIT_ASSERT(RawTcpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == RawTcpProtocol::RAWTCP_HEADER_LENGTH);
    CPPUNIT_ASSERT(value == "128");

}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestRawTcpProtocol);
