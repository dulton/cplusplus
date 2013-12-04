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

#ifndef _RV_RTPSENDER_H_
#define _RV_RTPSENDER_H_
#include "rvthread.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#define RV_SAMPLE_RTP_PACK_MAX_SIZE (1600)
struct RvRtpSessionObj__;

/* sender or receiving thread structure */
typedef struct RvSendThread__
{
    RvThread      thread;
    RvBool        launched; /* TRUE means that we need to destruct the thread */
    struct RvRtpSessionObj__* obj;
	
} RvSendThread;

RvStatus rtptStartSendThread(struct RvRtpSessionObj__* s);
/********************************************************************************************
 * rtptStartRecvThread
 * purpose : starts receiver thread (with function rtpReceiver())
 * input   : s - pointer to the session object
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStartRecvThread(struct RvRtpSessionObj__* s);

#ifdef __cplusplus
}
#endif

#endif /* _RV_RTPSENDER_H_ */

