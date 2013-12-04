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
 *                              <SigCompMgrObject.h>
 *
 * This file defines contains all functions that calls to callbacks.
 *
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/

#ifndef SIGCOMP_MGR_OBJECT_H
#define SIGCOMP_MGR_OBJECT_H

#if defined(__cplusplus)
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "rvloglistener.h"

#include "RvSigComp.h"
#include "SigCompCompartmentHandlerObject.h"
#include "SigCompStateHandlerObject.h"
#include "SigCompCompressorDispatcherObject.h"
#include "SigCompDecompressorDispatcherObject.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define RV_SIGCOMP_LOGFILE_NAME "SigComp.log"
#define SIGCOMP_VERSION (0x01)
    
/* the following values are according to "RFC5049 and 3GPP TS 24.229 "               */
#define MAX_DECOMPRESSION_BYTECODE_SIZE  (4096) /*  4096 is the maximum for SIP/SigComp     */
#define MAX_UDVM_MEMORY_SIZE			(65536) /* 65536 is the maximum for SigComp (3320,section 7) */
#define MIN_DECOMPRESSION_MEMORY_SIZE    (8192) /*  8192 is the minimum for SIP/SigComp     */
#define MIN_STATE_MEMORY_SIZE            (4096) /*  4096 is the minimum for SIP/SigComp according to 3GPP TS 24.229 */
#define MIN_CYCLES_PER_BIT                 (16) /*    16 is the minimum for SIP/SigComp     */
    
#define LOG_SOURCE_NAME_LEN (13)
#define LOG_MSG_PREFIX      (24)

#define DEFAULT_SC_MEM_FOR_DICT3485          (0x12E4) /* as per RFC-3485 */
#define DEFAULT_SC_OPEN_COMPARTMENTS         (10)
#define DEFAULT_SC_TEMP_COMPARTMENTS         (10)
#define DEFAULT_SC_MAX_COMPARTMENTS          (DEFAULT_SC_OPEN_COMPARTMENTS + DEFAULT_SC_TEMP_COMPARTMENTS)
#define DEFAULT_SC_MAX_STATES_PER_COMP       (16)
#define DEFAULT_SC_TIMEOUT                   (5000)
#define DEFAULT_SC_DECMOP_MEM_SIZE           (MIN_DECOMPRESSION_MEMORY_SIZE)
#define DEFAULT_SC_STATE_MEM_SIZE            (MIN_STATE_MEMORY_SIZE)
#define DEFAULT_SC_CYCLES                    (MIN_CYCLES_PER_BIT)
#define DEFAULT_SC_SMALL_BUF_SIZE            (64)
#define DEFAULT_SC_SMALL_BUF_AMOUNT          (8 * DEFAULT_SC_MAX_COMPARTMENTS)
#define DEFAULT_SC_MED_BUF_SIZE              (512)
#define DEFAULT_SC_MED_BUF_AMOUNT            (2 * DEFAULT_SC_MAX_COMPARTMENTS)
#define DEFAULT_SC_LARGE_BUF_SIZE            (8192)
#define DEFAULT_SC_LARGE_BUF_AMOUNT          (1 * DEFAULT_SC_MAX_COMPARTMENTS)
#define DEFAULT_SC_DYN_MAX_TRIP_TIME		 (32)
#define DEFAULT_SC_DYN_MAX_STATES_NO		 (20)
#define DEFAULT_SC_DYN_MAX_TOTAL_STATES_SIZE (200000)

typedef struct _SigCompMgr {
    RvUint32 sigCompVersion;
    RvUint32 maxOpenCompartments;   /* Needed for memory allocation     */
    RvUint32 maxTempCompartments;   /* Compartments not yet allocated to a session */
    RvUint32 maxStatesPerCompartment; 
    RvUint32 timeoutForTempCompartments; /* in mSec */
    RvUint32 localIdListUsedLength; /* in bytes */
    RvUint8  localIdList[512];      /* = ((39 local states) *
                                         (1 byte for length + 12 bytes for max partial ID) 
                                         + 5 bytes to round to a whole number) */
    RvBool   bStaticDic3485Mandatory;
    /* The following 3 variables are derived from the machine/host CPU/memory capabilities */
    RvUint32 decompMemSize; /* 8192 is the minimum for SIP/SigComp > */
    RvUint32 stateMemSize;  /* 4096 is the minimum for SIP/SigComp according to 3GPP TS 24.229 */
    RvUint32 cyclePerBit;   /*   16 is the minimum for SIP/SigComp > */
    /*there will be three pools of HPAGEs for the states, their parameters are as follow:*/
    RvUint32 smallBufPoolSize;      /* size of buffers in small pool    */
    RvUint32 smallPoolAmount;       /* amount of buffers in small pool  */
    RvUint32 mediumBufPoolSize;     /* size of buffers in medium pool   */
    RvUint32 mediumPoolAmount;      /* amount of buffers in medium pool */
    RvUint32 largeBufPoolSize;      /* size of buffers in large pool    */
    RvUint32 largePoolAmount;       /* amount of buffers in large pool  */
    RLIST_HANDLE hDictionaryList;   /* A ptr to a list of dictionaries  */
    RLIST_HANDLE hAlgorithmList;    /* A ptr to a list of algorithms    */
    RvSigCompAlgorithm *defaultAlgorithm; /* pointer to default algorithm */
    RLIST_POOL_HANDLE hDictionaryListPool;
    RLIST_POOL_HANDLE hAlgorithmListPool;
    SigCompCompartmentHandlerMgr    compartmentHandlerMgr;/* compartment manager             */
    SigCompCompressionDispatcherMgr compressorDispMgr;    /* compressor dispatcher manager   */
    SigCompDecompressorDispatcher   decompressorDispMgr;  /* decompressor dispatcher manager */
    SigCompStateHandlerMgr          stateHandlerMgr;      /* state handler manager           */
    RvSigCompLogEntryPrint          pfnLogEntryPrint;
    void                            *logEntryPrintContext;
    RvBool                          bLogFuncProvidedByAppl;
    RvLogMgr                        logMgr;
    RvLogMgr                        *pLogMgr;
    RvBool                          bLogMgrProvidedByAppl;
    RvLogListener                   listener;
    RvLogSource                     *pLogSource;
    RvUint32                        logFilter;
    RvMutex                         lock;   /* A lock object */
    RvMutex                         *pLock;
	/* New version parameters */ 
	RvUint32						dynMaxTripTime; /* In seconds - Max lifetime of dynamic unacknowledged states */ 
	RvUint32						dynMaxStatesNo; /* The max no of dynamic compression states, a compartment can keep */ 
	RvUint32						dynMaxTotalStatesSize; /* The max size of the total dynamic compression states, a compartment can keep */
} SigCompMgr;



/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_MGR_OBJECT_H */
