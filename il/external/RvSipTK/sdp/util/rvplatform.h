#if (0)
******************************************************************************
Filename    :
Description :
******************************************************************************
                Copyright (c) 1999 RADVision Inc.
************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
No part of this publication may be reproduced in any form whatsoever
without written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
******************************************************************************
$Revision: #1 $
$Date: 2011/08/05 $
$Author: songkamongkol $
******************************************************************************
#endif

#ifndef RV_PLATFORM_H
#define RV_PLATFORM_H

/*****************************************************************************
 * Platform options:
 * RV_THREAD_ (Thread implementation)
 *  POSIX - POSIX compliant implementation.
 *  WIN32 - Microsoft OS specific implementation.
 *  VXWORKS - VxWorks specific implementation.
 *  PSOS - PSoS specific implementation.
 * RV_SEMAPHORE_ (Semaphore implementation)
 *  POSIX - POSIX compliant implementation.
 *  WIN32 - Microsoft OS specific implementation.
 *  VXWORKS - VxWorks specific implementation.
 *  PSOS - PSoS specific implementation.
 *  OSE - OSE specific implementation.
 * RV_MUTEX_ (Recursive mutex implementation)
 *  GENERIC - Platform independent implementation.
 *  POSIX - POSIX plus semi-standard recursive attribute implementation.
 *  REDHAT - Red Hat specific implementation.
 *  WIN32 - Microsoft OS specific implementation.
 *  VXWORKS - VxWorks specific implementation.
 *  PSOS - PSoS specific implementation.
 * RV_COND_ (Condition variable implementation)
 *  GENERIC - Platform independent implementation.
 *  POSIX - POSIX compliant implementation.
 * RV_MEM_  (Dynamic memory interface)
 *  ANSI
 *  OSE - OSE Specific implementation
 *  NUCLEUS - Nucleus Specific implementation
 *  PSOS - pSOS Specific implementation
 *  OTHER - Customer defined implementation.
 * RV_IO_  (I/O implementation)
 *  ANSI
 *  NONE
 * RV_FILEIO_ (File I/O implementation)
 *  ANSI
 *  NONE
 * RV_SOCKETS_ (Sockets implementation)
 *  BSD - BSD compliant implementation.
 *  WSOCK - Microsoft OS specific implementation.
 *  PSOS - PSoS specific implementation.
 *  NUCLEUS - Nucleus specific implementation.
 * RV_SOCKETADDR_ (Socket address implementation)
 *  BSD43 - BSD 4.3 compliant implementation.
 *  BSD44 - BSD 4.4 (and BSD 4.3 Reno) compliant implementation.
 * RV_DNS_  (DNS (e.g. gethostbyname, gethostbyaddr, etc) implementation)
 *  POSIX - POSIX compliant implementation.
 *  BSD - BSD compliant implementation.
 *  WSOCK - Microsoft OS specific implementation.
 *  VXWORKS - VxWorks specific implementation.
 *  PSOS - PSoS specific implementation.
 *  OSE - OSE specific implementation.
 *  NONE - No DNS support.  Domain names will not be resolved.
 * RV_RESOLVER_ (Resolver (e.g. res_query) implementation)
 *  UNIX4 - Unix 4 compliant resolver implementation.
 *  UNIX5 - Unix 5 compliant resolver implementation.
 *  WSOCK - Microsoft OS specific implementation.
 *  VXWORKS - VxWorks specific implementation.
 *  OSE - OSE specific implementation.
 *  NONE - No resolver implementation.
 *
 *****************************************************************************/
#include "rvconfig.h"
#include "rvsdptypes.h"

#if 0


#if (RV_OS_TYPE ==  RV_OS_TYPE_SOLARIS)
 #define __sun RV_OS_TYPE_SOLARIS
#elif (RV_OS_TYPE ==  RV_OS_TYPE_LINUX)
 #define _REDHAT RV_OS_TYPE_LINUX
