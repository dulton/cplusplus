#include "../client.h"
#include "../connectionlistener.h"
#include "../discohandler.h"
#include "../disco.h"
#include "../rostermanager.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../messagehandler.h"
#include "../message.h"
#include "../presence.h"
#include "../pubsubmanager.h"
#include "../pubsubresulthandler.h"
#include "../pubsubitem.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

#define VERSION "1.0"
#define HARNESS "http://ntaforum.org/harness"
#define MAX_ITEMS "100"


/*	This script shuld be run by the server administrator to create the pubsub node
	that will be used by the NTAF tools.						*/

class CreateNode : public ConnectionListener, LogHandler, MessageHandler, PubSub::ResultHandler
{
  public:
    CreateNode(const std::string& server, const std::string& userName, const std::string& password, const std::string& nodename ) : servAdd(server), myUserName(userName), myPassword(password), nodeName(nodename){}
    virtual ~CreateNode() {}

    void start()
    {

      JID jid( myUserName + "@" + servAdd );
      j = new Client( jid, myPassword );

      //Setup connection listener for connection related events
      j->registerConnectionListener( this );
      j->registerMessageHandler( this );


      //Setup this identity http://xmpp.org/registrar/disco-categories.html 
      j->disco()->setVersion( "STC_XMPP_Client", VERSION );
      j->disco()->setIdentity( "client", "bot" );

      //setup pubsub manager
      pubsub = new PubSub::Manager( j );

      //Setup log event handler
      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

      j->connect();

      delete( j );
    }



    //this gets called once connect is established, presence is sent out automatically
    virtual void onConnect()
    {	

	//create and configure the node. will send back error if already exists
	//see handleNodeCreation

	pubsub->createNode(JID("pubsub." + servAdd),nodeName,NULL,this); 

	pubsub->getNodeConfig(JID("pubsub." + servAdd),nodeName, this);
	
	//pubsub->getDefaultNodeConfig(JID("pubsub." + servAdd), PubSub::NodeLeaf, this);

	


    }

    //this gets called when disconnect is called
    virtual void onDisconnect( ConnectionError e ) { printf( "disco_test: disconnected: %d auth error: %d\n", e, j->authError() ); }

    //this gets called when tls connection is established to view certificate
    //return true to authorize, have not seen this invoked yet
    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void onResourceBindError( ResourceBindError error )
    {
      printf( "onResourceBindError: %d\n", error );
    }

    virtual void onSessionCreateError( SessionCreateError error )
    {
      printf( "onSessionCreateError: %d\n", error );
    }



    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
        printf( "msg: %s\n", msg.tag()->xml().c_str() );
    }

	//will this receive all IQ's including items from pub?
/*    virtual bool handleIq  (const IQ& iq) {
	printf("received an iq -------------------------------------->\n");
    }*/

    //after node for ntaf is created, this function is called
    virtual void handleNodeCreation( 	const std::string & /*id*/, const JID & service, 
					const std::string & node, const Error * error )
    {
    	printf("created node\n");
    	if (error){
		std::string errorString = error->tag()->children().front()->name();
      		printf( "created node '%s' on '%s' error: %s\n", node.c_str(), service.bare().c_str(), 
      								errorString.c_str() );
      		}
    }


