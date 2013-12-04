/************************************************************************
 File Name     : rvdatabuffer.c
 Description   :
*************************************************************************
 Copyright (c)  2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#include "rvdatabuffer.h"

#ifdef _DEBUG
#include "stdio.h"
#endif


/******************************************************************
*   RvDataBuffer 
******************************************************************/

/*$
{function:
	{name:    rvDataBufferConstruct }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: Constructs a RvDataBuffer object with the specified capacity using 
            the supplied allocator.}
	}
	{proto: RvDataBuffer* rvDataBufferConstruct(RvDataBuffer* thisPtr, RvUint32 frontCapacity, RvUint32 backCapacity, RvMemory *regionPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
		{param: {n:frontCapacity} {d:The maximum amount of data that can be added to the front of the buffer.}}
		{param: {n:backCapacity}  {d:The maximum amount of data that can be added to the back of the buffer.}}
		{param: {n:allocatorPtr}  {d:A pointer to the allocator to use for the buffer.}}
	}
	{returns: RvDataBuffer *, A pointer to the RvDataBuffer object, if the object 
              constructs sucessfully, NULL otherwise. }
}
$*/
RvDataBuffer* rvDataBufferConstruct(RvDataBuffer* thisPtr, RvUint32 frontCapacity, RvUint32 backCapacity, RvMemory *regionPtr)
{
	RvStatus status;

	status = RvMemoryAlloc(regionPtr, frontCapacity + backCapacity, NULL, (void **)&thisPtr->buffer);

    if(status != RV_OK)
        return 0;

	thisPtr->dealloc      = RV_TRUE;
    thisPtr->bufferEnd    = thisPtr->buffer + frontCapacity + backCapacity;
    thisPtr->data         = thisPtr->bufferEnd - backCapacity;
    thisPtr->dataEnd      = thisPtr->bufferEnd - backCapacity;

    return thisPtr;
}


/*$
{function:
	{name:    rvDataBufferConstructImport }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: Constructs a RvDataBuffer object with a user supplied data buffer.}
	}
	{proto: RvDataBuffer* rvDataBufferConstructImport(RvDataBuffer* thisPtr, RvUint8* bufferPtr, RvUint32 bufferLength, RvUint32 dataOffset, RvUint32 dataLength, RvBool dealloc);}
    {params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
		{param: {n:bufferPtr}     {d:The buffer to use in the RvDataBuffer for storing data.}}
		{param: {n:bufferLength}  {d:The length of the buffer in bytes.}}
		{param: {n:dataOffset}    {d:The offset to the data from the pubberPtr in bytes.}}
		{param: {n:dataLength}    {d:The length of the data in bytes.}}
		{param: {n:allocatorPtr}  {d:A pointer to the allocator to use to deallocate the buffer on destruction. If 
                                     a NULL pointer is suppled the RvDataBuffer will not de-allocate the buffer
                                     on destruction.}}
	}
	{returns: RvDataBuffer *, A pointer to the RvDataBuffer object, if the object 
              constructs sucessfully, NULL otherwise. }
}
$*/
RvDataBuffer* rvDataBufferConstructImport(RvDataBuffer* thisPtr, RvUint8* bufferPtr, RvUint32 bufferLength, RvUint32 dataOffset, RvUint32 dataLength, RvBool dealloc)
{
    thisPtr->dealloc      = dealloc;
    thisPtr->buffer       = bufferPtr;
    thisPtr->bufferEnd    = thisPtr->buffer + bufferLength;
    thisPtr->data         = thisPtr->buffer + dataOffset;
    thisPtr->dataEnd      = thisPtr->buffer + dataOffset + dataLength;

    return thisPtr;
}

/*$
{function:
	{name:    rvDataBufferDestruct }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: Destructs a RvDataBuffer object. This frees the buffer the object is 
            using to store data in.}
	}
	{proto: void rvDataBufferDestruct(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}      {d:The RvDataBuffer object.}}
	}
}
$*/
void rvDataBufferDestruct(RvDataBuffer *thisPtr)
{
    if(thisPtr->dealloc == RV_TRUE)
        RvMemoryFree(thisPtr->buffer, NULL);
    thisPtr->buffer    = 0;
    thisPtr->bufferEnd = 0;
    thisPtr->data      = 0;
    thisPtr->dataEnd   = 0;
}


#ifdef _DEBUG
#include <rvansi.h>

