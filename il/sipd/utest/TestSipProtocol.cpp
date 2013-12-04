#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <vif/Packet.h>

#include "SdpMessage.h"
#include "SipAuthentication.h"
#include "SipMessage.h"
#include "SipProtocol.h"
#include "SipUri.h"
#include "SipUtils.h"

#include "TestUtilities.h"

USING_SIP_NS;
USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

class TestSipProtocol : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestSipProtocol);
    CPPUNIT_TEST(testBuildUriString);
    CPPUNIT_TEST(testParseUriString);
    CPPUNIT_TEST(testBuildViaString);
    CPPUNIT_TEST(testParseViaString);
    CPPUNIT_TEST(testBuildAddressString);
    CPPUNIT_TEST(testParseAddressString);
    CPPUNIT_TEST(testBuildCallIdString);
    CPPUNIT_TEST(testBuildCSeqString);
    CPPUNIT_TEST(testParseCSeqString);
    CPPUNIT_TEST(testBuildRAckString);
    CPPUNIT_TEST(testParseRAckString);
    CPPUNIT_TEST(testBuildSessionExpirationString);
    CPPUNIT_TEST(testParseSessionExpirationString);
    CPPUNIT_TEST(testParseDigestAuthChallengeString);
    CPPUNIT_TEST(testBuildDigestAuthCredentialsString);
    CPPUNIT_TEST(testBuildSdpString);
    CPPUNIT_TEST(testParseSdpString);
    CPPUNIT_TEST(testBuildRequest);
    CPPUNIT_TEST(testBuildStatus);
    CPPUNIT_TEST(testParseRequest);
    CPPUNIT_TEST(testParseStatus);
    CPPUNIT_TEST(testHeaderUtils);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) { }
    void tearDown(void) { }

