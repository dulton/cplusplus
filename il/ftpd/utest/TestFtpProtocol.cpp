#include <memory>
#include <string>

#include <ace/Message_Block.h>
#include <utils/MessageIterator.h>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "FtpProtocol.h"

USING_L4L7_UTILS_NS;
USING_FTP_NS;

///////////////////////////////////////////////////////////////////////////////

class TestFtpProtocol : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestFtpProtocol);
    CPPUNIT_TEST(testBuildRequestLine);
    CPPUNIT_TEST(testParseRequestLine);
    CPPUNIT_TEST(testBuildStatusLine);
    CPPUNIT_TEST(testParseStatusLine) ;
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }
    
protected:
    void testBuildRequestLine(void);
    void testParseRequestLine(void);
    void testBuildStatusLine(void);
    void testParseStatusLine(void);

private:
    std::auto_ptr<ACE_Message_Block> makeMessage(const std::string& str);
    ACE_INET_Addr BuildIpv6Address(uint8_t *ip6, uint16_t port) ; // assumes ip6 points to 16 bytes of data
    ACE_INET_Addr BuildIpv4Address(uint32_t ip4, uint16_t port) ; 
};

///////////////////////////////////////////////////////////////////////////////

std::auto_ptr<ACE_Message_Block> TestFtpProtocol::makeMessage(const std::string& str)
{
    std::auto_ptr<ACE_Message_Block> mb(new ACE_Message_Block(str.size()));
    mb->copy(str.data(), str.size());
    return mb;
}

///////////////////////////////////////////////////////////////////////////////

ACE_INET_Addr TestFtpProtocol::BuildIpv6Address(uint8_t *ip6, uint16_t port) 
{
    struct sockaddr_in6 in6 ;
    memset(&in6, 0, sizeof(struct sockaddr_in6)) ;
    in6.sin6_family = AF_INET6 ;
    in6.sin6_port = htons(port) ;
    memcpy(in6.sin6_addr.s6_addr, ip6, 16) ;
    return ACE_INET_Addr(reinterpret_cast<struct sockaddr_in *>(&in6),
                         sizeof(struct sockaddr_in6)) ;
}

///////////////////////////////////////////////////////////////////////////////

ACE_INET_Addr TestFtpProtocol::BuildIpv4Address(uint32_t ip4, uint16_t port) 
{
    return ACE_INET_Addr(port, ip4) ;
}

///////////////////////////////////////////////////////////////////////////////

void TestFtpProtocol::testBuildRequestLine(void)
{
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_USER, "SomeUserName") == "USER SomeUserName\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PASSWORD, "SomeUserPasswd") == "PASS SomeUserPasswd\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_TYPE, "I") == "TYPE I\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_STOR, "somefilename") == "STOR somefilename\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_RETR, "somefilename") == "RETR somefilename\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_QUIT, "") == "QUIT\r\n") ;

    uint8_t ip6addr[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00} ;
    ACE_INET_Addr ipv6 = BuildIpv6Address(ip6addr, 0x1045) ;
    ACE_INET_Addr ipv4 = BuildIpv4Address(0xC0101010, 0x1045) ;
 
    const std::string res =  FtpProtocol::BuildPortRequestString(ipv6) ;
    const std::string res2 = FtpProtocol::BuildPortRequestString(ipv4) ;
    
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PORT, res2) ==
                   "PORT 192,16,16,16,16,69\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PORT, res) ==
                   "PORT 17,34,51,68,85,102,119,136,153,170,187,204,221,238,255,0,16,69\r\n") ;
}


///////////////////////////////////////////////////////////////////////////////

