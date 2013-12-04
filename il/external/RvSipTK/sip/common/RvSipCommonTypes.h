
/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              RvSipCommonTypes.h
 *
 *	Common definitions of RADVISION SIP stack.
 *
 *    Author                         Date
 *    ------                        ------
 *  
 *********************************************************************************/


#ifndef _RV_SIP_COMMON_TYPES_H
#define _RV_SIP_COMMON_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif


    
typedef enum
{
    RVSIP_COMMON_STACK_OBJECT_TYPE_UNDEFINED = -1,
    RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG,
    RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION,
    RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_TRANSACTION,
    RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION,
    RVSIP_COMMON_STACK_OBJECT_TYPE_NOTIFICATION,
    RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT,
	RVSIP_COMMON_STACK_OBJECT_TYPE_PUB_CLIENT,
	RVSIP_COMMON_STACK_OBJECT_TYPE_CONNECTION,
    RVSIP_COMMON_STACK_OBJECT_TYPE_APP_OBJECT,
    RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_INVITE,
    RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSMITTER,
	RVSIP_COMMON_STACK_OBJECT_TYPE_SECURITY_AGREEMENT,
    RVSIP_COMMON_STACK_OBJECT_TYPE_SECURITY_OBJECT,
	RVSIP_COMMON_STACK_OBJECT_TYPE_COMPARTMENT
}RvSipCommonStackObjectType;

/* RvSipListLocation
 * --------------------
 * This enumeration represents the location of an element on a list.
 * 
 */
typedef enum
{
    RVSIP_FIRST_ELEMENT = 0,
	RVSIP_LAST_ELEMENT,
	RVSIP_NEXT_ELEMENT,
	RVSIP_PREV_ELEMENT
} RvSipListLocation;


/* Defines the log filters to be used by the RADVISION SIP stack modules. */ 
 typedef enum {
    RVSIP_LOG_DEBUG_FILTER    = 0x01,
    RVSIP_LOG_INFO_FILTER     = 0x02,
    RVSIP_LOG_WARN_FILTER     = 0x04,
    RVSIP_LOG_ERROR_FILTER    = 0x08,
    RVSIP_LOG_EXCEP_FILTER    = 0x10,
    RVSIP_LOG_LOCKDBG_FILTER  = 0x20,
	RVSIP_LOG_ENTER_FILTER    = 0x40,   /*only for common core loging*/
	RVSIP_LOG_LEAVE_FILTER    = 0x80
} RvSipLogFilters;


/* RvSipTimers
* -------------------
* The RvSipTimers structure is used to set different timeout values to different stack
* objects, overriding the values received from the stack configuration.
* The structure is also used to control the number of transmissions performed for requests
* and responses.
*
* T1Timeout - T1 determines several timers as defined in RFC3261. 
* For example - on UDP, a UAC retransmits INVITE request at interval that starts 
* with T1 seconds, and doubles after every retransmission.
* on UDP, a UAC retransmits general request at interval that starts with T1
* seconds, and doubles until it reaches T2.
*
* T2Timeout - Determines the maximum retransmission interval. 
* The parameter value cannot be less than 4000.
* For example, on UDP, general requests are retransmitted at an interval which 
* starts at T1 and doubles until it reaches T2. 
* If a provisional response is received, retransmissions continue but at an interval of T2.
*  
* T4Timeout - T4 represents the amount of time the network takes to clear messages between
* client and server transactions.
* For example, on UDP, T4 determines the time that a UAS waits after receiving an ACK
* message and before terminating the transaction.
*    
* genLingerTimeout - After a server sends a final response, the server cannot be sure that the
* client has received the response message. The server should be able to retransmit the 
* response upon receiving retransmissions of the request for generalLingerTimer milliseconds.
* Remarks:  if no value was given - genLingerTimeout = 64*T1
*      
* inviteLingerTimeout - After sending an ACK for an INVITE final response, a client cannot be 
* sure that the server has received the ACK message. 
* The client should be able to retransmit the ACK upon receiving retransmissions of the
* final response for inviteLingerTimeout milliseconds.
*
* cancelInviteNoResponseTimer - After sending a CANCEL for an INVITE request, a client will
* wait for a final response until the cancelInviteNoResponseTimer expires. 
* Remarks:  if no value was given - cancelInviteNoResponseTimer = 64*T1. must be greater than 0.
*
* generalRequestTimeoutTimeout - After sending general request, the transaction enters the 
* Client-General-Request-Sent state.  If no response is received when generalRequestTimeoutTimer
* is expired, the transaction is terminated automatically (in any transport).
* if no value was given - generalRequestTimeoutTimer = 64*T1    
*
* provisionalTimeout - The provisionalTimer is set when a provisional response is received for an 
* Invite request. The transaction will stop retransmissions of the Invite request and
* will wait for a final response until the provisionalTimeout expires. If you set the
* provisionalTimeout to zero (0), no timer is set. The Invite transaction will wait 
* indefinitely for the final response.
*
* maxInviteRequestRetransmissions  - limitation for number of INVITE request transmissions. 
* maxInviteResponseRetransmissions - limitation for number of INVITE response transmissions.
* maxGeneralRequestRetransmissions - limitation for number of general request transmissions.
* 
* Note - For all the above retransmissions counts, the value is applied for all transmissions 
* (initial and retransmission). for example, if you set the maxInviteRequestRetransmissions to be 5, 
* there will be 5 transmissions of INVITE request, 1 initial + 4 retransmissions.
*
*/
 typedef struct 
 {
	 /*timeouts*/
	 RvInt32  T1Timeout;
	 RvInt32  T2Timeout;
	 RvInt32  T4Timeout;
	 RvInt32  genLingerTimeout;
	 RvInt32  inviteLingerTimeout;
	 RvInt32  cancelInviteNoResponseTimeout;
	 RvInt32  generalRequestTimeoutTimeout;
	 RvInt32  provisionalTimeout;
	 
	 /*retransmissions limitation*/
	 RvInt8   maxInviteRequestRetransmissions;
	 RvInt8   maxInviteResponseRetransmissions;
	 RvInt8   maxGeneralRequestRetransmissions;
	 
 } RvSipTimers;
 


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _RV_SIP_COMMON_TYPES_H*/
