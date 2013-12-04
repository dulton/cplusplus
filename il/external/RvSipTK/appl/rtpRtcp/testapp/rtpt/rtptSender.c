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
#include "rvtypes.h"
#include "rvstdio.h"
#include "rvlog.h"
#include "rvtimestamp.h"
#include "rtptSender.h"
#include "rtptSession.h"
#include "rtptPayloads.h"


static RvLogMgr*    pAppMngLog = NULL;
#if defined(RTPT_TA)
#include "RTPT_init.h" /* for rtptGetRtptObj() */
#else
#include "RTPPlugin.h" /* for rtptGetRtptObj() */
#include "rtptObj.h"
#endif
#define pAppRvLog rtptGetLogSource(rtptGetRtptObj())

/********************************************************************************************
 * rtptStartSendThread
 * purpose : starts sender thread
 * input   : s - pointer to the session object
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/

RvStatus rtptStartSendThread(struct RvRtpSessionObj__* sObj)
{
    rtptSessionObj*   s= (rtptSessionObj*)sObj;
    RvStatus result = RV_OK;
    
    RvLogEnter(pAppRvLog,(pAppRvLog, "rtpStartSendThread"));
    if (!s->sender.launched)
    {
        s->sender.launched = RV_TRUE;
        s->sender.obj = sObj;
        RvThreadConstruct((RvThreadFunc) s->cb.rtpsendercb, 
            &s->sender, (RvLogMgr*)pAppMngLog, &s->sender.thread);
        RvThreadCreate(&s->sender.thread);
        result = RvThreadStart(&s->sender.thread);
    }
    RvLogLeave(pAppRvLog,(pAppRvLog, "rtpStartSendThread"));
    return result;
}
/********************************************************************************************
 * rtpReceiver
 * purpose : receiver thread function, which is used only when RTP session in blocking mode.
 * input   : s - pointer to the session object
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
void rtpReceiver(RvThread *threadPtr, void *dataPtr)
{
    RvSendThread     *thisPtr = (RvSendThread *)dataPtr;
    rtptSessionObj*   s= (rtptSessionObj*)thisPtr->obj;
    RvUint8           rtpPacket[RV_SAMPLE_RTP_PACK_MAX_SIZE];
    RvRtpParam        rtpParam;
    RvStatus          status = RV_OK;
    RvLogEnter(pAppRvLog,(pAppRvLog, "rvReceiverThreadMain"));
	
    s->terminiatedReceiver = RV_FALSE;
    while(s->open)
    {
        RvRtpSession  hRTP = s->hRTP;
        
        RvRtpParamConstruct(&rtpParam);
        status = RvRtpRead(hRTP, rtpPacket, sizeof(rtpPacket), &rtpParam);
        if(status < 0||!s->open)
        {
            int offset = 0;
            char buffer[128];
            offset  = sprintf(&buffer[offset],"Failed to receive RTP packet");
            offset += sprintf(&buffer[offset],".");
            /*RvLogWarning(pAppRvLog, (pAppRvLog, buffer));*/
            continue;
        }
        else
        {
            RvUint32    timestamp = 0;
            RvRtcpSession hRTCP = RvRtpGetRTCPSession(hRTP);
#if (RVRTP_SAMPLE_PAYLOAD_TYPE == RVRTP_SAMPLE_PCMU) || \
			(RVRTP_SAMPLE_PAYLOAD_TYPE == RVRTP_SAMPLE_AMR)
            RvInt64     currentTime = RvInt64Const(1,0,0);
			
            timestamp = rtptTimestamp(s, &currentTime);
            RvLogDebug(pAppRvLog,(pAppRvLog, "Receiving Timestamp:%lu", timestamp));
#else
            timestamp = 0; /* need a specific calculation */
#endif
#ifdef      __RTP_PRINT_PACK    
            {
                RvInt32 count;
                printf("\nreceived packet:\n");
                for (count = rtpParam.sByte; count < rtpParam.len + rtpParam.sByte; count++)
                    printf("%x%x ", rtpPacket[count]>>4, rtpPacket[count]&0x0F);
                printf("\n");
            }
#endif
            if (hRTCP)
                RvRtcpRTPPacketRecv(hRTCP, rtpParam.sSrc, timestamp,  rtpParam.timestamp, rtpParam.sequenceNumber);
			if (s->cb.rtpreadcb)
			    s->cb.rtpreadcb(hRTP, rtpPacket, rtpParam.len, &rtpParam);
            s->readStat++;
            if (s->readStat%200 == 0)
                RvLogInfo(pAppRvLog,(pAppRvLog, "RTP Session (%#x): received %u RTP packets", (RvUint32)hRTP, s->readStat));
            
        }
    }
    s->terminiatedReceiver = RV_TRUE;
    RvLogLeave(pAppRvLog,(pAppRvLog, "rvReceiverThreadMain"));
    RV_UNUSED_ARG(threadPtr);
}
/********************************************************************************************
 * rtptStartRecvThread
 * purpose : starts receiver thread (with function rtpReceiver())
 * input   : s - pointer to the session object
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStartRecvThread(struct RvRtpSessionObj__* sObj)
{
    rtptSessionObj*   s= (rtptSessionObj*)sObj;
    RvStatus result;

    RvLogEnter(pAppRvLog,(pAppRvLog, "rtptStartRecvThread"));
    s->receiver.obj = sObj;
    RvThreadConstruct((RvThreadFunc) rtpReceiver, 
		&s->receiver, (RvLogMgr*)pAppMngLog, &s->receiver.thread);
    RvThreadCreate(&s->receiver.thread);
    result = RvThreadStart(&s->receiver.thread);
	s->receiver.launched = RV_TRUE;
    RvLogLeave(pAppRvLog,(pAppRvLog, "rtptStartRecvThread"));
    return result;
}

