/******************************************************************************
Filename:    rvstring.c
Description: copy-on-write string class definitions
*******************************************************************************
                Copyright (c) 2000 RADVision Inc.
*******************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
No part of this publication may be reproduced in any form whatsoever
without written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
******************************************************************************/

#include "rvstring.h"
#include "rvstr.h"

#include "rvstdio.h"

/* Make macro
static RvStringHeader *rvStringGetHeader(const RvString *x)
{
    return (RvStringHeader *)(*x - sizeof(RvStringHeader));
}
*/

static char *rvStringConstructBuffer(RvString *x, RvSize_t stringlen, RvSize_t maxlen, RvAlloc *sdpAllocator)
{
    RvStringHeader *header;
    if(maxlen < 15)
        maxlen = 15;
    header = (RvStringHeader *)rvAllocAllocate(sdpAllocator, maxlen + 1 + sizeof(RvStringHeader));
    if(header == NULL)
        return *x = NULL;
    header->length = stringlen;
    header->maxlen = maxlen;
    header->refs = 1;
    header->alloc = sdpAllocator;
    return *x = (char *)header + sizeof(RvStringHeader);
}

static void rvStringReleaseBuffer(RvString *x)
{
    RvStringHeader *header = rvStringGetHeader(x);
    rvAllocDeallocate(header->alloc, header->maxlen + 1 + sizeof(RvStringHeader), header);
}

static RvString *rvStringIsolate(RvString *x, RvSize_t len, RvBool preserveContents)
{
    RvStringHeader *header = rvStringGetHeader(x);
    if(header->refs > 1)
    {
        char *oldBuffer = *x;
        --header->refs;
        if(rvStringConstructBuffer(x, len, len + header->length, header->alloc) == NULL) /* alloc extra */
            return NULL;
        if(preserveContents)
            strcpy(*x, oldBuffer);
    }
    else if(len > header->maxlen)
    {
        char *oldBuffer = *x;
        if(rvStringConstructBuffer(x, len, len + header->length, header->alloc) == NULL) {  /* alloc extra */
            rvStringReleaseBuffer(&oldBuffer); /* release old buffer */
            return NULL;
        }
        if(preserveContents)
            strcpy(*x, oldBuffer);
        rvStringReleaseBuffer(&oldBuffer);
    }
    else
        header->length = len;
    return x;
}

RvString *rvStringReserve(RvString *x, RvSize_t n)
{
    RvStringHeader *header = rvStringGetHeader(x);
    if(n > header->maxlen)
    {
        RvSize_t sz = header->length;
        if(rvStringIsolate(x, n, RV_TRUE) == NULL)
            return NULL;
        (*x)[rvStringGetHeader(x)->length = sz] = 0; /* 'header' no longer vaild */
    }
    return x;
}

RvString *rvStringResize(RvString *x, RvSize_t n)
{
    RvSize_t origsize = rvStringGetHeader(x)->length;
    if(rvStringIsolate(x, n, RV_TRUE) == NULL)
        return NULL;
    if(n > origsize)
        memset(*x + origsize, ' ', n - origsize);
    (*x)[n] = 0;
    return x;
}

/* Make macro
RvString* rvStringConstruct(RvString *x, const char *str, RvAlloc *allocator)
{
    return rvStringConstructN(x, str, strlen(str), allocator);
}
*/

RvString* rvStringConstructN(RvString *x, const char *str, RvSize_t n, RvAlloc *sdpAllocator)
{
    if(rvStringConstructBuffer(x, n, n, sdpAllocator) == NULL)
        return NULL;
    /*lint -e418*/
    strncpy(*x, str, n);
    (*x)[n] = 0;
    return x;
}

RvString* rvStringConstructAndReserve(RvString *x, RvSize_t n, RvAlloc *sdpAllocator)
{
    if(rvStringConstructBuffer(x, 0, n, sdpAllocator) == NULL)
        return NULL;
    **x = 0;
    return x;
}

RvString* rvStringConstructCopy(RvString *d, const RvString *s, RvAlloc *sdpAllocator)
{
    RvStringHeader *header = rvStringGetHeader(s);
    if(header->alloc != sdpAllocator)
        return rvStringConstructN(d, rvStringGetData(s), rvStringGetSize(s), sdpAllocator);
    *d = *s;
    ++header->refs;
    return d;
}

void rvStringDestruct(RvString *x)
{
    if(*x==NULL)
        return;
    if(--rvStringGetHeader(x)->refs == 0)
        rvStringReleaseBuffer(x);
}

RvString* rvStringCopy(RvString *d, const RvString *s)
{
    if(*d != *s)
    {
        RvStringHeader *dstHeader = rvStringGetHeader(d);
        RvStringHeader *srcHeader = rvStringGetHeader(s);
        if(dstHeader->alloc != srcHeader->alloc)
            return rvStringAssignN(d, rvStringGetData(s), rvStringGetSize(s));
        rvStringDestruct(d);
        *d = *s;
        ++srcHeader->refs;
    }
    return d;
}

