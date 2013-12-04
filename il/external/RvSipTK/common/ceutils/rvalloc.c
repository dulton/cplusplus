#if (0)
************************************************************************
Filename:
Description:
************************************************************************
                Copyright (c) 1999 RADVision Inc.
************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
No part of this publication may be reproduced in any form whatsoever
without written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
************************************************************************
************************************************************************
$Revision: #1 $
$Date: 2011/08/05 $
$Author: songkamongkol $
************************************************************************
#endif

#include "rvalloc.h"
#include "rvassert.h"
#include "rvstdio.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*$
{function:
    {name: rvAllocConstruct}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Construct an allocator object.}
    }
    {proto: RvAlloc* rvAllocConstruct(RvAlloc* a, void* pool, RvSize_t maxSize, void* (*alloc)(void*, RvSize_t), void (*dealloc)(void*, RvSize_t, void*));}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
        {param: {n: pool} {d: A pointer to a pool.}}
        {param: {n: maxSize} {d: The size of the largest memory block the
        allocator is capable of allocating.}}
        {param: {n: alloc} {d: A pointer to a function capable of allocating memory
        for the allocator.}}
        {param: {n: dealloc} {d: A pointer to a function capable of deallocating memory
        allocated by the allocator.}}
    }
    {returns: A pointer the allocator or NULL on error.}
    {notes:
        {note: The function specified for the alloc parameter is required to return NULL on
        error or memory exhaustion.}
    }
}
$*/
RvAlloc* rvAllocConstruct(RvAlloc* a, void* pool, RvSize_t maxSize,
  void* (*alloc)(void*, RvSize_t),
  void (*dealloc)(void*, RvSize_t, void*)) {
    a->pool = pool;
    a->maxSize = maxSize;
    a->alloc = alloc;
    a->dealloc = dealloc;
#ifdef _USE_ALLOC_TRACE
    a->alloc_TRACE   = NULL;
    a->dealloc_TRACE = NULL;
#endif
    return a;
}
#ifdef _USE_ALLOC_TRACE
RvAlloc* rvAllocConstruct_TRACE(RvAlloc* a, void* pool, RvSize_t maxSize,
  void* (*alloc)(void*, RvSize_t), void (*dealloc)(void*, RvSize_t, void*),
  void* (*alloc_TRACE)(void*, RvSize_t, char*, int),
  void (*dealloc_TRACE)(void*, RvSize_t, void*, char*, int)) {
    a->pool = pool;
    a->maxSize = maxSize;
    a->alloc = alloc;
    a->dealloc = dealloc;
    a->alloc_TRACE   = alloc_TRACE;
    a->dealloc_TRACE = dealloc_TRACE;
    return a;
}
#endif

#ifdef _USE_ALLOC_TRACE
void* rvAllocAllocate_TRACE(RvAlloc* a, RvSize_t s, char* file, int line) {
    if (s > a->maxSize)
        return NULL;
    if (a->alloc_TRACE)
      return a->alloc_TRACE(a->pool, s, file, line);
    else
      return a->alloc(a->pool, s);
}

void rvAllocDeallocate_TRACE(RvAlloc* a, RvSize_t s, void* ptr, char* file, int line) {
    if (a->alloc_TRACE)
      a->dealloc_TRACE(a->pool, s, ptr, file, line);
    else
      a->dealloc(a->pool, s, ptr);
}
#else
/*$
{function:
    {name: rvAllocAllocate}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Allocate a block of memory of s bytes.}
    }
    {proto: void* rvAllocAllocate(RvAlloc* a, RvSize_t s);}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
        {param: {n: s} {d: Number of bytes to allocate.}}
    }
    {returns:
        A pointer to a suitably sized and aligned
        block of memory or NULL if memory exhaustion occurs.
    }
}
$*/
void* rvAllocAllocate(RvAlloc* a, RvSize_t s) {
    if (s > a->maxSize)
        return NULL;
    return a->alloc(a->pool, s);
}

/*$
{function:
    {name: rvAllocDeallocate}
    {class: RvAlloc}
    {include: rvalloc.h}
    {description:
        {p: Deallocate a block of memory pointed to by ptr.
            The memory must have been previously allocated
            from the same allocator.
        }
    }
    {proto: void rvAllocDeallocate(RvAlloc* a, RvSize_t s, void* ptr);}
    {params:
        {param: {n: a} {d: A pointer to the allocator.}}
        {param: {n: s} {d: The size of the memory block.}}
        {param: {n: ptr} {d: A pointer to the memory block to deallocate.}}
    }
    {notes:
        {note:
            Depending on the underlying memory allocator, the size
            argument (s) my be required or optional.
        }
    }
}
$*/
void rvAllocDeallocate(RvAlloc* a, RvSize_t s, void* ptr) {
    a->dealloc(a->pool, s, ptr);
}
#endif

void rvAssert(int expression)
{
    RV_UNUSED_ARG(expression);
    RvAssert(expression);
}


#include "rvmemory.h"
#include "rvccoreglobals.h"

/* Default allocator */
static void* alloc_(void* pool, RvSize_t s) 
{
    void* memPtr;
    
    RV_UNUSED_ARG(pool);
    
    RvMemoryAlloc(NULL, s, NULL, &memPtr);
    
    return memPtr;
}

static void dealloc_(void *pool, RvSize_t s, void* x) 
{ 
    RV_UNUSED_ARG(pool);
    RV_UNUSED_ARG(s);
    
    RvMemoryFree(x, NULL); 
}


 
/* RvAlloc rvDefaultAlloc = {0, ~0U, alloc_, dealloc_}; */

#ifdef _USE_ALLOC_TRACE
const RvAlloc rvConstDefaultAlloc = {0, ~0U, alloc_, dealloc_, NULL, NULL};
#else
const RvAlloc rvConstDefaultAlloc = {0, ~0U, alloc_, dealloc_};
#endif

RVCOREAPI
RvAlloc* rvAllocGetDefaultAllocator(void) 
{
    RV_USE_CCORE_GLOBALS;
    return &rvDefaultAlloc;
}


#if defined(__cplusplus)
}
#endif