#elif (RV_OS_TYPE ==  RV_OS_TYPE_WIN32)
 #define _WIN32 RV_OS_TYPE_WIN32
#elif (RV_OS_TYPE ==  RV_OS_TYPE_VXWORKS)
 #define _VXWORKS RV_OS_TYPE_VXWORKS
#elif (RV_OS_TYPE ==  RV_OS_TYPE_PSOS)
 #define _PSOS RV_OS_TYPE_PSOS
#elif (RV_OS_TYPE ==  RV_OS_TYPE_OSE)
 #define _OSE RV_OS_TYPE_OSE
#elif (RV_OS_TYPE ==  RV_OS_TYPE_NUCLEUS)
 #define _NUCLEUS RV_OS_TYPE_NUCLEUS
#elif (RV_OS_TYPE ==  RV_OS_TYPE_HPUX)
 #define __hpux RV_OS_TYPE_HPUX
#elif (RV_OS_TYPE ==  RV_OS_TYPE_TRU64)
 #define _TRU64 RV_OS_TYPE_TRU64
#elif (RV_OS_TYPE ==  RV_OS_TYPE_UNIXWARE)
 #define _UNIXWARE RV_OS_TYPE_UNIXWARE
#elif (RV_OS_TYPE ==  RV_OS_TYPE_WINCE) /* WinCE is different enough from Win32 that it has its own OS type */
 #define _WINCE RV_OS_TYPE_WINCE
#elif (RV_OS_TYPE ==  RV_OS_TYPE_SYMBIAN)
 #define _SYMBIAN RV_OS_TYPE_SYMBIAN
#elif (RV_OS_TYPE ==  RV_OS_TYPE_MOPI)  /* MOPI is a proprietary (non RV) core abstraction layer */
 #define _MOPI RV_OS_TYPE_MOPI
#elif (RV_OS_TYPE ==  RV_OS_TYPE_INTEGRITY)
 #define _INTEGRITY RV_OS_TYPE_INTEGRITY
#endif

/* rvthread: RV_THREAD_TYPE - Select thread interface to use */
#if (RV_THREAD_TYPE == RV_THREAD_VXWORKS)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
/*#undef RV_THREAD_VXWORKS*/
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_SOLARIS)
/*#undef RV_THREAD_SOLARIS*/
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_POSIX)
#undef RV_THREAD_SOLARIS
/*#undef RV_THREAD_POSIX*/
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_PSOS)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
/*#undef RV_THREAD_PSOS*/
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_OSE)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
/*#undef RV_THREAD_OSE*/
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_NUCLEUS)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
/*#undef RV_THREAD_NUCLEUS*/
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_WIN32)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
/*#undef RV_THREAD_WIN32*/
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_WINCE)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
/*#undef RV_THREAD_WINCE*/
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_UNIXWARE)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
/*#undef RV_THREAD_UNIXWARE*/
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_SYMBIAN)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
/*#undef RV_THREAD_SYMBIAN*/
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_MOPI)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
/*#undef RV_THREAD_MOPI*/
#undef RV_THREAD_NONE /* Work as single threaded only */
#elif (RV_THREAD_TYPE == RV_THREAD_NONE)
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
/*#undef RV_THREAD_NONE*/ /* Work as single threaded only */
#endif
#endif

/* rvthread: RV_THREAD_TYPE - Select thread interface to use */
#undef RV_THREAD_SOLARIS
#undef RV_THREAD_POSIX
#undef RV_THREAD_VXWORKS
#undef RV_THREAD_PSOS
#undef RV_THREAD_OSE
#undef RV_THREAD_NUCLEUS
#undef RV_THREAD_WIN32
#undef RV_THREAD_WINCE
#undef RV_THREAD_UNIXWARE
#undef RV_THREAD_SYMBIAN
#undef RV_THREAD_MOPI
#undef RV_THREAD_NONE /* Work as single threaded only */

