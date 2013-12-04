/// @file
/// @brief SIP URI methods
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "SipUri.h"

USING_SIP_NS;

///////////////////////////////////////////////////////////////////////////////

const std::string SipUri::BuildUriString() const
{
    std::ostringstream oss;

    switch (GetScheme())
    {
      case SipUri::SIP_URI_SCHEME:
      {
          oss << "sip:";

          const std::string& user = GetSipUser();
          const std::string& host = GetSipHost();
	  const std::string& transport = GetSipTransport();
          const uint16_t port = GetSipPort();

          if (!user.empty())
              oss << user << "@";

          if (host.find_first_of(":") != std::string::npos && host.find_first_of("[") == std::string::npos)
              oss << "[" << host << "]";
          else
              oss << host;

          if (port)
              oss << ":" << port;

	  if(!transport.empty()) 
	    oss << ";transport=" << transport;

          break;
      }

      case SipUri::TEL_URI_SCHEME:
      {
	if(IsLocalTel()) {
	  string phoneContext=GetPhoneContext();
	  if(phoneContext.empty()) {
	    oss << "tel:" << GetTelNumber();
	  } else {
	    oss << "tel:" << GetTelNumber() << ";phone-context=" << phoneContext;
	  }
	} else {
	  oss << "tel:+" << GetTelNumber();
	}
	break;
      }

      case SipUri::NO_URI_SCHEME:
      default:
          break;
    }

    return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

const std::string SipUri::BuildRouteUriString() const {

  std::ostringstream oss;
  
  switch (GetScheme())
  {
    case SipUri::SIP_URI_SCHEME:
    {
      oss << "<sip:";
      
      const std::string& host = GetSipHost();
      const std::string& transport = GetSipTransport();
      const uint16_t port = GetSipPort();
      
      if (host.find_first_of(":") != std::string::npos && host.find_first_of("[") == std::string::npos)
	oss << "[" << host << "]";
      else
	oss << host;
      
      if (port)
	oss << ":" << port;

      if(!transport.empty()) 
	oss << ";transport=" << transport;
      
      if(GetLooseRouting()) {
	oss << ";lr";
      }
      
      oss << ">";
      
      break;
    }
    
  default:
    break;
  }

  return oss.str();
}

///////////////////////////////////////////////////////////////////////////////

std::ostream& SipUri::Print(std::ostream& strm, const SipUri& uri)
{
    return strm << uri.BuildUriString();
}

///////////////////////////////////////////////////////////////////////////////