protected:
    void testBuildUriString(void);
    void testParseUriString(void);
    void testBuildViaString(void);
    void testParseViaString(void);
    void testBuildAddressString(void);
    void testParseAddressString(void);
    void testBuildCallIdString(void);
    void testBuildCSeqString(void);
    void testParseCSeqString(void);
    void testBuildRAckString(void);
    void testParseRAckString(void);
    void testBuildSessionExpirationString(void);
    void testParseSessionExpirationString(void);
    void testParseDigestAuthChallengeString(void);
    void testBuildDigestAuthCredentialsString(void);
    void testBuildSdpString(void);
    void testParseSdpString(void);
    void testBuildRequest(void);
    void testBuildStatus(void);
    void testParseRequest(void);
    void testParseStatus(void);
    void testHeaderUtils(void);
};

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildUriString(void)
{
    // Test dotted decimal addresses (e.g. IPv4)
    _CPPUNIT_ASSERT(SipUri("foo.bar.com", 5063).BuildUriString() == "sip:foo.bar.com:5063");
    _CPPUNIT_ASSERT(SipUri("bob", "foo.bar.com", 5063).BuildUriString() == "sip:bob@foo.bar.com:5063");
    _CPPUNIT_ASSERT(SipUri("bob", "foo.bar.com", 0).BuildUriString() == "sip:bob@foo.bar.com");

    _CPPUNIT_ASSERT(SipUri("bob", "foo.bar.com", 0, "", true).BuildRouteUriString() == "<sip:foo.bar.com;lr>");
    _CPPUNIT_ASSERT(SipUri("bob", "1.2.3.4", 5060, "", false).BuildRouteUriString() == "<sip:1.2.3.4:5060>");
    _CPPUNIT_ASSERT(SipUri("bob", "1.2.3.4", 5063, "", true).BuildRouteUriString() == "<sip:1.2.3.4:5063;lr>");

    // Test colon-delimited addresses (e.g. IPv6)
    _CPPUNIT_ASSERT(SipUri("2000::2", 5063).BuildUriString() == "sip:[2000::2]:5063");
    _CPPUNIT_ASSERT(SipUri("bob", "2000::2", 5063).BuildUriString() == "sip:bob@[2000::2]:5063");

    _CPPUNIT_ASSERT(SipUri("bob", "1.2.3.4", 5063, "sctp", true).BuildRouteUriString() == "<sip:1.2.3.4:5063;transport=sctp;lr>");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseUriString(void)
{
    SipUri uri;

    _CPPUNIT_ASSERT(!SipProtocol::ParseUriString("", uri));
    _CPPUNIT_ASSERT(!SipProtocol::ParseUriString("<sip:foo.bar.com>", uri));
    _CPPUNIT_ASSERT(!SipProtocol::ParseUriString("foo:foo.bar.com", uri));

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:foo.bar.com", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(SipUtils::is_default_sip_port(uri.GetSipPort()));
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:foo-blatz.bar.com", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(SipUtils::is_default_sip_port(uri.GetSipPort()));
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:[2000::2]", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(SipUtils::is_default_sip_port(uri.GetSipPort()));
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:foo.bar.com:5063", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:foo-blatz.bar.com:5063", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:[2000::2]:5063", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000:secret@foo.bar.com:5063;foo=bar;lr", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000:secret@foo-blatz.bar.com:5063;foo=bar;lr", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);
    _CPPUNIT_ASSERT(uri.GetSipTransport() == "");

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000:secret@foo-blatz.bar.com:5063;bar=foo;transport=sctp;foo=bar;lr", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);
    _CPPUNIT_ASSERT(uri.GetSipTransport() == "sctp");
    
    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000@[2000::2]:5063;tag=39ec37b2;lr=on", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000@foo.bar.com:5063;maddr=10.1.1.1", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);

    _CPPUNIT_ASSERT(SipProtocol::ParseUriString("sip:1000@[2000::2]:5063;maddr=10.1.1.1", uri));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildViaString(void)
{
    SipUri uri("user1000", "foo.bar.com", 5063);
    std::string branch("z9hG4bK");
    
    _CPPUNIT_ASSERT(SipProtocol::BuildViaString(uri) == "SIP/2.0/UDP foo.bar.com:5063");
    _CPPUNIT_ASSERT(SipProtocol::BuildViaString(uri, &branch) == "SIP/2.0/UDP foo.bar.com:5063;branch=z9hG4bK");

    uri.SetSip("user1000", "2000::2", 5063);
    _CPPUNIT_ASSERT(SipProtocol::BuildViaString(uri) == "SIP/2.0/UDP [2000::2]:5063");
    _CPPUNIT_ASSERT(SipProtocol::BuildViaString(uri, &branch) == "SIP/2.0/UDP [2000::2]:5063;branch=z9hG4bK");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseViaString(void)
{
    SipUri uri;
    std::string branch;
    
    _CPPUNIT_ASSERT(!SipProtocol::ParseViaString("", uri));
    _CPPUNIT_ASSERT(!SipProtocol::ParseViaString("foo.bar.com", uri));
    
    _CPPUNIT_ASSERT(SipProtocol::ParseViaString("SIP/2.0/UDP 10.1.1.1:5063;branch=z9hG4bK", uri, &branch));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "10.1.1.1");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(branch == "z9hG4bK");

    _CPPUNIT_ASSERT(SipProtocol::ParseViaString("SIP / 2.0 / UDP [2000::2] : 5063; ttl=16; branch=z9hG4bK", uri, &branch));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(branch == "z9hG4bK");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildAddressString(void)
{
    SipUri uri("user1234", "foo.bar.com", 5063);
    std::string tag("39ec37b2");
    
    _CPPUNIT_ASSERT(SipProtocol::BuildAddressString(uri) == "user1234 <sip:user1234@foo.bar.com:5063>");
    _CPPUNIT_ASSERT(SipProtocol::BuildAddressString(uri, &tag) == "user1234 <sip:user1234@foo.bar.com:5063>;tag=39ec37b2");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseAddressString(void)
{
    SipUri uri;
    std::string tag;
    
    _CPPUNIT_ASSERT(!SipProtocol::ParseAddressString(""));
    _CPPUNIT_ASSERT(!SipProtocol::ParseAddressString("sip:foo.bar.com"));

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("Dave <sip:foo.bar.com>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("Dave <sip:foo-blatz.bar.com>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("Tom <sip:[2000::2]>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());
    
    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("<sip:foo.bar.com:5063>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("<sip:foo-blatz.bar.com:5063>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo-blatz.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString(" <sip:[2000::2]:5063>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser().empty());
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());
    
    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("Kate <sip:1000:secret@foo.bar.com:5063;lr>;tag=39ec37b2", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);
    _CPPUNIT_ASSERT(tag == "39ec37b2");

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("Abhitesh <sip:1000@[2000::2]:5063;lr=on>; foo=bar; tag=39ec37b2", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == true);
    _CPPUNIT_ASSERT(tag == "39ec37b2");

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("1000 <sip:1000@foo.bar.com:5063;maddr=10.1.1.1>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "foo.bar.com");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());

    _CPPUNIT_ASSERT(SipProtocol::ParseAddressString("1000 <sip:1000@[2000::2]:5063;maddr=10.1.1.1>", &uri, &tag));
    _CPPUNIT_ASSERT(uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(uri.GetSipUser() == "1000");
    _CPPUNIT_ASSERT(uri.GetSipHost() == "2000::2");
    _CPPUNIT_ASSERT(uri.GetSipPort() == 5063);
    _CPPUNIT_ASSERT(uri.GetLooseRouting() == false);
    _CPPUNIT_ASSERT(tag.empty());
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildCallIdString(void)
{
    std::string callId = SipProtocol::BuildCallIdString("foo.bar.com", 32);
    _CPPUNIT_ASSERT(callId.size() == 44);
    _CPPUNIT_ASSERT(callId.find("@foo.bar.com") == 32);

    callId = SipProtocol::BuildCallIdString("2000::2", 32);
    _CPPUNIT_ASSERT(callId.size() == 42);
    _CPPUNIT_ASSERT(callId.find("@[2000::2]") == 32);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildCSeqString(void)
{
    _CPPUNIT_ASSERT(SipProtocol::BuildCSeqString(2, SipMessage::SIP_METHOD_INVITE) == "2 INVITE");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseCSeqString(void)
{
    uint32_t cseq;
    SipMessage::MethodType method;
    
    _CPPUNIT_ASSERT(!SipProtocol::ParseCSeqString("", cseq, method));
    _CPPUNIT_ASSERT(!SipProtocol::ParseCSeqString("FOO", cseq, method));

    _CPPUNIT_ASSERT(SipProtocol::ParseCSeqString("1 INVITE", cseq, method));
    _CPPUNIT_ASSERT(cseq == 1);
    _CPPUNIT_ASSERT(method == SipMessage::SIP_METHOD_INVITE);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseCSeqString("3 FOO", cseq, method));
    _CPPUNIT_ASSERT(cseq == 3);
    _CPPUNIT_ASSERT(method == SipMessage::SIP_METHOD_UNKNOWN);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildRAckString(void)
{
    _CPPUNIT_ASSERT(SipProtocol::BuildRAckString(1972, 2, SipMessage::SIP_METHOD_INVITE) == "1972 2 INVITE");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseRAckString(void)
{
    uint32_t rseq, cseq;
    SipMessage::MethodType method;
    
    _CPPUNIT_ASSERT(!SipProtocol::ParseRAckString("", rseq, cseq, method));
    _CPPUNIT_ASSERT(!SipProtocol::ParseRAckString("FOO", rseq, cseq, method));

    _CPPUNIT_ASSERT(SipProtocol::ParseRAckString("1972 1 INVITE", rseq, cseq, method));
    _CPPUNIT_ASSERT(rseq == 1972);
    _CPPUNIT_ASSERT(cseq == 1);
    _CPPUNIT_ASSERT(method == SipMessage::SIP_METHOD_INVITE);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseRAckString("2008 3 FOO", rseq, cseq, method));
    _CPPUNIT_ASSERT(rseq == 2008);
    _CPPUNIT_ASSERT(cseq == 3);
    _CPPUNIT_ASSERT(method == SipMessage::SIP_METHOD_UNKNOWN);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildSessionExpirationString(void)
{
    _CPPUNIT_ASSERT(SipProtocol::BuildSessionExpirationString(SipSessionExpiration()) == "0");
    _CPPUNIT_ASSERT(SipProtocol::BuildSessionExpirationString(SipSessionExpiration(300)) == "300");
    _CPPUNIT_ASSERT(SipProtocol::BuildSessionExpirationString(SipSessionExpiration(1200, SipSessionExpiration::UAC_REFRESHER)) == "1200;refresher=uac");
    _CPPUNIT_ASSERT(SipProtocol::BuildSessionExpirationString(SipSessionExpiration(1200, SipSessionExpiration::UAS_REFRESHER)) == "1200;refresher=uas");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseSessionExpirationString(void)
{
    SipSessionExpiration se;

    _CPPUNIT_ASSERT(!SipProtocol::ParseSessionExpirationString("", se));
    _CPPUNIT_ASSERT(!SipProtocol::ParseSessionExpirationString("FOO", se));

    _CPPUNIT_ASSERT(SipProtocol::ParseSessionExpirationString("300", se));
    _CPPUNIT_ASSERT(se.deltaSeconds == 300);
    _CPPUNIT_ASSERT(se.refresher == SipSessionExpiration::NO_REFRESHER);

    _CPPUNIT_ASSERT(SipProtocol::ParseSessionExpirationString("300;refresher=xyz;foo=bar", se));
    _CPPUNIT_ASSERT(se.deltaSeconds == 300);
    _CPPUNIT_ASSERT(se.refresher == SipSessionExpiration::NO_REFRESHER);

    _CPPUNIT_ASSERT(SipProtocol::ParseSessionExpirationString("1200;foo=bar;refresher=uac", se));
    _CPPUNIT_ASSERT(se.deltaSeconds == 1200);
    _CPPUNIT_ASSERT(se.refresher == SipSessionExpiration::UAC_REFRESHER);
    
    _CPPUNIT_ASSERT(SipProtocol::ParseSessionExpirationString("1200 ; refresher=uas", se));
    _CPPUNIT_ASSERT(se.deltaSeconds == 1200);
    _CPPUNIT_ASSERT(se.refresher == SipSessionExpiration::UAS_REFRESHER);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseDigestAuthChallengeString(void)
{
    SipDigestAuthChallenge chal;
    
    _CPPUNIT_ASSERT(!SipProtocol::ParseDigestAuthChallenge("", chal));
    _CPPUNIT_ASSERT(!SipProtocol::ParseDigestAuthChallenge("FOO", chal));
    _CPPUNIT_ASSERT(!SipProtocol::ParseDigestAuthChallenge("Digest", chal));

    _CPPUNIT_ASSERT(SipProtocol::ParseDigestAuthChallenge("Digest realm=\"atlanta.com\", domain=\"sip:ss1.carrier.com\", qop=\"auth,auth-int\", nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", stale=FALSE, algorithm=MD5", chal));
    _CPPUNIT_ASSERT(chal.realm == "atlanta.com");
    _CPPUNIT_ASSERT(chal.domain == "sip:ss1.carrier.com");
    _CPPUNIT_ASSERT(chal.nonce == "f84f1cec41e6cbe5aea9c8e88d359");
    _CPPUNIT_ASSERT(chal.opaque.empty());
    _CPPUNIT_ASSERT(chal.stale == false);
    _CPPUNIT_ASSERT(chal.algorithm == SIP_AUTH_MD5);
    _CPPUNIT_ASSERT(chal.qop == (SIP_QOP_AUTH | SIP_QOP_AUTH_INT));

    _CPPUNIT_ASSERT(SipProtocol::ParseDigestAuthChallenge("Digest realm=\"raleigh-durham.com\", domain=\"sip:ss1-secure.carrier.com\", qop=\"auth,auth-int\", nonce=\"f84f1cec41e6cbe5aea9c8e88d359\", opaque=\"\", stale=FALSE, algorithm=MD5", chal));
    _CPPUNIT_ASSERT(chal.realm == "raleigh-durham.com");
    _CPPUNIT_ASSERT(chal.domain == "sip:ss1-secure.carrier.com");
    _CPPUNIT_ASSERT(chal.nonce == "f84f1cec41e6cbe5aea9c8e88d359");
    _CPPUNIT_ASSERT(chal.opaque.empty());
    _CPPUNIT_ASSERT(chal.stale == false);
    _CPPUNIT_ASSERT(chal.algorithm == SIP_AUTH_MD5);
    _CPPUNIT_ASSERT(chal.qop == (SIP_QOP_AUTH | SIP_QOP_AUTH_INT));
    
    _CPPUNIT_ASSERT(SipProtocol::ParseDigestAuthChallenge("Digest realm=\"\", domain=\"\", opaque=\"foo\", stale=TRUE, algorithm=MD5-sess", chal));
    _CPPUNIT_ASSERT(chal.realm.empty());
    _CPPUNIT_ASSERT(chal.domain.empty());
    _CPPUNIT_ASSERT(chal.nonce.empty());
    _CPPUNIT_ASSERT(chal.opaque == "foo");
    _CPPUNIT_ASSERT(chal.stale == true);
    _CPPUNIT_ASSERT(chal.algorithm == SIP_AUTH_MD5_SESS);
    _CPPUNIT_ASSERT(chal.qop == SIP_QOP_NONE);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildDigestAuthCredentialsString(void)
{
    SipDigestAuthCredentials cred;

    _CPPUNIT_ASSERT(SipProtocol::BuildDigestAuthCredentialsString(cred) == "Digest username=\"\", realm=\"\", nonce=\"\", uri=\"\", response=\"\"");
    
    cred.username = "bob";
    cred.realm = "biloxi.com";
    cred.nonce = "dcd98b7102dd2f0e8b11d0f600bfb0c093";
    cred.uri = "sip:bob@biloxi.com";
    cred.response = "245f23415f11432b3434341c022";
    cred.algorithm = SIP_AUTH_MD5;
    cred.cnonce = "0a4f113b";
    cred.nonceCount = 1;
    cred.opaque = "5ccc069c403ebaf9f0171e9517f40e41";
    cred.qop = SIP_QOP_AUTH;

    _CPPUNIT_ASSERT(SipProtocol::BuildDigestAuthCredentialsString(cred) ==
                   "Digest username=\"bob\", realm=\"biloxi.com\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", uri=\"sip:bob@biloxi.com\", "
                   "response=\"245f23415f11432b3434341c022\", algorithm=MD5, cnonce=\"0a4f113b\", nc=00000001, opaque=\"5ccc069c403ebaf9f0171e9517f40e41\", qop=auth");
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildSdpString(void)
{
    SdpMessage sdp;

    // Test IPv4 address with no media
    sdp.sessionId = 1;
    sdp.timestamp = 2;
    sdp.addrType = SdpMessage::SDP_ADDR_TYPE_IPV4;
    sdp.addrStr = "1.2.3.4";

    const std::string expMsgNullMedia =
        "v=0\r\n"
        "o=- 1 2 IN IP4 1.2.3.4\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP4 1.2.3.4\r\n"
        "t=2 0\r\n";

    _CPPUNIT_ASSERT(SipProtocol::BuildSdpString(sdp) == expMsgNullMedia);
    
    // Test IPv4 address with audio only
    sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_PCMU;
    sdp.audioTransportPort = 50050;

    const std::string expMsgIpv4Audio =
        "v=0\r\n"
        "o=- 1 2 IN IP4 1.2.3.4\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP4 1.2.3.4\r\n"
        "t=2 0\r\n"
        "m=audio 50050 RTP/AVP 0 100\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=ptime:20\r\n"
        "a=fmtp:100 0-15\r\n";

    _CPPUNIT_ASSERT(SipProtocol::BuildSdpString(sdp) == expMsgIpv4Audio);

    // Test IPv4 address with audio/video
    sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_G723;
    sdp.videoTransportPort = 50050;
    sdp.videoMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_H263;
    sdp.videoTransportPort = 50052;

    const std::string expMsgIpv4AudioVideo =
        "v=0\r\n"
        "o=- 1 2 IN IP4 1.2.3.4\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP4 1.2.3.4\r\n"
        "t=2 0\r\n"
        "m=audio 50050 RTP/AVP 4 100\r\n"
        "a=rtpmap:4 G723/8000\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=ptime:20\r\n"
        "a=fmtp:100 0-15\r\n"
        "m=video 50052 RTP/AVP 34\r\n"
        "a=rtpmap:34 H263/90000\r\n"
        "a=fmtp:34 QCIF=2\r\n";

    _CPPUNIT_ASSERT(SipProtocol::BuildSdpString(sdp) == expMsgIpv4AudioVideo);
    
    // Test IPv6 address with audio only
    sdp.addrType = SdpMessage::SDP_ADDR_TYPE_IPV6;
    sdp.addrStr = "2000::2";
    sdp.audioMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_G729;
    sdp.videoMediaFormat = SdpMessage::SDP_MEDIA_FORMAT_UNKNOWN;

    const std::string expMsgIpv6Audio =
        "v=0\r\n"
        "o=- 1 2 IN IP6 2000::2\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP6 2000::2\r\n"
        "t=2 0\r\n"
        "m=audio 50050 RTP/AVP 18 100\r\n"
        "a=rtpmap:18 G729/8000\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=ptime:20\r\n"
        "a=fmtp:100 0-15\r\n";

    _CPPUNIT_ASSERT(SipProtocol::BuildSdpString(sdp) == expMsgIpv6Audio);
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseSdpString(void)
{
    // Test parse of basic IPv4 announcement for audio/video
    const std::string goodIpv4Announce = 
        "v=0\r\n"
        "o=- 1 2 IN IP4 1.2.3.4\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP4 1.2.3.4\r\n"
        "t=2 0\r\n"
        "m=audio 50050 RTP/AVP 0 100\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=ptime:20\r\n"
        "a=fmtp:100 0-15\r\n"
        "m=video 50052 RTP/AVP 34\r\n"
        "a=rtpmap:34 H263/90000\r\n"
        "a=fmtp:34 QCIF=2\r\n";
    
    SdpMessage sdp;
    _CPPUNIT_ASSERT(SipProtocol::ParseSdpString(goodIpv4Announce, sdp));

    _CPPUNIT_ASSERT(sdp.addrType == SdpMessage::SDP_ADDR_TYPE_IPV4);
    _CPPUNIT_ASSERT(sdp.addrStr == "1.2.3.4");
    _CPPUNIT_ASSERT(sdp.audioTransportPort == 50050);
    _CPPUNIT_ASSERT(sdp.videoTransportPort == 50052);

    // Test parse of basic IPv6 announcement with audio only
    const std::string goodIpv6Announce =
        "v=0\r\n"
        "o=- 1 2 IN IP6 2000::2\r\n"
        "s=Spirent TestCenter\r\n"
        "c=IN IP6 2000::2\r\n"
        "t=2 0\r\n"
        "m=audio 50050 RTP/AVP 0 100\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=rtpmap:100 telephone-event/8000\r\n"
        "a=ptime:20\r\n"
        "a=fmtp:100 0-15\r\n";

    _CPPUNIT_ASSERT(SipProtocol::ParseSdpString(goodIpv6Announce, sdp));

    _CPPUNIT_ASSERT(sdp.addrType == SdpMessage::SDP_ADDR_TYPE_IPV6);
    _CPPUNIT_ASSERT(sdp.addrStr == "2000::2");
    _CPPUNIT_ASSERT(sdp.audioTransportPort == 50050);
    _CPPUNIT_ASSERT(sdp.videoTransportPort == 0);

    // Test parse of bad announcements (empty or bogus strings)
    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(std::string(), sdp));
    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(std::string(" "), sdp));
    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(std::string("\r\n"), sdp));
    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(std::string("\n"), sdp));
    
    // Test parse of bad announcement (missing network info)
    const std::string badAnnounceNoNet =
        "v=0\r\n"
        "o=- 1 2\r\n"
        "s=Spirent TestCenter\r\n"
        "m=audio 50050 RTP/AVP 0 100\r\n";

    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(badAnnounceNoNet, sdp));

    // Test parse of bad announcement (wrong media type)
    const std::string badAnnounceWrongMedia = 
        "v=0\r\n"
        "o=- 1 2 IN IP4 1.2.3.4\r\n"
        "s=Spirent TestCenter\r\n"
        "m=video 50050 MMS 0 100\r\n";

    _CPPUNIT_ASSERT(!SipProtocol::ParseSdpString(badAnnounceWrongMedia, sdp));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildRequest(void)
{
    SipMessage msg;
    Packet packet;
    
    // Test build of basic REGISTER request
    msg.type = SipMessage::SIP_MSG_REQUEST;
    msg.method = SipMessage::SIP_METHOD_REGISTER;
    msg.seq = 1;
    msg.request.uri = SipUri("5063", "biloxi.example.com", 0);
    msg.request.version = SipMessage::SIP_VER_2_0;

    msg.headers.clear();
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, "SIP/2.0/UDP biloxiServer.example.com:5061"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, "1j9FpLxk3uxtm8tn@biloxi.example.com"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, "1 REGISTER"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_EXPIRES, "60000"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTACT, "<sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    msg.body = "Hello\n";
    
    _CPPUNIT_ASSERT(SipProtocol::BuildPacket(msg, packet));

    const std::string expMsg1 =
        "REGISTER sip:5063@biloxi.example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP biloxiServer.example.com:5061\r\n"
        "From: Bob <sip:1234@biloxi.example.com>\r\n"
        "To: Bob <sip:1234@biloxi.example.com>\r\n"
        "Call-ID: 1j9FpLxk3uxtm8tn@biloxi.example.com\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Expires: 60000\r\n"
        "Contact: <sip:1234@biloxi.example.com>\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        "Hello\n";

    _CPPUNIT_ASSERT(packet.Length() == expMsg1.size());
    _CPPUNIT_ASSERT(memcmp(packet.Data(), expMsg1.data(), expMsg1.size()) == 0);

    // Test build of compact REGISTER request
    packet.Reset();
    msg.compactHeaders = true;

    _CPPUNIT_ASSERT(SipProtocol::BuildPacket(msg, packet));

    const std::string expMsg2 =
        "REGISTER sip:5063@biloxi.example.com SIP/2.0\r\n"
        "v: SIP/2.0/UDP biloxiServer.example.com:5061\r\n"
        "f: Bob <sip:1234@biloxi.example.com>\r\n"
        "t: Bob <sip:1234@biloxi.example.com>\r\n"
        "i: 1j9FpLxk3uxtm8tn@biloxi.example.com\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Expires: 60000\r\n"
        "m: <sip:1234@biloxi.example.com>\r\n"
        "l: 0\r\n"
        "\r\n"
        "Hello\n";

    _CPPUNIT_ASSERT(packet.Length() == expMsg2.size());
    _CPPUNIT_ASSERT(memcmp(packet.Data(), expMsg2.data(), expMsg2.size()) == 0);

    // Test invalid protocol version
    packet.Reset();
    msg.request.version = SipMessage::SIP_VER_UNKNOWN;
    _CPPUNIT_ASSERT(!SipProtocol::BuildPacket(msg, packet));

    // Test invalid header type
    packet.Reset();
    msg.request.version = SipMessage::SIP_VER_2_0;
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_UNKNOWN, "foo"));
    _CPPUNIT_ASSERT(SipProtocol::BuildPacket(msg, packet));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testBuildStatus(void)
{
    SipMessage msg;
    Packet packet;
    
    // Test build of basic status message
    msg.type = SipMessage::SIP_MSG_STATUS;
    msg.status.code = 100;
    msg.status.version = SipMessage::SIP_VER_2_0;
    msg.status.message = "Trying";

    msg.headers.clear();
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, "SIP/2.0/UDP biloxiServer.example.com:5061"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, "1j9FpLxk3uxtm8tn@biloxi.example.com"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, "1 REGISTER"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    msg.body = "Hello\n";
    
    _CPPUNIT_ASSERT(SipProtocol::BuildPacket(msg, packet));

    const std::string expMsg =
        "SIP/2.0 100 Trying\r\n"
        "Via: SIP/2.0/UDP biloxiServer.example.com:5061\r\n"
        "From: Bob <sip:1234@biloxi.example.com>\r\n"
        "To: Bob <sip:1234@biloxi.example.com>\r\n"
        "Call-ID: 1j9FpLxk3uxtm8tn@biloxi.example.com\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        "Hello\n";

    _CPPUNIT_ASSERT(packet.Length() == expMsg.size());
    _CPPUNIT_ASSERT(memcmp(packet.Data(), expMsg.data(), expMsg.size()) == 0);
    packet.Reset();

    // Test invalid protocol version
    packet.Reset();
    msg.status.version = SipMessage::SIP_VER_UNKNOWN;
    _CPPUNIT_ASSERT(!SipProtocol::BuildPacket(msg, packet));

    // Test invalid header type
    packet.Reset();
    msg.status.version = SipMessage::SIP_VER_2_0;
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_UNKNOWN, "foo"));
    _CPPUNIT_ASSERT(SipProtocol::BuildPacket(msg, packet));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseRequest(void)
{
    SipMessage msg;
    Packet packet;
    
    // Test parse of basic INVITE request
    const std::string goodReq =
        "INVITE sip:1234@biloxi.example.com SIP/2.0\r\n"
        "v: SIP/2.0/UDP biloxiServer.example.com:5061\r\n"
        "f: Alice <sip:5678@biloxi.example.com>\r\n"
        "t: Bob <sip:1234@biloxi.example.com>\r\n"
        "Call-ID: 12345600@biloxi.example.com\r\n"
        "CSeq: 1 INVITE\r\n"
        "Expires: 0\r\n"
        "m: <sip:5768@biloxi.example.com>\r\n"
        "c: application/sdp\r\n"
        "l: 0\r\n"
        "\r\n"
        "Hello\n";

    uint8_t *data = packet.Put(goodReq.size());
    memcpy(data, goodReq.data(), goodReq.size());

    _CPPUNIT_ASSERT(SipProtocol::ParsePacket(packet, msg));

    _CPPUNIT_ASSERT(msg.type == SipMessage::SIP_MSG_REQUEST);
    _CPPUNIT_ASSERT(msg.method == SipMessage::SIP_METHOD_INVITE);
    _CPPUNIT_ASSERT(msg.request.uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(msg.request.uri.GetSipUser() == "1234");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipHost() == "biloxi.example.com");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(msg.request.version = SipMessage::SIP_VER_2_0);

    _CPPUNIT_ASSERT(msg.headers.size() == 9);
    for (size_t i = 0; i < msg.headers.size(); i++)
    {
        const SipMessage::HeaderField& f = msg.headers[i];
        
        switch (i)
        {
          case 0:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_VIA);
              _CPPUNIT_ASSERT(f.value == "SIP/2.0/UDP biloxiServer.example.com:5061");
              break;

          case 1:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_FROM);
              _CPPUNIT_ASSERT(f.value == "Alice <sip:5678@biloxi.example.com>");
              break;

          case 2:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_TO);
              _CPPUNIT_ASSERT(f.value == "Bob <sip:1234@biloxi.example.com>");
              break;
              
          case 3:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CALL_ID);
              _CPPUNIT_ASSERT(f.value == "12345600@biloxi.example.com");
              break;
              
          case 4:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CSEQ);
              _CPPUNIT_ASSERT(f.value == "1 INVITE");
              break;
              
          case 5:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_EXPIRES);
              _CPPUNIT_ASSERT(f.value == "0");
              break;
              
          case 6:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CONTACT);
              _CPPUNIT_ASSERT(f.value == "<sip:5768@biloxi.example.com>");
              break;

          case 7:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CONTENT_TYPE);
              _CPPUNIT_ASSERT(f.value == "application/sdp");
              break;
              
          case 8:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CONTENT_LENGTH);
              _CPPUNIT_ASSERT(f.value == "0");
              break;
        }
    }

    _CPPUNIT_ASSERT(msg.body == "Hello\n");

    // Test parse of minimal ACK request
    const std::string minReq = "ACK sip:1234@biloxi.example.com SIP/2.0\r\n";

    packet.Reset();
    data = packet.Put(minReq.size());
    memcpy(data, minReq.data(), minReq.size());

    _CPPUNIT_ASSERT(SipProtocol::ParsePacket(packet, msg));

    _CPPUNIT_ASSERT(msg.type == SipMessage::SIP_MSG_REQUEST);
    _CPPUNIT_ASSERT(msg.method == SipMessage::SIP_METHOD_ACK);
    _CPPUNIT_ASSERT(msg.request.uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(msg.request.uri.GetSipUser() == "1234");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipHost() == "biloxi.example.com");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(msg.request.version = SipMessage::SIP_VER_2_0);
    _CPPUNIT_ASSERT(msg.headers.size() == 0);
    _CPPUNIT_ASSERT(msg.body.empty());

    // Test parse of invalid requests
    const std::string badReq1 = "ACK\r\n";

    packet.Reset();
    data = packet.Put(badReq1.size());
    memcpy(data, badReq1.data(), badReq1.size());

    _CPPUNIT_ASSERT(!SipProtocol::ParsePacket(packet, msg));

    const std::string badReq2 = "ACK sip:1234@biloxi.example.com SIP/3.0\r\n";

    packet.Reset();
    data = packet.Put(badReq2.size());
    memcpy(data, badReq2.data(), badReq2.size());

    _CPPUNIT_ASSERT(SipProtocol::ParsePacket(packet, msg));

    _CPPUNIT_ASSERT(msg.type == SipMessage::SIP_MSG_REQUEST);
    _CPPUNIT_ASSERT(msg.method == SipMessage::SIP_METHOD_ACK);
    _CPPUNIT_ASSERT(msg.request.uri.GetScheme() == SipUri::SIP_URI_SCHEME);
    _CPPUNIT_ASSERT(msg.request.uri.GetSipUser() == "1234");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipHost() == "biloxi.example.com");
    _CPPUNIT_ASSERT(msg.request.uri.GetSipPort() == 0);
    _CPPUNIT_ASSERT(msg.request.version == SipMessage::SIP_VER_UNKNOWN);
    
    // Test parse of empty request
    packet.Reset();
    _CPPUNIT_ASSERT(!SipProtocol::ParsePacket(packet, msg));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testParseStatus(void)
{
    SipMessage msg;
    Packet packet;
    
    // Test parse of basic status message
    const std::string goodResp =
        "SIP/2.0 100 Trying\r\n"
        "Via\t:\tSIP/2.0/UDP biloxiServer.example.com:5061\r\n"
        "From:Bob <sip:1234@biloxi.example.com>\r\n"
        "To: Bob <sip:1234@biloxi.example.com>\r\n"
        "Call-ID  :  1j9FpLxk3uxtm8tn@biloxi.example.com\r\n"
        "CSeq: 1\r\n"
        "\tREGISTER\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        "Hello\r\n"
        "\tHi There\n";

    uint8_t *data = packet.Put(goodResp.size());
    memcpy(data, goodResp.data(), goodResp.size());

    _CPPUNIT_ASSERT(SipProtocol::ParsePacket(packet, msg));

    _CPPUNIT_ASSERT(msg.type == SipMessage::SIP_MSG_STATUS);
    _CPPUNIT_ASSERT(msg.status.version == SipMessage::SIP_VER_2_0);
    _CPPUNIT_ASSERT(msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL);
    _CPPUNIT_ASSERT(msg.status.code == 100);
    _CPPUNIT_ASSERT(msg.status.message == "Trying");
    
    _CPPUNIT_ASSERT(msg.headers.size() == 6);
    for (size_t i = 0; i < msg.headers.size(); i++)
    {
        const SipMessage::HeaderField& f = msg.headers[i];
        
        switch (i)
        {
          case 0:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_VIA);
              _CPPUNIT_ASSERT(f.value == "SIP/2.0/UDP biloxiServer.example.com:5061");
              break;

          case 1:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_FROM);
              _CPPUNIT_ASSERT(f.value == "Bob <sip:1234@biloxi.example.com>");
              break;

          case 2:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_TO);
              _CPPUNIT_ASSERT(f.value == "Bob <sip:1234@biloxi.example.com>");
              break;
              
          case 3:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CALL_ID);
              _CPPUNIT_ASSERT(f.value == "1j9FpLxk3uxtm8tn@biloxi.example.com");
              break;
              
          case 4:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CSEQ);
              _CPPUNIT_ASSERT(f.value == "1 REGISTER");
              break;
              
          case 5:
              _CPPUNIT_ASSERT(f.type == SipMessage::SIP_HEADER_CONTENT_LENGTH);
              _CPPUNIT_ASSERT(f.value == "0");
              break;
        }
    }

    _CPPUNIT_ASSERT(msg.body == "Hello\r\n\tHi There\n");

    // Test parse of minimal response
    const std::string minResp = "SIP/2.0 201 FOO\r\n";

    packet.Reset();
    data = packet.Put(minResp.size());
    memcpy(data, minResp.data(), minResp.size());

    _CPPUNIT_ASSERT(SipProtocol::ParsePacket(packet, msg));

    _CPPUNIT_ASSERT(msg.type == SipMessage::SIP_MSG_STATUS);
    _CPPUNIT_ASSERT(msg.status.version == SipMessage::SIP_VER_2_0);
    _CPPUNIT_ASSERT(msg.status.Category() == SipMessage::SIP_STATUS_SUCCESS);
    _CPPUNIT_ASSERT(msg.status.code == 201);
    _CPPUNIT_ASSERT(msg.headers.size() == 0);
    _CPPUNIT_ASSERT(msg.body.empty());

    // Test parse of invalid responses
    const std::string badResp1 = "SIP/2.0\r\n";

    packet.Reset();
    data = packet.Put(badResp1.size());
    memcpy(data, badResp1.data(), badResp1.size());

    _CPPUNIT_ASSERT(!SipProtocol::ParsePacket(packet, msg));

    const std::string badResp2 =
        "SIP/2.0 201 FOO\r\n"
        "\tFoo\r\n"
        "Via: SIP/2.0/UDP biloxiServer.example.com:5061\r\n";

    packet.Reset();
    data = packet.Put(badResp2.size());
    memcpy(data, badResp2.data(), badResp2.size());

    _CPPUNIT_ASSERT(!SipProtocol::ParsePacket(packet, msg));
    
    // Test parse of empty response
    packet.Reset();
    _CPPUNIT_ASSERT(!SipProtocol::ParsePacket(packet, msg));
}

