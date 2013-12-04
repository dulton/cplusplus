/***********************************************************************
Copyright (c) 2003 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..
RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

/***********************************************************************
rtppayloads.c
  defines payload send and receive functions behaviour
  for the different payloads.
***********************************************************************/
#include "rtptG711.h"
#include "rtptToneGenerators.h"
#include "rvlog.h"
#include "rtptSender.h"
#include "rtptSession.h"

#include "payload.h"
#include "rvtimestamp.h"
#include "rtptPayloads.h"

static RvLogSource* pAppRvLog = NULL;

#if defined(RV_IOT)
#define INTEROP_TESTING
#endif

#define RV_RTPSAMPLE_PCMU_TONEFREQ      RvUint32Const(800)    /* Hz */
#define RV_RTPSAMPLE_PCMU_SAMPLERATE    RvUint32Const(8000) /* Hz */
#define RV_RTPSAMPLE_PCMU_PACKETRATE    RvUint32Const(20)   /* ms */
#define RV_RTPSAMPLE_PCMU_PACKETSAMPLES ((RV_RTPSAMPLE_PCMU_SAMPLERATE * RV_RTPSAMPLE_PCMU_PACKETRATE) / RvUint32Const(1000)) /* samples/packet */
#define RV_RTPSAMPLE_PCMU_WAVESECS      RvUint32Const(2)
#define RV_RTPSAMPLE_PCMU_SAMPLES       (RV_RTPSAMPLE_PCMU_WAVESECS * RV_RTPSAMPLE_PCMU_SAMPLERATE)
#define RV_RTPSAMPLE_PCMU_WAVESTEPS     RvUint32Const(16)
#define RV_RTPSAMPLE_PCMU_MAGSTEPSIZE   (RvUint32Const(0x7FFF) / (RV_RTPSAMPLE_PCMU_WAVESTEPS / RvUint32Const(2)))
#define RV_RTPSAMPLE_PCMU_WAVESTEPSIZE  (RV_RTPSAMPLE_PCMU_SAMPLES / RV_RTPSAMPLE_PCMU_WAVESTEPS)

