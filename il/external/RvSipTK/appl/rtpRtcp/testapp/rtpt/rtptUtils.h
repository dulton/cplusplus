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

#ifndef _RV_RTPT_UTILS_H_
#define _RV_RTPT_UTILS_H_

/***********************************************************************
rtptUtils.h

Utility functions widely used by this module.
***********************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rtptObj.h"




#ifdef __cplusplus
extern "C" {
#endif



/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/





/*-----------------------------------------------------------------------*/
/*                           FUNCTIONS HEADERS                           */
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
    IN rtptObj          *rtpt,
    IN const RvChar     *eventType,
    IN rtptSessionObj   *session,
    IN const RvChar     *eventStr, ...);


/******************************************************************************
 * rtptUtilsLog
 * ----------------------------------------------------------------------------
 * General: Indicate the application that it can log a message for its user.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - RTP Test object to use.
 *         session          - Session this event belongs to (NULL if none).
 *         logStr           - Log string, in printf() formatting.
 * Output: None.
 *****************************************************************************/
void rtptUtilsLog(
    IN rtptObj          *rtpt,
    IN rtptSessionObj   *session,
    IN const RvChar     *logStr, ...);


/******************************************************************************
 * rtptUtilsError
 * ----------------------------------------------------------------------------
 * General: Indicate the application that it can log an error message for its
 *          user.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - RTP Test object to use.
 *         session          - Session this event belongs to (NULL if none).
 *         logStr           - Log string, in printf() formatting.
 * Output: None.
 *****************************************************************************/
void rtptUtilsError(
    IN rtptObj          *rtpt,
    IN rtptSessionObj   *session,
    IN const RvChar     *logStr, ...);


/******************************************************************************
 * rtptUtilsGetError
 * ----------------------------------------------------------------------------
 * General: Get and reset the last known error that occurred in the endpoint.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - RTP Test object to use.
 * Output: errStr           - Last known error string. This string is reset
 *                            by the call to this function.
 *****************************************************************************/
void rtptUtilsGetError(
    IN  rtptObj         *rtpt,
    OUT const RvChar    **errStr);




#ifdef __cplusplus
}
#endif

#endif /* _RV_RTPT_UTILS_H_ */