void TestFtpProtocol::testParseRequestLine(void)
{
	std::auto_ptr<ACE_Message_Block> request ;

    FtpProtocol::FtpRequestParser reqParser ;
    FtpProtocol::FtpPortRequestParser portParser ;

    // USER
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_USER, "SomeUserName"))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_USER) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "SomeUserName") ;
	CPPUNIT_ASSERT(reqParser.Parse("user SomeUserName")) ; // test lower case...
	CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_USER) ;
	CPPUNIT_ASSERT(reqParser.GetRequestString() == "SomeUserName") ;

    // PASS
	// test this one through the message iterator interface
	request = makeMessage(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PASSWORD, "SomeUserPasswd")) ;
    CPPUNIT_ASSERT(reqParser.Parse(MessageBegin(request.get()), MessageEnd(request.get()))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_PASSWORD) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "SomeUserPasswd") ;

    // TYPE
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_TYPE, "I"))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_TYPE) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "I") ;

    // STOR
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_STOR, "somefilename"))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_STOR) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "somefilename") ;

    // RETR
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_RETR, "somefilename"))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_RETR) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "somefilename") ;

    // QUIT
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_QUIT, ""))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_QUIT) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == "") ;

    // PORT (ipv4 and ipv6)
    uint8_t ip6addr[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00} ;
    ACE_INET_Addr ipv6 = BuildIpv6Address(ip6addr, 0x1045) ;
    ACE_INET_Addr ipv4 = BuildIpv4Address(0xC0101010, 0x1045) ;
    const std::string res =  FtpProtocol::BuildPortRequestString(ipv6) ;
    const std::string res2 = FtpProtocol::BuildPortRequestString(ipv4) ;
    
    CPPUNIT_ASSERT(reqParser.Parse(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PORT, res2))) ;
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_PORT) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == res2) ;

	// test this one through the message iterator interface
	request = makeMessage(FtpProtocol::BuildRequestLine(FtpProtocol::FTP_METHOD_PORT, res)) ;
	CPPUNIT_ASSERT(reqParser.Parse(MessageBegin(request.get()), MessageEnd(request.get()))) ;	
    CPPUNIT_ASSERT(reqParser.GetMethodType() == FtpProtocol::FTP_METHOD_PORT) ;
    CPPUNIT_ASSERT(reqParser.GetRequestString() == res) ;

    // now, to test the port parser.
    CPPUNIT_ASSERT(portParser.Parse(res2)) ;
    const ACE_INET_Addr parserOutputIpv4 = portParser.GetPortInetAddr() ;
    const std::string res2_2 = FtpProtocol::BuildPortRequestString(parserOutputIpv4) ;    
    //std::cout << "\nOutput Ipv4 = " << res2_2 << std::endl ;
    CPPUNIT_ASSERT(res2 == res2_2) ;

    CPPUNIT_ASSERT(portParser.Parse(res)) ;
    const ACE_INET_Addr parserOutputIpv6 = portParser.GetPortInetAddr() ;
    const std::string res_2 = FtpProtocol::BuildPortRequestString(parserOutputIpv6) ;
    //std::cout << "\nOutput Ipv6 = " << res_2 << std::endl ;
    CPPUNIT_ASSERT(res == res_2) ;
    
}


///////////////////////////////////////////////////////////////////////////////

void TestFtpProtocol::testBuildStatusLine(void)
{
    CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN) == 
                   "125 Data connection already open; transfer starting.\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN) == 
                   "150 File status okay; about to open data connection.\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_OK) == 
                   "200 Command okay.\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SERVICE_READY) == 
                   "220 Service ready, proceed.\r\n") ;
    CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN) == 
                   "226 File transfer complete, closing data connection.\r\n") ;    
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN) == 
				   "125 Data connection already open; transfer starting.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN) == 
				   "150 File status okay; about to open data connection.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_OK) == 
				   "200 Command okay.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SERVICE_READY) == 
				   "220 Service ready, proceed.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN) == 
				   "226 File transfer complete, closing data connection.\r\n") ;    
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE) == 
				   "227 Entering passive mode \r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_USER_LOGGED_IN) == 
				   "230 User Logged in, proceed.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_ACTION_COMPLETE) == 
				   "250 Requested file action okay, completed.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_NEED_PASSWORD) == 
				   "331 User name okay, need password.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_FAILED) == 
				   "425 Can't open data connection.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED) == 
				   "426 Connection closed; transfer aborted.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN) == 
				   "452 Requested action not taken.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SYNTAX_ERROR) == 
				   "500 Syntax Error, command unrecognized.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED) == 
				   "502 Command not implemented.\r\n") ;
	CPPUNIT_ASSERT(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_NOT_LOGGED_IN) == 
				   "530 Not logged in.\r\n") ;
    CPPUNIT_ASSERT_THROW(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CODE_INVALID, ""), EPHXInternal) ;
}

