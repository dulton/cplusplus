/// @file
/// @brief SIP User Agent (UA) Binding object
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _USER_AGENT_BINDING_H_
#define _USER_AGENT_BINDING_H_

#include <list>
#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>

#include "SipAuthentication.h"
#include "SipCommon.h"
#include "SipUri.h"

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

struct UserAgentBinding
{
    struct AuthCredentials
    {
        AuthCredentials(void)
            : username("anonymous"),
              passwordDebug(false)
        {
        }
        
        std::string realm;                                      //< configured realm
        std::string username;                                   //< configured username
        std::string password;                                   //< configured password
        bool passwordDebug;                                     //< send password in plaintext for debug purposes
        boost::scoped_ptr<SipDigestAuthChallenge> wwwChal;      //< cached WWW auth challenge
        boost::scoped_ptr<SipDigestAuthChallenge> proxyChal;    //< cached proxy auth challenge
    };
    
    // Convenience typedefs
    typedef std::list<std::string> routeList_t;
    typedef std::list<std::string> uriList_t;

    SipUri addressOfRecord;                                     //< our address-of-record
    SipUri contactAddress;                                      //< our contact address
    std::vector<SipUri> extraContactAddresses;                  //< extra contact addresses (optional)
    AuthCredentials authCred;                                   //< authentication credentials (optional)
    routeList_t pathList;                                       //< path list (RFC 3327 extension)
    routeList_t serviceRouteList;                               //< service-route list (RFC 3608 extension)
    uriList_t assocUriList;                                     //< p-associated-uri list (RFC 3455 extension)
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
