/// @file
/// @brief Client Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _CLIENT_CONNECTION_HANDLER_H_
#define _CLIENT_CONNECTION_HANDLER_H_

#include <string>
#include <utility>
#include <sstream>
#include <vector>

#include <app/StreamSocketHandler.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <connectionlistener.h>
#include <connectiontcpclient.h>
#include <statisticshandler.h>
#include <loghandler.h>
#include <registrationhandler.h>
#include <pubsubresulthandler.h>
//#include <pubsubmanager.h>
#include <XmppClientPEPManager.h>
#include <registration.h>
#include <client.h>
#include <gloox.h>
#include <mutex.h>
#include <messagehandler.h>

#include <mucroom.h>
#include <rostermanager.h>

#include "XmppCommon.h"

DECL_XMPP_NS

// Forward declarations
class XmppClientRawStats;

///////////////////////////////////////////////////////////////////////////////

class ClientConnectionHandler : public L4L7_APP_NS::StreamSocketHandler, gloox::ConnectionListener, gloox::LogHandler, gloox::RegistrationHandler, gloox::ConnectionTCPClient, gloox::PubSub::ResultHandler, gloox::MessageHandler, gloox::MUCRoomHandler, gloox::RosterListener
{
  public:
    enum XMPPState
    {
        IDLE,
        STREAM,
        REGISTER,
        LOGIN,
        PUBSUB
    };  
    enum XMPPClientType
    {
        CLIENT_REGISTER,
        CLIENT_LOGIN,
        CLIENT_UNREGISTER
    };

    enum StatusNotification
    {
        STATUS_REGISTRATION_SUCCEEDED,
        STATUS_REGISTRATION_RETRYING,
        STATUS_REGISTRATION_FAILED,
    };

    typedef XmppClientRawStats stats_t;
    typedef boost::function3<void, size_t, StatusNotification, const ACE_Time_Value&> statusDelegate_t;
    
    explicit ClientConnectionHandler(uint32_t serial = 0);
    ~ClientConnectionHandler();

    void SetStatusDelegate(statusDelegate_t delegate) { mStatusDelegate = delegate; }
    void NotifyRegistrationCompleted(bool success,bool noRetry = false);

    /// @brief Set session type
    void SetSessionType(int8_t sessionType) { mSessionType = sessionType; }

    /// @brief Set client mode
    void SetClientMode(int8_t clientMode) { mClientMode = clientMode; }
    
    /// @brief Set the server domain
    void SetServerDomain(const std::string& serverDomain) { mServerDomain = serverDomain; }

    /// @brief Set duration of session
    void SetSessionDuration(uint32_t sessionDuration) { mSessionDuration.set(sessionDuration,0); }

    /// @brief Set timed pub/sub ramp type
    void SetTimedPubSubSessionRampType(uint32_t timedPubSubSessionRampType) { mTimedPubSubSessionRampType = timedPubSubSessionRampType; }

    /// @brief Set pub interval
    void SetPubInterval(uint32_t pubInterval) { mPubInterval.set(pubInterval, 0); }
    void SetFirstInterval(uint32_t pubInterval) { mFirstInterval.set(pubInterval, 0); }

    /// @brief Set pub item id
    void SetPubItemId(const std::string& pubItemId) { mPubItemId = pubItemId; }

    /// @brief Set the pub capabilities
    void SetPubCapabilities(const std::vector<gloox::Tag*>& pubCapabilities) { mPubCapabilities = &pubCapabilities; }
//    void SetPubFormat(const std::string& pubFormat) { mPubFormat = pubFormat; }   

    /// @brief Set stats instance
    void SetStatsInstance(stats_t& stats);

    /// @brief Called by XmppClientBlock to Set/Get completion (connection status logged) flag
    void SetComplete(bool complete) { mComplete = complete; }
    bool IsComplete() const { return mComplete; }

    bool InitiateLogout(const ACE_Time_Value& tv);

    //send stanza calls
    bool SendRegistration(void);
    bool SendPublish(void);
    bool SendLogin(void);

