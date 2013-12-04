/************************************************************************
 File Name     : rvrtpstatus.c
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
*************************************************************************/
#include "rvrtpstatus.h"

#ifdef RV_RTP_INCLUDE_TOSTRING_METHODS
#include "rvansi.h"

RvInt rvRtpStatusToString(RvRtpStatus status, RvChar* buffer)
{
    RvChar *stringPtr;

    switch(status)
    {
        case RV_RTPSTATUS_Succeeded:             
            stringPtr = "Operation succeeded";break;
        case RV_RTPSTATUS_MalformedPacket:        
            stringPtr = "Malformed packet";break;
        case RV_RTPSTATUS_SourceInProbation:     
            stringPtr = "Packet source is in probation";break;
        case RV_RTPSTATUS_LoopedBackPacket:       
            stringPtr = "Packet is looped back from this stack";break;
        case RV_RTPSTATUS_OutOfMemory:       
            stringPtr = "Memory allocation failed";break;
        case RV_RTPSTATUS_BadRtpPortValue:        
            stringPtr = "RTP port value is invalid";break;
        case RV_RTPSTATUS_UnableToOpenRtpSocket:  
            stringPtr = "Unable to open RTP socket";break;
        case RV_RTPSTATUS_UnableToOpenRtcpSocket: 
            stringPtr = "Unable to open RTCP socket";break;
        case RV_RTPSTATUS_SessionIsClosed:       
            stringPtr = "Operation invalid while the session is closed";break;
		case RV_RTPSTATUS_SessionIsOpen:       
			stringPtr = "Operation invalid while the session is open";break;
        case RV_RTPSTATUS_NotEnoughRoomInDataBuffer:
            stringPtr = "RvDataBuffer not large enough to fit all data needed"; break;  
        case RV_RTPSTATUS_Failed:
            stringPtr = "Operation failed";break;
		case RV_RTPSTATUS_NotSupported:
			stringPtr = "Request not supported";break;
		case RV_RTPSTATUS_SourceDeleted:       
			stringPtr = "Requested source has been deleted";break;
		case RV_RTPSTATUS_UnknownSource:       
			stringPtr = "Requested source does not exist";break;
		case RV_RTPSTATUS_SourceExists:       
			stringPtr = "Source already exists";break;
		case RV_RTPSTATUS_SourceCollision:
			stringPtr = "Collision with our own SSRC.";break;
		case RV_RTPSTATUS_SourceCsrcCollision:
			stringPtr = "Collision with one of our own CSRCs.";break;
		case RV_RTPSTATUS_SourceRemoteCollision:
			stringPtr = "Collision between two remote sources";break;
		case RV_RTPSTATUS_SourceLoop:
			stringPtr = "Loop of our own SSRC";break;
		case RV_RTPSTATUS_SourceCsrcLoop:
			stringPtr = "Loop of one of our own CSRCs";break;
		case RV_RTPSTATUS_SourceRemoteLoop:
			stringPtr = "Loop of remote source";break;
		case RV_RTPSTATUS_SourceConflicts:
			stringPtr = "Conflicting address continues to collide with our SSRC";break;
		case RV_RTPSTATUS_SourceCsrcConflicts:
			stringPtr = "Conflicting address continues to collide with one of our CSRCs";break;
		case RV_RTPSTATUS_NullPointer:
			stringPtr = "Required object parameter was NULL";break;
		case RV_RTPSTATUS_BadParameter:
			stringPtr = "Bad parameter value";break;
		case RV_RTPSTATUS_NotFound:
			stringPtr = "Item not found";break;
		case RV_RTPSTATUS_DuplicatePacket:
			stringPtr = "Duplcate packet received";break;
		default:
            stringPtr = "Unknown status";break;
    }

    return RvSprintf(buffer,"%s",stringPtr);
}
#endif