/* rvsemaphore: RV_SEMAPHORE_TYPE - Select semaphore interface to use */
#undef RV_SEMAPHORE_POSIX
#undef RV_SEMAPHORE_VXWORKS
#undef RV_SEMAPHORE_PSOS
#undef RV_SEMAPHORE_OSE 
#undef RV_SEMAPHORE_NUCLEUS 
#undef RV_SEMAPHORE_WIN32 
#undef RV_SEMAPHORE_SYMBIAN 
#undef RV_SEMAPHORE_NONE  /* No effect, single threaded environment only */

/* rvmutex: RV_MUTEX_TYPE - Select MUTEX interface to use */
#undef RV_MUTEX_SOLARIS
#undef RV_MUTEX_LINUX 
#undef RV_MUTEX_POSIX 
#undef RV_MUTEX_VXWORKS 
#undef RV_MUTEX_PSOS 
#undef RV_MUTEX_WIN32_MUTEX 
#undef RV_MUTEX_WIN32_CRITICAL  /* Use critical sections instead */
#undef RV_MUTEX_MOPI 
#undef RV_MUTEX_MANUAL 
#undef RV_MUTEX_NONE /* No locking, single threaded environment only */


/*
 * Must check for VxWorks before Win32 because the GNU compiler
 * for VxWorks hosted on Win32 platforms defines _WIN32
 */
/***** VxWorks *****/
#if defined(_VXWORKS)
# define RV_OS_VXWORKS
# define RV_THREAD_VXWORKS
# define RV_SEMAPHORE_VXWORKS
# define RV_MUTEX_VXWORKS
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_NONE
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD44
# define RV_DNS_VXWORKS
# define RV_RESOLVER_VXWORKS
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (2)
# define RV_PRIORITY_NORMAL   (126)
# define RV_PRIORITY_MIN    (254)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** WinCE *****/
#elif defined(_WIN32_WCE)
# define RV_OS_WINCE
# define RV_THREAD_WIN32
# define RV_SEMAPHORE_WIN32
# define RV_MUTEX_WIN32
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_WSOCK
# define RV_SOCKETADDR_BSD43
# define RV_WINSOCK_2
# define RV_DNS_WSOCK
# define RV_RESOLVER_NONE
# define RV_BUILD_EXECUTABLE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (2)
# define RV_PRIORITY_NORMAL   (0)
# define RV_PRIORITY_MIN    (-2)
# define RV_PRIORITY_INCREMENT  (+1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Windows *****/
#elif defined(_WIN32)
# define RV_OS_WIN32
# define RV_THREAD_WIN32
# define RV_SEMAPHORE_WIN32
# define RV_MUTEX_WIN32
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_WSOCK
# define RV_SOCKETADDR_BSD43
# define RV_WINSOCK_2
# define RV_DNS_WSOCK
# define RV_RESOLVER_NONE
# define RV_BUILD_EXECUTABLE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (2)
# define RV_PRIORITY_NORMAL   (0)
# define RV_PRIORITY_MIN    (-2)
# define RV_PRIORITY_INCREMENT  (+1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Solaris *****/
#elif defined(__sun)
# define RV_OS_SOLARIS
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX

/* Detect recursive mutex support */
/* The RV_MUTEX_POSIX is much more efficient than
 * RV_MUTEX_GENERIC.  However, Solaris 2.6 can not use
 * RV_MUTEX_POSIX, and Solaris 2.7 and 2.8 require
 * patches.  RV_MUTEX_GENERIC should be used with
 * Solaris 2.6 or with Solaris 2.7 or 2.8 when the
 * patches are not applied.  By default the code assumes
 * Solaris 2.7 and 2.8 have the patches applied.
*/
#include <pthread.h>
#if defined(PTHREAD_MUTEX_RECURSIVE)
# define RV_MUTEX_POSIX
#else
# define RV_MUTEX_GENERIC
#endif

# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_POSIX
# define RV_RESOLVER_UNIX5
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  0 /* 0 = OS default stack size */
# define RV_STACKSIZE_SOCKETENGINE 0
# define RV_STACKSIZE_TRANSPORT 0
# define RV_STACKSIZE_SHARER    0
# define RV_STACKSIZE_SUBAGENT  0
	/* Thread priority value */
# define RV_PRIORITY_MAX    (0)
# define RV_PRIORITY_NORMAL   (10)
# define RV_PRIORITY_MIN    (20)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Tru64 *****/
#elif defined(_TRU64)
# define RV_OS_TRU64
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_POSIX
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_BSD
# define RV_RESOLVER_UNIX4
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  0 /* 0 = OS default stack size */
# define RV_STACKSIZE_SOCKETENGINE 0
# define RV_STACKSIZE_TRANSPORT 0
# define RV_STACKSIZE_SHARER    0
# define RV_STACKSIZE_SUBAGENT  0
	/* Thread priority value */
# define RV_PRIORITY_MAX    (0)
# define RV_PRIORITY_NORMAL   (10)
# define RV_PRIORITY_MIN    (20)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** HPUX *****/
#elif defined(__hpux)
# define RV_OS_HPUX
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_POSIX
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_BSD
# define RV_RESOLVER_UNIX4
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  0 /* 0 = OS default stack size */
# define RV_STACKSIZE_SOCKETENGINE 0
# define RV_STACKSIZE_TRANSPORT 0
# define RV_STACKSIZE_SHARER    0
# define RV_STACKSIZE_SUBAGENT  0
	/* Thread priority value */
# define RV_PRIORITY_MAX    (0)
# define RV_PRIORITY_NORMAL   (10)
# define RV_PRIORITY_MIN    (20)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Red Hat Linux *****/
#elif defined(_REDHAT)
# define RV_OS_REDHAT
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_LINUX
# define RV_MUTEX_REDHAT      /* SPIRENT_BEGIN: for use in MEGACO and SipTK stacks */
# define RV_COND_GENERIC
# define RV_CLOCK_PENTIUM /* Pentium and higher. Comment out for other CPUs */
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_NONE       /* SPIRENT_BEGIN: was RV_FILEIO_ANSI */
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_BSD
# define RV_RESOLVER_UNIX5
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  0 /* 0 = OS default stack size */
# define RV_STACKSIZE_SOCKETENGINE 0
# define RV_STACKSIZE_TRANSPORT 0
# define RV_STACKSIZE_SHARER    0
# define RV_STACKSIZE_SUBAGENT  0
	/* Thread priority value */
# define RV_PRIORITY_MAX    (0)
# define RV_PRIORITY_NORMAL   (10)
# define RV_PRIORITY_MIN    (20)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Monta Vista Linux *****/
#elif defined(_MONTAVISTA)
# define RV_OS_MONTAVISTA
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_LINUX
# define RV_COND_GENERIC
/*# define RV_CLOCK_PENTIUM   Pentium and higher. Comment out for other CPUs */
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_BSD
# define RV_RESOLVER_UNIX5
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  0 /* 0 = OS default stack size */
# define RV_STACKSIZE_SOCKETENGINE 0
# define RV_STACKSIZE_TRANSPORT 0
# define RV_STACKSIZE_SHARER    0
# define RV_STACKSIZE_SUBAGENT  0
	/* Thread priority value */
