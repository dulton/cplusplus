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
#ifndef RV_UTIL_H
#define RV_UTIL_H

#include "rvalloc.h"
#include "rvtypes.h"
/*$
{package:
	{name: RvUtil}
	{superpackage: RvCore}
	{description:
		{p: This package provides core utility types and functions.}
	}
}
$*/

/* Pair template */
#define rvDeclarePair(T, U) \
	typedef struct { \
		T first; \
		U second; \
	} _RvPair##T##U; \
	RvPair(T, U)* _RvPair##T##U##Construct(RvPair(T, U)* p, T* t, U* u); \
	RvPair(T, U)* _RvPair##T##U##ConstructCopy(RvPair(T, U)* d, \
	  const RvPair(T, U)* s, RvAlloc* a); \
	void _RvPair##T##U##Destruct(RvPair(T, U)* p); \
	RvPair(T, U)* _RvPair##T##U##Copy(RvPair(T, U)* d, \
	  const RvPair(T, U)* s); \
	RvBool _RvPair##T##U##Equal(const RvPair(T, U)* a, const RvPair(T, U)* b); \
	RvSizeT _RvPair##T##U##Hash(RvPair(T, U)* p); \
	void _RvPair##T##U##Swap(RvPair(T, U)* a, RvPair(T, U)* b);

#define rvDefinePair(T, U) \
	RvPair(T, U)* _RvPair##T##U##Construct(RvPair(T, U)* p, T* t, U* u) { \
		T##ConstructCopy(&p->first, t, T##GetAllocator(t)); \
		U##ConstructCopy(&p->second, u, U##GetAllocator(u)); \
		return p; \
	} \
	RvPair(T, U)* _RvPair##T##U##ConstructCopy(RvPair(T, U)* d, \
	  const RvPair(T, U)* s, RvAlloc* a) { \
		T##ConstructCopy(&d->first, &s->first, a); \
		U##ConstructCopy(&d->second, &s->second, a); \
		return d; \
	} \
	void _RvPair##T##U##Destruct(RvPair(T, U)* p) { \
		T##Destruct(&p->first); \
		U##Destruct(&p->second); \
	} \
	RvPair(T, U)* _RvPair##T##U##Copy(RvPair(T, U)* d, \
	  const RvPair(T, U)* s) { \
		T##Copy(&d->first, &s->first); \
		U##Copy(&d->second, &s->second); \
		return d; \
	} \
	RvBool _RvPair##T##U##Equal(const RvPair(T, U)* a, const RvPair(T, U)* b) { \
		return T##Equal(&a->first, &b->first) && \
		  U##Equal(&a->second, &b->second); \
	} \
	void _RvPair##T##U##Swap(RvPair(T, U)* a, RvPair(T, U)* b) { \
		T##Swap(&a->first, &b->first); \
		U##Swap(&a->second, &b->second); \
	}

#define RvPair(T, U)    _RvPair##T##U
#define rvPairConstruct(T, U)  _RvPair##T##U##Construct
#define rvPairConstructCopy(T, U) _RvPair##T##U##ConstructCopy
#define rvPairDestruct(T, U)  _RvPair##T##U##Destruct
#define rvPairCopy(T, U)   _RvPair##T##U##Copy
#define rvPairEqual(T, U)   _RvPair##T##U##Equal
#define rvPairSwap(T, U)   _RvPair##T##U##Swap

/* Unsafe, but useful macros */
#define rvMin(a, b)      (((a) < (b)) ? (a) : (b))
#define rvMax(a, b)      (((a) > (b)) ? (a) : (b))

/* Natural alignment macro */
#define rvAlign(x)  (((RvPtrDiffT)(x) + (sizeof(RvPtrDiffT) - 1)) & (~(sizeof(RvPtrDiffT) - 1)))

/* Fixed width alignment macros */
#define rvAlign16(x) (((RvPtrDiffT)(x) + 1) & (~1))
#define rvAlign32(x) (((RvPtrDiffT)(x) + 3) & (~3))
#define rvAlign64(x) (((RvPtrDiffT)(x) + 7) & (~7))

#endif
