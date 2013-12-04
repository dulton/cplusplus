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
 *                              <SipTripleLock.h>
 *
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Boris Perlov                  Jule 2002
 *********************************************************************************/


#ifndef SIP_COMMON_STRUCT_H
#define SIP_COMMON_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rvmutex.h"
#include "rvsemaphore.h"
#include "rvtimer.h"
#include "rvtls.h"
#include "AdsCopybits.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#ifndef IPV4_LOCAL_ADDRESS
#define IPV4_LOCAL_ADDRESS        "0.0.0.0"
#endif /*IPV4_LOCAL_ADDRESS*/

#ifndef IPV4_LOCAL_LOOP
#define IPV4_LOCAL_LOOP           "127.0.0.1"
#endif /*IPV4_LOCAL_LOOP*/

#ifndef IPV6_LOCAL_ADDRESS
#define IPV6_LOCAL_ADDRESS        "[0:0:0:0:0:0:0:0]"
#endif /*IPV6_LOCAL_ADDRESS*/

#ifndef IPV6_LOCAL_ADDRESS_SZ
#define IPV6_LOCAL_ADDRESS_SZ    (sizeof(IPV6_LOCAL_ADDRESS)-1)
#endif

#ifndef MAX_HOST_NAME_LEN
#define MAX_HOST_NAME_LEN 255
#endif

#ifndef SIP_METHOD_LEN
#define SIP_METHOD_LEN (96)
#endif

#define OBJ_TERMINATION_DEFAULT_UDP_TIMEOUT   10
#define OBJ_TERMINATION_DEFAULT_TCP_TIMEOUT   10
#define OBJ_TERMINATION_DEFAULT_TLS_TIMEOUT   30
	
/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
/*Turn on the bit matching the specific callback we are entering now.*/
#define OBJ_ENTER_CB(pObj, eCbName) (BITS_SetBit((void*)(&pObj->cbBitmap), eCbName, 1))

/*Turn off the bit matching the specific callback we are leaving now.*/
#define OBJ_LEAVE_CB(pObj, eCbName) (BITS_SetBit((void*)(&pObj->cbBitmap), eCbName, 0))
	
    

/* SipTripleLock
 * ----------------------
 * Holds the triple lock of transaction, call-leg or reg-client.
 * hLock            - The object lock.
 * hProcessLock     - The object processing lock.
 * hApiLock         - Lock used for mutual exclusion of SIP stack
 *                    thread and user thread trying concurrently to
 *                    access same call-leg
 * apiCnt           - API counter, used with the hApiLock
 * apiLockCnt       - The value of the 'apiCnt' variable in the last API successful locking
 * threadIdLockCount - The object lock counter that was at the time we updated the thread id.
 */

typedef struct
{
    RvBool                         bIsInitialized;
    RvMutex                        hLock;
    RvMutex                        hProcessLock;
    RvSemaphore                    hApiLock;
    RvUint32                       apiCnt;
    RvThreadId                     objLockThreadId;
    RvInt32                        threadIdLockCount;
} SipTripleLock;


/* SipTimer
 * ----------------------
 * The SIP Timer structure based on the common core RvTimer.
 * In addition to the basic RvTimer item this structure includes a valid/not-valid flag.
 * hTimer           - The object timer.
 * bTimerStarted    - A boolean flag indicating whether the timer has been started already.
 */

typedef struct
{
    RvTimer                         hTimer;
    RvBool                         bTimerStarted;
} SipTimer;

#if (RV_TLS_TYPE != RV_TLS_NONE)
/* SipTlsEngine
 * ----------------------
 * The SipTlsEngine structure based on the common core RvTLSEngine.
 * It holds the engine and a lock for the engine
 * tlsEngine     - Core TLS Engine.
 * engineLock    - lock for the engine
 */
typedef struct
{
    RvTLSEngine                    tlsEngine;
    RvMutex                        engineLock;
} SipTlsEngine;
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE)*/

/* SipCSeq
 * ----------------------
 * The SIP CSeq which is mostly useful in case of extended CSeq range (2^32-1) 
 * for identifying (un)initialized UInt32 variables.
 * val          - The CSeq value
 * bInitialized - A boolean flag indicating whether the CSeq was initialized.
 */
typedef struct
{
	RvInt32	  val;
#ifdef RV_SIP_UNSIGNED_CSEQ
    RvBool    bInitialized;
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
} SipCSeq;

/* SipRSeq
 * ----------------------
 * This structure is designed for identifying (un)initialized UInt32 variables.
 * val          - The RSeq value
 * bInitialized - A boolean flag indicating whether the RSeq was initialized.
 */
typedef struct
{
	RvUint32  val;
    RvBool    bInitialized;
} SipRSeq;

#ifdef __cplusplus
}
#endif

#endif
