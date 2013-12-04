#include <base/BaseCommon.h>
#include <vif/IfSetupCommon.h>
#include <cstdio>
#include <ace/Signal.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "XmppCommon.h"
#include "XmppClientBlock.h"
#include "CapabilitiesParser.h"
//#include "UniqueIndexTracker.h"
#include <XmppClientPEPManager.h>
#include <client.h>
#include <connectiontcpclient.h>
#include <pubsubresulthandler.h>
#include <pubsubitem.h>

USING_XMPP_NS;

class TestXmppUtilities : public CPPUNIT_NS::TestCase
{
    CPPUNIT_TEST_SUITE(TestXmppUtilities);
    CPPUNIT_TEST(testCapabilitiesParser);
    CPPUNIT_TEST(testIndexTracker);
    CPPUNIT_TEST(testPEPManager);
//    CPPUNIT_TEST(testMaxClients);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void) {    ACE_OS::signal(SIGPIPE, SIG_IGN);}
    void tearDown(void) {}

protected:
    void testCapabilitiesParser(void);
    void testIndexTracker(void);
    void testPEPManager(void);
    void testMaxClients(void);
};
///////////////////////////////////////////////////////////////////////////////

void TestXmppUtilities::testCapabilitiesParser(void)
{
    FILE* pFile;
    pFile = fopen("capfile.xml","w");
    CPPUNIT_ASSERT(pFile != NULL);
    fputs("<cloud-element-capabilities>\n\t<performance-capabilities att1='val1' att2='val2'>\n\t\t<capability>\n\t\t\t<capability-type>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</performance-capabilities>\n\t<network-capabilities>\n\t\t<capability>\n\t\t\t<capability-type>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</network-capabilities>\n\t<firewall-capabilities att3='val3' att4='val4'>\n\t\t<capability>\n\t\t\t<capability-type>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</firewall-capabilities>\n\t<load-balancer-capabilities>\n\t\t<capability>\n\t\t\t<capability-type>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</load-balancer-capabilities>\n\t<compute-capabilities>\n\t\t<capability>\n\t\t\t<capability-type att5='val5' att6='val6'>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</compute-capabilities>\n\t<storage-capabilities att7='val7' att8='val8'>\n\t\t<capability>\n\t\t\t<capability-type>[string]</capability-type>\n\t\t\t<allocation-units>[string]</allocation-units>\n\t\t\t<quantity>[integer]</quantity>\n\t\t\t<aggregation-method>[string]</aggregation-method>\n\t\t</capability>\n\t</storage-capabilities>\n</cloud-element-capabilities>",pFile);
    fclose(pFile);

    std::vector<gloox::Tag*> mPubCapabilities;
    CapabilitiesParser parser(mPubCapabilities);
    parser.Parse("capfile.xml");
    CPPUNIT_ASSERT(remove("capfile.xml") == 0);
    CPPUNIT_ASSERT(mPubCapabilities[0]->name() == "performance-capabilities");
    CPPUNIT_ASSERT(mPubCapabilities[1]->name() == "network-capabilities");
    CPPUNIT_ASSERT(mPubCapabilities[2]->name() == "firewall-capabilities");
    CPPUNIT_ASSERT(mPubCapabilities[3]->name() == "load-balancer-capabilities");
    CPPUNIT_ASSERT(mPubCapabilities[4]->name() == "compute-capabilities");
    CPPUNIT_ASSERT(mPubCapabilities[5]->name() == "storage-capabilities");

    CPPUNIT_ASSERT(mPubCapabilities[0]->findAttribute("att1") == "val1");
    CPPUNIT_ASSERT(mPubCapabilities[0]->findAttribute("att2") == "val2");
    CPPUNIT_ASSERT(mPubCapabilities[2]->findAttribute("att3") == "val3");
    CPPUNIT_ASSERT(mPubCapabilities[2]->findAttribute("att4") == "val4");
    CPPUNIT_ASSERT(mPubCapabilities[5]->findAttribute("att7") == "val7");
    CPPUNIT_ASSERT(mPubCapabilities[5]->findAttribute("att8") == "val8");

    CPPUNIT_ASSERT(mPubCapabilities[4]->findChild("capability")->findChild("capability-type")->hasAttribute("att5","val5"));
    CPPUNIT_ASSERT(mPubCapabilities[4]->findChild("capability")->findChild("capability-type")->hasAttribute("att6","val6"));

    for (int i = 0; i<mPubCapabilities.size(); ++i)
    {
        CPPUNIT_ASSERT(mPubCapabilities[i]->hasChild("capability"));
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->hasChild("capability-type"));
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->findChild("capability-type")->cdata() == "[string]");
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->hasChild("allocation-units"));
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->findChild("allocation-units")->cdata() == "[string]");
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->hasChild("quantity"));
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->findChild("quantity")->cdata() == "[integer]");
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->hasChild("aggregation-method"));
        CPPUNIT_ASSERT(mPubCapabilities[i]->findChild("capability")->findChild("aggregation-method")->cdata() == "[string]");
        delete mPubCapabilities[i];
    }
   
}