    int8_t GetClientMode(void) const { return mClientMode; }
    int8_t GetSessionType(void) const { return mSessionType; }
    stats_t *GetStats(void) const {return mStats;}
    bool NeedsPubSubTimerRestart(void);

    void SetUsername(const std::string& username) { mUserID<<username; }
    void SetPassword(const std::string& pw) { mUserPW<<pw; }
    void SetClientIndex(uint32_t index) { mClientIndex = index; }
    uint32_t GetClientIndex(void) { return mClientIndex; }
    void SetClientType(XMPPClientType type) { mClientType = type; }
    XMPPClientType GetClientType(void) { return mClientType; }
    bool IsRegistered() { return mRegistered; }
    void SetAutoGenID(bool set) { mAutoGenID = set; }
    void SetRegRetryCount (uint32_t retries) { mRegRetryCount = retries; }

    /// @brief Inherited Gloox methods for send
    virtual bool send(const std::string &data);
    virtual void cleanup();
    virtual gloox::ConnectionError recv( int /*timeout*/ ) { return gloox::ConnNoError; }
    virtual gloox::ConnectionError connect();

    /// @brief ConnectionListener inherited methods
    virtual void onConnect();
    virtual void onDisconnect(gloox::ConnectionError e);
    virtual void onResourceBind(const std::string &resource);
    virtual void onResourceBindError(const gloox::Error *error);
    virtual bool onTLSConnect(const gloox::CertInfo &info);
    virtual void onStreamEvent(gloox::StreamEvent event);

    /// @brief LogHandler inherited method
    virtual void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string &message);

    /// @brief RegistrationHandler inherited methods
    virtual void handleRegistrationFields(const gloox::JID &from, int fields, std::string instructions);
    virtual void handleAlreadyRegistered(const gloox::JID &from);
    virtual void handleRegistrationResult(const gloox::JID &from, gloox::RegistrationResult regResult);
    virtual void handleDataForm(const gloox::JID &from, const gloox::DataForm &form);    virtual void handleOOB(const gloox::JID &from, const gloox::OOB &oob);

    void handleErrorCntr(const gloox::StanzaError error);

    /// @brief PubSub ResultHandler inherites methods
    void handleItem(const gloox::JID&, const std::string&, const gloox::Tag*) {}
    void handleItems(const std::string&, const gloox::JID&, const std::string&, const gloox::PubSub::ItemList&, const gloox::Error*) {}
    void handleItemPublication(const std::string &id, const gloox::JID &service, const std::string &node, const gloox::PubSub::ItemList &itemList, const gloox::Error *error); 
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

    virtual void handleMessage(const gloox::Message &msg, gloox::MessageSession *session=0);

    //mucroom handler
    void handleMUCParticipantPresence (gloox::MUCRoom *room, const gloox::MUCRoomParticipant participant, const gloox::Presence &presence) {}
    void handleMUCMessage (gloox::MUCRoom *room, const gloox::Message &msg, bool priv) {}
    bool handleMUCRoomCreation (gloox::MUCRoom *room) { return true; }
    void handleMUCSubject (gloox::MUCRoom *room, const std::string &nick, const std::string &subject) {}
    void handleMUCInviteDecline (gloox::MUCRoom *room, const gloox::JID &invitee, const std::string &reason) {}
    void handleMUCError (gloox::MUCRoom *room, gloox::StanzaError error);
    void handleMUCInfo (gloox::MUCRoom *room, int features, const std::string &name, const gloox::DataForm *infoForm) {}
    void handleMUCItems (gloox::MUCRoom *room, const gloox::Disco::ItemList &items) {}

    //rosterlistener methods
    void handleItemAdded (const gloox::JID &jid) {}
    void handleItemSubscribed (const gloox::JID &jid) {}
    void handleItemRemoved (const gloox::JID &jid) {}
    void handleItemUpdated (const gloox::JID &jid) {}
    void handleItemUnsubscribed (const gloox::JID &jid) {}
    void handleRoster (const gloox::Roster &roster) {}
    void handleRosterPresence (const gloox::RosterItem &item, const std::string &resource, gloox::Presence::PresenceType presence, const std::string &msg) {}
    void handleSelfPresence (const gloox::RosterItem &item, const std::string &resource, gloox::Presence::PresenceType presence, const std::string &msg) {}
    bool handleSubscriptionRequest (const gloox::JID &jid, const std::string &msg);
    bool handleUnsubscriptionRequest (const gloox::JID &jid, const std::string &msg);
    void handleNonrosterPresence (const gloox::Presence &presence) {}
    void handleRosterError (const gloox::IQ &iq);

    bool registerClient() const {return (mClientType == CLIENT_REGISTER || mClientType == CLIENT_UNREGISTER);}
    gloox::Client* glClient() { return mGlooxClient;}
    bool glClientConnected() { return (mState > STREAM);}
	bool isRegistering() { return (mState == REGISTER); }
 
  private:

    enum TimerTypes
    {
        CAPABILITY_PUBSUB_TIMEOUT,
        SESSION_DURATION_TIMEOUT
    };

    /// @brief Update Goodput Tx
    void UpdateGoodputTxSent(uint64_t sent);
    
    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// Start timer for next capability pub/sub timeout
    void StartCapPubSubTimer(void);

    /// Cancel timer for next capability pub/sub timeout
    void CancelCapPubSubTimer(void);

    /// ACE timer hook to handle capability pub/sub timer timeouts
    int HandleCapPubSubTimeout(const ACE_Time_Value& tv, const void *act);

    /// Start timer for session duration
    void StartSessionDurationTimer(void);

    /// Cancel timer for session duration
    void CancelSessionDurationTimer(void);

    /// ACE timer hook to handle session duration timer timeouts
    int HandleSessionDurationTimeout(const ACE_Time_Value& tv, const void *act);

    
    /// Class data
    int8_t mSessionType;                        ///< client session type
    int8_t mClientMode;                         ///< client mode
    std::string mServerDomain;                  ///< server domain
    ACE_Time_Value mSessionDuration;            ///< duration of the session
    uint32_t mTimedPubSubSessionRampType;       ///< timed pub/sub ramp type
    ACE_Time_Value mPubInterval;                ///< pub interval
    ACE_Time_Value mFirstInterval;
    std::string mPubItemId;                     ///< pub item id
    const std::vector<gloox::Tag*>* mPubCapabilities; ///< pub capabilities
    //std::vector<std::string> mPubCapBlocks;     //blocks of capabilities grouped
    uint64_t mPubCounter;			//counter for ramp
