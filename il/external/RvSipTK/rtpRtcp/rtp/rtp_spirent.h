
/* FILE HEADER */

#if defined(UPDATED_BY_SPIRENT)

#ifndef __rtp_spirent_H__
#define __rtp_spirent_H__


/***************************************************************************
 * FILE: $RCSfile$
 *
 * PURPOSE: 
 *
 * DESCRIPTION:
 *
 * WRITTEN BY: Alexander (Sasha) Fainkichen, May 26 2004.
 * Oleg Moskalenko, 2006
 *
 * CONFIDENTIAL  
 * Proprietary Information of Spirent Communications Plc.
 * 
 * Copyright (c) Spirent Communications Plc., 2004 
 * All rights reserved.
 *
 ***************************************************************************/
#include <netinet/in.h>

#include "rtp.h"
#include "rtcp.h"

#include "rvrtpseli.h"
#include "rvnetaddress.h"

#include "RV_DEF.h"
#include "RV_ADS_DEF.h"	

#define use_smp_media() (1)

#ifdef __cplusplus
extern "C" {
#endif

  void rtpprintf(const char* format,...);
  
  int RvRtpGetSocket(RvRtpSession hRTP); //missed in the RTP defs
  
  RvNetIpv4* RTP_getRvNetIp4(const void *addr);
  
#if (RV_NET_TYPE & RV_NET_IPV6)
  RvNetIpv6* RTP_getRvNetIp6(const void *addr);
#endif
  
  RvStatus 		RTP_makeRvNetAddress	(sa_family_t, const RvChar *if_name, 
						 RvUint16 vdevblock, const RvUint8 *addr, 
						 RvUint16 port, int scope, RvNetAddress * ip);
  RvNetAddress** RTP_getHostAddresses	(); 
  RvUint16			RTP_getPort		(RvNetAddress *);
  void				RTP_setPort		(RvNetAddress *,RvUint16 );
  void				RTP_printRvNetAddress	(RvNetAddress *);
  void				RTP_initRvNetAddress    (sa_family_t, const RvChar *if_name, 
							 RvUint16 vdevblock, const char * str, RvNetAddress *);
  
  const char		*	RTP_getIpString		(RvNetAddress *);
  RvBool		        RTP_isIpValid (RvNetAddress *);
  RvBool			RTP_cmpRvNetAddresses	(RvNetAddress *,RvNetAddress *,RvBool checkPorts);
  
  int RvRtpResolveCollision(RvRtpSession  hRTP);
  
  int RvRtpSetTypeOfServiceSpirent(RvRtpSession hRTP,int typeOfService, RvNetAddress  *addr);
  int RvRtcpSetTypeOfServiceSpirent(RvRtcpSession hRTCP,int typeOfService, RvNetAddress  *addr);
  
  void RvRtpSetSSRC(RvRtpSession  hRTP, unsigned int ssrc);
  
#ifdef __cplusplus
}
#endif

#endif // __rtp_spirent_H__

#endif //#if defined(UPDATED_BY_SPIRENT)
