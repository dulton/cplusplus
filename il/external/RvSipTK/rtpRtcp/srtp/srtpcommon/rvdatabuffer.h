/************************************************************************
 File Name	   : rvdatabuffer.h
 Description   :
*************************************************************************
 Copyright (c)	2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#if !defined(RVDATABUFFER_H)
#define RVDATABUFFER_H

#include "rvtypes.h"
#include "rvmemory.h"
#include "rvnet2host.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************
*	RvDataBuffer 
******************************************************************/
/*$
{type:
	{name: RvDataBuffer}
	{superpackage: Util}	
	{include: rvdatabuffer.h}	
	{description:	
		{p: This class represents a network ordered data buffer. 
			The buffer consists of the physical buffer and the data
			located within this buffer. The data length can grow/shrink
			in size from both the front and the back within the limits
			of the buffer.}
		{p: Read functions pull data off the front of the data section of
			the buffer.}
		{p: Write functions push data onto the front of the data section of
			the buffer.}
		{p: ReadBack functions pull data off the back of the data section of
			the buffer.}
		{p: WriteBack functions push data onto the back of the data section of
			the buffer.}
		{p: ReadAt functions read data from an offset into the data section of 
			the buffer. This does not affect the size of the data section.}
		{p: WriteAt functions write data to an offset into the data section of 
			the buffer. This does not affect the size of the data section.}
		{p: All of these functions translate from/to network/host order as 
			requires. The data in the buffer is in network order. All functions 
			return or take host order data.}
	}
}
$*/


/**************************************************************************
 Buffer organization:

 buffer 			data					dataEnd 		  bufferEnd
   |				 |						   |				 |
   V				 V						   V				 V
   +-----------------+-------------------------+-----------------+
   |   Free Space	 |			Data		   |	Free Space	 |
   +-----------------+-------------------------+-----------------+
  
   |<------------------------ Capacity ------------------------->| 

   |<-- Available -->|<------- Length -------->|<-- Available -->|
		  Front 									  Back
		Capacity									Capacity

  NOTES:
  o Read/Writes occur at the "data" pointer. A write moves the pointer towards
	the "buffer" pointer then writes the data in network order. A read gets the 
	data at the "data" pointer and converts it to host order, then moves the 
	"data" pointer towards the "dataEnd" pointer. 
  o Rewind adds bytes to Data without writing by moving the "data" pointer towards 
	the "buffer" pointer.
  o Skip removes bytes from Data without reading by moving the "data" pointer 
	towards the "dataEnd" pointer.
  o RewindBack adds bytes to Data without writing by moving the "dataEnd" pointer 
	towards the "bufferEnd" pointer.
  o SkipBack removes bytes from Data without reading by moving the "dataEnd" pointer 
	towards the "data" pointer.

	
***************************************************************************/   


typedef struct
{
	RvUint8  *buffer;
	RvUint8  *bufferEnd;
	RvUint8  *data;
	RvUint8  *dataEnd;
	RvBool   dealloc;
} RvDataBuffer;

/* public functions */
RvDataBuffer* rvDataBufferConstruct(RvDataBuffer* thisPtr, RvUint32 frontCapacity, RvUint32 backCapacity, RvMemory *regionPtr);
RvDataBuffer* rvDataBufferConstructImport(RvDataBuffer* thisPtr, RvUint8* bufferPtr, RvUint32 bufferLength, RvUint32 dataOffset, RvUint32 dataLength, RvBool dealloc);
void		  rvDataBufferDestruct(RvDataBuffer *thisPtr);

#define  rvDataBufferGetData(thisPtr)					 ((thisPtr)->data)
#define  rvDataBufferSetData(thisPtr, d)				 ((thisPtr)->data = (d))
#define  rvDataBufferGetLength(thisPtr) 				 ((RvUint32)((thisPtr)->dataEnd - (thisPtr)->data))
#define  rvDataBufferSetLength(thisPtr, l)				 ((thisPtr)->data = (thisPtr)->dataEnd - (l))
#define  rvDataBufferGetBuffer(thisPtr) 				 ((thisPtr)->buffer)
#define  rvDataBufferGetCapacity(thisPtr)				 ((RvUint32)((thisPtr)->bufferEnd - (thisPtr)->buffer))
#define  rvDataBufferGetAvailableFrontCapacity(thisPtr)  ((RvUint32)((thisPtr)->data - (thisPtr)->buffer))
#define  rvDataBufferGetAvailableBackCapacity(thisPtr)	 ((RvUint32)((thisPtr)->bufferEnd - (thisPtr)->dataEnd))
#define  rvDataBufferSetDataPosition(thisPtr, h, l) 	 ((thisPtr)->data = (thisPtr)->buffer + (h),(thisPtr)->dataEnd = (thisPtr)->data + (l))
#define  rvDataBufferGetDataEnd(thisPtr)				 ((thisPtr)->dataEnd)
#define  rvDataBufferGetBufferEnd(thisPtr)				 ((thisPtr)->bufferEnd)

