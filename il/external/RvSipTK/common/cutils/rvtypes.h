/***********************************************************************
Filename   : rvtypes.h
Description: type definitions for all RADVISION modules
************************************************************************
        Copyright (c) 2001 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/
#ifndef RV_TYPES_H
#define RV_TYPES_H

#include <stddef.h>
#include "rvconfig.h"

/* A number of constants need to be defined in the configuration headers
   in order for this file to work properly. The values should be set
   properly for the architecture, os, compiler, memory model, etc. The
   definitions required are:

   RV_SIZET_TYPE          size_t type (should always be size_t)
   RV_PTRDIFFT_TYPE       ptrdiff_t type (should always be ptrdiff_t)
   RV_CHAR_TYPE           standard character type (for char and char * only)

   RV_VAR_INT_TYPE        standard variable (non-fixed) size signed variable
   RV_VAR_UINT_TYPE       standard variable (non-fixed) size unsigned variable
   RV_SIGNED_INT8_TYPE    8 bit signed variable
   RV_UNSIGNED_INT8_TYPE  8 bit unsigned variable
   RV_SIGNED_INT16_TYPE   16 bit signed variable
   RV_UNSIGNED_INT16_TYPE 16 bit unsigned variable
   RV_SIGNED_INT32_TYPE   32 bit signed variable
   RV_UNSIGNED_INT32_TYPE 32 bit unsigned variable
   RV_SIGNED_INT64_TYPE   64 bit signed variable
   RV_UNSIGNED_INT64_TYPE 64 bit unsigned variable

   RV_VAR_INT_SUFFIX(n)        standard variable (non-fixed) size signed constant suffix
   RV_VAR_UINT_SUFFIX(n)       standard variable (non-fixed) size unsigned constant suffix
   RV_SIGNED_INT8_SUFFIX(n)    8 bit signed constant suffix
   RV_UNSIGNED_INT8_SUFFIX(n)  8 bit ubsigned constant suffix
   RV_SIGNED_INT16_SUFFIX(n)   16 bit signed constant suffix
   RV_UNSIGNED_INT16_SUFFIX(n) 16 bit unsigned constant suffix
   RV_SIGNED_INT32_SUFFIX(n)   32 bit signed constant suffix
   RV_UNSIGNED_INT32_SUFFIX(n) 32 bit unsigned constant suffix
   RV_SIGNED_INT64_SUFFIX(n)   64 bit signed constant suffix
   RV_UNSIGNED_INT64_SUFFIX(n) 64 bit unsigned constant suffix

   RV_VAR_INT_MAX  maximum value of the standard variable (non-fixed) size signed variable
   RV_VAR_INT_MIN  minimum value of the standard variable (non-fixed) size signed variable
   RV_VAR_UINT_MAX maximum value of the standard variable (non-fixed) size unsigned variable
*/

