#if (0)
******************************************************************************
Filename :
Description :
******************************************************************************
				Copyright (c) 1999 RADVision Inc.
************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
No part of this publication may be reproduced in any form whatsoever
without written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
******************************************************************************
$Revision: #1 $
$Date: 2011/08/05 $
$Author: songkamongkol $
******************************************************************************
#endif

#ifndef RV_SEM_H
#define RV_SEM_H

#include "rvsem_.h"

/* OS independent semaphore interface */

/*$
{type publish="no":
	{name: RvSem}
	{superpackage: RvKernel}
	{include: rvsem.h}
	{description:
		{p: This type represents a semaphore. The semaphore will block
			on a Wait if its count is zero. Posting increments the count,
			Wait decrements the count.}
	}
}
$*/
typedef RvSem_ RvSem;

#if defined(__cplusplus)
extern "C" {
#endif

RvSem* rvSemConstruct(RvSem *s, RvUint initialValue);
void rvSemDestruct(RvSem* s);
void rvSemWait(RvSem *s);
void rvSemPost(RvSem *s);

#if defined(__cplusplus)
}
#endif

#endif

