
/* FILE HEADER */

#ifndef __rvexternal_H__
#define __rvexternal_H__

#include <stdint.h>

/***************************************************************************
 * FILE: $RCSfile$
 *
 * PURPOSE: SPIRENT proprietary include files
 *
 * DESCRIPTION:
 *
 * WRITTEN BY: Oleg Moskalenko, 2007
 *
 * CONFIDENTIAL  
 * Proprietary Information of Spirent Communications Plc.
 * 
 * Copyright (c) Spirent Communications Plc., 2004 
 * All rights reserved.
 *
 ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void rtpprintf(const char* format,...);
uint64_t _getMilliSeconds(void);

#ifdef __cplusplus
}
#endif

#if defined(dprintf)
#undef dprintf
#endif

#if defined(dtprintf)
#undef dtprintf
#endif

#if defined(dprintf_T)
#undef dprintf_T
#endif

#define dtprintf rtpprintf
#define dprintf dtprintf
#define dprintf_T rtpprintf

#define smp_system() (1)

#if !defined(ul_after)
#define ul_after(a,b)         ((long)(b) - (long)(a) < 0)
#endif

#if !defined(ul_before)
#define ul_before(a,b)        ul_after(b,a)
#endif

#if !defined(ul_minus)
#define ul_minus(a,b)         ((unsigned long)(a)-(unsigned long)(b))
#endif

#define RVSIP_MAX_NUM_VDEVBLOCK (256)
#define RVSIP_ADJUST_VDEVBLOCK(A) { if(A >= RVSIP_MAX_NUM_VDEVBLOCK) A = A % RVSIP_MAX_NUM_VDEVBLOCK; }
#define RVSIP_TRANSPORT_IFNAME_SIZE (65)

#endif /*__rvexternal_H__*/

