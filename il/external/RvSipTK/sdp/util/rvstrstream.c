/******************************************************************************
Filename:    rvstrstream.c
Description: Stream string class
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
******************************************************************************
Revision:
Date:   10.5.00
Author: Dan Elbert
******************************************************************************/
#include "rvstrstream.h"

#include "rvstdio.h"

/******************************************************************************/
/*         Null Allocator                                                     */
/******************************************************************************/
static void* rvNullAllocAlloc(void* pool, RvSize_t s) 
{ 
	RV_UNUSED_ARG(pool);
	RV_UNUSED_ARG(s);
	return NULL; 
}
static void rvNullAllocDealloc(void *pool, RvSize_t s, void* x) 
{ 
	RV_UNUSED_ARG(pool);
	RV_UNUSED_ARG(s);
	RV_UNUSED_ARG(x);
	return; 
}
#ifdef _USE_ALLOC_TRACE
 RvAlloc rvNullAlloc = {0, 0, rvNullAllocAlloc, rvNullAllocDealloc, NULL, NULL};
#else
RvAlloc rvNullAlloc = {0, 0, rvNullAllocAlloc, rvNullAllocDealloc};
#endif


/******************************************************************************/
/*    rvStrStreamBuf methods                                                  */
/******************************************************************************/
#define RV_STRSTREAMBUF_MINALLOC 64

void rvStrStreamBufSetP(RvStrStreamBuf * x,char * begin,char * next,char * end) {
    x->obegin = begin;
    x->onext = next;
    x->oend = end;
}

char * rvStrStreamBufReserve(RvStrStreamBuf * x,RvSize_t n) {
    RvSize_t oldlen = x->onext - x->obegin;
	RvSize_t newlen = RvMax(oldlen + RvMax(n,oldlen),RV_STRSTREAMBUF_MINALLOC);
	RvChar*  buf;
	buf = (RvChar*)rvAllocAllocate(x->alloc, newlen);

    if(buf!=NULL ) {
        memcpy((void*)buf,(void*)x->obegin,oldlen);
        rvAllocDeallocate(x->alloc,x->oend-x->obegin,x->obegin);
        rvStrStreamBufSetP(x,buf,buf+oldlen,buf+newlen);
    }
    else
        rvStrStreamBufSetP(x,NULL,NULL,NULL);

    return x->obegin;
}

void rvStrStreamBufPutC(RvStrStreamBuf * x,char c) {
    if(rvStrStreamBufMustGrow(x,1)==RV_TRUE) {
        if(rvStrStreamBufReserve(x,1)==NULL)
            return;
    }
    *(x->onext)=c;
    x->onext++;
}

void rvStrStreamBufPutN(RvStrStreamBuf * x,const char * s,RvSize_t n) {
    if(rvStrStreamBufMustGrow(x,n)==RV_TRUE) {
        if(rvStrStreamBufReserve(x,n)==NULL)
            return;
    }
    memcpy((void*)x->onext,(void*)s,n);
    x->onext+=n;
}

/* Call to disable buffer */
void rvStrStreamBufInvalidate(RvStrStreamBuf * x) {
    rvStrStreamBufDestruct(x);
}

/* Constructs using an allocator for storage */
void rvStrStreamBufConstructA(RvStrStreamBuf * x,RvSize_t n,RvAlloc * a) {
	RvChar* buf;
	x->alloc = a;
	/* Initialize always with minimal size if size not set */
	n = n==0 ? RV_STRSTREAMBUF_MINALLOC : n;

	buf = (RvChar*)rvAllocAllocate(x->alloc, n);
	if(buf != NULL)
        rvStrStreamBufSetP(x,buf,buf,buf+n);
    else
        rvStrStreamBufSetP(x,NULL,NULL,NULL);
}

/* Constructs using an external buffer for storage */
void rvStrStreamBufConstructEB(RvStrStreamBuf * x,RvSize_t n,char * buf) {
    x->alloc = &rvNullAlloc;
    rvStrStreamBufSetP(x,buf,buf,buf+n);
}

/* Destructor */
void rvStrStreamBufDestruct(RvStrStreamBuf * x) {
    if(rvStrStreamBufIsValid(x)) {
        rvAllocDeallocate(x->alloc,x->oend-x->obegin,x->obegin);
        rvStrStreamBufSetP(x,NULL,NULL,NULL);
    }
}

void rvStrStreamBufSeekOffCur(RvStrStreamBuf * x,int off) {
    /* If fails, make the buffer invalid */
    /* Maybe should grow the stream ? */
    if(x->onext+off>x->oend || x->onext+off<x->obegin)
        rvStrStreamBufSetP(x,NULL,NULL,NULL);
    else
        x->onext+=off;
}

RvSize_t rvStrStreamBufTellPos(RvStrStreamBuf * x) {
    return (x->onext - x->obegin);
}

void rvStrStreamBufSeekPos(RvStrStreamBuf * x,RvSize_t pos) {
    x->onext = x->obegin + pos;
    if(x->onext > x->oend)
        rvStrStreamBufSetP(x,NULL,NULL,NULL);
}


char * rvStrStreamBufGetStr(RvStrStreamBuf * x) {
    return x->obegin;
}

