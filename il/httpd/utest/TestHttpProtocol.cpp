#include <memory>
#include <string>

#include <ace/Message_Block.h>
#include <utils/MessageIterator.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "HttpProtocol.h"

USING_L4L7_UTILS_NS;
USING_HTTP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestHttpProtocol : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestHttpProtocol);
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

std::auto_ptr<ACE_Message_Block> TestHttpProtocol::makeMessage(const std::string& str)
{
    std::auto_ptr<ACE_Message_Block> mb(new ACE_Message_Block(str.size()));
    mb->copy(str.data(), str.size());
    return mb;
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testBuildRequestLine(void)
{
    // Test HTTP/0.9 request line
    CPPUNIT_ASSERT(HttpProtocol::BuildRequestLine(HttpProtocol::HTTP_METHOD_GET, "/index.html", HttpProtocol::HTTP_VER_0_9) == "GET /index.html HTTP/0.9\r\n");

    // Test HTTP/1.0 request line
    CPPUNIT_ASSERT(HttpProtocol::BuildRequestLine(HttpProtocol::HTTP_METHOD_POST, "/foo.html", HttpProtocol::HTTP_VER_1_0) == "POST /foo.html HTTP/1.0\r\n");

    // Test HTTP/1.1 request line
    CPPUNIT_ASSERT(HttpProtocol::BuildRequestLine(HttpProtocol::HTTP_METHOD_HEAD, "/bar.html", HttpProtocol::HTTP_VER_1_1) == "HEAD /bar.html HTTP/1.1\r\n");
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testParseRequestLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    HttpProtocol::MethodType method;
    std::string uri;
    HttpProtocol::Version version;

    // Test blank lines
    line = makeMessage("");
    CPPUNIT_ASSERT(HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
    CPPUNIT_ASSERT(method == HttpProtocol::HTTP_METHOD_BLANK_LINE);
    CPPUNIT_ASSERT(uri.empty());
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_UNKNOWN);

    line = makeMessage("\r\n");
    CPPUNIT_ASSERT(HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
    CPPUNIT_ASSERT(method == HttpProtocol::HTTP_METHOD_BLANK_LINE);
    CPPUNIT_ASSERT(uri.empty());
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_UNKNOWN);
    
    // Test HTTP/0.9 GET
    line = makeMessage("GET /foo HTTP/0.9\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
    CPPUNIT_ASSERT(method == HttpProtocol::HTTP_METHOD_GET);
    CPPUNIT_ASSERT(uri == "/foo");
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_0_9);
                   
    // Test HTTP/1.0 GET
    line = makeMessage("GET /bar HTTP/01.00\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
    CPPUNIT_ASSERT(method == HttpProtocol::HTTP_METHOD_GET);
    CPPUNIT_ASSERT(uri == "/bar");
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_1_0);
    
    // Test HTTP/1.1 GET
    line = makeMessage("GET /blatz HTTP/1.1\r");

    CPPUNIT_ASSERT(HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
    CPPUNIT_ASSERT(method == HttpProtocol::HTTP_METHOD_GET);
    CPPUNIT_ASSERT(uri == "/blatz");
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_1_1);
    
    // Test garbage
    line = makeMessage("get /blatz http/1.1");
    CPPUNIT_ASSERT(!HttpProtocol::ParseRequestLine(MessageBegin(line.get()), MessageEnd(line.get()), method, uri, version));
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testBuildStatusLine(void)
{
    // Test HTTP/0.9 status lines
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_0_9, HttpProtocol::HTTP_STATUS_OK) == "HTTP/0.9 200 OK\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_0_9, HttpProtocol::HTTP_STATUS_NOT_FOUND) == "HTTP/0.9 404 Not Found\r\n");

    // Test HTTP/1.0 status lines
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_0, HttpProtocol::HTTP_STATUS_OK) == "HTTP/1.0 200 OK\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_0, HttpProtocol::HTTP_STATUS_BAD_REQUEST) == "HTTP/1.0 400 Bad Request\r\n");

    // Test HTTP/1.1 status lines
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_1, HttpProtocol::HTTP_STATUS_OK) == "HTTP/1.1 200 OK\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildStatusLine(HttpProtocol::HTTP_VER_1_1, HttpProtocol::HTTP_STATUS_METHOD_NOT_ALLOWED) == "HTTP/1.1 405 Method Not Allowed\r\n");
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testParseStatusLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    HttpProtocol::Version version;
    HttpProtocol::StatusCode statusCode;

    // Test blank lines
    line = makeMessage("");
    CPPUNIT_ASSERT(!HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_UNKNOWN);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_UNKNOWN);

    line = makeMessage("\r\n");
    CPPUNIT_ASSERT(!HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_UNKNOWN);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_UNKNOWN);
    
    // Test HTTP/0.9 200 OK
    line = makeMessage("HTTP/0.9 200 OK\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_0_9);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_OK);
                   
    // Test HTTP/1.0 400 Bad Request
    line = makeMessage("HTTP/1.0 400 Bad Request\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_1_0);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_BAD_REQUEST);
    
    // Test HTTP/1.1 404 Not Found
    line = makeMessage("HTTP/1.1 404 Not Found\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_1_1);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_NOT_FOUND);
    
    // Test garbage
    line = makeMessage("http/1.1 0 Whatever");
    CPPUNIT_ASSERT(!HttpProtocol::ParseStatusLine(MessageBegin(line.get()), MessageEnd(line.get()), version, statusCode));
    CPPUNIT_ASSERT(version == HttpProtocol::HTTP_VER_UNKNOWN);
    CPPUNIT_ASSERT(statusCode == HttpProtocol::HTTP_STATUS_UNKNOWN);
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testBuildHeaderLine(void)
{
    // Test various header lines
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONNECTION, "close") == "Connection: close\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_LENGTH, 64) == "Content-Length: 64\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_CONTENT_TYPE, "text/html") == "Content-Type: text/html\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_EXPIRES, ACE_Time_Value::zero) == "Expires: Thu, 01 Jan 1970 00:00:00 GMT\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_LAST_MODIFIED, ACE_Time_Value((3 * 3600) + (2 * 60) + 1, 0)) == "Last-Modified: Thu, 01 Jan 1970 03:02:01 GMT\r\n");
    CPPUNIT_ASSERT(HttpProtocol::BuildHeaderLine(HttpProtocol::HTTP_HEADER_SERVER, "Microsoft-IIS/6.0") == "Server: Microsoft-IIS/6.0\r\n");
}

