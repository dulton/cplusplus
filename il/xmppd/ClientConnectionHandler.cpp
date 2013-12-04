
/// @file
/// @brief Client Connection Handler implementation
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include <cstdlib>
#include <sstream>

#include <ace/Message_Block.h>
#include <utils/MessageUtils.h>
#include <utils/MessageIterator.h>
#include <xmppvj_1_port_server.h>
#include <hal/TimeStamp.h>
#include <pubsubitem.h>

#include "ClientConnectionHandler.h"
#include "XmppClientRawStats.h"
#include "XmppdLog.h"
#include "UniqueIndexTracker.h"


USING_XMPP_NS;
using namespace XmppvJ_1_port_server;
using namespace xmppvj_1_port_server;
#define CCHGuard(A) ACE_Guard<ACE_Recursive_Thread_Mutex> __lock__##__LINE__(A) 

///////////////////////////////////////////////////////////////////////////////

ClientConnectionHandler::ClientConnectionHandler(uint32_t serial)
    : L4L7_APP_NS::StreamSocketHandler(serial),
      ConnectionTCPClient(mLoger, "testdomain"),
      mSessionType(EnumSessionType_LOGIN_ONLY),
      mClientMode(EnumClientMode_PUB),
      mSessionDuration(0),
      mTimedPubSubSessionRampType(EnumTimedPubSubSessionRamp_RAMP_UP),
      mPubInterval(0),
      mPubCapabilities(0),
      mPubCounter(0),
      mStats(0),
      mState(IDLE),
      mComplete(false),
      mUserResource("spirent"),
      mCapPubSubTimeoutDelegate(fastdelegate::MakeDelegate(this, &ClientConnectionHandler::HandleCapPubSubTimeout)),
      mSessionDurationTimeoutDelegate(fastdelegate::MakeDelegate(this, &ClientConnectionHandler::HandleSessionDurationTimeout)),
      mRegistered(false),
      mClientIndex(UniqueIndexTracker::INVALID_INDEX),
      mRegRetryCount(0),
      mAutoGenID(false),
      mGlooxClient(0),
      m_reg(0),
      mPubSub(0),
      mMsgID(0),
      mMUCRoom(0)
{
}

///////////////////////////////////////////////////////////////////////////////

gloox::ConnectionError ClientConnectionHandler::connect()
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    m_sendMutex.lock();
    if( !m_handler )
    {
        m_sendMutex.unlock();
        return gloox::ConnNotConnected;
    }
 
    m_sendMutex.unlock();

    m_state = gloox::StateConnected;
 
    m_cancel = false;
    m_handler->handleConnect( this );
    return gloox::ConnNoError;
}


bool ClientConnectionHandler::send(const std::string &data) 
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);

	m_sendMutex.lock();
	m_totalBytesOut += (int)data.length();
	m_sendMutex.unlock();
	return Send(data);
}


void ClientConnectionHandler::cleanup()
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if( !m_sendMutex.trylock() )
        return;
  
    if( !m_recvMutex.trylock() )
    {
        m_sendMutex.unlock();
        return;
    }
  
    m_state = gloox::StateDisconnected;
    m_cancel = true;
    m_totalBytesIn = 0;
    m_totalBytesOut = 0;
  
    m_recvMutex.unlock(),
    m_sendMutex.unlock();
}