/******************************************************************************/
/*    rvStrStream methods                                                     */
/******************************************************************************/
void rvStrStreamConstruct(RvStrStream * x,RvSize_t size,RvAlloc * a) {
    rvStrStreamBufConstructA(&x->sbuf,size,a);
    if(rvStrStreamBufIsValid(&x->sbuf))
        x->status = RV_OK;
    else
        x->status = RV_STRSTREAM_ERROR_ALLOC;
}

void rvStrStreamConstructBuf(RvStrStream * x,RvSize_t size,char * buf) {
    rvStrStreamBufConstructEB(&x->sbuf,size,buf);
    if(rvStrStreamBufIsValid(&x->sbuf))
        x->status = RV_OK;
    else
        x->status = RV_STRSTREAM_ERROR_ALLOC;
}

void rvStrStreamDestruct(RvStrStream * x) {
    rvStrStreamBufDestruct(&x->sbuf);
}

void rvStrStreamSetUserData(RvStrStream * x,void * data) {
    x->data = data;
}

void * rvStrStreamGetUserData(RvStrStream * x) {
    return x->data;
}

void rvStrStreamSetErrorStatus(RvStrStream * x,RvStatus status) {
    x->status = status;
    rvStrStreamBufInvalidate(&x->sbuf);
}

RvStatus rvStrStreamGetStatus(RvStrStream * x) {
    if(x->status==RV_OK) {
        if(!rvStrStreamBufIsValid(&x->sbuf))
            x->status=RV_STRSTREAM_ERROR_ALLOC;
    }
    return x->status;
}

char* rvStrStreamGetStr(RvStrStream * x) {
    return rvStrStreamBufGetStr(&x->sbuf);
}

/* The actual stream size */
RvSize_t rvStrStreamGetSize(RvStrStream * x) {
    return rvStrStreamBufTellPos(&x->sbuf);
}

RvAlloc* rvStrStreamGetAllocator(RvStrStream * x) {
    return x->sbuf.alloc;
}

void rvStrStreamPut(RvStrStream * x,char c) {
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutC(&x->sbuf,c);
}

void rvStrStreamWriteStrN(RvStrStream * x,const char * s,RvSize_t len) {
	RvSize_t len_ = RvMin(len,strlen(s));
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf,s,len_);
}

void rvStrStreamWriteStr(RvStrStream * x,const char * s) {
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf,s,strlen(s));
}

void rvStrStreamWriteString(RvStrStream * x,const RvString * s) {
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf,rvStringGetData(s),rvStringGetSize(s));
}

void rvStrStreamWriteMem(RvStrStream * x,const char * s,RvSize_t len) {
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf,s,len);
}

void rvStrStreamWriteUInt(RvStrStream *x, RvUint32 i)
{
    char buf[3 * sizeof(i)];
    char *end = buf + sizeof(buf), *cp = end;
    do {
        *--cp = '0' + (i % 10);
        i /= 10;
    } while(i);
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf, cp, end - cp);
}

void rvStrStreamWriteUIntW(RvStrStream *x, RvUint32 i, RvSize_t width, char fill)
{
    char buf[3 * sizeof(i)];
    char *end = buf + sizeof(buf), *cp = end;
    int pad;
    do {
        *--cp = '0' + (i % 10);
        i /= 10;
    } while(i);
    pad = width - (end - cp);
    while(pad-- > 0 && rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutC(&x->sbuf, fill);
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf, cp, end - cp);
}
/* SPIRENT_BEGIN */
void rvStrStreamWriteInt(RvStrStream *x, RvInt32 i)
{
	RvUint32 abs;

	if(i >= 0)
		abs = (RvUint32)i;
	else
	{
		rvStrStreamPut(x, '-');
		abs = (RvUint32)(-i);
	}

	rvStrStreamWriteUInt(x, abs);
}
/* SPIRENT_END */
void rvStrStreamWriteHexInt(RvStrStream *x, RvUint32 i)
{
    char buf[2 * sizeof(i)];
    char *end = buf + sizeof(buf), *cp = end;
    do {
        *--cp = "0123456789abcdef"[i & 15];
        i >>= 4;
    } while(i);

    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf, cp, end - cp);
}

void rvStrStreamWriteHexIntW(RvStrStream *x, RvUint32 i, RvSize_t width, char fill)
{
    char buf[2 * sizeof(i)];
    char *end = buf + sizeof(buf), *cp = end;
    int pad;
    do {
        *--cp = "0123456789abcdef"[i & 15];
        i >>= 4;
    } while(i);
    pad = width - (end - cp);
    while(pad-- > 0 && rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutC(&x->sbuf, fill);
    if(rvStrStreamBufIsValid(&x->sbuf))
        rvStrStreamBufPutN(&x->sbuf, cp, end - cp);
}

void rvStrStreamEndl(RvStrStream * x) {
    if(rvStrStreamBufIsValid(&x->sbuf)) {
#ifndef RV_STRSTRING_DOSENDLINE
        rvStrStreamBufPutN(&x->sbuf,"\r\n",2);
#else
        rvStrStreamBufPutN(&x->sbuf,"\n",1);
#endif
    }
    return;
}

void rvStrStreamEnds(RvStrStream * x) {
	if(rvStrStreamBufIsValid(&x->sbuf)) {
		rvStrStreamBufPutC(&x->sbuf,'\0');
	}
}
