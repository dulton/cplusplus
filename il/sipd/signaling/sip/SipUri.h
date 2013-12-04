/// @file
/// @brief SIP URI class header
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_URI_H_
#define _SIP_URI_H_

#include <iostream>
#include <string>

#include <boost/variant.hpp>

#include "SipCommon.h"

DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

class SipUri
{
  public:
    // Enumerated constants
    enum Scheme
    {
        NO_URI_SCHEME = 0,
        SIP_URI_SCHEME,
        TEL_URI_SCHEME
    };

    SipUri(void)
        : mScheme(NO_URI_SCHEME)
    {
    }

    SipUri(const std::string& host, uint16_t port, std::string transport = "", bool looseRouting = false)
    {
        SetSip(host, port, transport, looseRouting);
    }

    SipUri(const std::string& user, const std::string& host, uint16_t port, std::string transport = "", bool looseRouting = false)
    {
        SetSip(user, host, port, transport, looseRouting);
    }

    SipUri(const SipUri& other) {
      mScheme=other.mScheme;
      mImpl=other.mImpl;
    }

    SipUri& operator=(const SipUri& other) {
      mScheme=other.mScheme;
      mImpl=other.mImpl;
      return *this;
    }
    
    void Reset(void)
    {
        mScheme = NO_URI_SCHEME;
    }

    bool empty() const {
      return (mScheme == NO_URI_SCHEME);
    }

    Scheme GetScheme(void) const { return mScheme; }

    const std::string& GetSipUser(void) const
    {
        const SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        return sipImpl.user;
    }
    
    const std::string& GetSipHost(void) const
    {
        const SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        return sipImpl.host;
    }
    
    const std::string& GetSipTransport(void) const
    {
        const SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        return sipImpl.transport;
    }

    uint16_t GetSipPort(void) const
    {
        const SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        return sipImpl.port;
    }
    
    void SetSip(const std::string& host, uint16_t port, std::string transport = "", bool looseRouting = false)
    {
        mScheme = SIP_URI_SCHEME;
        mImpl = SipSchemeImpl();
        SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        sipImpl.host = host;
        sipImpl.port = port;
	sipImpl.transport=transport;
	sipImpl.looseRouting = looseRouting;
    }
    
    void SetSip(const std::string& user, const std::string& host, uint16_t port, std::string transport = "", bool looseRouting = false)
    {
        mScheme = SIP_URI_SCHEME;
        mImpl = SipSchemeImpl();
        SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
        sipImpl.user = user;
        sipImpl.host = host;
        sipImpl.port = port;
	sipImpl.transport = transport;
	sipImpl.looseRouting = looseRouting;
    }

    const std::string& GetTelNumber(void) const
    {
        const TelSchemeImpl& telImpl = boost::get<TelSchemeImpl>(mImpl);
        return telImpl.number;
    }
    
    void SetTelGlobal(const std::string& number0)
    {
      std::string number=number0;
      while(!number.empty() && ((number[0]==' ')||(number[0]=='+'))) number=number.substr(1);

      mScheme = TEL_URI_SCHEME;
      mImpl = TelSchemeImpl();
      TelSchemeImpl& telImpl = boost::get<TelSchemeImpl>(mImpl);
      telImpl.number = number;
      telImpl.localNumber=false;
      telImpl.phoneContext="";
    }

    void SetTelLocal(const std::string& number, const std::string& phoneContext)
    {
      mScheme = TEL_URI_SCHEME;
      mImpl = TelSchemeImpl();
      TelSchemeImpl& telImpl = boost::get<TelSchemeImpl>(mImpl);
      telImpl.number = number;
      telImpl.localNumber=true;
      telImpl.phoneContext=phoneContext;
    }

    bool GetLooseRouting(void) const { 
      if(mScheme == SIP_URI_SCHEME) {
	const SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
	return sipImpl.looseRouting; 
      }
      return false;
    }
    void SetLooseRouting(bool looseRouting) { 
      if(mScheme == SIP_URI_SCHEME) {
	SipSchemeImpl& sipImpl = boost::get<SipSchemeImpl>(mImpl);
	sipImpl.looseRouting = looseRouting;
      }
    }

    static std::ostream& Print(std::ostream& strm, const SipUri& uri);

    bool IsLocalTel() const {
      if(GetScheme()==TEL_URI_SCHEME) {
	const TelSchemeImpl& telImpl = boost::get<TelSchemeImpl>(mImpl);
        return telImpl.localNumber;
      }
      return false;
    }

    std::string GetPhoneContext() const {
      if(GetScheme()==TEL_URI_SCHEME) {
	const TelSchemeImpl& telImpl = boost::get<TelSchemeImpl>(mImpl);
        return telImpl.phoneContext;
      }
      return string("");
    }

    const std::string BuildUriString() const;
    const std::string BuildRouteUriString() const;
    
  private:
    struct SipSchemeImpl
    {
      std::string user;
      std::string host;
      uint16_t port;
      bool looseRouting;
      std::string transport;

      SipSchemeImpl() {
	user="";
	host="";
	transport="";
	port=0;
	looseRouting=false;
      }

      bool operator==(const SipSchemeImpl& other) const {
	if(this==&other) return true;
	if(user!=other.user) return false;
	if(host!=other.host) return false;
	if(transport!=other.transport) return false;
	if(looseRouting!=other.looseRouting) return false;
	if(port==other.port) return true;
	if(port==0 && other.port==DEFAULT_SIP_PORT_NUMBER) return true;
	if(other.port==0 && port==DEFAULT_SIP_PORT_NUMBER) return true;
	return false;
      }
    };

    struct TelSchemeImpl
    {
      std::string number;
      bool localNumber;
      std::string phoneContext;
    };
    
    Scheme mScheme;
    boost::variant<SipSchemeImpl, TelSchemeImpl> mImpl;
};
    
///////////////////////////////////////////////////////////////////////////////

END_DECL_SIP_NS

///////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& strm, const SIP_NS::SipUri& uri)
{
    return uri.Print(strm, uri);
}

///////////////////////////////////////////////////////////////////////////////

#endif