//onConnect is called once stream is established or authentication is complete
void ClientConnectionHandler::onConnect()
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);

    switch(mClientType)
    {
      //if Register only, send registration
      case CLIENT_REGISTER:
          if(!SendRegistration())
          {
              NotifyRegistrationCompleted(false);
              return;
          }
          break;

      //if unregister only
      case CLIENT_UNREGISTER:
          if(mState == STREAM) { //client must login to unregister
              if(!SendLogin()) {
                  if (mStats) {
                      ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                      mStats->loginFailureCount++;
                  }
              }
          } else {
              if (mStats)
              {
                  ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                  mStats->loginSuccessCount++;
              }
              //send unregister logout
              const ACE_Time_Value now = ACE_OS::gettimeofday();

              InitiateLogout(now);
          }
          break;
      case CLIENT_LOGIN:
          if(mState == STREAM)
          {   
              if(mAutoGenID)
              {
                  //Register
                  if(!SendRegistration())
                  {
                      NotifyRegistrationCompleted(false);
                      return;
                  }
                  //mState = REGISTER;
              } else //if standard protocol and not auto generate, immediately send login
              {
                  if(!SendLogin()) {
                      if (mStats) {
                          ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                          mStats->loginFailureCount++;
                      }
                      return;
                  }
              }
          }  else { //client is logged in
              if (mStats)
              {
                  ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                  mStats->loginSuccessCount++;
              }

              //join mucroom
              if(mMUCRoom)
                  mMUCRoom->join();
      
              switch(GetSessionType())
              {
                case EnumSessionType_LOGIN_ONLY:
                    break;
                case EnumSessionType_FULL_PUB_SUB:
                case EnumSessionType_TIMED_PUB_SUB:
                    // just for scheduling sake.  full/timed differentiation is in SendPublish()
                    if (GetClientMode() == EnumClientMode_PUB)
                    {
                        mState = PUBSUB;
                        ScheduleTimer(CAPABILITY_PUBSUB_TIMEOUT, 0, mCapPubSubTimeoutDelegate, ACE_OS::gettimeofday() + mFirstInterval);
                    } else if (GetClientMode() == EnumClientMode_SUB) {
                        // TODO:
                        // do nothing for subscribe for now
                    }
                    break;
                default:
                    TC_LOG_ERR(0, "ClientConnectionHandler::LoginState::ProcessInput(): unsupported session type ");
              }
          }
          break;
      default:
          break;
    }
}

void ClientConnectionHandler::onDisconnect(gloox::ConnectionError e)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    switch(e) 
    {
        case gloox::ConnNoError:
            break;
        case gloox::ConnStreamError:
            break;
        case gloox::ConnAuthenticationFailed:
            {
                if(mStats){
                    ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                    mStats->loginFailureCount++;
                }
            }
            break;
        default:
            if(e != gloox::ConnUserDisconnected)
                TC_LOG_ERR(0, "Client "<<mClientIndex<<" Connection error.:"<<e);
			break;
    }
}

void ClientConnectionHandler::handleErrorCntr(const gloox::StanzaError error)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if(mStats){
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        switch(error)
        {
            case gloox::StanzaErrorBadRequest:
                mStats->rxResponseCode400++;
                break;
            case gloox::StanzaErrorConflict:
                mStats->rxResponseCode409++;
                break;
            case gloox::StanzaErrorFeatureNotImplemented:
                mStats->rxResponseCode501++;
                break;
            case gloox::StanzaErrorForbidden:
                mStats->rxResponseCode403++;
                break;
            case gloox::StanzaErrorGone:
                mStats->rxResponseCode302++;
                break;
            case gloox::StanzaErrorInternalServerError:
                mStats->rxResponseCode500++;
                break;
            case gloox::StanzaErrorItemNotFound:
                mStats->rxResponseCode404++;
                break;
            case gloox::StanzaErrorJidMalformed:
                mStats->rxResponseCode400++;
                break;
            case gloox::StanzaErrorNotAcceptable:
                mStats->rxResponseCode406++;
                break;
            case gloox::StanzaErrorNotAllowed:
                mStats->rxResponseCode405++;
                break;
            case gloox::StanzaErrorNotAuthorized:
                mStats->rxResponseCode401++;
                break;
            case gloox::StanzaErrorNotModified:     //does not correspond with legacy error code
                break;
            case gloox::StanzaErrorPaymentRequired:
                mStats->rxResponseCode402++;
                break;
            case gloox::StanzaErrorRecipientUnavailable:
                mStats->rxResponseCode404++;
                break;
            case gloox::StanzaErrorRedirect:
                mStats->rxResponseCode302++;
                break;
            case gloox::StanzaErrorRegistrationRequired:
                mStats->rxResponseCode407++;
                break;
            case gloox::StanzaErrorRemoteServerNotFound:
                mStats->rxResponseCode404++;
                break;
            case gloox::StanzaErrorRemoteServerTimeout:
                mStats->rxResponseCode504++;
                break;
            case gloox::StanzaErrorResourceConstraint:
                mStats->rxResponseCode500++;
                break;
            case gloox::StanzaErrorServiceUnavailable:
                mStats->rxResponseCode503++;
                break;
            case gloox::StanzaErrorSubscribtionRequired:
                mStats->rxResponseCode407++;
                break;
            case gloox::StanzaErrorUndefinedCondition:
                mStats->rxResponseCode500++;
                break;
            case gloox::StanzaErrorUnexpectedRequest:
                mStats->rxResponseCode400++;
                break;
            case gloox::StanzaErrorUnknownSender:    //does not correspond with legacy error code
                break;
            case gloox::StanzaErrorUndefined:
                break;
        }
    }
}