#if defined(__cplusplus)
extern "C" {
#endif

/* New typedef to be used in the SIMPLE Client API */ 
typedef double RvDouble;

/* Our types for size_t and ptrdiff_t, should never be anything */
/* different than ANSI standard but we define it just in case. */
typedef RV_SIZET_TYPE RvSize_t;
typedef RV_PTRDIFFT_TYPE RvPtrdiff_t;

/* Character type specifically for characters, not values */
typedef RV_CHAR_TYPE RvChar;

/* Variable size types, set to optimum variable size */
typedef RV_VAR_INT_TYPE RvInt;
#define RvIntConst(_n) (RV_VAR_INT_SUFFIX(_n))
#define RV_INT_MAX RV_VAR_INT_MAX
#define RV_INT_MIN RV_VAR_INT_MIN

typedef RV_VAR_UINT_TYPE RvUint;
#define RvUintConst(_n) (RV_VAR_UINT_SUFFIX(_n))
#define RV_UINT_MAX RV_VAR_UINT_MAX
#define RV_UINT_MIN RvUintConst(0)

typedef RV_VAR_LONG_TYPE RvLong;
typedef RV_VAR_ULONG_TYPE RvUlong;
/* Fixed size types */
typedef RV_SIGNED_INT8_TYPE RvInt8;
#define RvInt8Const(_n) (RV_SIGNED_INT8_SUFFIX(_n))
#define RV_INT8_MAX RvInt8Const(127)
#define RV_INT8_MIN (RvInt8Const(-127) - RvInt8Const(1))

typedef RV_UNSIGNED_INT8_TYPE RvUint8;
#define RvUint8Const(_n) (RV_UNSIGNED_INT8_SUFFIX(_n))
#define RV_UINT8_MAX RvUint8Const(255)
#define RV_UINT8_MIN RvUint8Const(0)

typedef RV_SIGNED_INT16_TYPE RvInt16;
#define RvInt16Const(_n) (RV_SIGNED_INT16_SUFFIX(_n))
#define RV_INT16_MAX RvInt16Const(32767)
#define RV_INT16_MIN (RvInt16Const(-32767) - RvInt16Const(1))

typedef RV_UNSIGNED_INT16_TYPE RvUint16;
#define RvUint16Const(_n) (RV_UNSIGNED_INT16_SUFFIX(_n))
#define RV_UINT16_MAX RvUint16Const(65535)
#define RV_UINT16_MIN RvUint16Const(0)

typedef RV_SIGNED_INT32_TYPE RvInt32;
#define RvInt32Const(_n) (RV_SIGNED_INT32_SUFFIX(_n))
#define RV_INT32_MAX RvInt32Const(2147483647)
#define RV_INT32_MIN (RvInt32Const(-2147483647) - RvInt32Const(1))

typedef RV_UNSIGNED_INT32_TYPE RvUint32;
#define RvUint32Const(_n) (RV_UNSIGNED_INT32_SUFFIX(_n))
#define RV_UINT32_MAX RvUint32Const(4294967295)
#define RV_UINT32_MIN RvUint32Const(0)

#if (RV_64BITS_TYPE == RV_64BITS_STANDARD)
/* standard 64 bit usage */
typedef RV_SIGNED_INT64_TYPE RvInt64;
#define RvInt64ShortConst(_n) (RV_SIGNED_INT64_SUFFIX(_n))
#define RvInt64Const2(_n)     (RV_SIGNED_INT64_SUFFIX(_n))
#define RvInt64Const(_s, _m, _l) ((RvInt64)(_s) * (((RvInt64)(_m) * RV_SIGNED_INT64_SUFFIX(0x100000000)) + (RvInt64)(_l)))
#define RV_INT64_MAX RvInt64Const(1,0x7FFFFFFF,0xFFFFFFFF)
#define RV_INT64_MIN RvInt64Const(-1,0x80000000,0)

typedef RV_UNSIGNED_INT64_TYPE RvUint64;
#define RvUint64ShortConst(_n) (RV_UNSIGNED_INT64_SUFFIX(_n))
#define RvUint64Const2(_n)     (RV_UNSIGNED_INT64_SUFFIX(_n))
#define RvUint64Const(_m, _l) (((RvUint64)(_m) * RV_UNSIGNED_INT64_SUFFIX(0x100000000)) + (RvUint64)(_l))
#define RV_UINT64_MAX RvUint64Const(0xFFFFFFFF,0xFFFFFFFF)
#define RV_UINT64_MIN RvUint64Const(0,0)

#else
/* manual 64 bit usage */
typedef struct {
    RvUint16 words[4];
} Rv64BitsBase;

typedef Rv64BitsBase RvInt64;
#define RvInt64ShortConst(_n) (RvInt64ShortAssign((_n)))
#define RvInt64Const2(_n)     (RvInt64ShortAssign((_n)))
#define RvInt64Const(_s, _m, _l) (RvInt64Assign((_s), (_m), (_l)))
#define RV_INT64_MAX RvInt64Const(1,0x7FFFFFFF,0xFFFFFFFF)
#define RV_INT64_MIN RvInt64Const(-1,0x80000000,0)

typedef Rv64BitsBase RvUint64;
#define RvUint64ShortConst(_n) (RvUint64Assign(0, (_n)))
#define RvUint64Const2(_n)     (RvUint64Assign(0, (_n)))
#define RvUint64Const(_m, _l) (RvUint64Assign((_m), (_l)))
#define RV_UINT64_MAX RvUint64Const(0xFFFFFFFF,0xFFFFFFFF)
#define RV_UINT64_MIN RvUint64Const(0,0)
#endif

#define RV_UINT64_ZERO RvUint64Const(0, 0)
#define RV_UINT64_ONE  RvUint64Const(0, 1)

#define RvUint64Incr(n) RvUint64Add(n, RV_UINT64_ONE)


/* RvIntPtr, RvUintPtr is defined as integer types capable to accommodate
 *  pointer data type without loosing 
 */


#if (RV_ARCH_BITS == RV_ARCH_BITS_32)
typedef RvInt32 RvIntPtr;
typedef RvUint32 RvUintPtr;
#elif (RV_ARCH_BITS == RV_ARCH_BITS_64)
typedef RvInt64 RvIntPtr;
typedef RvUint64 RvUintPtr;
#endif

/* the macro used to cast the pointer to integer 
 * gets the special meaning in the 64 bits world where such casting
 * truncates the pointer */
#if (RV_ARCH_BITS == RV_ARCH_BITS_64) && (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#define RvPtrToInt32(_n)    PtrToLong((_n))
#define RvPtrToUint32(_n)   PtrToUlong((_n))
#define RvPtrToLong(_n)     PtrToLong((_n))
#define RvPtrToUlong(_n)    PtrToUlong((_n))
 /*
The other way to define those macros:
    #define RvPtrToInt32(_n) ((RvInt32)(RvIntPtr)(_n))
    #define RvPtrToUint32(_n) ((RvUint32)(RvUintPtr)(_n))
    #define RvPtrToInt32(_n) ((RvInt32)(RvIntPtr)(_n))
    #define RvPtrToUint32(_n) ((RvUint32)(RvUintPtr)(_n))
fits not only the windows case but any.
*/
#else
#define RvPtrToInt32(_n)    _n
#define RvPtrToUint32(_n)   _n
#define RvPtrToLong(_n)     _n
#define RvPtrToUlong(_n)    _n
#endif


typedef RvIntPtr RvOffset;

/* Other standard types */
typedef RvUint RvBool;
#if (!defined(RV_FALSE))
#define RV_FALSE  RvUintConst(0)
#endif
#if (!defined(RV_TRUE))
#define RV_TRUE RvUintConst(1)
#endif

/* Misc Macros. Perhaps these should go elsewhere. */
#define RvMin(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
#define RvMax(_a, _b)   (((_a) > (_b)) ? (_a) : (_b))

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#pragma warning(push)
#pragma warning(disable : 4121)	/* alignment of a member was sensitive to packing */
#endif
/* Alignment macros. Perhaps these should go elsewhere. */
struct RvAlign_s { /* used only for natural data alignment macro */
	/* DO NOT CHANGE THE ORDER OF THE FIELDS */
    char x;
    double y;
};

typedef double RvAlignmentType;

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#pragma warning(pop)
#endif

/* Not pretty, but allows numbytes to be any valid size_t value which is */
/* a power of 2 and works no matter what the relationship between pointer */
/* and data sizes. Pay attention: argument to these macros may be evaluated */
/* multiple times, so no side effects, please */
 
#define RvAlignTo(_ptr, _numbytes) ((void *)((RvInt8 *)(_ptr) + (((RvSize_t)(_numbytes) - ((RvSize_t)(_ptr) & (RvSize_t)((_numbytes) - 1))) & (RvSize_t)((_numbytes) - 1))))

#define RvAlignValueTo(x, p2) ((x) + ( ((p2) - 1) - (((x) - 1) & ((p2) - 1)) ))


/* Natural alignment sizes and macros */
#define RV_ALIGN_DATASIZE (offsetof(struct RvAlign_s, y))
#define RV_ALIGN_PTRSIZE (sizeof(void *))
#define RV_ALIGN_SIZE /*lint -save -e506 */(RvMax(RV_ALIGN_DATASIZE, RV_ALIGN_PTRSIZE)) /*lint -restore*/
#define RvAlignPtr(_x) (RvAlignTo((_x), RV_ALIGN_PTRSIZE))
#define RvAlignData(_x) (RvAlignTo((_x), RV_ALIGN_DATASIZE))
#define RvAlign(_x) (RvAlignTo((_x), RV_ALIGN_SIZE))
#define RvAlignValue(x) RvAlignValueTo(x, RV_ALIGN_SIZE)

/* Fixed width alignment macros */
#define RvAlign16(_x) (RvAlignTo((_x), 2))
#define RvAlign32(_x) (RvAlignTo((_x), 4))
#define RvAlign64(_x) (RvAlignTo((_x), 8))

/* Round up size_t size to a multiple of size_t numbytes (size must be > 0 and <= maxsize - numbytes) */
#define RvRoundToSize(_size, _numbytes) ((RvSize_t)((_size) + (_numbytes) - 1 - ((RvSize_t)((_size) - 1) % (RvSize_t)(_numbytes))))


/*******************/
/* Unused arguments macro */
#define RV_UNUSED_ARG(_arg) (void)(_arg);

/* For more than 1 argument in argList it should be placed between parenthesis, e.g.
 * RV_UNUSED_ARGS((a, b, c, d));
 *
 */
#define RV_UNUSED_ARGS(argList)   ((void)(argList))

/* to get an offset of the field within the structure */
#define RV_OFFSETOF(_type,_field) /*lint -e(413) */ ((char*)&(((_type*)0)->_field)-(char*)0)



/* Some "empty" definitons that we can use for readability of the code */
#define IN
#define OUT
#define INOUT

/************************************************************************
 * RV_DECLARE_HANDLE
 * Definition used to create a general purpose handle. It is declared
 * in this weird way to allow better type checking with C compilers.
 ************************************************************************/
#define RV_DECLARE_HANDLE(_handleName) \
                                      typedef struct { int unused; } _handleName##__ ; \
                                      typedef const _handleName##__ * _handleName; \
                                      typedef _handleName*  LP##_handleName


/* Macro used to calculate the pointer position of a struct, given one of its
   internal fields */
#define RV_GET_STRUCT(_structTypedef, _fieldName, _fieldPtr) \
    ((_structTypedef *)( ((char *)(_fieldPtr)) - offsetof(_structTypedef, _fieldName) ))


/*****************************************/
/* Macro used for useful pragma messages */
#define Value2String(m)     #m
#define Macro2String(m)     Value2String(m)

#define RvReminder          __FILE__ "(" Macro2String(__LINE__) ") : Reminder: "
#define RvWarning           __FILE__ "(" Macro2String(__LINE__) ") : warning XXXX: "
#define RvError             __FILE__ "(" Macro2String(__LINE__) ") : error XXXX: "

/* Usage:
#pragma message(RvReminder "Remove these definitions before release")
*/

#define RvFMT(fmt) Value2String(fmt)

#define FMT_TSTAMP <%d:%9.9d>

#define FMT_TSTAMP_VAL(x) RvTimestampGetSecs(x), RvTimestampGetNsecs(x)

/* SPIRENT_BEGIN */
/*
 * The following types, macros, constants were defined in the old core files; but they are
 * deleted in this new core file.  So, we bring them back to make the existing MGCP,MEGACO codes
 * work w/ this core files.
 */

/* Typedefs for making basic types a single token for container preprocesor macros */
typedef void* RvVoidPtr;
typedef char* RvCharPtr;
typedef size_t RvSizeT;
typedef ptrdiff_t RvPtrDiffT;
typedef int RvMilliseconds;

/* Copy constructor for basic types */
#define rvDefaultConstructCopy(d, s, a) ((void)(a), *(d) = *(s), (d))
#define charConstructCopy(d, s, a)  rvDefaultConstructCopy(d, s, a)
#define intConstructCopy(d, s, a)  rvDefaultConstructCopy(d, s, a)
#define floatConstructCopy(d, s, a)  rvDefaultConstructCopy(d, s, a)
#define doubleConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvIntConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvInt8ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvInt16ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvInt32ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvUint8ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvUint16ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvUint32ConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvBoolConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvUintConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvVoidPtrConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)
#define RvCharPtrConstructCopy(d, s, a) rvDefaultConstructCopy(d, s, a)

/* Destructor for basic types */
#define rvDefaultDestruct(x) do {} while(0)
#define charDestruct(x)     rvDefaultDestruct(x)
#define intDestruct(x)     rvDefaultDestruct(x)
#define floatDestruct(x)    rvDefaultDestruct(x)
#define doubleDestruct(x)    rvDefaultDestruct(x)
#define RvIntDestruct(x)    rvDefaultDestruct(x)
#define RvInt8Destruct(x)    rvDefaultDestruct(x)
#define RvInt16Destruct(x)    rvDefaultDestruct(x)
#define RvInt32Destruct(x)    rvDefaultDestruct(x)
#define RvUint8Destruct(x)    rvDefaultDestruct(x)
#define RvUint16Destruct(x)    rvDefaultDestruct(x)
#define RvUint32Destruct(x)    rvDefaultDestruct(x)
#define RvBoolDestruct(x)    rvDefaultDestruct(x)
#define RvUintDestruct(x)    rvDefaultDestruct(x)
#define RvVoidPtrDestruct(x)   rvDefaultDestruct(x)
#define RvCharPtrDestruct(x)   rvDefaultDestruct(x)

/* Assignment operator for basic types */
#define rvDefaultCopy(d, s)    (*(d) = *(s), (d))
#define charCopy(d, s)     rvDefaultCopy(d, s)
#define intCopy(d, s)     rvDefaultCopy(d, s)
#define floatCopy(d, s)     rvDefaultCopy(d, s)
#define doubleCopy(d, s)    rvDefaultCopy(d, s)
#define RvIntCopy(d, s)    rvDefaultCopy(d, s)
#define RvInt8Copy(d, s)    rvDefaultCopy(d, s)
#define RvInt16Copy(d, s)    rvDefaultCopy(d, s)
#define RvInt32Copy(d, s)    rvDefaultCopy(d, s)
#define RvUint8Copy(d, s)    rvDefaultCopy(d, s)
#define RvUint16Copy(d, s)    rvDefaultCopy(d, s)
#define RvUint32Copy(d, s)    rvDefaultCopy(d, s)
#define RvBoolCopy(d, s)    rvDefaultCopy(d, s)
#define RvUintCopy(d, s)    rvDefaultCopy(d, s)
#define RvVoidPtrCopy(d, s)    rvDefaultCopy(d, s)
#define RvCharPtrCopy(d, s)    rvDefaultCopy(d, s)

/* Equal operator for basic types */
#define rvDefaultEqual(a, b)   (*(a) == *(b))
#define charEqual(a, b)     rvDefaultEqual(a, b)
#define intEqual(a, b)     rvDefaultEqual(a, b)
#define floatEqual(a, b)    rvDefaultEqual(a, b)
#define doubleEqual(a, b)    rvDefaultEqual(a, b)
#define RvIntEqual(a, b)    rvDefaultEqual(a, b)
#define RvInt8Equal(a, b)    rvDefaultEqual(a, b)
#define RvInt16Equal(a, b)    rvDefaultEqual(a, b)
#define RvInt32Equal(a, b)    rvDefaultEqual(a, b)
#define RvUint8Equal(a, b)    rvDefaultEqual(a, b)
#define RvUint16Equal(a, b)    rvDefaultEqual(a, b)
#define RvUint32Equal(a, b)    rvDefaultEqual(a, b)
#define RvBoolEqual(a, b)    rvDefaultEqual(a, b)
#define RvUintEqual(a, b)    rvDefaultEqual(a, b)
#define RvVoidPtrEqual(a, b)   rvDefaultEqual(a, b)
#define RvCharPtrEqual(a, b)   (strcmp(*(a), *(b)) == 0)

/* Equal only if a and b are the same object. */
#if !defined(rvNeverEqual)
# define rvNeverEqual(a, b)   ((a) == (b))
#endif

/* Less operator for basic types */
#define rvDefaultLess(a, b)    (*(a) < *(b))
#define charLess(a, b)     rvDefaultLess(a, b)
#define intLess(a, b)     rvDefaultLess(a, b)
#define floatLess(a, b)     rvDefaultLess(a, b)
#define doubleLess(a, b)    rvDefaultLess(a, b)
#define RvIntLess(a, b)    rvDefaultLess(a, b)
#define RvInt8Less(a, b)    rvDefaultLess(a, b)
#define RvInt16Less(a, b)    rvDefaultLess(a, b)
#define RvInt32Less(a, b)    rvDefaultLess(a, b)
#define RvUint8Less(a, b)    rvDefaultLess(a, b)
#define RvUint16Less(a, b)    rvDefaultLess(a, b)
#define RvUint32Less(a, b)    rvDefaultLess(a, b)
#define RvBoolLess(a, b)    rvDefaultLess(a, b)
#define RvUintLess(a, b)    rvDefaultLess(a, b)
#define RvVoidPtrLess(a, b)    rvDefaultLess(a, b)
#define RvCharPtrLess(a, b)    (strcmp(*(a), *(b)) < 0)

/* GetAllocator operator for basic types */
#define rvDefaultGetAllocator(x)  (0)
#define charGetAllocator(x)    rvDefaultGetAllocator(x)
#define intGetAllocator(x)    rvDefaultGetAllocator(x)
#define floatGetAllocator(x)   rvDefaultGetAllocator(x)
#define doubleGetAllocator(x)   rvDefaultGetAllocator(x)
#define RvIntGetAllocator(x)   rvDefaultGetAllocator(x)
#define RvInt8GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvInt16GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvInt32GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvUint8GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvUint16GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvUint32GetAllocator(x)   rvDefaultGetAllocator(x)
#define RvBoolGetAllocator(x)   rvDefaultGetAllocator(x)
#define RvUintGetAllocator(x)   rvDefaultGetAllocator(x)
#define RvVoidPtrGetAllocator(x)  rvDefaultGetAllocator(x)
#define RvCharPtrGetAllocator(x)  rvDefaultGetAllocator(x)

/* Swap operator for basic */
#define rvDeclareSwap(T)    void _rvSwap_##T(T* a, T* b);
#define rvDefineSwap(T)              \
void _rvSwap_##T(T* a, T* b) {            \
	T t;                 \
	T##ConstructCopy(&t, a, T##GetAllocator(a));       \
	T##Copy(a, b);               \
	T##Copy(b, &t);               \
	T##Destruct(&t);              \
}
#define rvSwap(T)      _rvSwap_##T

