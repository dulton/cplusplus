/***********************************************************************
        Copyright (c) 2002 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#include "rvstdio.h"
#include "rvnetaddress.h"
#include "rvaddress.h"

#ifdef __cplusplus
extern "C" {
#endif

RVAPI    
RvStatus RVCALLCONV RvNetCreateIpv4(INOUT RvNetAddress* pNetAddress,
                         IN RvNetIpv4* IpV4)
{
    if (NULL!=pNetAddress)
    {
      /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
      pNetAddress->if_name[0]=0;
      pNetAddress->vdevblock=0;
#endif
/* SPIRENT_END */
      RvAddressConstructIpv4((RvAddress*)pNetAddress->address, IpV4->ip, IpV4->port);
      return RV_OK;
    }
    return RV_ERROR_NULLPTR;
}


RVAPI
RvStatus RVCALLCONV RvNetCreateIpv6(INOUT RvNetAddress* pNetAddress,
                         IN RvNetIpv6* IpV6)
{
    if (NULL!=pNetAddress)
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
      pNetAddress->if_name[0]=0;
      pNetAddress->vdevblock=0;
      RvAddressConstructIpv6((RvAddress*)pNetAddress->address, IpV6->ip, IpV6->port, IpV6->scopeId, IpV6->flowinfo);
#else
        RvAddressConstructIpv6((RvAddress*)pNetAddress->address, IpV6->ip, IpV6->port, IpV6->scopeId);
#endif
/* SPIRENT_END */
        return RV_OK;
#else
		RV_UNUSED_ARG(IpV6);
        return RV_ERROR_NOTSUPPORTED;		
#endif
    }
    return RV_ERROR_NULLPTR;
}

RVAPI
RvStatus RVCALLCONV RvNetGetIpv4(OUT RvNetIpv4* IpV4,
                      IN RvNetAddress* pNetAddress)
{
    if (NULL!=pNetAddress&&NULL!=IpV4)
    {
        RvAddressIpv4* pIpv4 = (RvAddressIpv4*)RvAddressGetIpv4((RvAddress*)pNetAddress->address);
        IpV4->ip = pIpv4->ip;
        IpV4->port = pIpv4->port;
        return RV_OK;
    }
    
    return RV_ERROR_NULLPTR;
}

RVAPI
RvStatus RVCALLCONV RvNetGetIpv6(OUT RvNetIpv6* IpV6,
                      IN RvNetAddress* pNetAddress)
{
    if (NULL!=pNetAddress&&NULL!=IpV6)
    {
#if (RV_NET_TYPE & RV_NET_IPV6)
        RvAddressIpv6* pIpv6 = (RvAddressIpv6 *) RvAddressGetIpv6((RvAddress*)pNetAddress->address);
        memcpy(IpV6->ip, pIpv6->ip, sizeof(IpV6->ip));
        IpV6->port = pIpv6->port;
        IpV6->scopeId = pIpv6->scopeId;
        //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
        IpV6->flowinfo = pIpv6->flowinfo;
#endif
        //SPIRENT_END
        return RV_OK;
#else
        return RV_ERROR_NOTSUPPORTED;
#endif
    }
    return RV_ERROR_NULLPTR;
}

RVAPI
RvNetAddressType RVCALLCONV RvNetGetAddressType(IN RvNetAddress* pNetAddress)
{
    if (NULL!=pNetAddress)
    { 
        switch(RvAddressGetType((RvAddress*)pNetAddress->address))
        {
        case RV_ADDRESS_TYPE_IPV4:
            return RVNET_ADDRESS_IPV4;
        case RV_ADDRESS_TYPE_IPV6:
            return RVNET_ADDRESS_IPV6;
        default:
            ;
        }
    }
    return RVNET_ADDRESS_NONE;
}

#ifdef __cplusplus
}
#endif