/* Front functions */
#define  rvDataBufferRewind(thisPtr, size)			   ((thisPtr)->data -= (size))
#define  rvDataBufferSkip(thisPtr, size)			   ((thisPtr)->data += (size))
#define  rvDataBufferReadUint8(thisPtr, b)			   (*(b) = (RvUint8)(*(thisPtr)->data++))
#define  rvDataBufferWriteUint8(thisPtr, b) 		   (*--(thisPtr)->data = (RvUint8)(b))
#define  rvDataBufferReadUint8Array(thisPtr, bPtr, l)  (memcpy((RvUint8 *)(bPtr),(thisPtr)->data, (l)),(thisPtr)->data += (l))
#define  rvDataBufferWriteUint8Array(thisPtr, bPtr, l) ((thisPtr)->data -= (l),memcpy((thisPtr)->data,(RvUint8 *)(bPtr), (l)))
#define  rvDataBufferFillUint8(thisPtr, b, l)		   ((thisPtr)->data -= (l),memset((thisPtr)->data,(RvUint8)b, (l)))
#define  rvDataBufferReadInt8(thisPtr, b)			   (*(b) = (RvInt8)(*(thisPtr)->data++))
#define  rvDataBufferWriteInt8(thisPtr, b)			   (*--(thisPtr)->data = (RvUint8)(b))

#ifdef ALIGNED_ACCESS
#define  rvDataBufferReadUint16(thisPtr, s) 		   ((*(s) = (RvUint16)RvConvertNetworkToHost16(*((RvUint16 *)(thisPtr)->data))), ((thisPtr)->data += 2))
#define  rvDataBufferWriteUint16(thisPtr, s)		   (*((RvUint16 *)((thisPtr)->data -= 2)) = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadInt16(thisPtr, s)			   ((*(s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16 *)(thisPtr)->data))), ((thisPtr)->data += 2))
#define  rvDataBufferWriteInt16(thisPtr, s) 		   (*((RvUint16 *)((thisPtr)->data -= 2)) = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadUint32(thisPtr, l) 		   ((*(l) = (RvUint32)RvConvertNetworkToHost32(*((RvUint32 *)(thisPtr)->data))), ((thisPtr)->data += 4))
#define  rvDataBufferWriteUint32(thisPtr, l)		   (*((RvUint32 *)((thisPtr)->data -= 4))= RvConvertHostToNetwork32((RvUint32)(l)))
#define  rvDataBufferReadInt32(thisPtr, l)			   ((*(l) = (RvInt32 )RvConvertNetworkToHost32(*((RvUint32 *)(thisPtr)->data))), ((thisPtr)->data += 4))
#define  rvDataBufferWriteInt32(thisPtr, l) 		   (*((RvUint32 *)((thisPtr)->data -= 4)) = RvConvertHostToNetwork32((RvUint32)(l)))
#else

#define RV_FREAD2BYTES(a,b)   ((b)[0] = (*(a)++),(b)[1] = (*(a)++))
#define RV_FWRITE2BYTES(a,b)  ((*--(a))=(b)[1],(*--(a))=(b)[0])
#define RV_FREAD4BYTES(a,b)   ((b)[0]=(*(a)++),(b)[1]=(*(a)++),(b)[2]=(*(a)++),(b)[3]=(*(a)++))
#define RV_FWRITE4BYTES(a,b)  ((*--(a))=(b)[3],(*--(a))=(b)[2],(*--(a))=(b)[1],(*--(a))=(b)[0])

#define  rvDataBufferReadUint16(thisPtr, s) 		   (RV_FREAD2BYTES((thisPtr)->data,(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteUint16(thisPtr, s)		   {RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_FWRITE2BYTES((thisPtr)->data, (RvUint8*)&t);}
#define  rvDataBufferReadInt16(thisPtr, s)			   (RV_FREAD2BYTES((thisPtr)->data,(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteInt16(thisPtr, s) 		   {RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_FWRITE2BYTES((thisPtr)->data, (RvUint8*)&t);}
#define  rvDataBufferReadUint32(thisPtr, l) 		   (RV_FREAD4BYTES((thisPtr)->data,(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteUint32(thisPtr, l)		   {RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_FWRITE4BYTES((thisPtr)->data, (RvUint8*)&t);}
#define  rvDataBufferReadInt32(thisPtr, l)			   (RV_FREAD4BYTES((thisPtr)->data,(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteInt32(thisPtr, l) 		   {RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_FWRITE4BYTES((thisPtr)->data, (RvUint8*)&t);}
#endif

