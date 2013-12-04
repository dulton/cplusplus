/* rvadclock.c - POSIX adapter clock functions */
/************************************************************************
        Copyright (c) 2001 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/
#include "rvadclock.h"
#if (RV_CLOCK_TYPE == RV_CLOCK_VXWORKS)
#include <vxWorks.h>
#include <timers.h>
#else
#include <time.h>
#include <sys/time.h>
#endif


void RvAdClockInit(void)
{
}


void RvAdClockGet(
    OUT RvTime*         t)
{
    struct timespec ts;
    
    clock_gettime(CLOCK_REALTIME, &ts);
    RvTimeSetSecs(t, ts.tv_sec);
    RvTimeSetNsecs(t, ts.tv_nsec);
}


RvStatus RvAdClockSet(
    IN  const RvTime*   t)
{
    if(clock_settime(CLOCK_REALTIME, (struct timespec *)((void*)t)) != 0)
        return (RV_ERROR_UNKNOWN);

    return RV_OK;
}


void RvAdClockResolution(
    OUT RvTime*         t)
{
    clock_getres(CLOCK_REALTIME, (struct timespec *)((void*)t));
}