# define RV_PRIORITY_MAX    (0)
# define RV_PRIORITY_NORMAL   (10)
# define RV_PRIORITY_MIN    (20)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** PSoS *****/
#elif defined(_PSOS)
# define RV_OS_PSOS
# define RV_THREAD_PSOS
# define RV_SEMAPHORE_PSOS
# define RV_MUTEX_PSOS
# define RV_COND_GENERIC
# define RV_MEM_PSOS
# define RV_IO_ANSI
# define RV_FILEIO_NONE
# define RV_SOCKETS_PSOS
# define RV_SOCKETADDR_BSD43
# define RV_DNS_PSOS
# define RV_RESOLVER_NONE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (240)
# define RV_PRIORITY_NORMAL   (120)
# define RV_PRIORITY_MIN    (1)
# define RV_PRIORITY_INCREMENT  (+1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** OSE *****/
#elif defined(_OSE)
# define RV_OS_OSE
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_OSE
# define RV_MUTEX_GENERIC
# define RV_COND_GENERIC
# define RV_MEM_OSE
# error Entered OSE platform (temporary)  /* SPIRENT_BEGIN */
# define RV_IO_OSE
# define RV_FILEIO_NONE
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD43
# define RV_DNS_OSE
# define RV_RESOLVER_OSE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (1)
# define RV_PRIORITY_NORMAL   (15)
# define RV_PRIORITY_MIN    (31)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** Nucleus *****/
#elif defined(_NUCLEUS)
# define RV_OS_NUCLEUS
# define RV_THREAD_NUCLEUS
# define RV_SEMAPHORE_NUCLEUS
# define RV_MUTEX_GENERIC
# define RV_COND_GENERIC
# define RV_MEM_NUCLEUS
# define RV_IO_ANSI
# define RV_FILEIO_NONE
# define RV_SOCKETS_NUCLEUS
# define RV_SOCKETADDR_BSD43
# define RV_DNS_NUCLEUS
# define RV_RESOLVER_NONE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  32768
# define RV_STACKSIZE_SOCKETENGINE 32768
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (4)
# define RV_PRIORITY_NORMAL   (127)
# define RV_PRIORITY_MIN    (255)
# define RV_PRIORITY_INCREMENT  (-1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** INTEGRITY *****/
#elif defined(_INTEGRITY)
# define RV_OS_INTEGRITY
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_POSIX
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_NONE
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD44
# define RV_DNS_BSD
# define RV_RESOLVER_NONE
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  5120
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 12288
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (253)
# define RV_PRIORITY_NORMAL   (127)
# define RV_PRIORITY_MIN    (2)
# define RV_PRIORITY_INCREMENT  (1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

/***** QNX *****/
#elif defined(_QNX)
# define RV_OS_QNX
# define RV_THREAD_POSIX
# define RV_SEMAPHORE_POSIX
# define RV_MUTEX_POSIX
# define RV_COND_GENERIC
# define RV_MEM_ANSI
# define RV_IO_ANSI
# define RV_FILEIO_ANSI
# define RV_SOCKETS_BSD
# define RV_SOCKETADDR_BSD44
# define RV_DNS_POSIX
# define RV_RESOLVER_UNIX4
	/* Thread stack sizes */
# define RV_STACKSIZE_TIMERMGR  8192
# define RV_STACKSIZE_SOCKETENGINE 5120
# define RV_STACKSIZE_TRANSPORT 49152
# define RV_STACKSIZE_SHARER   5120
# define RV_STACKSIZE_SUBAGENT  5120
	/* Thread priority value */
# define RV_PRIORITY_MAX    (63)
# define RV_PRIORITY_NORMAL   (32)
# define RV_PRIORITY_MIN    (1)
# define RV_PRIORITY_INCREMENT  (1)
	/* Debugging */
# if defined(DEBUG) || defined(_DEBUG)
#  define RV_DEBUG_ON
# endif

#else
# error Unknown platform implementation
#endif

/* Alias for older priority values */
#define RV_PRIORITYVALUE_MAX  RV_PRIORITY_MAX
#define RV_PRIORITYVALUE_NORMAL  RV_PRIORITY_NORMAL
#define RV_PRIORITYVALUE_MIN  RV_PRIORITY_MIN
#define RV_PRIORITYVALUE_INCREMENT RV_PRIORITY_INCREMENT

#endif