/********************************************************************************************
 * rtpSenderPCMU
 * purpose : sender thread function which sends g711 - PCMU packets
 * input   : threadPtr - pointer to thread object
 *           dataPtr - context (pointer to the RvSendThread object)
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
#include <stdio.h>
void rtpSenderPCMU(RvThread *threadPtr, void *dataPtr)
{
    RvSendThread     *thisPtr;
    RvUint8          rtpPacket[RV_SAMPLE_RTP_PACK_MAX_SIZE];
    RvRtpParam       rtpParam;
    rtptSessionObj   *sessionObj;
    RvSawtoothWave   sawtoothWave;
    RvToneGenerator  toneGenerator;
    RvInt16          *sawtoothBufferPtr;
    RvInt16          *sawtoothDataPtr;
    RvInt16          *sawtoothEndPtr;
    RvUint8          *dataStartPtr;
    RvUint32         dataSize;
    RvUint32         magnitude;
    RvUint32         i;
    RvRtpSession     rtpSession;
    RvUint32         extension[2] = {123,125};
    RvUint32  prate; /* payload rate */

    RvRtpParamConstruct(&rtpParam);
    RvLogEnter(pAppRvLog,(pAppRvLog, "rvSenderThreadMain"));

    thisPtr     = (RvSendThread *)dataPtr;
    sessionObj  = (rtptSessionObj*)thisPtr->obj;
    sessionObj->terminiatedSender = RV_FALSE;
    rtpSession = sessionObj->hRTP;;
    /* Get buffer for sawtooth wave */
    if(RvMemoryAlloc(NULL, RV_RTPSAMPLE_PCMU_SAMPLES * sizeof(*sawtoothBufferPtr), NULL, (void **)&sawtoothBufferPtr) != RV_OK) {
        RvLogError(pAppRvLog, (pAppRvLog,"Couldn't allocate sawtooth buffer"));
        return;
    }
    sawtoothEndPtr = &sawtoothBufferPtr[RV_RTPSAMPLE_PCMU_SAMPLES];

    /* Generate a sawtooth wave that rises in magnitude then drops in magnitude */
    rvSawtoothWaveGetToneGenerator(&sawtoothWave, &toneGenerator);
    sawtoothDataPtr = sawtoothBufferPtr;
    magnitude = RvUint32Const(0);
    for(i = 0; i < RV_RTPSAMPLE_PCMU_WAVESTEPS; i++) {
        if(i >= (RV_RTPSAMPLE_PCMU_WAVESTEPS / RvUint32Const(2)))
            magnitude -= RV_RTPSAMPLE_PCMU_MAGSTEPSIZE;
        rvSawtoothWaveInitialize(&sawtoothWave, RV_RTPSAMPLE_PCMU_SAMPLERATE, RV_RTPSAMPLE_PCMU_TONEFREQ, (RvInt16)magnitude);
        rvToneGeneratorGenerate(&toneGenerator, sawtoothDataPtr, RV_RTPSAMPLE_PCMU_WAVESTEPSIZE);
        sawtoothDataPtr += RV_RTPSAMPLE_PCMU_WAVESTEPSIZE;
        if(i < (RV_RTPSAMPLE_PCMU_WAVESTEPS / RvUint32Const(2)))
            magnitude += RV_RTPSAMPLE_PCMU_MAGSTEPSIZE;
    }

    /* Figure out all the buffer and padding sizes */
    dataSize = sessionObj->rtpRate; /* PCMU is 1 byte per sample */
    rtpParam.sByte = RvRtpPCMUGetHeaderLength(); /* includes RTP header */

    /* Keep marker flag off. Should normally be set according to payload requirements. */
    rtpParam.marker = RV_FALSE;

    /* Get payload type. Some payloads may have dynamic payload types, but not this one. */

    /* Initialize RTP timestamp (lock and mark time for getting intermediate timestamps). */


    RvLogInfo(pAppRvLog, (pAppRvLog, "PCMU Thread Sending"));
    /****************************************************************************
     *  This code needed for true calculation RTP timestamp in of RTCP SR. 
     *  In case of standard payload types this code is not necessary.
     *  But in case of dynamic payload types, user must specify the payload type 
     ****************************************************************************/
    prate =  8000;
    RvRtcpSessionSetParam(sessionObj->hRTCP, RVRTCP_PARAMS_RTPCLOCKRATE, &prate);
    
    /* Send RTP packets at proper rate */
    sawtoothDataPtr = sawtoothBufferPtr;
    while(sessionObj->open)
    {
        /* Convert wave data to mu-law and place it in the data buffer */
#ifdef RVRTP_OLD_CONVENTION_API
        rtpParam.extensionBit = RV_TRUE;
        rtpParam.extensionLength = 2;
        rtpParam.extensionProfile = 100;
        rtpParam.extensionData = extension;
        rtpParam.sByte = RvRtpParamGetExtensionSize(&rtpParam);
        rtpParam.sByte += RvRtpPCMUGetHeaderLength();
#else
        RV_UNUSED_ARG(extension);
        rtpParam.sByte = RvRtpPCMUGetHeaderLength();
#endif
        dataStartPtr =    rtpPacket + rtpParam.sByte;
        rvG711PCMToULaw(sawtoothDataPtr, dataStartPtr, sessionObj->rtpRate);

        rtpParam.timestamp       = sessionObj->rtpTimestamp;
        rtpParam.sequenceNumber  = sessionObj->seqNum++;
        /* Serialize any payload header and footer information */
        RvRtpPCMUPack(rtpPacket, dataSize + rtpParam.sByte, &rtpParam, NULL);
        rtpParam.len = sizeof(rtpPacket); /* for encryption paddings > (dataSize + rtpParam.sByte) */
        /* Send RTP packet */
		if (sessionObj->open&&sessionObj->hRTP!=NULL)
		{
#ifdef __RTP_PRINT_PACK
            {
                RvUint32 count;
                printf("\nsending packet:\n");
                for (count = 12; count < dataSize + rtpParam.sByte; count++)
                    printf("%x%x ", rtpPacket[count]>>4, rtpPacket[count]&0x0F);
                printf("\n");
            }
#endif
#ifdef INTEROP_TESTING
        sprintf((RvInt8*)dataStartPtr, "RV SRTP Testing sn=%d\n", (RvInt32) rtpParam.sequenceNumber);
        dataSize =  strlen((RvInt8*)dataStartPtr)+1;
#endif
            RvRtpWrite(sessionObj->hRTP, rtpPacket, dataSize + rtpParam.sByte, &rtpParam);
            RvLogDebug(pAppRvLog,(pAppRvLog, "Sending Timestamp:%lu", rtpParam.timestamp));
			/* Wait until next send time (sleep is not that accurate, but */
			/* good enough for this sample app). System clock needs to be */
			/* <= period for this to be close. */

#ifdef INTEROP_TESTING
			RvThreadNanosleep(RV_TIME64_NSECPERSEC, NULL);
#else
			RvThreadNanosleep(sessionObj->rtpPeriod, NULL);
#endif
		}
		if (sessionObj->open)
		{
			/* Update the timestamp (lock and mark time for getting intermediate timestamps). */
			RvLockGet(&sessionObj->timeLock, NULL);
			sessionObj->rtpTimestamp += sessionObj->rtpRate;
			sessionObj->rtpTimestampTime = RvTimestampGet(NULL); /* Current time in nanoseconds */
			RvLockRelease(&sessionObj->timeLock, NULL);
			
			/* Cycle through sawtooth data */
			sawtoothDataPtr += sessionObj->rtpRate;
			if(sawtoothDataPtr >= sawtoothEndPtr)
				sawtoothDataPtr = sawtoothBufferPtr;
		}
    }

    RvLogInfo(pAppRvLog, (pAppRvLog, "PCMU Thread Exiting"));

    RvMemoryFree(sawtoothBufferPtr, NULL);
    sessionObj->terminiatedSender = RV_TRUE;
    RV_UNUSED_ARG(threadPtr);
    RvLogLeave(pAppRvLog,(pAppRvLog, "rvSenderThreadMain"));
}

