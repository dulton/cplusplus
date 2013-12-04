/// @file
/// @brief SIP User Agent (UA) Dialog object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_DIALOG_H_
#define _USER_AGENT_DIALOG_H_

#include <algorithm>
#include <list>
#include <string>

#include <ace/INET_Addr.h>
#include <boost/function.hpp>
#include <boost/utility.hpp>
#include <phxexception/PHXException.h>

#include "SipCommon.h"
#include "SipUri.h"
#include "UserAgentBinding.h"

DECL_CLASS_FORWARD_SIP_NS(SipDigestAuthChallenge);
DECL_CLASS_FORWARD_SIP_NS(SipDigestAuthCredentials);

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Forward declarations
class SipMessage;

class UserAgentDialog : boost::noncopyable
{
  public:
    typedef boost::function1<bool, SipMessage&> sendRequestDelegate_t;
    typedef boost::function1<bool, const SipMessage&> sendResponseDelegate_t;
    
    UserAgentDialog(uint16_t port, UserAgentBinding& binding, const ACE_INET_Addr& peer, const SipUri& toUri);
    UserAgentDialog(uint16_t port, UserAgentBinding& binding, const SipMessage& msg);
    virtual ~UserAgentDialog();

    // Delegate mutators
    void SetSendRequestDelegate(const sendRequestDelegate_t& delegate) { mSendRequestDelegate = delegate; }
    void SetSendResponseDelegate(const sendResponseDelegate_t& delegate) { mSendResponseDelegate = delegate; }
    
    // Option mutators
    void SetCompactHeaders(bool enable) { mCompactHeaders = enable; }
    
    // Accessors
    uint16_t Port(void) const { return mPort; }
    const SipUri& Contact(void) const { return mBinding.contactAddress; }
    const SipUri& FromUri(void) const { if(mFromUri.empty()) return mBinding.addressOfRecord; return mFromUri; }
    const std::string &FromTag(void) const { return mFromTag; }
    const ACE_INET_Addr& Peer(void) const { return mPeer; }
    const SipUri& ToUri(void) const { return mToUri; }
    const std::string &ToTag(void) const { return mToTag; }
    const SipUri& RemoteTarget(void) const { return mRemoteTarget; }
    const std::string& CallId(void) const { return mCallId; }

    // Test for dialog id match
    bool Matches(const SipMessage& msg) const;

    // Update dialog target
    void UpdateTarget(const SipMessage& msg);

    // Sequence number methods
    uint32_t NextSeq(void)
    {
        return ++mLocalSeq;
    }

    bool InSeq(uint32_t remoteSeq)
    {
        if (!mRemoteSeq || (remoteSeq >= mRemoteSeq))
        {
            mRemoteSeq = remoteSeq;
            return true;
        }
        else
            return false;
    }

    // Dialog-specific accessors
    virtual bool Active(void) const = 0;

    // Dialog-specific controls
    virtual void Abort(void) = 0;
    
    // Dialog-specific events
    virtual void ProcessRequest(const SipMessage& msg) = 0;
    virtual void ProcessResponse(const SipMessage& msg) = 0;
    virtual void TransactionError(void) = 0;

  protected:
    // Convenience typedefs
    typedef std::list<std::string> routeList_t;
    typedef std::list<std::string> uriList_t;

    // Protected accessors
    void ForEachContactAddress(const boost::function1<void, const SipUri&>& delegate) const
    {
        delegate(mBinding.contactAddress);
        std::for_each(mBinding.extraContactAddresses.begin(), mBinding.extraContactAddresses.end(), delegate);
    }
    
    routeList_t& PathList(void) { return mBinding.pathList; }
    const routeList_t& PathList(void) const { return mBinding.pathList; }
    routeList_t& ServiceRouteList(void) { return mBinding.serviceRouteList; }
    const routeList_t& ServiceRouteList(void) const { return mBinding.serviceRouteList; }
    uriList_t& AssocUriList(void) { return mBinding.assocUriList; }
    const uriList_t& AssocUriList(void) const { return mBinding.assocUriList; }
    bool CompactHeaders(void) const { return mCompactHeaders; }
    
    // Generate a new 'From' tag
    void RefreshFromTag(void);
    
    // Authentication methods
    bool WWWAuthenticateChallenge(const SipMessage& msg);
    void WWWAuthenticate(SipMessage& msg) const;
    bool ProxyAuthenticateChallenge(const SipMessage& msg);
    void ProxyAuthenticate(SipMessage& msg) const;

    // Invoke send message delegates
    bool SendRequestInternal(SipMessage& msg) const
    {
        WWWAuthenticate(msg);
        ProxyAuthenticate(msg);
        
        if (mSendRequestDelegate)
            return mSendRequestDelegate(msg);
        else
            throw EPHXInternal("UserAgentDialog::SendRequest");
    }
    
    bool SendResponseInternal(const SipMessage& msg) const
    {
        if (mSendResponseDelegate)
            return mSendResponseDelegate(msg);
        else
            throw EPHXInternal("UserAgentDialog::SendResponse");
    }

    // Class data
    static const std::string ALLOWED_METHODS;
    
  private:
    const uint16_t mPort;                               //< physical port
    UserAgentBinding& mBinding;                         //< local UA's address binding
    std::string mFromTag;                               //< local UA's tag
    const ACE_INET_Addr mPeer;                          //< peer address
    SipUri mToUri;                                      //< remote UA's URI
    SipUri mFromUri;                                    //< local UA's URI
    std::string mToTag;                                 //< remote UA's tag
    SipUri mRemoteTarget;                               //< remote target URI
    std::string mCallId;                                //< dialog Call-ID
    uint32_t mLocalSeq;                                 //< local sequence number
    uint32_t mRemoteSeq;                                //< remote sequence number
    
    sendRequestDelegate_t mSendRequestDelegate;         //< send request
    sendResponseDelegate_t mSendResponseDelegate;       //< send response
    bool mCompactHeaders;                               //< send with compact headers?
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