///////////////////////////////////////////////////////////////////////////////

void TestHttpProtocol::testParseHeaderLine(void)
{
    std::auto_ptr<ACE_Message_Block> line;
    HttpProtocol::HeaderType header;
    std::string value;

    // Test blank lines
    line = makeMessage("");
    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_BLANK_LINE);
    CPPUNIT_ASSERT(value.empty());
    
    line = makeMessage("\r\n");
    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_BLANK_LINE);
    CPPUNIT_ASSERT(value.empty());
    
    // Test headers with various spacing and capitalizations
    line = makeMessage("cOnNeCtIoN: close\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_CONNECTION);
    CPPUNIT_ASSERT(value == "close");
                   
    line = makeMessage("Content-Length:\t64\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_CONTENT_LENGTH);
    CPPUNIT_ASSERT(value == "64");

    line = makeMessage("content-type:   text/html\r\n");

    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_CONTENT_TYPE);
    CPPUNIT_ASSERT(value == "text/html");
    
    // Test other header type
    line = makeMessage("x-Spirent-Header: foo");
    CPPUNIT_ASSERT(HttpProtocol::ParseHeaderLine(MessageBegin(line.get()), MessageEnd(line.get()), header, value));
    CPPUNIT_ASSERT(header == HttpProtocol::HTTP_HEADER_UNKNOWN);
    CPPUNIT_ASSERT(value == "foo");
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestHttpProtocol);