/* Back functions */
#define  rvDataBufferRewindBack(thisPtr, size)			   ((thisPtr)->dataEnd += (size))
#define  rvDataBufferSkipBack(thisPtr, size)			   ((thisPtr)->dataEnd -= (size))
#define  rvDataBufferReadBackUint8(thisPtr, b)			   (*(b) = (RvUint8)(*--(thisPtr)->dataEnd))
#define  rvDataBufferWriteBackUint8(thisPtr, b) 		   (*(thisPtr)->dataEnd++ = (RvUint8)(b))
#define  rvDataBufferReadBackInt8(thisPtr, b)			   (*(b) = (RvInt8)(*--(thisPtr)->dataEnd))
#define  rvDataBufferWriteBackInt8(thisPtr, b)			   (*(thisPtr)->dataEnd++ = (RvUint8)(b))
#define  rvDataBufferReadBackUint8Array(thisPtr, bPtr, l)  ((thisPtr)->dataEnd -= (l),memcpy((RvUint8 *)(bPtr),(thisPtr)->dataEnd, (l)))
#define  rvDataBufferWriteBackUint8Array(thisPtr, bPtr, l) (memcpy((thisPtr)->dataEnd,(RvUint8 *)(bPtr), (l)),(thisPtr)->dataEnd += (l))
#define  rvDataBufferFillBackUint8(thisPtr, b, l)		   (memset((thisPtr)->dataEnd,(RvUint8)b	   , (l)),(thisPtr)->dataEnd += (l))
#ifdef ALIGNED_ACCESS
#define  rvDataBufferReadBackUint16(thisPtr, s) 		   (*(s) = (RvUint16)RvConvertNetworkToHost16(*--((RvUint16 *)(thisPtr)->dataEnd)))
#define  rvDataBufferWriteBackUint16(thisPtr, s)		   (*((RvUint16 *)(thisPtr)->dataEnd)++ = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadBackInt16(thisPtr, s)			   (*(s) = (RvInt16)RvConvertNetworkToHost16(*--((RvUint16 *)(thisPtr)->dataEnd)))
#define  rvDataBufferWriteBackInt16(thisPtr, s) 		   (*((RvUint16 *)(thisPtr)->dataEnd)++ = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadBackUint32(thisPtr, s) 		   (*(s) = (RvUint32)RvConvertNetworkToHost32(*--((RvUint32 *)(thisPtr)->dataEnd)))
#define  rvDataBufferWriteBackUint32(thisPtr, s)		   (*((RvUint32 *)(thisPtr)->dataEnd)++ = RvConvertHostToNetwork32((RvUint32)(s)))
#define  rvDataBufferReadBackInt32(thisPtr, s)			   (*(s) = (RvInt32)RvConvertNetworkToHost32(*--((RvUint32 *)(thisPtr)->dataEnd)))
#define  rvDataBufferWriteBackInt32(thisPtr, s) 		   (*((RvUint32 *)(thisPtr)->dataEnd)++ = RvConvertHostToNetwork32((RvUint32)(s)))
#else

#define RV_RREAD2BYTES(a,b)   ((b)[1] = (*--(a)),(b)[0] = (*--(a)))
#define RV_RWRITE2BYTES(a,b)  ((*(a)++)=(b)[0],(*(a)++)=(b)[1])
#define RV_RREAD4BYTES(a,b)   ((b)[3] = (*--(a)),(b)[2] = (*--(a)),(b)[1] = (*--(a)),(b)[0] = (*--(a)))
#define RV_RWRITE4BYTES(a,b)  ((*(a)++)=(b)[0],(*(a)++)=(b)[1],(*(a)++)=(b)[2],(*(a)++)=(b)[3])

