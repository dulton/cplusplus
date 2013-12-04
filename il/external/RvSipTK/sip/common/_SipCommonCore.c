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
 *                              <SipCommon.c>
 *
 *  The SipCommonCore.c wraps common core function and converts type defs and
 *  status code.
 *  layers.
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                  Oct 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "_SipCommonCore.h"
#include "_SipCommonUtils.h"
#include "rvstdio.h"
#include "RvSipCommonTypes.h"

/*-----------------------------------------------------------------------*/
/*                              MACROS                                   */
/*-----------------------------------------------------------------------*/
#define LOG_SOURCE_NAME_LEN              (13)
#define LOG_MSG_PREFIX                   (24)

#define CORE_ADDRESS_TYPE_IPV4_STR       "IPV4"
#define CORE_ADDRESS_TYPE_IPV6_STR       "IPV6"
#define CORE_ADDRESS_TYPE_UNDEFINED_STR  "UNDEFINED"

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                      LOG  FUNCTIONS                                   */
/*-----------------------------------------------------------------------*/

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
                                            IN  RvLogMessageType    coreMask)
{
    RvLogMessageType sipFilters;

    sipFilters = RV_LOGLEVEL_NONE;
    if(coreMask & RV_LOGLEVEL_DEBUG)
    {
        sipFilters |= RVSIP_LOG_DEBUG_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_INFO)
    {
        sipFilters |= RVSIP_LOG_INFO_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_WARNING)
    {
        sipFilters |= RVSIP_LOG_WARN_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_ERROR)
    {
        sipFilters |= RVSIP_LOG_ERROR_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_EXCEP)
    {
        sipFilters |= RVSIP_LOG_EXCEP_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_SYNC)
    {
        sipFilters |= RVSIP_LOG_LOCKDBG_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_ENTER)
    {
        sipFilters |= RVSIP_LOG_ENTER_FILTER;
    }
    if(coreMask & RV_LOGLEVEL_LEAVE)
    {
        sipFilters |= RVSIP_LOG_LEAVE_FILTER;
    }
    return sipFilters;
}
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
void RVCALLCONV SipCommonCoreRvTimerInit(IN SipTimer          *timer)
{
    timer->bTimerStarted = RV_FALSE;
}

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
                                               IN void              *cbContext)
{
    RvStatus crv;
    RvInt64  delay_nsec;

    delay_nsec = RvInt64Mul(RvInt64FromRvUint32(delay), RV_TIME64_NSECPERMSEC);
    crv = RvTimerStart(&timer->hTimer, &pSelect->tqueue, RV_TIMER_TYPE_ONESHOT,
                       delay_nsec, timerCb, cbContext);
    if (crv == RV_OK)
    {
        timer->bTimerStarted = RV_TRUE;
        return RV_OK;
    }

    return CRV2RV(crv);
}

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
                                               IN void              *cbContext)
{
    RvStatus crv;
    RvInt64  delay_nsec;

    delay_nsec = RvInt64Mul(RvInt64FromRvUint32(delay), RV_TIME64_NSECPERMSEC);
    crv = RvTimerStartEx(&timer->hTimer, &pSelect->tqueue, RV_TIMER_TYPE_ONESHOT,
                         delay_nsec, timerCb, cbContext);
    if (crv == RV_OK)
    {
        timer->bTimerStarted = RV_TRUE;
        return RV_OK;
    }

    return CRV2RV(crv);
}

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
                                                       IN RvTimer  *timerInfo)
{
    RvBool bIgnore = RV_FALSE;


	if(RvTimerIsEqual(&ObjTimer->hTimer,timerInfo) == RV_FALSE)
    {
        /* timer is not the same - was already set to a new value */
        bIgnore = RV_TRUE;
    }
    else
    {
        if(RV_FALSE == SipCommonCoreRvTimerStarted(ObjTimer))
        {
            /* timer was already released */
            bIgnore = RV_TRUE;
        }
    }
    return bIgnore;

}
/***************************************************************************
* SipCommonCoreRvTimerCancel
* ------------------------------------------------------------------------
* General: Common core timer canceling
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
RvStatus RVCALLCONV SipCommonCoreRvTimerCancel(IN SipTimer          *timer)
{
    RvStatus  crv;

    if (!SipCommonCoreRvTimerStarted(timer))
        return RV_OK;

    crv = RvTimerCancel(&timer->hTimer, RV_TIMER_CANCEL_DONT_WAIT_FOR_CB);
    if (crv == RV_OK  ||  RvErrorGetCode(crv)==RV_TIMER_WARNING_BUSY)
    {
        timer->bTimerStarted = RV_FALSE;
        return RV_OK;
    }

    return CRV2RV(crv);
}

/***************************************************************************
* SipCommonCoreRvTimerStarted
* ------------------------------------------------------------------------
* General: Common core timer triggering checking
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
RvBool RVCALLCONV SipCommonCoreRvTimerStarted(IN SipTimer        *timer)
{
    return timer->bTimerStarted;
}

/***************************************************************************
* SipCommonCoreRvTimerExpired
* ------------------------------------------------------------------------
* General: Mark that the timer expired
* Return Value: status
* ------------------------------------------------------------------------
* Arguments:
* Input:    timer     - pointer to SipTimer structure
***************************************************************************/
void RVCALLCONV SipCommonCoreRvTimerExpired(IN SipTimer        *timer)
{
    timer->bTimerStarted = RV_FALSE;
}

