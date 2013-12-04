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
rtptUtils.c

Utility functions widely used by this module.
***********************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rtptUtils.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef USE_ATT
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32) || (RV_OS_TYPE == RV_OS_TYPE_WINCE)
#include <stdio.h>
#define RvVsnprintf _vsnprintf
#else
#include <stdio.h>
#include <stdarg.h>
#define RvVsnprintf vsnprintf
#endif
#endif



/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/





/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/





/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/******************************************************************************
 * rtptUtilsEvent
 * ----------------------------------------------------------------------------
 * General: Send indication of an event to the application that uses this
 *          module.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - RTP Test object to use.
 *         eventType        - Type of the event indicated.
 *         session          - Session this event belongs to (NULL if none).
 *         eventStr         - Event string, in printf() formatting.
 * Output: None.
 *****************************************************************************/
void rtptUtilsEvent(
    IN rtptObj        *rtpt,
    IN const RvChar *eventType,
    IN rtptSessionObj *session,
    IN const RvChar *eventStr, ...)
{
#if defined(RVRTPSAMPLE_EMBEDDED)
    RV_UNUSED_ARG(rtpt);
    RV_UNUSED_ARG(eventType);
    RV_UNUSED_ARG(session);
    RV_UNUSED_ARG(eventStr);
#else
    RvChar msg[1024];
    va_list varArg;
    va_start(varArg, eventStr);
    RvVsnprintf(msg, sizeof(msg), eventStr, varArg);
    va_end(varArg);

    if (rtpt != NULL && rtpt->cb.rtptEventIndication != NULL)
        rtpt->cb.rtptEventIndication(rtpt, eventType, session, msg);    
#endif
}


/******************************************************************************
 * rtptUtilsLog
 * ----------------------------------------------------------------------------
 * General: Indicate the application that it can log a message for its user.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt               - Endpoint object to use.
 *         call             - Call this log message belongs to (NULL if none).
 *         logStr           - Log string, in printf() formatting.
 * Output: None.
 *****************************************************************************/
void rtptUtilsLog(
    IN rtptObj        *rtpt,
    IN rtptSessionObj    *call,
    IN const RvChar *logStr, ...)
{
#if defined(RVRTPSAMPLE_EMBEDDED)
    RV_UNUSED_ARG(rtpt);
    RV_UNUSED_ARG(call);
    RV_UNUSED_ARG(logStr);
#else
    RvChar msg[2048];
    va_list varArg;

    va_start(varArg, logStr);
    RvVsnprintf(msg, sizeof(msg), logStr, varArg);
    va_end(varArg);

    rtpt->cb.rtptLog(rtpt, call, msg);    
#endif
}


/******************************************************************************
 * rtptUtilsError
 * ----------------------------------------------------------------------------
 * General: Indicate the application that it can log an error message for its
 *          user.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt               - Endpoint object to use.
 *         call             - Call this log message belongs to (NULL if none).
 *         logStr           - Log string, in printf() formatting.
 * Output: None.
 *****************************************************************************/
void rtptUtilsError(
    IN rtptObj        *rtpt,
    IN rtptSessionObj    *call,
    IN const RvChar *logStr, ...)
{
#if defined(RVRTPSAMPLE_EMBEDDED)
    RV_UNUSED_ARG(rtpt);
    RV_UNUSED_ARG(call);
    RV_UNUSED_ARG(logStr);
#else
    RvChar msg[2048];
    va_list varArg;

    strcpy(msg, "Error: ");
    va_start(varArg, logStr);
    RvVsnprintf(msg+7, (RvSize_t)(sizeof(msg) - 7), logStr, varArg);
    RvVsnprintf(rtpt->lastError, sizeof(rtpt->lastError), logStr, varArg);
    va_end(varArg);

    rtpt->resetError = RV_FALSE;
    rtpt->lastError[sizeof(rtpt->lastError)-1] = '\0';
    rtpt->cb.rtptLog(rtpt, call, msg);
#endif
}


/******************************************************************************
 * rtptUtilsGetError
 * ----------------------------------------------------------------------------
 * General: Get and reset the last known error that occurred in the endpoint.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt               - Endpoint object to use.
 * Output: errStr           - Last known error string. This string is reset
 *                            by the call to this function.
 *****************************************************************************/
void rtptUtilsGetError(
    IN  rtptObj           *rtpt,
    OUT const RvChar    **errStr)
{
    if (rtpt->resetError)
    {
        *errStr = NULL;
    }
    else
    {
        *errStr = rtpt->lastError;
        rtpt->resetError = RV_TRUE;
    }
}








/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/






#ifdef __cplusplus
}
#endif