#define  rvDataBufferReadBackUint16(thisPtr, s) 		   (RV_RREAD2BYTES((thisPtr)->dataEnd,(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteBackUint16(thisPtr, s)		   {RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_RWRITE2BYTES((thisPtr)->dataEnd, (RvUint8*)&t);}
#define  rvDataBufferReadBackInt16(thisPtr, s)			   (RV_RREAD2BYTES((thisPtr)->dataEnd,(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteBackInt16(thisPtr, s) 		   {RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_RWRITE2BYTES((thisPtr)->dataEnd, (RvUint8*)&t);}
#define  rvDataBufferReadBackUint32(thisPtr, l) 		   (RV_RREAD4BYTES((thisPtr)->dataEnd,(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteBackUint32(thisPtr, l)		   {RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_RWRITE4BYTES((thisPtr)->dataEnd, (RvUint8*)&t);}
#define  rvDataBufferReadBackInt32(thisPtr, l)			   (RV_RREAD4BYTES((thisPtr)->dataEnd,(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteBackInt32(thisPtr, l) 		   {RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_RWRITE4BYTES((thisPtr)->dataEnd, (RvUint8*)&t);}
#endif

/* Random access functions */
#define  rvDataBufferReadAtUint8(thisPtr, i, b) 	   (*(b) = (RvUint8)(*((thisPtr)->data + (i))))
#define  rvDataBufferWriteAtUint8(thisPtr, i, b)	   (*((thisPtr)->data + (i)) = (RvUint8)(b))
#define  rvDataBufferReadIntAt8(thisPtr, i, b)		   (*(b) = (RvInt8)(*((thisPtr)->data + (i))))
#define  rvDataBufferWriteAtInt8(thisPtr, i, b) 	   (*((thisPtr)->data + (i)) = (RvUint8)(b))
#ifdef ALIGNED_ACCESS
#define  rvDataBufferReadAtUint16(thisPtr, i, s)	   (*(s) = (RvUint16)RvConvertNetworkToHost16(*((RvUint16 *)((thisPtr)->data + (i)))))
#define  rvDataBufferWriteAtUint16(thisPtr, i, s)	   (*((RvUint16 *)((thisPtr)->data + (i))) = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadAtInt16(thisPtr, i, s) 	   (*(s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16 *)((thisPtr)->data + (i)))))
#define  rvDataBufferWriteAtInt16(thisPtr, i, s)	   (*((RvUint16 *)((thisPtr)->data + (i))) = RvConvertHostToNetwork16((RvUint16)(s)))
#define  rvDataBufferReadAtUint32(thisPtr, i, l)	   (*(l) = (RvUint32)RvConvertNetworkToHost32(*((RvUint32 *)((thisPtr)->data + (i)))))
#define  rvDataBufferWriteAtUint32(thisPtr, i, l)	   (*((RvUint32 *)((thisPtr)->data + (i))) = RvConvertHostToNetwork32((RvUint32)(l)))
#define  rvDataBufferReadAtInt32(thisPtr, i, l) 	   (*(l) = (RvInt32 )RvConvertNetworkToHost32(*((RvUint32 *)((thisPtr)->data + (i)))))
#define  rvDataBufferWriteAtInt32(thisPtr, i, l)	   (*((RvUint32 *)((thisPtr)->data + (i))) = RvConvertHostToNetwork32((RvUint32)(l)))
#else

#define RV_READ2BYTES(a,b)	 ((b)[0]=(a)[0],(b)[1]=(a)[1])
#define RV_WRITE2BYTES(a,b)  ((a)[0]=(b)[0],(a)[1]=(b)[1])
#define RV_READ4BYTES(a,b)	 ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define RV_WRITE4BYTES(a,b)  ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define  rvDataBufferReadAtUint16(thisPtr, i, s)			(RV_READ2BYTES((thisPtr)->data + (i),(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtUint16(thisPtr, i, s)			{RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_WRITE2BYTES((thisPtr)->data + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt16(thisPtr, i, s) 			(RV_READ2BYTES((thisPtr)->data + (i),(RvUint8*)(s)),*((RvUint16*)s) = RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtInt16(thisPtr, i, s)			{RvUint16 t = RvConvertHostToNetwork16((RvUint16)(s)); RV_WRITE2BYTES((thisPtr)->data + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtUint32(thisPtr, i, l)			(RV_READ4BYTES((thisPtr)->data + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtUint32(thisPtr, i, l)			{RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_WRITE4BYTES((thisPtr)->data + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt32(thisPtr, i, l) 			(RV_READ4BYTES((thisPtr)->data + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtInt32(thisPtr, i, l)			{RvUint32 t = RvConvertHostToNetwork32((RvUint32)(l)); RV_WRITE4BYTES((thisPtr)->data + (i), (RvUint8*)&t);}
#endif

#ifdef _DEBUG
int rvDataBufferToString(const RvDataBuffer *thisPtr, 
						 RvChar *buffer, RvSize_t bufferSize,
						 const RvChar *prefix);
#endif

#ifdef __cplusplus
}
#endif

#endif