/* Make macro
RvSize_t rvStringGetSize(const RvString *x)
{
    return rvStringGetHeader(x)->length;
}
*/

/* Make macro
RvSize_t rvStringGetCapacity(const RvString *x)
{
    return rvStringGetHeader(x)->maxlen;
}
*/

RvString* rvStringAssign(RvString *x, const char *str)
{
    if(rvStringIsolate(x, strlen(str), RV_FALSE) == NULL)
        return NULL;
    strcpy(*x, str);
    return x;
}

RvString* rvStringAssignN(RvString *x, const char* str, RvSize_t n)
{
    if(rvStringIsolate(x, n, RV_FALSE) == NULL)
        return NULL;
    strncpy(*x, str, n);
    (*x)[n] = 0;
    return x;
}

RvString* rvStringMakeUppercase(RvString *x)
{
    char *p;
    if(rvStringIsolate(x, rvStringGetSize(x), RV_TRUE) == NULL)
        return NULL;
    for(p = *x; *p; p++)
        *p = (char)toupper(*p);
    return x;
}

RvString* rvStringMakeLowercase(RvString *x)
{
    char *p;
    if(rvStringIsolate(x, rvStringGetSize(x), RV_TRUE) == NULL)
        return NULL;
    for(p = *x; *p; p++)
        *p = (char)tolower(*p);
    return x;
}

/* Make macro
RvString* rvStringConcatenate(RvString *x, const char *str)
{
    return rvStringConcatenateN(x, str, strlen(str));
}
*/

RvString* rvStringConcatenateN(RvString *x, const char* str, RvSize_t n)
{
    RvSize_t len = rvStringGetSize(x);
    if(rvStringIsolate(x, len + n, RV_TRUE) == NULL)
        return NULL;
    strncpy(*x + len, str, n);
    (*x)[len + n] = 0;
    return x;
}

/* Make macro
RvString* rvStringConcatenateString(RvString *x, const RvString *s)
{
    return rvStringConcatenateN(x, rvStringGetData(s), rvStringGetSize(s));
}
*/


RvString* rvStringPushBack(RvString *x, char c)
{
    RvSize_t len = rvStringGetSize(x);
    if(rvStringIsolate(x, len + 1, RV_TRUE) == NULL)
        return NULL;
    (*x)[len] = c;
    (*x)[len + 1] = 0;
    return x;
}

/* Make macro
const char* rvStringGetData(const RvString *x)
{
    return *x;
}
*/

RvBool rvStringEqual(const RvString *a, const RvString *b)
{
    return rvStringGetSize(a) == rvStringGetSize(b) &&
        !strcmp(rvStringGetData(a), rvStringGetData(b));
}

RvBool rvStringEqualIgnoreCase(const RvString *a, const RvString *b)
{
    return rvStringGetSize(a) == rvStringGetSize(b) &&
        !rvStrIcmp(rvStringGetData(a), rvStringGetData(b));
}

RvBool rvStringLess(const RvString *a, const RvString *b)
{
    return strcmp(rvStringGetData(a), rvStringGetData(b)) < 0;
}

RvBool rvStringLessIgnoreCase(const RvString *a, const RvString *b)
{
    return rvStrIcmp(rvStringGetData(a), rvStringGetData(b)) < 0;
}

RvAlloc *rvStringGetAllocator(const RvString *x)
{
    return rvStringGetHeader(x)->alloc;
}

RvSize_t rvStringHash(const RvString* x)
{
    RvSize_t hash = 0;
    const char* dataPtr;

    for(dataPtr = rvStringGetData(x); *dataPtr != '\0'; ++dataPtr)
        hash = 5 * hash + *dataPtr;

    return hash;
}

RvSize_t rvStringHashIgnoreCase(const RvString* x)
{
    RvSize_t hash = 0;
    const char* dataPtr;

    for(dataPtr = rvStringGetData(x); *dataPtr != '\0'; ++dataPtr)
        hash = 5 * hash + tolower(*dataPtr);

    return hash;
}


rvSdpDefineVector(RvString)

void rvStringListAdd(RvStringList *x, const char *str)
{
    RvString s;
	if(rvStringConstructA(&s, str, rvSdpVectorGetAllocator(x))!=NULL) {
		rvSdpVectorPushBack(RvString)(x, &s);
        rvStringDestruct(&s);
    }
}

void rvStringListAddN(RvStringList *x, const char *str, RvSize_t n)
{
    RvString s;
	if(rvStringConstructN(&s, str, n, rvSdpVectorGetAllocator(x))!=NULL) {
		rvSdpVectorPushBack(RvString)(x, &s);
        rvStringDestruct(&s);
    }
}

const char* rvStringListGetElem(const RvStringList *x, RvSize_t idx)
{
	return rvStringGetData(rvSdpVectorAt(x, idx));
}

RvString* rvStringListGetString(const RvStringList *x, RvSize_t idx)
{
	return rvSdpVectorAt(x, idx);
}
