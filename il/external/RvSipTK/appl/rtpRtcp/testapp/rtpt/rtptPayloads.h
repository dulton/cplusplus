#ifndef _RV_RTPPAYLOADS_H_
#define _RV_RTPPAYLOADS_H_
#include "rvthread.h"

#ifdef __cplusplus
extern "C" {
#endif
/********************************************************************************************
 * rtpSenderPCMU
 * purpose : sender thread function which sends g711 - PCMU packets
 * input   : threadPtr - pointer to thread object
 *           dataPtr - context (pointer to the RvSendThread object)
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/	
void rtpSenderPCMU(RvThread *threadPtr, 
				  void *dataPtr);
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
                                RvRtpParam* rtpParamPtr);

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
RvUint32 rtptTimestamp     (rtptSessionObj* s, RvInt64 *currentTimePtr);
/********************************************************************************************
 * rtptTimestampInit
 * purpose : inits timestamp related data
 * input   : s - pointer to the session object
 *           payload - payload enumerator 0-g711
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptTimestampInit (rtptSessionObj* s, RvUint32 payload);



#ifdef __cplusplus
}
#endif


#endif /* _RV_RTPPAYLOADS_H_ */