///////////////////////////////////////////////////////////////////////////////

void TestSipProtocol::testHeaderUtils(void)
{
    SipMessage msg;
    
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, "SIP/2.0/UDP mobileServer.example.com:5063"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, "SIP/2.0/UDP biloxiServer.example.com:5063"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_VIA, "SIP/2.0/UDP biloxi.example.com:5063"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_FROM, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_TO, "Bob <sip:1234@biloxi.example.com>"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CALL_ID, "1j9FpLxk3uxtm8tn@biloxi.example.com"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CSEQ, "1 REGISTER"));
    msg.headers.push_back(SipMessage::HeaderField(SipMessage::SIP_HEADER_CONTENT_LENGTH, "0"));

    // Test FindHeader()
    const SipMessage::HeaderField *hdr = msg.FindHeader(SipMessage::SIP_HEADER_VIA);
    _CPPUNIT_ASSERT(hdr != 0 && hdr->type == SipMessage::SIP_HEADER_VIA && hdr->value == "SIP/2.0/UDP mobileServer.example.com:5063");
    _CPPUNIT_ASSERT(msg.FindHeader(SipMessage::SIP_HEADER_EXPIRES) == 0);

    // Test PushHeader functor
    SipMessage copy;
    msg.ForEachHeader(SipMessage::SIP_HEADER_VIA, SipMessage::PushHeader(copy));
    _CPPUNIT_ASSERT(copy.headers.size() == 3);
    hdr = copy.FindHeader(SipMessage::SIP_HEADER_VIA);
    _CPPUNIT_ASSERT(hdr != 0 && hdr->type == SipMessage::SIP_HEADER_VIA && hdr->value == "SIP/2.0/UDP mobileServer.example.com:5063");
}

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(TestSipProtocol);
