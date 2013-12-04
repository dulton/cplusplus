#if (0)
************************************************************************
Filename    : FILENAME
Description : SHORT DESCRIPTION OF THE FILE HERE
************************************************************************
                Copyright (c) 1999 RADVision Inc.
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

#include "rvsem.h"
/* SPIRENT_BEGIN */
//#include "rvlog.h"

#define rvLogEnter(l, txt)
#define rvLogLeave(l, txt)
/* SPIRENT_END   */

/* OS independent interface */
/* Very minimal implementation. */

RvSem* rvSemConstruct(RvSem *s, RvUint initialValue) {
	rvLogEnter(&rvLog, "rvSemConstruct");
	rvSemConstruct_(s, initialValue);
	rvLogLeave(&rvLog, "rvSemConstruct");
	return s;
}

void rvSemDestruct(RvSem* s) {
	rvLogEnter(&rvLog, "rvSemDestruct");
	rvSemDestruct_(s);
	rvLogLeave(&rvLog, "rvSemDestruct");
}

void rvSemWait(RvSem *s) {
	rvLogEnter(&rvLog, "rvSemWait");
	rvSemWait_(s);
	rvLogLeave(&rvLog, "rvSemWait");
}

void rvSemPost(RvSem *s) {
	rvLogEnter(&rvLog, "rvSemPost");
	rvSemPost_(s);
	rvLogLeave(&rvLog, "rvSemPost");
}

