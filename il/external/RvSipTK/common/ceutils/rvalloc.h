#if (0)
******************************************************************************
Filename:
Description: C/Macro-based allocator
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

#ifndef RV_ALLOC_H
#define RV_ALLOC_H
#include "rvtypes.h"
#include "rverror.h"

#undef _USE_ALLOC_TRACE
//#define _USE_ALLOC_TRACE

/*$
{type:
	{name: RvAlloc}
	{superpackage: Util}	
	{include: rvalloc.h}
	{description:	
		{p: Describes a generic interface to a dynamic memory allocator.}
		{p: Allocators are used to parameterize memory management function
			for a given algorithm.  By parameterizing the allocator the 
			algorithm is no longer bound to a single memory management 
			methodology.
		}
	}
	{methods:
		{method: RvAlloc* rvAllocConstruct(RvAlloc* a, void* pool, RvSize_t maxSize, void* (*alloc)(void*, RvSize_t), void (*dealloc)(void*, RvSize_t, void*));}
		{method: void rvAllocDestruct(RvAlloc* a);}
		{method: void* rvAllocAllocate(RvAlloc* a, RvSize_t s);}
		{method: void rvAllocDeallocate(RvAlloc* a, RvSize_t s, void* ptr);}
		{method: void* rvAllocGetPool(RvAlloc* a);}
		{method: RvSize_t rvAllocGetMaxSize(RvAlloc* a);}
	}
}
$*/
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct RvAlloc_ {
    void* pool;
    RvSize_t maxSize;
    void* (*alloc)(void* pool, RvSize_t s);
    void (*dealloc)(void* pool, RvSize_t s, void* x);
#ifdef _USE_ALLOC_TRACE
    void* (*alloc_TRACE)(void* pool, RvSize_t s, char* file, int line);
    void (*dealloc_TRACE)(void* pool, RvSize_t s, void* x, char* file, int line);
#endif
} RvAlloc;


RVCOREAPI 
RvAlloc* rvAllocConstruct(RvAlloc* a, void* pool, RvSize_t maxSize,
  void* (*alloc)(void*, RvSize_t), void (*dealloc)(void*, RvSize_t, void*));

#ifdef _USE_ALLOC_TRACE
RvAlloc* rvAllocConstruct_TRACE(RvAlloc* a, void* pool, RvSize_t maxSize,
  void* (*alloc)(void*, RvSize_t), void (*dealloc)(void*, RvSize_t, void*),
  void* (*alloc_TRACE)(void*, RvSize_t, char*, int),
  void (*dealloc_TRACE)(void*, RvSize_t, void*, char*, int));
#endif

/*$
{function:
    {name: rvAllocDestruct}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Destruct an allocator object.}
    }
    {proto: void rvAllocDestruct(RvAlloc* a);}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
    }
}
$*/
#define rvAllocDestruct(a)

#define rvAllocConstructCopy rvDefaultConstructCopy

#ifdef _USE_ALLOC_TRACE
  #define rvAllocAllocate(a,s)         rvAllocAllocate_TRACE(a,s,__FILE__,__LINE__)

  #define rvAllocDeallocate(a,s,ptr)   rvAllocDeallocate_TRACE(a,s,ptr,__FILE__,__LINE__)

void* rvAllocAllocate_TRACE(RvAlloc* a, RvSize_t s, char* file, int line);
void rvAllocDeallocate_TRACE(RvAlloc* a, RvSize_t s, void* ptr, char* file, int line);
#else

RVCOREAPI
void* rvAllocAllocate(RvAlloc* a, RvSize_t s);

RVCOREAPI
void rvAllocDeallocate(RvAlloc* a, RvSize_t s, void* ptr);

#endif

RVCOREAPI
RvAlloc *rvAllocGetDefaultAllocator(void);

#define rvAllocAllocateObject(a,t)    (t *)rvAllocAllocate(a, sizeof(t))
#define rvAllocDeallocateObject(a,t,o)  if((o) != NULL){ rvAllocDeallocate(a, sizeof(t), (o));(o) = NULL; }

/*$
{function:
    {name: rvAllocGetPool}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Get the allocator's pool.}
    }
    {proto: void* rvAllocGetPool(RvAlloc* a);}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
    }
    {returns: A pointer to the pool.}
}
$*/
#define rvAllocGetPool(a)                       ((a)->pool)
/*$
{function:
    {name: rvAllocGetMaxSize}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Get the maximum size (in bytes) the allocator can allocate.}
    }
    {proto: RvSize_t rvAllocGetMaxSize(RvAlloc* a);}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
    }
    {returns: The maximum allocation size.}
}
$*/
#define rvAllocGetMaxSize(a)                    ((a)->maxSize)

void rvAssert(int expression);

/* Some general definitions 
 * They has nothing to do with allocations actually,
 *  but there is no better place for them
 */

/* SPIRENT_BEGIN */
/* the following types have been moved to rvtypes.h 
#define rvDefaultConstructCopy(d, s, a) ((void)(a), *(d) = *(s), (d))
#define rvDefaultDestruct(x)			
#define rvDefaultEqual(a, b)			(*(a) == *(b))

typedef void* RvVoidPtr;

#define RvVoidPtrConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvVoidPtrDestruct(x)			rvDefaultDestruct(x)
#define RvVoidPtrEqual(a, b)			rvDefaultEqual(a, b)
*/
/* SPIRENT_END */

extern const RvAlloc rvConstDefaultAlloc;

#if defined(__cplusplus)
}
#endif

#endif