virtual void handleItem(const gloox::JID&, const std::string&, const gloox::Tag*)
{printf("hellop\n");}
virtual void handleItems(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*)
{printf("helloo\n");}
virtual void handleItemPublication(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*)
{printf("helloqwd\n");}
virtual void handleItemDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*)
{printf("hellon\n");}
virtual void handleSubscriptionResult(const std::string&, const gloox::JID&, const std::string&, const std::string&, const gloox::JID&, gloox::PubSub::SubscriptionType, const gloox::Error*)
{printf("hellom\n");}
virtual void handleUnsubscriptionResult(const std::string&, const gloox::JID&, const gloox::Error*)
{printf("hellol\n");}
virtual void handleSubscriptionOptions(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const gloox::DataForm*, const gloox::Error*)
{printf("hellok\n");}
virtual void handleSubscriptionOptionsResult(const std::string&, const gloox::JID&, const gloox::JID&, const std::string&, const gloox::Error*)
{printf("helloj\n");}
virtual void handleSubscribers(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriberList*, const gloox::Error*)
{printf("helloi\n");}
virtual void handleSubscribersResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::SubscriberList*, const gloox::Error*)
{printf("helloh\n");}
virtual void handleAffiliates(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*)
{printf("hellog\n");}
virtual void handleAffiliatesResult(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::AffiliateList*, const gloox::Error*)
{printf("hellof\n");}
virtual void handleNodeConfig(const std::string&, const gloox::JID&, const std::string&, const gloox::DataForm* dForm, const gloox::Error*)
{   //printf("hellaergliubno\n\n\n\n\n");

	//1. prints out current configuration
	//2. Creates new dataform and sends it in

	//printf("NODE CONFIGURATION\n\n\n %s\n\n\n\n",dForm->tag()->xml().c_str());
	
 	//get form data
 	Tag* newTag = dForm->tag();
 	
 	//set access model to open
 	newTag->findChild("field","var","pubsub#publish_model")->findChild("value")->setCData("open");
 	
 	//set persistent items true and max items
 	newTag->findChild("field","var","pubsub#persist_items")->findChild("value")->setCData("1");
 	newTag->findChild("field","var","pubsub#max_items")->findChild("value")->setCData(MAX_ITEMS);
 	
 	//change form type to submit
 	newTag->removeAttribute("type");
 	newTag->addAttribute("type","submit");  //very important to properly set the node config
 	
 	printf("publish model type is %s\n\n\n\n", newTag->findChild("field","var","pubsub#publish_model")->findChild("value")->xml().c_str());
 	
 	DataForm* newDForm = new DataForm(newTag);
 	
 	printf("new configuration has model set to %s\n\n\n",newDForm->tag()->findChild("field","var","pubsub#publish_model")->findChild("value")->xml().c_str());
 	
 	//set new configuration
 	pubsub->setNodeConfig(JID("pubsub." + servAdd), nodeName, newDForm, this);
	
	j->disconnect();
	
	

}
virtual void handleNodeConfigResult(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{printf("\n\n\nhellon\n\n\n");}

virtual void handleNodeDeletion(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{printf("helloe\n");}
virtual void handleNodePurge(const std::string&, const gloox::JID&, const std::string&, const gloox::Error*)
{printf("hellod\n");}
virtual void handleSubscriptions(const std::string&, const gloox::JID&, const gloox::PubSub::SubscriptionMap&, const gloox::Error*)
{printf("helloc\n");}
virtual void handleAffiliations(const std::string&, const gloox::JID&, const gloox::PubSub::AffiliationMap&, const gloox::Error*)
{printf("hellob\n");}
virtual void handleDefaultNodeConfig(const std::string&, const gloox::JID&, const gloox::DataForm* dForm, const gloox::Error* error)
{printf("helloa\n");

	if (error){

		printf("error has occurred\n");
	}
	else if (dForm){
		//printf("DEFAULT NODE CONFIGURATION\n\n\n %s\n\n\n\n",dForm->tag()->xml().c_str());
	}


}

  private:
    	Client *j;
	std::string servAdd,myUserName,myPassword;
	PubSub::Manager *pubsub;
	std::string nodeName;
};

int main( int argc, char* argv[] )
{
  std::string server,userName,password, nodename;
  if (argc > 4) {
	server.assign(argv[1]);
	userName.assign(argv[2]);
	password.assign(argv[3]);
	nodename.assign(argv[4]);
  } else {
	perror("Correct Params: <server address> <username> <password> <node name>");
	return 1;
  }

  CreateNode *r = new CreateNode(server,userName,password, nodename);
  r->start();
  delete( r );
  return 0;
}
