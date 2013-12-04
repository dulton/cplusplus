
/* FILE HEADER */

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
 * Copyright (c) Spirent Communications Plc., 2004, 2005, 2006 
 * All rights reserved.
 *
 ***************************************************************************/
/* END OF FILE HEADER */

#if defined(UPDATED_BY_SPIRENT)

#include <netdb.h>	/* gethostbyname2(), ...	*/
#include <stdio.h>	/* fopen(),...			*/
#include <errno.h>	/* error reporting		*/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <syscall.h>

#include <pthread.h>

#include "rtp_spirent.h"
#include "rvaddress.h"
#include "rtputil.h"

#include "RtcpTypes.h"

//Defs:
#define MAXRTPSESSIONMEMBERS      4
#define MAXIPS                    RV_RTP_MAXIPS

//external functions from Abacus code:
#include "rvexternal.h"

static RvAddress* toRvAddress(RvNetAddress* addr) {
   return (RvAddress*)(addr->address);
}

RvNetIpv4* RTP_getRvNetIp4(const void *addr) {
   return (RvNetIpv4*)RvAddressGetIpv4(toRvAddress((RvNetAddress*)addr));
}

#if (RV_NET_TYPE & RV_NET_IPV6)
RvNetIpv6* RTP_getRvNetIp6(const void *addr) {
   return (RvNetIpv6*)RvAddressGetIpv6(toRvAddress((RvNetAddress*)addr));
}
#endif

RvStatus RTP_makeRvNetAddress
(
 sa_family_t		af,
 const RvChar           *       if_name, 
 RvUint16                 vdevblock,
 const RvUint8		*	data,
 RvUint16			port,
 int			scope,	/* ignored for IPv4 */
 RvNetAddress		*	out
 )
{
  RvStatus res = RV_OK;

  memset ( ( char * )out,0,sizeof (RvNetAddress));

  RVSIP_ADJUST_VDEVBLOCK(vdevblock);
  
  switch (af) {

  case AF_INET:
    {
      RvNetIpv4 ipv4;

      ipv4.ip	= *( RvUint32 * )data;
      ipv4.port	= port;

      RvNetCreateIpv4(out,&ipv4);
    }
    
    break;

#if (RV_NET_TYPE & RV_NET_IPV6)
  case AF_INET6:
    {
      RvNetIpv6 ipv6;
      
      ipv6.port	= port;
      ipv6.scopeId	= scope;
      ipv6.flowinfo = 0;
      memcpy ( ipv6.ip,data,sizeof ( ipv6.ip ));

      RvNetCreateIpv6(out,&ipv6);

    }
    break;
#endif

  default:
    dtprintf ( "makeRvNetAddress(): address family %d is not supported\n",af );
    res = -1;
    
  } /* switch */

  if(res == RV_OK) {
    out->vdevblock = vdevblock;
    if(if_name) {
      strncpy(out->if_name, if_name, sizeof(out->if_name)-1);
    }
  }

  return ( res );
}


RvNetAddress ** RTP_getHostAddresses() 
{
  static RvNetAddress * 	res[MAXIPS+1];
  static RvNetAddress 		res1[MAXIPS];
  unsigned int 				i = 0;
  char			**	ipv4;
  
  /* make a clean start	*/
  memset ( res,	 0, sizeof (RvNetAddress *));
  memset ( res1, 0, sizeof (RvNetAddress));
      
  /* IPv4 addresses 	*/

  {
    char 				hostName[0xFF];
  
    if ( gethostname ( hostName,sizeof ( hostName)) != 0 ) {

      dtprintf( "getHostAddresses(): gethostname() failed: %s\n",strerror (errno));

    } else {
    
       struct hostent  	hostEntry;
       struct hostent *heresp=0;
       int bufsize=10000*MAXIPS;
       char *hebuf=(char*)(malloc(bufsize));
       int err=0;
       
       int retres = gethostbyname2_r(hostName,AF_INET,&hostEntry,hebuf,bufsize,
          &heresp,&err);
       
       if (retres) {
	 //dtprintf ("getHostAddresses(): gethostbyname2_r() failed: %d: %s\n",err, hstrerror(err));
       } else {
          ipv4 = hostEntry.h_addr_list;
          for ( ; ipv4[i] && i < MAXIPS;i++ ) {
             RTP_makeRvNetAddress (AF_INET, NULL, 0, ( RvUint8 * ) ipv4[i],0,0,&res1[i]);
             res[i] = &res1[i];
          }
       }

       free(hebuf);
    }

#if (RV_NET_TYPE & RV_NET_IPV6)
    /* IPv6 addresses	*/

    {
      FILE * file = fopen ( "/proc/net/if_inet6","r" );
    
      if ( file ) {
        unsigned int addr[4];
        int plen, scope, dad_status, if_idx;
        char devname[21];

        while ( ( fscanf ( file,"%8x%8x%8x%8x %x %x %x %x %20s\n",
                           addr,addr+1,addr+2,addr+3,&if_idx, &plen, &scope, &dad_status, devname ) != EOF
                  ) 
                &&
                (i < MAXIPS)
                ) {
          int j;

          for (j=0;j<4;j++  ){
            addr[j] = htonl ( addr[j]);
          }

          /* make and store a new RvAddress 	*/
          RTP_makeRvNetAddress (AF_INET6, NULL, 0, ( RvUint8 * )addr ,0,if_idx,&res1[i]);
          res[i] = &res1[i];
          i++;
        
        } /* while */

        fclose ( file );
      
      } else {
        dtprintf("getHostAddresses(): failed to open if_inet6: %s\n", strerror(errno));
      }
    }
#endif

  }
    
  return (res);
}