void ClientConnectionHandler::onResourceBind(const std::string &resource){
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    return;
}

void ClientConnectionHandler::onResourceBindError(const gloox::Error *error){
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    handleErrorCntr(error->error());

}

bool ClientConnectionHandler::onTLSConnect(const gloox::CertInfo &info){
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    return true;
}

void ClientConnectionHandler::onStreamEvent(gloox::StreamEvent event)
{
    CCHGuard(mLock);
    std::string eventStr("unknown event");
    switch(event) {
        case gloox::StreamEventConnecting: 		
            eventStr = "The Client is about to initaite the connection.";
            break;
        case gloox::StreamEventEncryption: 
            eventStr = "The Client is about to negotiate encryption.";
            break;
        case gloox::StreamEventCompression: 		
            eventStr = "The Client is about to negotiate compression.";
            break;
        case gloox::StreamEventAuthentication: 		
            eventStr = "The Client is about to authenticate.";
            break;
        case gloox::StreamEventSessionInit: 		
            eventStr = "The Client is about to create a session.";
            break;
        case gloox::StreamEventResourceBinding: 	
            eventStr = "The Client is about to bind a resource to the stream.";
            break;
        case gloox::StreamEventSessionCreation: 	
            eventStr = "The Client is about to create a session.";
            break;
        case gloox::StreamEventRoster: 			
            eventStr = "The Client is about to request the roster.";
            break;
        case gloox::StreamEventFinished: 		
            eventStr = "The log-in phase is completed.";
            break; 
    }
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s %s.",__func__,eventStr.c_str());
}

void ClientConnectionHandler::handleLog(gloox::LogLevel /*level*/, gloox::LogArea /*area*/, const std::string& message){
    CCHGuard(mLock);
    PHX_LOG_LOCAL(PHX_LOG_DEBUG,LOG_CLIENT,"%s",message.c_str());
    return;
}

void ClientConnectionHandler::handleRegistrationFields(const gloox::JID &from, int fields, std::string instructions){
    CCHGuard(mLock);
    return;
}

//if createaccount is called when stream is already authenticated
void ClientConnectionHandler::handleAlreadyRegistered(const gloox::JID &from){
    CCHGuard(mLock);
    return;
}