/*$
{function:
	{name:    rvDataBufferToString }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: Puts the current state of the buffer into a string. The state of
            the pointers are outputted, as well as the buffer in hex and ASCII
            format.}
	}
	{proto: int rvDataBufferToString(const RvDataBuffer* thisPtr, RvChar* buffer, RvSize_t bufferSize, const RvChar* prefix);}
	{params:
		{param: {n:thisPtr}     {d:The RvDataBuffer object.}}
		{param: {n:buffer}      {d:The buffer to write the information to.}}
		{param: {n:bufferSize}  {d:The size of the buffer.}}
		{param: {n:prefix}      {d:A string to append to the begining of any new
                                   line.}}
	}
}
$*/
int rvDataBufferToString(const RvDataBuffer *thisPtr, 
                         RvChar *buffer, RvSize_t bufferSize,
                         const RvChar *prefix)
{
    RvInt offset = RvIntConst(0);
    RvUint8 *dataPtr = thisPtr->data;
    RvUint8 *linePtr;
    RvInt      x;

    RV_UNUSED_ARG(bufferSize);
    offset += RvSprintf(&buffer[offset],"%sBuffer     : 0x%08X  Capacity                 : %ld\n",prefix,thisPtr->buffer,rvDataBufferGetCapacity(thisPtr));
    offset += RvSprintf(&buffer[offset],"%sBuffer End : 0x%08X  Length                   : %ld\n",prefix,thisPtr->bufferEnd,rvDataBufferGetLength(thisPtr));
    offset += RvSprintf(&buffer[offset],"%sData       : 0x%08X  Available Front Capacity : %ld\n",prefix,thisPtr->data,(thisPtr->data - thisPtr->buffer));
    offset += RvSprintf(&buffer[offset],"%sData End   : 0x%08X  Available Back Capacity  : %ld\n",prefix,thisPtr->dataEnd,(thisPtr->bufferEnd - thisPtr->dataEnd));

    while(dataPtr != thisPtr->dataEnd)
    {
        linePtr = dataPtr;

        offset += RvSprintf(&buffer[offset],"%s  ",prefix);

        /* Print Binary */
        for(x = 0; x < 16; x++)
        {
            offset += RvSprintf(&buffer[offset],"%02X", *dataPtr++);

            if(x%4 == 3)
                offset += RvSprintf(&buffer[offset]," ");

            if(dataPtr == thisPtr->dataEnd)
                break;
        }

        /* Pad out the line */
        for(x++; x < 16; x++)
        {
            if(x%4 == 3)
                offset += RvSprintf(&buffer[offset]," ");

            offset += RvSprintf(&buffer[offset],"  ");
        }
        
        /* Print Seperator */
        offset += RvSprintf(&buffer[offset],"  |  ");

        dataPtr = linePtr;

        /* Print ASCII */
        for(x = 0; x < 16; x++)
        {
            if(isprint(*dataPtr))
                offset += RvSprintf(&buffer[offset],"%c", *dataPtr);
            else
                offset += RvSprintf(&buffer[offset],".", *dataPtr);

            dataPtr++;

            if(dataPtr == thisPtr->dataEnd)
                break;
        }
    
        offset += RvSprintf(&buffer[offset],"\n");
    }

    return offset;
}
#endif

/*$
{function:
	{name:    rvDataBufferGetData }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns a pointer to the begining of the buffer's data.}
	}
	{proto: RvUint8* rvDataBufferGetData(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: A pointer to the begining of the buffer's data. }
}
$*/

/*$
{function:
	{name:    rvDataBufferGetLength }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns the length of the buffer's data in bytes.}
	}
	{proto: RvUint32 rvDataBufferGetLength(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: The length of the buffer's data in bytes. }
}
$*/


/*$
{function:
	{name:    rvDataBufferSetLength }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method set the length of the buffer's data in bytes. This moves the 
            head of the data to 'length' bytes before the current end of the buffer.
            Caution should be used when using this method to not set the length 
            beyond the front of the buffer.}
	}
	{proto: void rvDataBufferSetLength(RvDataBuffer* thisPtr, RvUint32 length);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
		{param: {n:length}       {d:The RvDataBuffer object.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferGetBuffer }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns a pointer to the begining of the buffer.}
	}
	{proto: RvUint8* rvDataBufferGetBuffer(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: A pointer to the begining of the buffer. }
}
$*/

/*$
{function:
	{name:    rvDataBufferGetCapacity }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns the maximum number of bytes that the
            buffer can hold.}
	}
	{proto: RvUint32 rvDataBufferGetCapacity(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: The maximum number of bytes that the buffer can hold. }
}
$*/
 