void RTP_printRvNetAddress (RvNetAddress *addr )
{
  char buf[INET6_ADDRSTRLEN];
  
  if ( addr ) {

    switch ( RvNetGetAddressType(addr) ) {
    case RVNET_ADDRESS_IPV4:
      {
        RvNetIpv4 * ipv4 = RTP_getRvNetIp4(addr) ;
        inet_ntop (AF_INET, ( void *)&(ipv4 -> ip), buf,INET6_ADDRSTRLEN);
        dtprintf ( "IPv4 : %s[%s]:%d:%d\n",buf,addr->if_name,(int)ipv4 -> port,(int)addr->vdevblock);
      }
      
      break;

#if (RV_NET_TYPE & RV_NET_IPV6)
    case RVNET_ADDRESS_IPV6:
      {
        RvNetIpv6 * ipv6 = RTP_getRvNetIp6(addr) ;
        inet_ntop (AF_INET6, ( void * ) ipv6 -> ip,buf,INET6_ADDRSTRLEN);
        dtprintf ( "IPv6 : %s[%s]:%d scope %d flowinfo %lu\n",buf,addr->if_name,ipv6 -> port, ipv6 -> scopeId, ipv6->flowinfo);
      }
      break;
#endif

    case RVNET_ADDRESS_NONE:	/* fall through */
    default:
      dtprintf ( "RTP_printRvNetAddress(): Unbeknownst address type %d\n",RvNetGetAddressType(addr));
      break;
      
    } /* switch */
    
  } else {

    dtprintf ( "NULL\n" );

  }
  
  return;
}


void RTP_initRvNetAddress(sa_family_t af, const RvChar *if_name, RvUint16 vdevblock, const char *src, RvNetAddress *addr)
{   

  RVSIP_ADJUST_VDEVBLOCK(vdevblock);

  switch ( af ) {
  case AF_INET:
    {
      struct in_addr ipv4;
      inet_pton ( af,src, ( void * )&ipv4 );
      RTP_makeRvNetAddress (af, if_name, vdevblock, ( RvUint8 * )&(ipv4.s_addr),0,0,addr);
    }
    break;

#if (RV_NET_TYPE & RV_NET_IPV6)
  case AF_INET6:
    {
      struct in6_addr ipv6;
      inet_pton ( af,src, ( void * )&ipv6 );
      RTP_makeRvNetAddress (af, if_name, vdevblock, ( RvUint8 * )ipv6.s6_addr,0,0,addr);
    }
    break;
#endif

  default:
    dtprintf ( "RTP_initRvNetAddress(): address family %d is not supported\n",af );
  }

  return;
}

RvUint16 RTP_getPort(RvNetAddress *addr)
{
   if(addr) {
      return RvAddressGetIpPort(toRvAddress(addr));
   } else {
      return 0;
   }
}

void	RTP_setPort(RvNetAddress *addr,RvUint16 port)
{
   if(addr) {
      RvAddressSetIpPort(toRvAddress(addr),port);
   }
}

/* NOTE: non-reentrant */
const char * RTP_getIpString(RvNetAddress *addr)
{
  static char buf[INET6_ADDRSTRLEN];

  memset (buf,0,sizeof (buf));
  
  if ( addr ) {

    RvAddressGetString(toRvAddress(addr),sizeof(buf),buf);
      
  } else {

    dtprintf ( "RTP_getIpString (NULL ) \n" );

  }
  
  return (buf) ;
    
}

RvBool RTP_cmpRvNetAddresses(RvNetAddress *addr1,RvNetAddress *addr2, RvBool checkPorts)
{
  int compareType=RV_ADDRESS_BASEADDRESS;
  
  if(checkPorts) compareType=RV_ADDRESS_FULLADDRESS;

  return RvAddressCompare(toRvAddress(addr1),toRvAddress(addr2),compareType); 
}