//    std::string mPubFormat;                     ///< publish format
    stats_t *mStats;                            ///< client stats
    XMPPState mState;                        ///< current state
    messagePtr_t mInBuffer;                     ///< input buffer
    std::string mPeerAddrStr;                   ///< IP address of our peer in string format
    bool mComplete;                             ///< conn. complete flag
    std::ostringstream mUserID;			///< clients username
    std::ostringstream mUserPW;
    std::string mUserResource;			///< clients resource for gloox::JID
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mCapPubSubTimeoutDelegate; ///< delegate to handle cap pub/sub timeouts
    IL_DAEMON_LIB_NS::TimerQueue::delegate_t mSessionDurationTimeoutDelegate; ///< delegate to handle session duration timeouts
    bool mRegistered;
    uint32_t mClientIndex;
    XMPPClientType mClientType;
    statusDelegate_t mStatusDelegate;                   //< owner's status delegate   
    uint32_t mRegRetryCount;                            //< current REGISTER retry count
    ACE_Time_Value mRegStartTime;                       //< start time for REGISTER transaction
    bool mAutoGenID;

    /// Setup Gloox
    gloox::Client *mGlooxClient;
    gloox::Registration  *m_reg;
    gloox::PubSub::PEPManager *mPubSub;
    const gloox::LogSink mLoger;
    int mMsgID;

    //muc manager
    gloox::MUCRoom *mMUCRoom;
    //locker
    ACE_Recursive_Thread_Mutex mLock;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_XMPP_NS

#endif