void ClientConnectionHandler::handleRegistrationResult(const gloox::JID &from, gloox::RegistrationResult regResult){
    CCHGuard(mLock);
    const ACE_Time_Value now = ACE_OS::gettimeofday();

    switch(regResult) 
    {
        case gloox::RegistrationSuccess: //if registration or account removal were successful
            // Case that auto generate is enabled, client will register first, get result, then login.
            if( mClientType == CLIENT_LOGIN && mState == REGISTER ) 
            {
                if (mStats)
                {
                    ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                    mStats->registrationSuccessCount++;
                }
                if(!SendLogin()) {
                    if (mStats) {
                        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                        mStats->loginFailureCount++;
                    }
                }
            }
            else if ( mClientType == CLIENT_REGISTER )
            {
                NotifyRegistrationCompleted(true);
            }
            break;
        case gloox::RegistrationNotAcceptable:  //Not all necessary information provided
            if (mStats)
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                mStats->rxResponseCode406++;
            }
            break;
        case gloox::RegistrationConflict:  //Username alreday exists
            if (mStats)
            {
                ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
                mStats->rxResponseCode409++;
            }
            if ( mClientType == CLIENT_REGISTER )
            {
                NotifyRegistrationCompleted(true);
            }
            break;
        case gloox::RegistrationRequired:  //The entity sending the remove request was not previously registered
            break;
        case gloox::RegistrationUnexpectedRequest:  //registration unexpected request
        case gloox::RegistrationNotAllowed:  //Password change: The server or service does not allow password changes
        case gloox::RegistrationUnknownError:  //An unknown error condition occured
        case gloox::RegistrationNotAuthorized:  //registration not authorized
        case gloox::RegistrationBadRequest:  //registration bad request
        case gloox::RegistrationForbidden:  //The sender does not have sufficient permissions to cancel the registration.
        default: 
            TC_LOG_ERR(0, "ClientConnectionHandler::handleRegistrationResult:  failed");
            if ( mClientType == CLIENT_REGISTER ) {
                //if any other errors do not retry
                mRegRetryCount = 0;
                NotifyRegistrationCompleted(false);
            }
            break;
    }
}

void ClientConnectionHandler::handleDataForm(const gloox::JID &from, const gloox::DataForm &form){
    CCHGuard(mLock);
    return;
}

void ClientConnectionHandler::handleOOB(const gloox::JID &from, const gloox::OOB &oob){
    CCHGuard(mLock);
    return;
}

///////////////////////////////////////////////////////////////////////////////

ClientConnectionHandler::~ClientConnectionHandler()
{
    CCHGuard(mLock);
    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        mStats->totalClientCount--;
    }
    UnregisterUpdateTxCountDelegate();
    if(m_reg){
        m_reg->removeRegistrationHandler();
        delete m_reg;
        m_reg = 0;
    }
    if(mGlooxClient){
        if(!registerClient())
            mGlooxClient->rosterManager()->removeRosterListener();

        if(mMUCRoom){
            mMUCRoom->leave();
            mMUCRoom->removeMUCRoomHandler();
            delete mMUCRoom;
            mMUCRoom = 0 ;
        }

        mGlooxClient->removeMessageHandler(this);
        mGlooxClient->setConnectionImpl(NULL);
        mGlooxClient->logInstance().removeLogHandler( this);
        mGlooxClient->removeConnectionListener(this);
        this->gloox::ConnectionTCPClient::registerConnectionDataHandler(0);
        if(mPubSub){
            delete mPubSub;
            mPubSub = 0;
        }
        delete mGlooxClient;
        mGlooxClient = 0;
    }

}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::SendRegistration(void) {
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if(!m_reg)
        return false;
    gloox::RegistrationFields vals;
    vals.username = mUserID.str();
    vals.password = mUserPW.str();

    if (mStats) {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), false);
        mStats->registrationAttemptCount++;
    }

    if (m_reg->createAccount(77,vals))
    {
        mState = REGISTER;
        return true;
    }
    return false;

}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::SendPublish(void) {
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if(!mPubSub)
        return false;
    int lowerTag = 0;
    int upperTag = (int)mPubCapabilities->size();

    //setup mesage id and insert to stanza
    std::ostringstream msgid;
    msgid<<++mMsgID;


    gloox::Tag* pubTag;
    gloox::PubSub::ItemList ps_itemList;
    gloox::PubSub::Item* pubItem;
    std::string node("http://jabber.org/protocol/capability-root");

    if ( mSessionType == EnumSessionType_FULL_PUB_SUB )
    {
    }
    else
    {
        switch(mTimedPubSubSessionRampType)
        {
          case EnumTimedPubSubSessionRamp_RAMP_DOWN:
              upperTag = mPubCapabilities->size() - (mPubCounter%mPubCapabilities->size());
              break;
          case EnumTimedPubSubSessionRamp_RAMP_UP:
              upperTag = mPubCounter%mPubCapabilities->size()+1;
              break;
          case EnumTimedPubSubSessionRamp_FLAT_CONTINUOUS:
              lowerTag = mPubCounter%mPubCapabilities->size();
              upperTag = lowerTag + 1;
              break;
        }
    }

    for (int i = lowerTag; i < upperTag; i++)
    {
        pubTag = new gloox::Tag("item");
        pubTag->addChildCopy((*mPubCapabilities)[i]);
        pubItem = new gloox::PubSub::Item(pubTag);
        delete pubTag;
        ps_itemList.push_back(pubItem);
        mPubSub->publishPEPItem(node,ps_itemList,NULL,this);
        if (mStats)
        {
            ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), false);
            mStats->pubAttemptCount++;
        }
        ps_itemList.clear();
    }

    ++mPubCounter;

    return true;
}