RvBool RTP_isIpValid( RvNetAddress * addr )
{
   // Check if IP is valid.
   RvBool res =RV_FALSE;
  
   switch ( RvNetGetAddressType(addr) ) {
   case RVNET_ADDRESS_IPV4:
      res = (RvBool)( (RTP_getRvNetIp4(addr)->ip != 0) && (RTP_getRvNetIp4(addr)->ip != INADDR_NONE) );
      break;
#if (RV_NET_TYPE & RV_NET_IPV6)
   case RVNET_ADDRESS_IPV6:
      res = RV_TRUE;
      break;
#endif
   case RVNET_ADDRESS_NONE:	/* fall through */
   default:
      break;
    } /* switch */
    
  return (res);
}

int RvRtpResolveCollision(RvRtpSession  hRTP) {
   
   if(hRTP) {
      
      if(RvRtpGetRTCPSession(hRTP)) {
         RvRtcpDisconnectOnCollision(RvRtpGetRTCPSession(hRTP), RvRtpGetSSRC(hRTP));
      }
      
      RvRtpRegenSSRC(hRTP);
      
      if(RvRtpGetRTCPSession(hRTP)) {  
         RvRtcpSetSSRC(RvRtpGetRTCPSession(hRTP), RvRtpGetSSRC(hRTP));
      }
      
      return 1;
   }
   return 0;
}

int RvRtpSetTypeOfServiceSpirent(RvRtpSession  hRTP, int value, RvNetAddress *addr)
{ 
    int res=-1;
    int val;
   
    RvRtpSessionInfo* si =(RvRtpSessionInfo*) hRTP;

    if(si) {
       
#if (RV_NET_TYPE & RV_NET_IPV6)
       if (RvNetGetAddressType(addr) == RVNET_ADDRESS_IPV6) {
          int val = 1; // send flow information
          res = RvSocketSetSockOpt(&si->socket, IPPROTO_IPV6, IPV6_FLOWINFO_SEND , (void *) &val, sizeof(val));
          RTP_getRvNetIp6(addr)->flowinfo = ((unsigned long) value) << 4;
	  {
	    int tclass = value & 0xff;
	    res = RvSocketSetSockOpt(&si->socket, SOL_IPV6, IPV6_TCLASS , &tclass, sizeof(tclass));
	  }
       } else 
#endif // RV_IPV6
       {
          res = RvRtpSetTypeOfService(hRTP,value);
       }
    }
    return res;
}

int RvRtcpSetTypeOfServiceSpirent(RvRtcpSession  hRTCP, int value, RvNetAddress *addr)
{ 
    int res=-1;
    int val;
   
    rtcpSession* si =(rtcpSession*) hRTCP;

    if(si) {
       
#if (RV_NET_TYPE & RV_NET_IPV6)
       if (RvNetGetAddressType(addr) == RVNET_ADDRESS_IPV6) {
          int val = 1; // send flow information
          res = RvSocketSetSockOpt(&si->socket, IPPROTO_IPV6, IPV6_FLOWINFO_SEND , (void *) &val, sizeof(val));
          RTP_getRvNetIp6(addr)->flowinfo = ((unsigned long) value) << 4;
	  {
	    int tclass = value & 0xff;
	    res = RvSocketSetSockOpt(&si->socket, SOL_IPV6, IPV6_TCLASS , &tclass, sizeof(tclass));
	  }
       } else 
#endif // RV_IPV6
       {
          res = RvRtcpSetTypeOfService(hRTCP,value);
       }
    }
    return res;
}

void RvRtpSetSSRC(RvRtpSession  hRTP, unsigned int ssrc)
{
    RvRtpSessionInfo *s = (RvRtpSessionInfo *)hRTP;
    if(s) {
      s->sSrc=ssrc;
    }
}

////////////////////////////////////////////

static FILE* rtplogfile=0;

static pthread_mutex_t rtpprintf_mutex = PTHREAD_MUTEX_INITIALIZER;

static pid_t gettid(void) {
  return (pid_t)(syscall(SYS_gettid));
}

void rtpprintf(const char* format,...) {

   int tid=(int)gettid();
   unsigned long t = (unsigned long)time(0);

   va_list ap;

   pthread_mutex_lock(&rtpprintf_mutex);

   if(!rtplogfile) {
     char fname[129];
     sprintf(fname,"/tmp/ch.rtp.%d.log",tid);
     if(!rtplogfile) {
       rtplogfile=fopen(fname,"w");
     }
   }

   {
     fprintf(rtplogfile,"%d: %lu: ",tid,t);

     va_start(ap, format);
     vfprintf( rtplogfile, format, ap);
     va_end(ap);
     
     fflush(rtplogfile);
   }

   pthread_mutex_unlock(&rtpprintf_mutex);
}

#endif //#if defined(UPDATED_BY_SPIRENT)
