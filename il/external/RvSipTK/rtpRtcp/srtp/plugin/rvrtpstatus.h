/************************************************************************
 File Name	   : rvrtpstatus.h
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
#if !defined(RV_RTPSTATUS_H)
#define RV_RTPSTATUS_H

#include "rvtypes.h"
#include "rvrtpconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/*$
{type:
	{name: RvRtpStatus}
	{superpackage: RvRtp}
	{include: rvrtpstatus.h}
	{description:	
		{p: This is an enumeration of status codes used by the RvRtp package.}
	}
	{enumeration:
		{value: {n: RV_RTPSTATUS_Succeeded} {d: The operation succeeded.}}
		{value: {n: RV_RTPSTATUS_MalformedPacket} {d: The received packet was malformed.}}
		{value: {n: RV_RTPSTATUS_SourceInProbation} {d: The received packet's source is still in probation.}}
		{value: {n: RV_RTPSTATUS_LoopedBackPacket} {d: The received packet is one of our own looped backed. This can occur
			on systems that don't allow multicast loop-back to be turned off, such as Microsoft Windows operating
			systems.}}
		{value: {n: RV_RTPSTATUS_OutOfMemory} {d: The operation failed because it could not acquire the needed memory.}}
		{value: {n: RV_RTPSTATUS_BadRtpPortValue} {d: The supplied ports are illegal.}}
		{value: {n: RV_RTPSTATUS_UnableToOpenRtpSocket} {d: The RTP socket could not be opened.}}
		{value: {n: RV_RTPSTATUS_UnableToOpenRtcpSocket} {d: The RTCP socket could not be opened.}}
		{value: {n: RV_RTPSTATUS_SessionIsClosed} {d: The operation could not be executed because the session is closed.}}
		{value: {n: RV_RTPSTATUS_SessionIsOpen} {d: The operation could not be executed because the session is open.}}
		{value: {n: RV_RTPSTATUS_NotEnoughRoomInDataBuffer} {d: The operation could not be executed because the data buffer
			is not large enough to hold the required data.	This can occur if the front or the back of the data buffer
			runs out of room.}}
		{value: {n: RV_RTPSTATUS_Failed} {d:General purpose failure.}}
		{value: {n: RV_RTPSTATUS_NotSupported} {d:Request not supported.}}
		{value: {n: RV_RTPSTATUS_SourceDeleted} {d:Source has been deleted.}}
		{value: {n: RV_RTPSTATUS_UnknownSource} {d:Requested source does not exist.}}
		{value: {n: RV_RTPSTATUS_SourceExists} {d:Attempted to create a source that already exists.}}
		{value: {n: RV_RTPSTATUS_SourceCollision} {d:Source collision detected.}}
		{value: {n: RV_RTPSTATUS_SourceCsrcCollision} {d:Contributing source collision detected.}}
		{value: {n: RV_RTPSTATUS_SourceRemoteCollision} {d:Remote source collision detected.}}
		{value: {n: RV_RTPSTATUS_SourceLoop} {d:Source loop detected.}}
		{value: {n: RV_RTPSTATUS_SourceCsrcLoop} {d:Contributing source loop detected.}}
		{value: {n: RV_RTPSTATUS_SourceRemoteLoop} {d:Remote source loop detected.}}
		{value: {n: RV_RTPSTATUS_SourceConflicts} {d:Source conflict occurring.}}
		{value: {n: RV_RTPSTATUS_SourceCsrcConflicts} {d:Contributing source conflict occurring.}}
		{value: {n: RV_RTPSTATUS_NullPointer} {d:Required object parameter was NULL.}}
		{value: {n: RV_RTPSTATUS_BadParameter} {d:Bad parameter value.}}
		{value: {n: RV_RTPSTATUS_NotFound} {d:Item not found.}}
		{value: {n: RV_RTPSTATUS_DuplicatePacket} {d:Duplcate packet received.}}
	}
}
$*/
typedef enum 
{
	RV_RTPSTATUS_Succeeded,				/* The operation completed successfully. */
	RV_RTPSTATUS_MalformedPacket,		/* The received packet was in an invalid format. */   
	RV_RTPSTATUS_SourceInProbation,		/* The received packet is from a source in probation. */
	RV_RTPSTATUS_LoopedBackPacket,		/* The received packet is our own looped back. */
	RV_RTPSTATUS_OutOfMemory,			/* Unable to allocate required memory for operation. */
	RV_RTPSTATUS_BadRtpPortValue,		/* RTP port supplied was odd, RTP ports must be even. */
	RV_RTPSTATUS_UnableToOpenRtpSocket,	/* Failed to open RTP socket. */
	RV_RTPSTATUS_UnableToOpenRtcpSocket, /* Failed to open RTCP socket. */
	RV_RTPSTATUS_SessionIsClosed,		/* Failed because the Session is closed. */
	RV_RTPSTATUS_SessionIsOpen,			/* Failed because the Session is open. */
	RV_RTPSTATUS_NotEnoughRoomInDataBuffer, /* RvDataBuffer not large enough to fit all data */	 
	RV_RTPSTATUS_Failed,				/* The operation failed */
	RV_RTPSTATUS_NotSupported,          /* Request not supported. */
	RV_RTPSTATUS_SourceDeleted,			/* Received data from a deleted source. */
	RV_RTPSTATUS_UnknownSource,         /* Requested source does not exist. */
	RV_RTPSTATUS_SourceExists,			/* Attempted to create source that already exists. */
	RV_RTPSTATUS_SourceCollision,		/* Received data caused a collision with our own SSRC. */
	RV_RTPSTATUS_SourceCsrcCollision,	/* Received data caused a collision with one of our own CSRCs. */
	RV_RTPSTATUS_SourceRemoteCollision, /* Received data from a remote source that collides with another remote source */
	RV_RTPSTATUS_SourceLoop,			/* Received data from previously known conflicting source (probably a loop). */
	RV_RTPSTATUS_SourceCsrcLoop,	    /* Received data from our own CSRC looped back. */
	RV_RTPSTATUS_SourceRemoteLoop,		/* Received data from a remote source that has been looped */
	RV_RTPSTATUS_SourceConflicts,		/* Received data from a conflicting address continues to collide with our own SSRC. */
	RV_RTPSTATUS_SourceCsrcConflicts,	/* Received data from a conflicting address continues to collide with one of our own CSRCs. */
	RV_RTPSTATUS_NullPointer,	        /* Required object parameter was NULL. */
	RV_RTPSTATUS_BadParameter,	        /* Bad parameter value. */
    RV_RTPSTATUS_WrongParameter = RV_RTPSTATUS_BadParameter,
	RV_RTPSTATUS_NotFound,		        /* Item not found. */
	RV_RTPSTATUS_DuplicatePacket        /* Duplcate packet received. */
} RvRtpStatus;

/*$
{function:
    {name: rvRtpStatusToString }
    {class: RvRtpStatus }
    {include: rvrtpstatus.h}
    {description:
        {p: This method copies a text version of the status into a 
            buffer.}
    }
    {proto: RvInt rvRtpStatusToString(const RvRtpStatus* thisPtr, RvChar* buffer);}
    {params:
        {param: {n: thisPtr} {d: The status to convert to a string.}}
        {param: {n: buffer} {d: The buffer to copy the string into.}}
    }
    {returns: The number of characters copied into the buffer.}
}
$*/
#ifdef RV_RTP_INCLUDE_TOSTRING_METHODS
#define RV_RTPSTATUS_MAXSTRINGSIZE 80
RvInt rvRtpStatusToString(RvRtpStatus status, RvChar* buffer);
#endif


#ifdef __cplusplus
}
#endif

#endif
