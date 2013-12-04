#if (0)
************************************************************************
Filename   :
Description:
************************************************************************
   Copyright (c) 2000 RADVision Inc.
   ************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
   No part of this publication may be reproduced in any form whatsoever
   without written prior approval by RADVision LTD..

   RADVision LTD. reserves the right to revise this publication and make
   changes without obligation to notify any person of such revisions or
   changes.
   ************************************************************************
   ************************************************************************
   $Revision: #1 $
   $Date: 2011/08/05 $
   $Author: songkamongkol $
   ************************************************************************
#endif

#include "rvsem_.h"

/* OS Speicifc Semaphore functions */

/* POSIX */
#if defined(RV_SEMAPHORE_POSIX)
#include <errno.h>
RvInt rvSemWait_(RvSem_ *s)
{
	RvInt status;

	/* If semaphore is woken up by a signal, just try again. */
	do {
		status = sem_wait(s);
	} while((status == -1) && (errno == EINTR));
	return status;
}
#endif

/* OSE */
#if defined(RV_SEMAPHORE_OSE)
RvSem_ *rvSemConstruct_(RvSem_ *s, RvUint i)
{
	*s = create_sem(i);
	return s;
}
#endif