/********************************************************************************************
 * rtpReceivePCMUPacket
 * purpose : g711 PCMU data processing
 * input   : rtpSession - handle of RTP session
 *           rtpPacketPtr - pointer to data buffeer which contains g.711 data
 *           dataSize    -  size of data buffeer which contains g.711 data
 *           rtpParamPtr - pointer RvRtpParam staructure of received RTP packet 
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtpReceivePCMUPacket(RvRtpSession rtpSession,
                              RvUint8* rtpPacketPtr,
                              RvInt32 dataSize,
                              RvRtpParam* rtpParamPtr)
{
    RvUint32   pcmuDataSize;
    RvUint8  *pcmuDataStartPtr;
	RvInt16   rvRtpSamplePCMUBuf[RV_RTPSAMPLE_PCMU_PACKETSAMPLES]; /* must be used by application */
    /* Unserialize any payload header information */
    RvRtpPCMUUnpack(rtpPacketPtr, dataSize, rtpParamPtr, NULL); /* no payload */
    /* Check payload type - simple for constant payload types */
    if(rtpParamPtr->payload != RV_RTP_PAYLOAD_PCMU) {
        return RV_ERROR_BADPARAM;
    }
	
    /* Calculate sizes and data locations */
    pcmuDataSize     = dataSize - rtpParamPtr->sByte;
    pcmuDataStartPtr = (RvUint8 *)rtpPacketPtr + rtpParamPtr->sByte;