/***************************************************************************
* SipCommonCoreRvTimestampGet
* ------------------------------------------------------------------------
* General: Common core get timestamp
* Return Value: time in msec/sec
* ------------------------------------------------------------------------
* Arguments:
* Input:    unitCode  - how to return the timestamp: in sec or msec
***************************************************************************/
RvUint32 RVCALLCONV SipCommonCoreRvTimestampGet(IN SipTimestampUnits unitCode)
{
    return RvInt64ToRvUint32((RvInt64Div(RvTimestampGet(NULL),
        (unitCode == IN_SEC ? RV_TIME64_NSECPERSEC : RV_TIME64_NSECPERMSEC))));
}

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
                                         OUT RvChar      **strFinalText)
{
    RvChar         *ptr;
    const RvChar   *strSourceName =  RvLogSourceGetName(logRecord->source);
    const RvChar   *mtypeStr;


    /* Find the message type string */
    switch (RvLogRecordGetMessageType(logRecord))
    {
        case RV_LOGID_EXCEP:   mtypeStr = "EXCEP  - "; break;
        case RV_LOGID_ERROR:   mtypeStr = "ERROR  - "; break;
        case RV_LOGID_WARNING: mtypeStr = "WARN   - "; break;
        case RV_LOGID_INFO :   mtypeStr = "INFO   - "; break;
        case RV_LOGID_DEBUG:   mtypeStr = "DEBUG  - "; break;
        case RV_LOGID_ENTER:   mtypeStr = "ENTER  - "; break;
        case RV_LOGID_LEAVE:   mtypeStr = "LEAVE  - "; break;
        case RV_LOGID_SYNC:    mtypeStr = "SYNC   - "; break;
        default:
           mtypeStr = "NOT_VALID"; break;
    }

    ptr = (char *)logRecord->text - LOG_MSG_PREFIX;
    *strFinalText = ptr;

    /* Format the message type */
    /*ptr = (char *)strFinalText;*/
    strcpy(ptr, mtypeStr);
    ptr += strlen(ptr);

    /* Pad the module name */
    memset(ptr, ' ', LOG_SOURCE_NAME_LEN+2);

    memcpy(ptr, strSourceName, strlen(strSourceName));
    ptr += LOG_SOURCE_NAME_LEN;
    *ptr = '-'; ptr++;
    *ptr = ' '; ptr++;

}
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
RvChar *RVCALLCONV SipCommonCoreGetAddressTypeStr(IN RvInt addrType)
{
    switch (addrType)
    {
    case RV_ADDRESS_TYPE_IPV4:
        return CORE_ADDRESS_TYPE_IPV4_STR;
    case RV_ADDRESS_TYPE_IPV6:
        return CORE_ADDRESS_TYPE_IPV6_STR; 
    default:
        return CORE_ADDRESS_TYPE_UNDEFINED_STR;
    }
}

/***************************************************************************
* SipCommonCoreGetAddressTypeInt
* ------------------------------------------------------------------------
* General: Converts a String to CORE Address Type (RvInt).
* Return Value: The conversion result string.
* ------------------------------------------------------------------------
* Arguments:
* Input:   strAddrType - The string that represents CORE Address Type.
***************************************************************************/
RvInt RVCALLCONV SipCommonCoreGetAddressTypeInt(IN RvChar *strAddrType)
{
    if (SipCommonStricmp(strAddrType,CORE_ADDRESS_TYPE_IPV4_STR) == RV_TRUE)
    {
        return RV_ADDRESS_TYPE_IPV4;
    }
    else if (SipCommonStricmp(strAddrType,CORE_ADDRESS_TYPE_IPV6_STR) == RV_TRUE)
    {
        return RV_ADDRESS_TYPE_IPV6;
    }

    return RV_ADDRESS_TYPE_NONE;
}