/* Swap for basic types */
#define charSwap(a, b)     _rvSwap_char(a, b)
#define shortSwap(a, b)     _rvSwap_short(a, b)
#define intSwap(a, b)     _rvSwap_int(a, b)
#define longSwap(a, b)     _rvSwap_long(a, b)
#define floatSwap(a, b)     _rvSwap_float(a, b)
#define doubleSwap(a, b)    _rvSwap_double(a, b)
#define RvIntSwap(a, b)    _rvSwap_RvInt8(a, b)
#define RvInt8Swap(a, b)    _rvSwap_RvInt8(a, b)
#define RvInt16Swap(a, b)    _rvSwap_RvInt16(a, b)
#define RvInt32Swap(a, b)    _rvSwap_RvInt32(a, b)
#define RvUint8Swap(a, b)    _rvSwap_RvUint8(a, b)
#define RvUint16Swap(a, b)    _rvSwap_RvUint16(a, b)
#define RvUint32Swap(a, b)    _rvSwap_RvUint32(a, b)
#define RvBoolSwap(a, b)    _rvSwap_RvBool(a, b)
#define RvUintSwap(a, b)    _rvSwap_RvUint(a, b)
#define RvVoidPtrSwap(a, b)    _rvSwap_RvVoidPtr(a, b)
#define RvCharPtrSwap(a, b)    _rvSwap_RvCharPtr(a, b)

/* Declare swap functions */
rvDeclareSwap(char)
rvDeclareSwap(int)
rvDeclareSwap(float)
rvDeclareSwap(double)
rvDeclareSwap(RvInt)
rvDeclareSwap(RvInt8)
rvDeclareSwap(RvInt16)
rvDeclareSwap(RvInt32)
rvDeclareSwap(RvUint8)
rvDeclareSwap(RvUint16)
rvDeclareSwap(RvUint32)
rvDeclareSwap(RvBool)
rvDeclareSwap(RvUint)
rvDeclareSwap(RvVoidPtr)
rvDeclareSwap(RvCharPtr)

/* SPIRENT_END */



#include "rv64bits.h"

#if defined(__cplusplus)
}
#endif

#endif /* RV_TYPES_H */