void ClientConnectionHandler::handleItemPublication (const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if (error) 
        handleErrorCntr(error->error());

    if (mStats)
    {
        ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        if (error && error->error() != gloox::StanzaErrorUndefined)
        {
            mStats->pubFailureCount++;
        }
        else
        {
            mStats->pubSuccessCount++;
        }
    }
}

void ClientConnectionHandler::handleMessage(const gloox::Message &msg, gloox::MessageSession* /*session*/)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if(!mPubSub)
        return;

    mPubSub->m_PEPtrackMapMutex.lock();
    if (!mPubSub->m_PEPTrackMap.empty())
    {
        gloox::PubSub::PEPManager::PEPHandlerTrackMap::iterator it = mPubSub->m_PEPTrackMap.begin();
        mPubSub->removeIqTrack((*it).first); //FIXME for now remove any first id
        mPubSub->m_PEPTrackMap.erase(it);
        if(mStats){
        	ACE_GUARD(stats_t::lock_t, guard, mStats->Lock());
        	mStats->pubSuccessCount++;
        }
    }
    mPubSub->m_PEPtrackMapMutex.unlock();
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::SendLogin(void)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if(!mGlooxClient)
        return false;
    mGlooxClient->setUsername(mUserID.str());
    mGlooxClient->setPassword(mUserPW.str());
    mGlooxClient->setResource(mUserResource);
    if(mStats){
    	ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(),false);
    	mStats->loginAttemptCount++;
    }
    mState = LOGIN;

    bool loginSuccess = false;
    loginSuccess = mGlooxClient->login();
    return loginSuccess;
}