#ifdef ___PRINTINGS
    /* printing the buffer */
    {
        RvChar buffer[16*3+1]; /* print buffer */
        RvUint32 Count = 0, offset = 0;
        RvLogInfo(pAppRvLog, (pAppRvLog, "Received packet:"));
		
        for (Count=0;Count<pcmuDataSize;Count++)
        {
            if ((Count%16 == 0)&& (Count>0))
            {
                RvLogInfo(pAppRvLog, (pAppRvLog, "%d:%s", Count, buffer));
                offset=0;
            }
            offset += sprintf(buffer + offset, "%x%x ", (pcmuDataStartPtr[Count]>>4),
                pcmuDataStartPtr[Count]&0x0F);
        }
        RvLogInfo(pAppRvLog, (pAppRvLog, "%d:%s", Count, buffer));
    }
#endif
    /* Decode the data (assumes g711 u-law encoding)*/
    rvG711ULawToPCM(pcmuDataStartPtr, (void*)rvRtpSamplePCMUBuf, pcmuDataSize);
    /* using decoded data from rvRtpSamplePCMUBuf */
    RV_UNUSED_ARG(rtpSession);
    return RV_OK;
}

/********************************************************************************************
 * rtptTimestampInit
 * purpose : inits timestamp related data
 * input   : s - pointer to the session object
 *           payload - payload enumerator 0-g711
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptTimestampInit(rtptSessionObj* s, RvUint32 payload)
{
	RvRandom          randval;
	RvRandomGeneratorGetValue(&s->randomGenerator, &randval);

    /* Store basic information about data stream */
	switch(payload)
	{
	default:
	case 0:
			s->rtpPeriod = RvInt64Mul(RV_TIME64_NSECPERMSEC, RvInt64FromRvUint32(RV_RTPSAMPLE_PCMU_PACKETRATE));
			s->rtpRate = RV_RTPSAMPLE_PCMU_PACKETSAMPLES; /* samples each period */
			break;
	}
	RvLockGet(&s->timeLock, NULL);
	s->rtpTimestamp = (RvUint32)randval; /* random value, as per RFC */
	s->rtpTimestampTime = RvTimestampGet(NULL); /* Current time in nanoseconds */
	RvLockRelease(&s->timeLock, NULL);
    return RV_OK;
}

/********************************************************************************************
 * rtptTimestamp
 * purpose : allows to obtain timestamp of received packet in units of sending timestamp.
 *           This function is used for transfering to RTCP accurate RTP timestamp of receiving packet
 *           in timestamp of sender.
 * input   : s - pointer to the session object
 *           currentTimePtr - pointer to time when RTP packet was received.
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvUint32 rtptTimestamp(rtptSessionObj* s, RvInt64 *currentTimePtr)
{
    RvUint32 rtpTimestamp;
    RvUint32 rtpTimestampDelta;
    RvInt64  delaySinceTimestamp;
	
    /* Prevent doing calculation while timestamp is being updated. */
    RvLockGet(&s->timeLock, NULL);
	
    if(s->sender.launched == RV_TRUE) {
        *currentTimePtr = RvTimestampGet(NULL); /* Get the current time in nanoseconds */
        delaySinceTimestamp = RvInt64Sub(*currentTimePtr, s->rtpTimestampTime);
        rtpTimestampDelta = RvInt64ToRvUint32(
			RvInt64Div(
			RvInt64Mul(RvInt64FromRvUint32(s->rtpRate) , delaySinceTimestamp),
			s->rtpPeriod));
        rtpTimestamp = s->rtpTimestamp + rtpTimestampDelta;
    } else {
        *currentTimePtr = RvInt64Const(1,0,0);
        rtpTimestamp = 0;
    }
	
    RvLockRelease(&s->timeLock, NULL);
    return rtpTimestamp;
}
