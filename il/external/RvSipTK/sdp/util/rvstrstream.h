/******************************************************************************
Filename:    rvstrstream.h
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
#ifndef RV_STRSTREAM_H
#define RV_STRSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rverror.h"

#include "rvstring.h"

/* Fail to allocate enough space for the operation */
#define RV_STRSTREAM_ERROR_ALLOC -1

/******************************************************************************/
/*    rvStrStreamBuf definitions                                              */
/******************************************************************************/
typedef struct {
    RvAlloc *alloc;
    char * obegin;
    char * onext;
    char * oend;
/* SPIRENT_BEGIN */
	int    status;  /* contains RV_STRSTREAMBUFF_STATUS_xxx */
/* SPIRENT_END   */
} RvStrStreamBuf;

#define rvStrStreamBufIsValid(x) ((x)->obegin!=NULL)
#define rvStrStreamBufMustGrow(x,n) ( (x)->onext+n > (x)->oend )

void rvStrStreamBufSetP(RvStrStreamBuf * x,char * begin,char * next,char * end);
char * rvStrStreamBufReserve(RvStrStreamBuf * x,RvSize_t n);
void rvStrStreamBufPutC(RvStrStreamBuf * x,char c);
void rvStrStreamBufPutN(RvStrStreamBuf * x,const char * s,RvSize_t n);
void rvStrStreamBufInvalidate(RvStrStreamBuf * x);
void rvStrStreamBufConstructA(RvStrStreamBuf * x,RvSize_t n,RvAlloc * a);
void rvStrStreamBufConstructEB(RvStrStreamBuf * x,RvSize_t n,char * buf);
/* Destructor */
void rvStrStreamBufDestruct(RvStrStreamBuf * x);

void rvStrStreamBufSeekOffCur(RvStrStreamBuf * x,int off);
RvSize_t rvStrStreamBufTellPos(RvStrStreamBuf * x);
void rvStrStreamBufSeekPos(RvStrStreamBuf * x,RvSize_t pos);
char * rvStrStreamBufGetStr(RvStrStreamBuf * x);

/******************************************************************************/
/*    rvStrStream definitions                                                 */
/******************************************************************************/

typedef struct {
    RvStrStreamBuf sbuf;
    RvStatus status;
    void *   data;
} RvStrStream;

/* Constructors */
void rvStrStreamConstruct(RvStrStream * x,RvSize_t size,RvAlloc * a);
void rvStrStreamConstructBuf(RvStrStream * x,RvSize_t size,char * buf);

/* Destructor */
void rvStrStreamDestruct(RvStrStream * x);

/* Access functions */
void        rvStrStreamSetUserData(RvStrStream * x,void * data);
void *      rvStrStreamGetUserData(RvStrStream * x);
void        rvStrStreamSetErrorStatus(RvStrStream * x,RvStatus status);
RvStatus    rvStrStreamGetStatus(RvStrStream * x);
char*       rvStrStreamGetStr(RvStrStream * x);
RvSize_t      rvStrStreamGetSize(RvStrStream * x);
RvAlloc*    rvStrStreamGetAllocator(RvStrStream * x);

/* Write (stream) functions */
void rvStrStreamPut(RvStrStream * x,char c);
void rvStrStreamWriteStrN(RvStrStream * x,const char * s,RvSize_t len);
void rvStrStreamWriteStr(RvStrStream * x,const char * s);
void rvStrStreamWriteString(RvStrStream * x,const RvString * s);
void rvStrStreamWriteMem(RvStrStream * x,const char * s,RvSize_t len);
void rvStrStreamWriteUInt(RvStrStream * x,RvUint32 i);
void rvStrStreamWriteUIntW(RvStrStream *x, RvUint32 i, RvSize_t width, char fill);

/* SPIRENT_BEGIN */
void rvStrStreamWriteInt(RvStrStream *x, RvInt32 i);
/* SPIRENT_END */

void rvStrStreamWriteHexInt(RvStrStream *x, RvUint32 i);
void rvStrStreamWriteHexIntW(RvStrStream *x, RvUint32 i, RvSize_t width, char fill);
void rvStrStreamEndl(RvStrStream * x);
void rvStrStreamEnds(RvStrStream * x);

/* Stream manipulation functions */
/* Move the current put position by offset off */
#define rvStrStreamSeekOffCur(x,off) rvStrStreamBufSeekOffCur(&(x)->sbuf,(off))
#define rvStrStreamTellPos(x)        rvStrStreamBufTellPos(&(x)->sbuf)
#define rvStrStreamSeekPos(x,pos)    rvStrStreamBufSeekPos(&(x)->sbuf,(pos))
/* SPIRENT_BEGIN */
#define rvStrStreamGetCapacity(x) ((x)->sbuf.oend - (x)->sbuf.obegin)
/* SPIRENT_END */
#ifdef __cplusplus
}
#endif

#endif /* RV_STRSTREAM_H */