//////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::InitiateLogout(const ACE_Time_Value& tv)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    if (m_reg)
        m_reg->removeAccount();
    mState = IDLE;

    if(mMUCRoom){
        mMUCRoom->leave();
        mMUCRoom->removeMUCRoomHandler();
        delete mMUCRoom;
        mMUCRoom = 0 ;
    }

    if(mGlooxClient)
        mGlooxClient->disconnect();

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::SetStatsInstance(stats_t& stats)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    mStats = &stats;
}

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::UpdateGoodputTxSent(uint64_t sent)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleOpenHook(void)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    //add client count
    if (mStats)
    {
        ACE_GUARD_RETURN(stats_t::lock_t, guard, mStats->Lock(), -1);
        mStats->totalClientCount++;
    }

    ACE_INET_Addr peerAddr;
    peer().get_remote_addr(peerAddr);
    
    char buffer[64];
    mPeerAddrStr = peerAddr.get_host_addr(buffer, sizeof(buffer));

    //if client is login type, setup client info and pub info
    if (mClientType == CLIENT_LOGIN)
    {
        if (mAutoGenID)  //if auto generate, generate username and pw
        {
            if (mStats){
                mUserID << "xmppclient-" << mStats->Handle() << "." << Serial() << "." << Hal::TimeStamp::getTimeStamp();
            } else {
                mUserID << "xmppclient-default." << Serial() << "." << Hal::TimeStamp::getTimeStamp();
            }
            mUserPW<<"spirent";
      
        }           
        StartSessionDurationTimer();
    }
    else if (mClientType == CLIENT_REGISTER)
    {
        mRegStartTime = ACE_OS::gettimeofday();
    }

    //setup new Gloox client
    mGlooxClient = new gloox::Client(mServerDomain);
    if(!registerClient())
        mPubSub = new gloox::PubSub::PEPManager(mGlooxClient);
    this->gloox::ConnectionTCPClient::registerConnectionDataHandler(mGlooxClient);
    mGlooxClient->setTls(gloox::TLSOptional);
    mGlooxClient->setCompression(false);
    mGlooxClient->registerConnectionListener(this);
    mGlooxClient->logInstance().registerLogHandler( gloox::LogLevelDebug, gloox::LogAreaAll, this);
    mGlooxClient->setConnectionImpl(this);
    mGlooxClient->registerMessageHandler(this);

    //setup muc
    if(registerClient() || mAutoGenID){
        m_reg = new gloox::Registration(mGlooxClient);
        m_reg->registerRegistrationHandler(this);
    }
    if(!registerClient()){
        gloox::JID nick( "capability-root@conference." + mServerDomain + "/" + mUserID.str() + "@" + mServerDomain + "/" + mUserResource);
        mMUCRoom = new gloox::MUCRoom( mGlooxClient, nick, this, 0 );
        ////setup rostermanager
        mGlooxClient->rosterManager()->registerRosterListener(this);
    }

    //connect gloox client for stream setup
    if (!mGlooxClient->connect(false))
    {
        TC_LOG_ERR(0,"ClientConectionHandler::HandleOpenHook() XMPP Client did not successfully connect to the server");
    }


    mState = STREAM;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleInputHook(void)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    const ACE_Time_Value now = ACE_OS::gettimeofday();

    // Receive pending data into message blocks and append to current input buffer
    messagePtr_t mb(Recv());
    if (!mb)
        return -1;
    mInBuffer = L4L7_UTILS_NS::MessageAppend(mInBuffer, mb);

    int err = 0;
    while (!err)
    {
        const char * const pos = mInBuffer->rd_ptr();

        //get input string
        L4L7_UTILS_NS::MessageIterator begin = L4L7_UTILS_NS::MessageBegin(mInBuffer.get());
        L4L7_UTILS_NS::MessageIterator end = L4L7_UTILS_NS::MessageEnd(mInBuffer.get());
    
        std::string msg_in(begin,end);
    
        end.block()->rd_ptr(end.ptr());
        mInBuffer = L4L7_UTILS_NS::MessageTrim(mInBuffer, end.block());   

        //send string to gloox client to process
        m_recvMutex.lock();

        if( m_cancel )
        {
            PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s XMPP Client not connected.",__func__);
            m_recvMutex.unlock();
            err = -1;
        }

        m_totalBytesIn += (int)msg_in.length();
        m_recvMutex.unlock();

        if (mGlooxClient && mState > IDLE)
        {
            mGlooxClient->handleReceivedData(this, msg_in);
        }
        //return 0; //check what value i shuold be passing back

        //if buffer does not contain anything
        if (!mInBuffer || pos == mInBuffer->rd_ptr())		//if buffer is empty or pointer not changed by state machine then break
            break;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

bool ClientConnectionHandler::NeedsPubSubTimerRestart()
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    return ((mSessionType == EnumSessionType_TIMED_PUB_SUB) && (mPubCounter < (uint64_t)mPubCapabilities->size()) 
            || (mTimedPubSubSessionRampType == EnumTimedPubSubSessionRamp_FLAT_CONTINUOUS));
}    

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::StartCapPubSubTimer() 
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    ScheduleTimer(CAPABILITY_PUBSUB_TIMEOUT, 0, mCapPubSubTimeoutDelegate, ACE_OS::gettimeofday() + mPubInterval);
}    

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::CancelCapPubSubTimer() 
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    CancelTimer(CAPABILITY_PUBSUB_TIMEOUT) ;
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleCapPubSubTimeout(const ACE_Time_Value& tv, const void *act)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    SendPublish();
    
    // Restart the timer if not done ramping up/down
    if (NeedsPubSubTimerRestart()){
        PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s restart CapPubSubTimer....",__func__);
        StartCapPubSubTimer();
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::StartSessionDurationTimer() 
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    ScheduleTimer(SESSION_DURATION_TIMEOUT, 0, mSessionDurationTimeoutDelegate, ACE_OS::gettimeofday() + mSessionDuration);
}    