///////////////////////////////////////////////////////////////////////////////

void TestFtpProtocol::testParseStatusLine(void)
{
	std::auto_ptr<ACE_Message_Block> status ;
    FtpProtocol::FtpResponseParser respParser ;
    FtpProtocol::FtpPasvResponseParser pasvParser ;

    CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN) ;
	CPPUNIT_ASSERT(respParser.GetStatusMsg() == "Data connection already open; transfer starting.") ;

	// test this one through the message iterator interface
	status = makeMessage(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN)) ;
    CPPUNIT_ASSERT(respParser.Parse(MessageBegin(status.get()), MessageEnd(status.get()))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN) ;
	CPPUNIT_ASSERT(respParser.GetStatusMsg() == "File status okay; about to open data connection.") ;

    CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SERVICE_READY))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_SERVICE_READY) ;

    CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN))) ; 
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN) ;

	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_DATA_CONN_ALREADY_OPEN) ;

	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_FILE_STATUS_OK_OPENING_DATA_CONN) ;

	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_OK))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_OK) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SERVICE_READY))); 
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_SERVICE_READY) ;

	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_CLOSING_DATA_CONN) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_USER_LOGGED_IN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_USER_LOGGED_IN) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_FILE_ACTION_COMPLETE))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_FILE_ACTION_COMPLETE) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_NEED_PASSWORD))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_NEED_PASSWORD) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_DATA_CONN_FAILED))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_DATA_CONN_FAILED) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_CONN_CLOSED_TRANSFER_ABORTED) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_ACTION_NOT_TAKEN) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_SYNTAX_ERROR))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_SYNTAX_ERROR) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_COMMAND_NOT_IMPLEMENTED) ;
				   
	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_NOT_LOGGED_IN))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_NOT_LOGGED_IN) ;
				   

	// Test PASV response handling for Ipv6 and Ipv4
	uint8_t ip6addr[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00} ;
	ACE_INET_Addr ipv6 = BuildIpv6Address(ip6addr, 0x1045) ;
	ACE_INET_Addr ipv4 = BuildIpv4Address(0xC0101010, 0x1045) ;

	const std::string res =  FtpProtocol::BuildPasvPortString(ipv6) ;
	const std::string res2 = FtpProtocol::BuildPasvPortString(ipv4) ;

	CPPUNIT_ASSERT(respParser.Parse(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE,
																 res))) ;
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE) ;	
	CPPUNIT_ASSERT(pasvParser.Parse(respParser.GetStatusMsg())) ;
	ACE_INET_Addr parsedIpv6 = pasvParser.GetPasvInetAddr() ;
	const std::string res_2 = FtpProtocol::BuildPasvPortString(parsedIpv6) ;	
	CPPUNIT_ASSERT(res == res_2) ;


	// test this one through the message iterator interface
	status = makeMessage(FtpProtocol::BuildStatusLine(FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE, res2)) ;
	CPPUNIT_ASSERT(respParser.Parse(MessageBegin(status.get()), MessageEnd(status.get()))) ;	
	CPPUNIT_ASSERT(respParser.GetStatusCode() == FtpProtocol::FTP_STATUS_ENTERING_PASSIVE_MODE) ;	
	CPPUNIT_ASSERT(pasvParser.Parse(respParser.GetStatusMsg())) ;
	ACE_INET_Addr parsedIpv4 = pasvParser.GetPasvInetAddr() ;
	const std::string res2_2 = FtpProtocol::BuildPasvPortString(parsedIpv4) ;	
	CPPUNIT_ASSERT(res2 == res2_2) ;

}

///////////////////////////////////////////////////////////////////////////////


CPPUNIT_TEST_SUITE_REGISTRATION(TestFtpProtocol);
