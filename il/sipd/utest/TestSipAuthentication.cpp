#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "SipAuthentication.h"
#include "SipleanAuthentication.h"
#include "SipMessage.h"

#include "TestUtilities.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSipAuthentication : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestSipAuthentication);
    CPPUNIT_TEST(testAlgorithmAndQopUnspecified);
    CPPUNIT_TEST(testAuthAndAlgorithmUnspecified);
    CPPUNIT_TEST(testAuthAndMd5);
    CPPUNIT_TEST(testAuthAndMd5Sess);
    CPPUNIT_TEST(testAuthIntAndMd5);
    CPPUNIT_TEST(testAuthIntAndMd5Sess);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }

protected:
    void testAlgorithmAndQopUnspecified(void);
    void testAuthAndAlgorithmUnspecified(void);
    void testAuthAndMd5(void);
    void testAuthAndMd5Sess(void);
    void testAuthIntAndMd5(void);
    void testAuthIntAndMd5Sess(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAlgorithmAndQopUnspecified(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.1
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_NONE;
    chal.qop = SIP_QOP_NONE;

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "bf57e4e0d0bffc0fbaedce64d59add5e");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_NONE);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_NONE);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAuthAndAlgorithmUnspecified(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.2
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_NONE;
    chal.qop = SIP_QOP_AUTH;

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    _CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "89eb0059246c02b2f6ee02c7961d5ea3");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_NONE);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_AUTH);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAuthAndMd5(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.3
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_MD5;
    chal.qop = SIP_QOP_AUTH;

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    _CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "89eb0059246c02b2f6ee02c7961d5ea3");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_MD5);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_AUTH);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAuthAndMd5Sess(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.4
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_MD5_SESS;
    chal.qop = SIP_QOP_AUTH;

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    _CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "e4e4ea61d186d07a92c9e1f6919902e9");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_MD5_SESS);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_AUTH);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAuthIntAndMd5(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.5
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_MD5;
    chal.qop = static_cast<SipQualityOfProtection>(SIP_QOP_AUTH | SIP_QOP_AUTH_INT);

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    msg.body = "v=0\r\no=bob 2890844526 2890844526 IN IP4 media.biloxi.com\r\ns=\r\nc=IN IP4 media.biloxi.com\r\nt=0 0\r\nm=audio 49170 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\nm=video 51372 RTP/AVP 31\r\na=rtpmap:31 H261/90000\r\nm=video 53000 RTP/AVP 32\r\na=rtpmap:32 MPV/90000\r\n";
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    _CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "41f1bde42dcddbee8ae7d65fd3474dc0");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_MD5);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_AUTH_INT);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipAuthentication::testAuthIntAndMd5Sess(void)
{
    // From draft-smith-sip-auth-examples-00.txt, Section 3.6
    SipDigestAuthChallenge chal;
    chal.realm = "biloxi.com";
    chal.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    chal.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    chal.algorithm = SIP_AUTH_MD5_SESS;
    chal.qop = static_cast<SipQualityOfProtection>(SIP_QOP_AUTH | SIP_QOP_AUTH_INT);

    SipMessage msg;
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_INVITE;
    msg.request.uri = SipUri("bob", "biloxi.com", 0);
    msg.body = "v=0\r\no=bob 2890844526 2890844526 IN IP4 media.biloxi.com\r\ns=\r\nc=IN IP4 media.biloxi.com\r\nt=0 0\r\nm=audio 49170 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\nm=video 51372 RTP/AVP 31\r\na=rtpmap:31 H261/90000\r\nm=video 53000 RTP/AVP 32\r\na=rtpmap:32 MPV/90000\r\n";
    
    const std::string username("bob");
    const std::string password("zanzibar");
    const std::string cnonce("0a4f113b");
    
    SipDigestAuthCredentials cred;
    _CPPUNIT_ASSERT_NO_THROW(SipleanAuthentication::Md5Digest(chal, msg, username, password, cred, &cnonce));

    _CPPUNIT_ASSERT(cred.username == username);
    _CPPUNIT_ASSERT(cred.realm == chal.realm);
    _CPPUNIT_ASSERT(cred.nonce == chal.nonce);
    _CPPUNIT_ASSERT(cred.uri == "sip:bob@biloxi.com");
    _CPPUNIT_ASSERT(cred.response == "10e4c79b16d21d51995ab98083d134d8");
    _CPPUNIT_ASSERT(cred.algorithm == SIP_AUTH_MD5_SESS);
    _CPPUNIT_ASSERT(cred.cnonce == cnonce);
    _CPPUNIT_ASSERT(cred.nonceCount == 1);
    _CPPUNIT_ASSERT(cred.opaque == chal.opaque);
    _CPPUNIT_ASSERT(cred.qop == SIP_QOP_AUTH_INT);
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSipAuthentication);