///////////////////////////////////////////////////////////////////////////////

void ClientConnectionHandler::CancelSessionDurationTimer() 
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    CancelTimer(SESSION_DURATION_TIMEOUT) ;
}

///////////////////////////////////////////////////////////////////////////////

int ClientConnectionHandler::HandleSessionDurationTimeout(const ACE_Time_Value& tv, const void *act)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    InitiateLogout(tv);
    return 0;
 
}

void ClientConnectionHandler::NotifyRegistrationCompleted(bool success,bool noRetry)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    // Notify owner of status update
    StatusNotification status;
    if (success)
    {
        mRegistered = true;
        status = STATUS_REGISTRATION_SUCCEEDED;
    }
    else if (mRegRetryCount && !noRetry)
    {
        status = STATUS_REGISTRATION_RETRYING;
        mRegRetryCount--;
    }
    else
    {
        status = STATUS_REGISTRATION_FAILED;
    }

    if (mStatusDelegate)
        mStatusDelegate(mClientIndex, status, ACE_Time_Value(ACE_OS::gettimeofday() - mRegStartTime));

    // If we need to retry the registration, start that now
    if (status == STATUS_REGISTRATION_RETRYING)
    {
        if (!SendRegistration())
        {
//            if (mStatusDelegate)
//                mStatusDelegate(mClientIndex, STATUS_REGISTRATION_FAILED, ACE_Time_Value(ACE_OS::gettimeofday() - mRegStartTime));
            NotifyRegistrationCompleted(false);
        }
    } else
    {
        const ACE_Time_Value now = ACE_OS::gettimeofday();
        InitiateLogout(now);
        //this->close();
    }
    return;
}

void ClientConnectionHandler::handleMUCError (gloox::MUCRoom *room, gloox::StanzaError error)
{
    PHX_LOG_PORT_LOCAL(0, PHX_LOG_DEBUG,LOG_CLIENT,"%s.",__func__);
    CCHGuard(mLock);
    handleErrorCntr(error);   
}

bool ClientConnectionHandler::handleSubscriptionRequest (const gloox::JID &jid, const std::string& /*msg*/)
{
    CCHGuard(mLock);
    if(!mGlooxClient)
        return false;
    gloox::StringList groups;
    gloox::JID id( jid );
    mGlooxClient->rosterManager()->subscribe( id, "", groups, "" );
    return true;
}

bool ClientConnectionHandler::handleUnsubscriptionRequest (const gloox::JID &jid, const std::string& /*msg*/)
{
    CCHGuard(mLock);
    if(!mGlooxClient)
        return false;
    gloox::JID id( jid );
    mGlooxClient->rosterManager()->unsubscribe(id);
    return true;
}

void ClientConnectionHandler::handleRosterError (const gloox::IQ &iq)
{
    CCHGuard(mLock);
    handleErrorCntr(iq.error()->error());
}
