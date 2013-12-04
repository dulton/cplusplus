/***********************************************************************
Filename   : rvbuffer.h
Description:
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
#ifndef RV_BUFFER_H
#define RV_BUFFER_H

#if defined(__cplusplus)
extern "C" {
#endif 

#include "rvtypes.h"

/*$
{package:
	{name: RvBuffer}
	{superpackage: CUtils}
	{include: rvbuffer.h}
	{description:	
		{p: This module provides some utility functions for doing
			operations on memory buffers.}
	}
}
$*/

RvUint8 *RvBufferShiftRight(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t shift, RvSize_t bufLen);
RvUint8 *RvBufferShiftLeft(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t shift, RvSize_t bufLen);
RvUint8 *RvBufferXor(RvUint8 *inputBuf1, RvUint8 *inputBuf2, RvUint8 *outputBuf, RvSize_t bufLen);

/* Function Docs */
/*$
{function:
	{name: RvBufferShiftRight}
	{superpackage: RvBuffer}
	{include: rvbuffer.h}
	{description:
        {p: This function right bit shifts a buffer. If the input and
			output buffer point to the same buffer, the buffer will be
			shifted in place.}
        {p: The outout buffer must be >= the size of the input buffer.}
	}
	{proto: RvUint8 *RvBufferShiftRight(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t shift, RvSize_t bufLen);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: input buffer .}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: output buffer.}}
		{param: {t: RvSize_t} {n: shift} {d: number of bits to shift buffer.}}
		{param: {t: RvSize_t} {n: bufLen} {d: size of the input buffer (in bytes).}}
	}
	{returns: A pointer to the output buffer or NULL if the operation could not be done.}
}
$*/
/*$
{function:
	{name: RvBufferShiftLeft}
	{superpackage: RvBuffer}
	{include: rvbuffer.h}
	{description:
	        {p: This function left bit shifts a buffer. If the input
				and output buffer point to the same buffer, the buffer will
				be shifted in place.}
			{p: The output buffer must be >= the size of the input buffer.}
	}
	{proto: RvUint8 *RvBufferShiftLeft(RvUint8 *inputBuf, RvUint8 *outputBuf, RvSize_t shift, RvSize_t bufLen);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf} {d: input buffer .}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: output buffer.}}
		{param: {t: RvSize_t} {n: shift} {d: number of bits to shift buffer.}}
		{param: {t: RvSize_t} {n: bufLen} {d: size of the input buffer (in bytes).}}
	}
	{returns: A pointer to the output buffer or NULL if the operation could not be done.}
}
$*/
/*$
{function:
	{name: RvBufferXor}
	{superpackage: RvBuffer}
	{include: rvbuffer.h}
	{description:
        {p: This function performs an xor on two buffers. If the output
			buffer points to the same buffer as one of the input buffers,
			then the result will overwrite that buffer.}
        {p: The output buffer must be >= the size of the input buffers.}
	}
	{proto: RvUint8 *RvBufferXor(RvUint8 *inputBuf1, RvUint8 *inputBuf2, RvUint8 *outputBuf, RvSize_t bufLen);}
	{params:
		{param: {t: RvUint8 *} {n: inputBuf1} {d: input buffer 1.}}
		{param: {t: RvUint8 *} {n: inputBuf2} {d: input buffer 2.}}
		{param: {t: RvUint8 *} {n: outputBuf} {d: output buffer.}}
		{param: {t: RvSize_t} {n: bufLen} {d: size of the input buffers (in bytes).}}
	}
	{returns: A pointer to the output buffer or NULL if the operation could not be done.}
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
