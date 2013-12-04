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
 *                              <_SipCommonCore.h>
 *
 *  The SipCommon.c file contains Internal API functions used by all the stack
 *  layers.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  Oct 2003
 *********************************************************************************/



#ifndef SIP_COMMON_H
#define SIP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonTypes.h"
#include "RvSipCommonTypes.h"
#include "rvtimestamp.h"
#include "rvmemory.h"
#include "rvmutex.h"
#include "rvselect.h"
#include "rvcbase.h"
#include "rvlog.h"

/*-----------------------------------------------------------------------*/
/*                              MACROS                                  */
/*-----------------------------------------------------------------------*/
typedef enum {
    IN_SEC,
    IN_MSEC
} SipTimestampUnits;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
* SipCommonCoreConvertCoreToSipLogMask
* ------------------------------------------------------------------------
* General: converts a core log mask to sip log mask.
* Return Value:sip log mask
* ------------------------------------------------------------------------
* Arguments: coreMask - core log mask
***************************************************************************/
RvInt32 RVCALLCONV SipCommonCoreConvertCoreToSipLogMask(
                                            IN  RvLogMessageType    coreMask);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


/*-----------------------------------------------------------------------*/
/*                             TIMER FUNCTIONS                           */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* SipCommonCoreRvTimerInit
* ------------------------------------------------------------------------
* General: Common core timer initialization
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
void RVCALLCONV SipCommonCoreRvTimerInit(IN SipTimer          *timer);

/***************************************************************************
* SipCommonCoreRvTimerStart
* ------------------------------------------------------------------------
* General: Common core timer triggering
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
*           pSelect   - pointer to the thread's select engine
*           delay     - time delay in milliseconds
*           timerCb   - a callback function
*           cbContext - user data for the callback routine
***************************************************************************/
RvStatus RVCALLCONV SipCommonCoreRvTimerStart(IN SipTimer          *timer,
                                               IN RvSelectEngine    *pSelect,
                                               IN RvUint32         delay,
                                               IN RvTimerFunc       timerCb,
                                               IN void              *cbContext);

/***************************************************************************
* SipCommonCoreRvTimerStartEx
* ------------------------------------------------------------------------
* General: Common core timer triggering
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
*           pSelect   - pointer to the thread's select engine
*           delay     - time delay in milliseconds
*           timerCb   - a callback function
*           cbContext - user data for the callback routine
***************************************************************************/
RvStatus RVCALLCONV SipCommonCoreRvTimerStartEx(IN SipTimer          *timer,
                                               IN RvSelectEngine    *pSelect,
                                               IN RvUint32         delay,
                                               IN RvTimerFuncEx       timerCb,
                                               IN void              *cbContext);

/***************************************************************************
* SipCommonCoreRvTimerIgnoreExpiration
* ------------------------------------------------------------------------
* General: Decides whether to ignore the timer expiration event.
*          The function is used in every timer expiration callback, to verify
*          that the timer was not released or changed while waiting in the
*          event queue.
* Return Value: RV_TRUE - If equal, RV_FALSE - If not equal.
* ------------------------------------------------------------------------
* Arguments:
* Input:    ObjTimer    - pointer to SipTimer structure in the SIP object
*           timerInfo   - pointer to the timers given in the expiration callback.
***************************************************************************/
RvBool RVCALLCONV SipCommonCoreRvTimerIgnoreExpiration(IN SipTimer  *ObjTimer,
                                                       IN RvTimer   *timerInfo);

/***************************************************************************
* SipCommonCoreRvTimerCancel
* ------------------------------------------------------------------------
* General: Common core timer triggering
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
RvStatus RVCALLCONV SipCommonCoreRvTimerCancel(IN SipTimer          *timer);

/***************************************************************************
* SipCommonCoreRvTimerStarted
* ------------------------------------------------------------------------
* General: Common core timer triggering checking
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
RvBool RVCALLCONV SipCommonCoreRvTimerStarted(IN SipTimer        *timer);


/***************************************************************************
* SipCommonCoreRvTimerExpired
* ------------------------------------------------------------------------
* General: mark that the timer expired
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
void RVCALLCONV SipCommonCoreRvTimerExpired(IN SipTimer        *timer);

/***************************************************************************
* SipCommonCoreRvTimestampGet
* ------------------------------------------------------------------------
* General: Common core get timestamp
* Return Value: time in msec/sec
* ------------------------------------------------------------------------
* Arguments:
* Input:    unitCode  - how to return the timestamp: in sec or msec
***************************************************************************/
RvUint32 RVCALLCONV SipCommonCoreRvTimestampGet(IN SipTimestampUnits unitCode);


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
* SipCommonCoreFormatLogMessage
* ------------------------------------------------------------------------
* General: Adds the message type and log source to the core log message.
*          Note - there is no need to lock the static buffer since it is locked
*          by the common core before the callback.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:     logRecord - Core log record
*           strFinalText - The formated text
***************************************************************************/
void RVCALLCONV SipCommonCoreFormatLogMessage(
                                         IN  RvLogRecord  *logRecord,
                                         OUT RvChar      **strFinalText);



#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/***************************************************************************
* SipCommonCoreGetAddressTypeStr
* ------------------------------------------------------------------------
* General: Converts a CORE Address Type to String.
* Return Value: The conversion result string.
* ------------------------------------------------------------------------
* Arguments:
* Input:   addrType - The CORE Address Type.
***************************************************************************/
RvChar *RVCALLCONV SipCommonCoreGetAddressTypeStr(IN RvInt addrType);

/***************************************************************************
* SipCommonCoreGetAddressTypeInt
* ------------------------------------------------------------------------
* General: Converts a String to CORE Address Type (RvInt).
* Return Value: The conversion result string.
* ------------------------------------------------------------------------
* Arguments:
* Input:   strAddrType - The string that represents CORE Address Type.
***************************************************************************/
RvInt RVCALLCONV SipCommonCoreGetAddressTypeInt(IN RvChar *strAddrType);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_CALL_LEG_H */

