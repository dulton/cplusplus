/***********************************************************************
Filename   : rvbuffer.c
Description: rvbuffer
************************************************************************
        Copyright (c) 2005 RADVISION Inc. and RADVISION Ltd.
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

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rvbuffer.h"
#include "rverror.h"
#include "rvstdio.h" /* memset on linux */

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define BIT_2_BYTE(bitNum)        ((bitNum) >> 3)

/*-----------------------------------------------------------------------*/
/*                   external functions                                  */
/*-----------------------------------------------------------------------*/
/*************************************************************************
* RvBufferShiftLeft
* ------------------------------------------------------------------------
* General: shifts left a buffer which is larger than 32 bits
*
* Return Value: RvUint8*
* ------------------------------------------------------------------------
* Arguments:
* Input:    *inputBuf - a pointer to the input buffer
*           shift - the amount to shift
*           bufLen - size of buffer
*
* Output    *outputBuf - a pointer to the output buffer
*************************************************************************/
RvUint8 *RvBufferShiftLeft
(
    RvUint8  *inputBuf, 
    RvUint8  *outputBuf, 
    RvSize_t shift, 
    RvSize_t bufLen
)
{
    RvUint8  leftMostByte = 0;
    RvUint16 tempWord;
    RvSize_t byteShift;
    RvSize_t bitShift;
    RvUint8  *copyTo;
    RvUint8  *copyFrom;
    
    if ((NULL == inputBuf) || (NULL == outputBuf))
		return NULL;
    
    if(outputBuf != inputBuf)
		memcpy(outputBuf, inputBuf, bufLen);

    /*first devide the shifting into 8x + y
      the shifting will be done first bytes then bits*/
    byteShift = BIT_2_BYTE(shift);
    bitShift = shift & 0x7;
    
    if (byteShift > bufLen)
        return NULL;
    
    if (0 != byteShift)
    {
        /*shift bytes - shifting will be done 1 byte at a time*/
        copyTo = ((outputBuf + bufLen) - 1);
        copyFrom = (copyTo - byteShift);
        
        while (copyFrom >= outputBuf)
        {
            *copyTo = *copyFrom;
            copyFrom--;
            copyTo--;
        }
        /*clear the rest of the buffer*/
        memset(outputBuf, 0, byteShift);
    }

    /*now do the shifting when less then a byte*/
    if (0 != bitShift)
    {
        copyFrom = outputBuf;

        while (copyFrom < (outputBuf + bufLen))
        {
            tempWord = *copyFrom;
            tempWord <<= bitShift;
            tempWord |= leftMostByte;
            
            memcpy(copyFrom, &tempWord, 1);
            
            leftMostByte = (RvUint8) (tempWord >> 8); /*take the upper byte*/
            copyFrom++;
        }
    }
    
    return outputBuf;
}

/*************************************************************************
* RvBufferShiftRight
* ------------------------------------------------------------------------
* General: shifts right a buffer that is larger than 32 bits
*
* Return Value: RvUint8*
* ------------------------------------------------------------------------
* Arguments:
* Input:    *inputBuf - a pointer to the input buffer
*           shift - the amount to shift
*           bufLen - size of buffer
*
* Output    *outputBuf - a pointer to the output buffer
*************************************************************************/
RvUint8 *RvBufferShiftRight
(
    RvUint8 *inputBuf, 
    RvUint8 *outputBuf, 
    RvSize_t shift, 
    RvSize_t bufLen
)
{
    RvUint8  newRightMostByte = 0;
    RvUint8  oldRightMostByte = 0;
    RvUint16 tempWord;
    RvSize_t byteShift;
    RvSize_t bitShift;
    RvUint8  *copyTo;
    RvUint8  *copyFrom;
    
    if ((NULL == inputBuf) || (NULL == outputBuf))
		return NULL;

	if(inputBuf != outputBuf)
		memcpy(outputBuf, inputBuf, bufLen);
    
    /*first devide the shifting into 8x + y
      the shifting will be done first bytes then bits*/
    byteShift = BIT_2_BYTE(shift);
    bitShift = shift & 0x7;
    if (byteShift > bufLen)
        return NULL;

    if (0 != byteShift)
    {
        /*shift bytes - shifting will be done 1 byte at a time*/
        copyFrom = outputBuf + byteShift;
        copyTo = outputBuf;
        
        while (copyFrom < (outputBuf + bufLen))
        {
            *copyTo = *copyFrom;
            copyFrom++;
            copyTo++;
        }
        /*clear the rest of the buffer*/
        memset(copyTo, 0, byteShift);
    }
    
    /*now do the shifting when less then a byte*/
    if (0 != bitShift)
    {
        copyFrom = (outputBuf + bufLen) - 1;
        
        while (copyFrom >= outputBuf)
        {
            tempWord = *copyFrom;
            /*move the byte to the "upper" bits - so you wont lose the right bits*/
            tempWord <<= 8;
            tempWord >>= bitShift;
            newRightMostByte = (RvUint8)tempWord;
            tempWord >>= 8;
            tempWord |= oldRightMostByte;
            memcpy(copyFrom, &tempWord, 1);
            oldRightMostByte = newRightMostByte;
            copyFrom--;
        }
    }
    
    return outputBuf;
}
/*************************************************************************
* RvBufferXor
* ------------------------------------------------------------------------
* General: xors buffer that are larger than 32 bits
*          the function will xor bufLen bytes of inputBuf1 and inputBuf2
*          even if they are larger
*          in case either A or B are smaller the remainder will be filled 
*          in with zeros 
*
* Return Value: RvUint8*
* ------------------------------------------------------------------------
* Arguments:
* Input:    *inputBuf1 - a pointer to buffer A
*           *inputBuf2 - a pointer to buffer B
*           bufLen - size of output buffer 
*
* Output    *outputBuf - a pointer to the output buffer
*************************************************************************/
RvUint8 *RvBufferXor
(
    RvUint8  *inputBuf1, 
    RvUint8  *inputBuf2, 
    RvUint8  *outputBuf, 
    RvSize_t bufLen
)
{
    RvUint32 cnt;
    
    if ((NULL == inputBuf1) || (NULL == inputBuf2) || (NULL == outputBuf))
		return NULL;
    
    for (cnt = 0; cnt < bufLen; cnt++)
    {
        outputBuf[cnt] = (RvUint8)((*inputBuf1) ^ (*inputBuf2));
        inputBuf1++;
        inputBuf2++;
    }
    return outputBuf;
}
