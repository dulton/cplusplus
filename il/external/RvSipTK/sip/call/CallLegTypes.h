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
 *                              <CallLegTypes.h>
 *
 * The CallLegTypes.h file contains all type definitions which are not API
 * for the Call-leg module.
 *
 * includes:
 * Session Timer Type definitaions
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Michal Mashiach               July 2002
 *********************************************************************************/


#ifndef CALL_LEG_TYPES_H
#define CALL_LEG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "_SipCommonTypes.h"
#include "RvSipSessionExpiresHeader.h"

/*-----------------------------------------------------------------------*/
/*                         MACRO DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* CallLegSessionTimerStatus
 * ---------------------------------
 * The session timer status enum indicates whether the appliaction
 * support the session timer.
*/
typedef enum
{
    RVSIP_CALL_LEG_SESSION_TIMER_UNDEFINED = -1, /*the application doesn't support session timer*/
    RVSIP_CALL_LEG_SESSION_TIMER_SUPPORTED      /*the application support session timer*/
} CallLegSessionTimerStatus;

/* The Session Timer structure*/
typedef struct
{
    SipTimer                                    hSessionTimerTimer; /* a timer handle */
    RvInt32                                     sessionExpires; /* the Session-Expires of this call*/
    RvSipSessionExpiresRefresherType            eRefresherType; /* the refresher side of this call */
    RvSipCallLegSessionTimerRefresherPreference eCurrentRefresher; /* the refresher side of this call*/
    RvInt32                                     minSE;/* the Min-SE header value of this call*/
    RvInt32                                     alertTime;/* the time in which the stack will
                                                             notify the application to send a refresh before the session ends */

    RvInt32                                     defaultAlertTime; /* the default time in which the stack will
                                                                     notify the application to send a refresh before the session ends
                                                                     in case of undefined alertTime */
    RvSipCallLegSessionTimerNegotiationReason   eNegotiationReason;
    RvBool                                      bAddReqMinSE;    /* Indication for a min-SE that was */
                                                                 /* set by the application, thus has */
                                                                 /* to added to added to each og req */
    RvBool                                      bAddOnceMinSE;   /* Indicates if the the min-SE must */
                                                                 /* be added to the next message but */
                                                                 /* only once (in the following msg) */
}CallLegSessionTimer;


/*CallLegNegotiationSessionTimer*/
/*--------------------------------*/
/* This structure save the session timer params of the call before this parameters become
   active in the call (in the case of re-INVITE or UPDATE before we send or receive 2xx) */
typedef struct
{
    RvInt32                                     sessionExpires; /* the Session-Expires of this call*/
    RvInt32                                     minSE;/* the Min-SE header value of this call*/
    RvSipCallLegSessionTimerRefresherPreference eRefresherPreference; /* the refresher side of this call*/
    RvSipSessionExpiresRefresherType            eRefresherType; /* the refresher side of this call */
    RvBool                                      bInRefresh; /* indicate whether the application call to
                                                                refresh function */
}CallLegNegotiationSessionTimer;


#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef CALL_LEG_TYPES_H */