///////////////////////////////////////////////////////////////////////////////

void TestXmppUtilities::testIndexTracker(void)
{
    UniqueIndexTracker mTracker;

    for (uint32_t i = 0; i < 1000; ++i)
    {
        CPPUNIT_ASSERT(mTracker.Assign() == i);
    } 

    for (uint32_t j = 0; j < 1000; j+=2)
    {
        mTracker.Release(j);
    }
    mTracker.Flip();
    for (uint32_t k = 1; k < 1000; k+=2)
    {
        CPPUNIT_ASSERT(mTracker.Assign() == k);
    }

}

///////////////////////////////////////////////////////////////////////////////

class unitTestClient : public gloox::ConnectionTCPClient, public gloox::PubSub::ResultHandler
{
public:
    unitTestClient( const std::string& server, gloox::LogSink& sink ) : ConnectionTCPClient(sink,server) 
    {
        m_state = gloox::StateConnected;
    }
    bool send(const std::string &data)
    {
        //printf("%s\n",data.c_str());
        pubOut = data;
        return true;
    }

    void handleItem(const gloox::JID&, const std::string&, const gloox::Tag*) {}
    void handleItems(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*) {}
    void handleItemPublication(const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error) {}
    void handleItemDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*) {}
    void handleSubscriptionResult(const std::string&, const gloox::JID&, const std::string&, const std::string&, const gloox::JID&, const gloox::PubSub::SubscriptionType, const gloox::Error*) {}
    void handleUnsubscriptionResult(const std::string&, const gloox::JID&, const gloox::Error*) {}
    void handleSubscriptionOptions(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const gloox::DataForm*, const gloox::Error*) {}
    void handleSubscriptionOptionsResult(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const gloox::Error*) {}
    void handleSubscribers(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriberList*, const gloox::Error*) {}
    void handleSubscribersResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriberList*, const gloox::Error*) {}
    void handleAffiliates(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*) {}
    void handleAffiliatesResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*) {}
    void handleNodeConfig(const std::string&, const gloox::JID&, const std::string&, const gloox::DataForm*, const gloox::Error*) {}
    void handleNodeConfigResult(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*) {}
    void handleNodeCreation(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*) {}
    void handleNodeDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*) {}
    void handleNodePurge(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*) {}
    void handleSubscriptions(const std::string&, const gloox::JID&, const gloox::PubSub::SubscriptionMap&, const gloox::Error*) {}
    void handleAffiliations(const std::string&, const gloox::JID&, const gloox::PubSub::AffiliationMap&, const gloox::Error*) {}
    void handleDefaultNodeConfig(const std::string&, const gloox::JID&, const gloox::DataForm*, const gloox::Error*) {}

    std::string pubOut;
};

void TestXmppUtilities::testPEPManager(void)
{
    gloox::Client* j = new gloox::Client("testserver");
    unitTestClient* utc = new unitTestClient("testdomain",j->logInstance());
    j->setConnectionImpl(utc);
    j->setUsername("testuser");
    gloox::PubSub::PEPManager* mPubSub = new gloox::PubSub::PEPManager(j);
    //CPPUNIT_ASSERT(false);
    gloox::Tag* pubTag; // = new gloox::Tag("item");
    gloox::PubSub::ItemList ps_itemList;
    gloox::PubSub::Item* pubItem;
    std::string node("http://jabber.org/protocol/capability-root");

    gloox::Tag* pubCap = new gloox::Tag("capability");
    pubCap->addAttribute("att1","val1");
    gloox::Tag* child1 = new gloox::Tag(pubCap,"child1","child cdata");
    child1->addAttribute("childatt","childval");
    gloox::Tag* child2 = new gloox::Tag(pubCap,"child2");
    gloox::Tag* gChild = new gloox::Tag(child2,"grandbaby");

    pubTag = new gloox::Tag("item");
    pubTag->addChild(pubCap);
    pubItem = new gloox::PubSub::Item(pubTag);
    ps_itemList.push_back(pubItem);
    std::string iqID = mPubSub->publishPEPItem(node,ps_itemList,NULL,utc);

    CPPUNIT_ASSERT(utc->pubOut == "<iq id='" + iqID + "' type='set' xmlns='jabber:client'><pubsub xmlns='http://jabber.org/protocol/pubsub'><publish node='" + node + "'><item><capability att1='val1'><child1 childatt='childval'>child cdata</child1><child2><grandbaby/></child2></capability></item></publish></pubsub></iq>");

    delete j;
    delete mPubSub;
    
}

///////////////////////////////////////////////////////////////////////////////

void TestXmppUtilities::testMaxClients(void)
{


}

CPPUNIT_TEST_SUITE_REGISTRATION(TestXmppUtilities);




