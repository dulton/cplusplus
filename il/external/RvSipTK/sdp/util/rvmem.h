#if (0)
******************************************************************************
Filename:
Description:
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

#ifndef RV_MEM_H
#define RV_MEM_H

#include "rvtypes.h"
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#   include "rvalloc.h"
#else
#   include "rvdefalloc.h"
#endif
/* SPIRENT_END */

/*$
{callback:
	{name: RvMemHandler}
	{superpackage: RvUtil}
    {include: rvmem.h}
	{description:
		{p: A function for handling dynamic heap exhaustion.}
	}
	{proto: RvBool RvMemHandler(RvSizeT n);}
	{params:
		{param: {n: n} {d: The number of bytes that caused the dynamic heap exhaustion.}}
	}
	{returns:
		rvTrue if the handler has successfully returned n bytes to the dynamic heap and
		rvFalse if it could not.
	}
    {see_also:
        {n: RvMemHandler rvMemSetHandler(RvMemHandler h);}
        {n: RvMemHandler rvMemGetHandler(void);}
    }
}
$*/
typedef RvBool (*RvMemHandler)(RvSizeT n);

#if defined(__cplusplus)
extern "C" {
#endif

// Use rvDefaultAlloc, memory in osmem will be accessed.
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#define rvMemAlloc(n) (rvAllocGetDefaultAllocator()->alloc(NULL, n))
#define rvMemFree(p)  (rvAllocGetDefaultAllocator()->dealloc(NULL, 0, p))
#else
#define rvMemAlloc(n) (rvDefaultAlloc.alloc(NULL, n))
#define rvMemFree(p)  (rvDefaultAlloc.dealloc(NULL, 0, p))
#endif
/* SPIRENT_END */
#define rvMemInit()   (rvTrue)
#define rvMemEnd()
// void* rvMemCalloc(RvSizeT num, RvSizeT size);

RvMemHandler rvMemGetHandler(void);
RvMemHandler rvMemSetHandler(RvMemHandler h);

#ifdef RV_MEMTRACE_ON
void rvMemTraceCheckpoint(const char * chkpname);
void rvMemTraceStart(const char * chkpname);
void rvMemTraceStop(const char * chkpname);
void rvMemTraceConstruct(void);
void rvMemTraceDestruct(void);
RvInt rvMemTraceGetMemOut();
RvInt rvMemTraceGetPeakMemUsage();
#else
#define rvMemTraceCheckpoint(c)
#define rvMemTraceStart(c)
#define rvMemTraceStop(c)
#define rvMemTraceConstruct()
#define rvMemTraceDestruct()
#define rvMemTraceGetMemOut() 0
#define rvMemTraceGetPeakMemUsage() 0
#endif

#if defined(__cplusplus)
}
#endif

#endif