/*$
{function:
	{name:    rvDataBufferGetAvailableFrontCapacity }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns the number of bytes still available at the front 
            of the buffer.}
	}
	{proto: RvUint32 rvDataBufferGetAvailableFrontCapacity(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: The number of bytes still available at the front of the buffer.}
}
$*/

/*$
{function:
	{name:    rvDataBufferGetAvailableBackCapacity }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method returns the number of bytes still available at the back 
            of the buffer.}
	}
	{proto: RvUint32 rvDataBufferGetAvailableBackCapacity(RvDataBuffer* thisPtr);}
	{params:
		{param: {n:thisPtr}       {d:The RvDataBuffer object.}}
	}
	{returns: The number of bytes still available at the back of the buffer.}
}
$*/

/*$
{function:
	{name:    rvDataBufferSetDataPosition }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method sets the position of the data within the buffer. This only 
            moves the internal pointers within the buffer, it does not move the data
            values in the buffer. If you have manually inserted data into the buffer 
            using the buffer returned from rvDataBufferGetBuffer, you can use this 
            method to let the RvDataBuffer know were the data has been placed.}
	}
	{proto: void rvDataBufferSetDataPosition(RvDataBuffer* thisPtr, RvUint32 offset, RvUint32 length);}
	{params:
		{param: {n:thisPtr}      {d:The RvDataBuffer object.}}
		{param: {n:offset}       {d:The offset into the buffer, in bytes, where the data begins.}}
		{param: {n:length}       {d:The length of the data in bytes.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferRewind }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method moves the front reading pointer backwords by n bytes to allow
            older data to be read again.}
	}
	{proto: void rvDataBufferRewind(RvDataBuffer* thisPtr, RvUint32 size);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:size}    {d:The number of bytes to rewind.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferSkip }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method moves the front reading pointer forwords by a variable number 
            of bytes without reading the value.}
	}
	{proto: void rvDataBufferSkip(RvDataBuffer* thisPtr, RvUint32 size);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:size}    {d:The number of bytes to skip.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadUint8Array }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an array of RvUint8s from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadUint8Array(RvDataBuffer* thisPtr, RvUint8* dataPtr, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the array to hold the data read from the buffer.}}
		{param: {n:length}  {d:A length of the buffer in bytes.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint8Array }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an array RvUint8s onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteUint8Array(RvDataBuffer* thisPtr, RvUint8* dataPtr, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The RvUint8 buffer to write into the data buffer.}}
		{param: {n:length}  {d:A length of the RvUint8 buffer in bytes.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferFillUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes a variable number of an RvUint8 value onto the front of the data buffer.}
	}
	{proto: void rvDataBufferFillUint8(RvDataBuffer* thisPtr, RvUint8 data, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to fill into the data buffer.}}
		{param: {n:length}  {d:The number of RvUint8s to fill into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint8 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadUint8(RvDataBuffer* thisPtr, RvUint8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint8 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteUint8(RvDataBuffer* thisPtr, RvUint8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadInt8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt8 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadInt8(RvDataBuffer* thisPtr, RvInt8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt8 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteInt8(RvDataBuffer* thisPtr, RvInt8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint16 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadUint16(RvDataBuffer* thisPtr, RvUint16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint16 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteUint16(RvDataBuffer* thisPtr, RvUint16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadInt16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt16 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadInt16(RvDataBuffer* thisPtr, RvInt16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt16 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteInt16(RvDataBuffer* thisPtr, RvInt16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint32 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadUint32(RvDataBuffer* thisPtr, RvUint32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint32 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteUint32(RvDataBuffer* thisPtr, RvUint32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadInt32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt32 from the front of the data buffer.}
	}
	{proto: void rvDataBufferReadInt32(RvDataBuffer* thisPtr, RvInt32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt32 onto the front of the data buffer.}
	}
	{proto: void rvDataBufferWriteInt32(RvDataBuffer* thisPtr, RvInt32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferRewindBack }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method moves the back reading pointer backwords by n bytes to allow
            older data to be read again.}
	}
	{proto: void rvDataBufferRewindBack(RvDataBuffer* thisPtr, RvUint32 size);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:size}    {d:The number of bytes to rewind.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferSkipBack }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method moves the back reading pointer forwords by a variable number 
            of bytes without reading the value.}
	}
	{proto: void rvDataBufferSkipBack(RvDataBuffer* thisPtr, RvUint32 size);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:size}    {d:The number of bytes to skip.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint8 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackUint8(RvDataBuffer* thisPtr, RvUint8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint8 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackUint8(RvDataBuffer* thisPtr, RvUint8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackUint8Array }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an array of RvUint8s from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackUint8Array(RvDataBuffer* thisPtr, RvUint8* dataPtr, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the array to hold the data read from the buffer.}}
		{param: {n:length}  {d:A length of the buffer in bytes.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint8Array }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an array RvUint8s onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackUint8Array(RvDataBuffer* thisPtr, RvUint8* dataPtr, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The RvUint8 buffer to write into the data buffer.}}
		{param: {n:length}  {d:A length of the RvUint8 buffer in bytes.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferFillBackUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes a variable number of an RvUint8 value onto the back of the data buffer.}
	}
	{proto: void rvDataBufferFillBackUint8(RvDataBuffer* thisPtr, RvUint8 data, RvUint32 length);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to fill into the data buffer.}}
		{param: {n:length}  {d:The number of RvUint8s to fill into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackInt8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt8 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackInt8(RvDataBuffer* thisPtr, RvInt8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt8 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackInt8(RvDataBuffer* thisPtr, RvInt8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint16 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackUint16(RvDataBuffer* thisPtr, RvUint16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint16 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackUint16(RvDataBuffer* thisPtr, RvUint16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackInt16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt16 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackInt16(RvDataBuffer* thisPtr, RvInt16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt16 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackInt16(RvDataBuffer* thisPtr, RvInt16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvUint32 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackUint32(RvDataBuffer* thisPtr, RvUint32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvUint32 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackUint32(RvDataBuffer* thisPtr, RvUint32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadBackInt32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads and pops an RvInt32 from the back of the data buffer.}
	}
	{proto: void rvDataBufferReadBackInt32(RvDataBuffer* thisPtr, RvInt32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteBackInt32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method pushes an RvInt32 onto the back of the data buffer.}
	}
	{proto: void rvDataBufferWriteBackInt32(RvDataBuffer* thisPtr, RvInt32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadAtUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvUint8 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtUint8(RvDataBuffer* thisPtr, RvUint32 index, RvUint8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtUint8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvUint8 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtUint8(RvDataBuffer* thisPtr, RvUint32 index, RvUint8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadAtInt8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvInt8 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtInt8(RvDataBuffer* thisPtr, RvUint32 index, RvInt8* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtInt8 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvInt8 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtInt8(RvDataBuffer* thisPtr, RvUint32 index, RvInt8 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadAtUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvUint16 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtUint16(RvDataBuffer* thisPtr, RvUint32 index, RvUint16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtUint16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvUint16 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtUint16(RvDataBuffer* thisPtr, RvUint32 index, RvUint16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadAtInt16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvInt16 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtInt16(RvDataBuffer* thisPtr, RvUint32 index, RvInt16* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtInt16 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvInt16 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtInt16(RvDataBuffer* thisPtr, RvUint32 index, RvInt16 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/


/*$
{function:
	{name:    rvDataBufferReadAtUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvUint32 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtUint32(RvDataBuffer* thisPtr, RvUint32 index, RvUint32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtUint32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvUint32 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtUint32(RvDataBuffer* thisPtr, RvUint32 index, RvUint32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferReadAtInt32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method reads a RvInt32 from the data buffer from a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferReadAtInt32(RvDataBuffer* thisPtr, RvUint32 index, RvInt32* dataPtr);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to read the value from.}}
		{param: {n:dataPtr} {d:A pointer the the variable to hold the data read from the buffer.}}
	}
}
$*/

/*$
{function:
	{name:    rvDataBufferWriteAtInt32 }
	{class:   RvDataBuffer}
	{include: rvdatabuffer.h}
	{description:
		{p: This method writes a RvInt32 into the data buffer at a specified
            byte offset. This method does not affect the buffer's front or back 
            data pointers.}
	}
	{proto: void rvDataBufferWriteAtInt32(RvDataBuffer* thisPtr, RvUint32 index, RvInt32 data);}
	{params:
		{param: {n:thisPtr} {d:The RvDataBuffer object.}}
		{param: {n:index}   {d:The byte offset to write the value to.}}
		{param: {n:data}    {d:The value to write into the buffer.}}
	}
}
$*/
