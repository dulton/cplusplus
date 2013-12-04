#if (0)
******************************************************************************
Filename    :
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

#ifndef RV_SEM__H
#define RV_SEM__H

#include "rvtypes.h"
#include "rvplatform.h"

/* OS dependent semaphore interface */
/* The interfaces in the this file are not for general consumption. */
/* Use rvsem.h instead. */

/* WIN32 implementation */
#if defined(RV_SEMAPHORE_WIN32)

#include <windows.h>

#if defined (RV_OS_WINCE)
#include <winbase.h>
#else
#include <process.h>
#endif

typedef HANDLE RvSem_;

#define rvSemConstruct_(s, i) ((*(s) = CreateSemaphore(0, (RvInt)(i), 0xFFFFFFF, 0)), (s))
#define rvSemDestruct_(s)  (CloseHandle(*(s)))
#define rvSemWait_(s)   (WaitForSingleObject(*(s), INFINITE))
#define rvSemPost_(s)   (ReleaseSemaphore(*(s), 1, 0))

/* POSIX implementation */
#elif defined(RV_SEMAPHORE_POSIX)

#include <semaphore.h>

typedef sem_t RvSem_;

#define rvSemConstruct_(s, i) ((sem_init((s), 0, (i))), (s))
#define rvSemDestruct_(s)  (sem_destroy(s))
#define rvSemPost_(s)   (sem_post(s))
RvInt rvSemWait_(RvSem_ *s);

/* VxWorks implementation */
#elif defined(RV_SEMAPHORE_VXWORKS)
#include "semLib.h"

typedef SEM_ID RvSem_;

#define rvSemConstruct_(m, i) ((*(m) = semCCreate(SEM_Q_PRIORITY, (i))), (m))
#define rvSemDestruct_(m)  (semDelete(*(m)))
#define rvSemWait_(m)   (semTake(*(m), WAIT_FOREVER))
#define rvSemPost_(m)   (semGive(*(m)))


/* PSoS */
#elif defined(RV_SEMAPHORE_PSOS)
#include <psos.h>

typedef unsigned long RvSem_;

#define rvSemConstruct_(m, i) (sm_create("", (i), SM_LOCAL | SM_PRIOR ,(m)), (m))
#define rvSemDestruct_(m)  (sm_delete(*(m)))
#define rvSemWait_(m)   (sm_p(*(m), SM_WAIT, 0))
#define rvSemPost_(m)   (sm_v(*(m)))

#elif defined(RV_SEMAPHORE_OSE)
/* OSE implementation */

#include <ose.h>

typedef SEMAPHORE *RvSem_;
RvSem_ *rvSemConstruct_(RvSem_ *s, RvUint i);
#define rvSemDestruct_(s)  (kill_sem(*(s)))
#define rvSemWait_(s)   (wait_sem(*(s)))
#define rvSemPost_(s)   (signal_sem(*(s)))


/* Nucleus implementation */
#elif defined(RV_SEMAPHORE_NUCLEUS)
#include "nucleus.h"

typedef NU_SEMAPHORE RvSem_;

#define rvSemConstruct_(m, i) (NU_Create_Semaphore((m), "rvsema", (i), NU_PRIORITY))
#define rvSemDestruct_(m)  (NU_Delete_Semaphore((m)))
#define rvSemWait_(m)   (NU_Obtain_Semaphore((m), NU_SUSPEND))
#define rvSemPost_(m)   (NU_Release_Semaphore((m)))


#else
# error Unknown semaphore implementation
#endif

#endif

