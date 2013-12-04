#include "../client.h"
#include "../connectionlistener.h"
#include "../registration.h"
#include "../logsink.h"
#include "../loghandler.h"

#include <stdio.h>
#include <locale.h>
#include <string>

#include <cstdio> // [s]print[f]

using namespace gloox;


class RegSTC : public RegistrationHandler, ConnectionListener, LogHandler
{
  public:
	
    //Constructor. Takes in server address, username, password for future reference
    RegSTC( const std::string& server, const std::string& userName, const std::string& password ) : 
		servAdd(server), myUserName(userName), myPassword(password) {}

    //Destructor
    virtual ~RegSTC() {}

    void start()
    {

	//sets up server information
	j = new Client( servAdd );
	j->disableRoster();

	j->registerConnectionListener( this );	//connection listener listens for this connection
						//will call onConnect once connection and authentication is established
						//presence stanza will be sent 

	m_reg = new Registration( j );			//setup new registration instance
	m_reg->registerRegistrationHandler( this );	//register it so received events

	j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );   //setup log

	j->connect();   //blocks here until the connection is finished
	
	delete m_reg;
	delete j;
    }

	//called when initial connection and authorization setup with server
    virtual void onConnect()
    {

	//request fields from registration
	m_reg->fetchRegistrationFields();

    }

    virtual void onDisconnect( ConnectionError e ) { 

	printf( "register_test: disconnected error: %d\n", e); 
	}

    //TLS connection automatically setup
    virtual bool onTLSConnect( const CertInfo& info )
    {
      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
              info.status, info.issuer.c_str(), info.server.c_str(),
              info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
              info.compression.c_str() );
      return true;
    }

    virtual void handleRegistrationFields( const JID& /*from*/, int fields, std::string instructions )
    {
      printf( "fields: %d\ninstructions: %s\n", fields, instructions.c_str() );
      RegistrationFields vals;
      vals.username = myUserName;
      vals.password = myPassword;
      m_reg->createAccount( fields, vals );

    }

    virtual void handleRegistrationResult( const JID& from, RegistrationResult result )
    {
      printf( "result: %d\n", result );

	if (result == RegistrationConflict) 
		handleAlreadyRegistered( from );

      j->disconnect();

    }

    virtual void handleAlreadyRegistered( const JID& /*from*/ )
    {
      printf( "the account already exists.\n" );
    }

    virtual void handleDataForm( const JID& /*from*/, const DataForm& /*form*/ )
    {
      printf( "datForm received\n" );
    }

    virtual void handleOOB( const JID& /*from*/, const OOB& oob )
    {
      printf( "OOB registration requested. %s: %s\n", oob.desc().c_str(), oob.url().c_str() );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

 
    private:
	    //jabber server address
    	    Registration *m_reg;
    	    Client *j;
    	    std::string servAdd,myUserName,myPassword;

};

int main( int argc, char* argv[] )
{
  std::string server,userName,password;
  if (argc > 3) {
	server.assign(argv[1]);
	userName.assign(argv[2]);
	password.assign(argv[3]);
  } else {
	perror("Correct Params: <server address> <username> <password>");
	return 1;
  }
  RegSTC *r = new RegSTC(server,userName,password);
  r->start();
  delete( r );
  return 0;
}
