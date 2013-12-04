/***********************************************************************
Filename   : SigCompDecompressorDispatcherObject.c

Description: DecompressorDispatcher implementation

  Decompressor dispatcher orchestrates over decompression process.
  It accepts decompression request, parses compressed message,
  allocates and constructs fresh UDVM machine, builds input and output streams
  for it,  computes initial number of cycles to run and runs UDVM.

  At any given time decompressor dispatcher may hold zero or more
  instances of UDVM machine. Only one of them may be in the running state,
  the rest are in the pending state waiting for the valid compartment id to
  complete decompression process (create and free states, forward
  information to the state handler, etc).

  If UDVM completes decompression process and doesn't request compartment id,
  it's destroyed and freed.

  If compartment id wasn't accepted during some pre-configured time, associated
  UDVM is also destroyed and freed.


************************************************************************
      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
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


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "SigCompDecompressorDispatcherObject.h"
#include "SigCompMgrObject.h"
#include "SigCompCompartmentHandlerObject.h"
#include "SigCompCallbacks.h"
#include "SigCompCommon.h"
#include "SigCompDynZTime.h"

#include "rvclock.h"
#include "rvtimestamp.h"
#include <string.h> /* for memset & memcpy */


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define GET_RA_PRINT_FUNC(disp) (0)

/*
 * DDLOG macros family implements some logging shortcuts
 */

/* Fetches RvLogSource object */
#define DDLOG_SOURCE (pSelf->pLogSource)

/* Fetches RvLogMgr object */
#define DDLOG_MANAGER (pSelf->pLogMgr)

/* Log error macro with multiple parameters
 * Usage:
 *  DDLOG_ERROR((DDLOG_SOURCE, format,...));
 */

#define DDLOG_ERROR(params) RvLogError(DDLOG_SOURCE, params)

/* Log info macro with multiple parameters
 * Usage:
 *  DDLOG_INFO((DDLOG_SOURCE, format,...));
 */
#define DDLOG_INFO(params)  RvLogInfo(DDLOG_SOURCE, params)

/*
 *  Makes long function name from the short one
 */
#define FUNC_NAME(name) "SigCompDecompressorDispatcher" #name


/* Log function entry macro
 * Usage:
 *  DDLOG_ENTER(shortFunctionName);
 *  shortFunctionName - function name without object name, e.g
 *   short name of 'SigCompDecompressorDispatcherConstruct' is just 'Construct'
 *   and macros call should be:
 *
 *  DDLOG_ENTER(Construct);
 *
 * Pay attention: no quotes needed
 */
#define DDLOG_ENTER(funcName) \
RvLogEnter(DDLOG_SOURCE, (DDLOG_SOURCE, FUNC_NAME(funcName)))

/* Log function leave macro
 * Usage:
 *  DDLOG_LEAVE(shortFunctionName);
 */
#define DDLOG_LEAVE(funcName) \
RvLogLeave(DDLOG_SOURCE, (DDLOG_SOURCE, FUNC_NAME(funcName)))


/*
 *  Logs simple error message
 *  Arguments:
 *    func - short function name
 *    msg - message to log
 */
#define DDLOG_ERROR1(func, msg) DDLOG_ERROR((DDLOG_SOURCE, FUNC_NAME(func) " - " msg))

/*
 *  Logs simple info message
 *  Arguments:
 *    func - short function name
 *    msg - message to log
 */
#define DDLOG_INFO1(func, msg)  DDLOG_INFO((DDLOG_SOURCE, FUNC_NAME(func) " - " msg))

/*
 *  Logs simple error message and leave message afterwards
 */
#define DDLOG_ERROR_LEAVE(funcName, msg) \
{\
    DDLOG_ERROR1(funcName, msg); \
    DDLOG_LEAVE(funcName); \
}

#define UDVM_ID_BITS 10
#define UDVM_ID_MASK 0x3ff

/*
 *  Gets pointer to structure of type 'S' given pointer
 *    to the field named 'f'
 */
#define S_FROM_F(S, f) RV_GET_STRUCT(S, f, f)

/************************************************************************
* SigCompMsgParser
* ------------------------------------------------------------------------
* Description:
*   Holds parsed SigComp message
*
*************************************************************************/

typedef struct
{
    RvUint8  *pReturnedFeedbackItem;
    RvUint32 returnedFeedbackItemLen;
    RvUint8  *pStateId;
    RvUint32 stateIdLen;
    RvUint8  *pCode;
    RvUint32 codeLen;
    RvUint16 codeDestination;
    RvUint8  *pInputArea;
    RvUint16 inputAreaSize;
    RvLogSource *pLogSource;
} SigCompMsgParser;


/*-----------------------------------------------------------------------*/
/*                           HELPER FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
static void PerUdvmBlockDestruct(SigCompPerUdvmBlock *pSelf);

static void
DecompressorDispatcherFreeExpiredMachines(SigCompDecompressorDispatcher *pSelf,
                                          RvUint32                      curTime);


static RvStatus OutputStreamGetOutput(SigCompOutputStream *pSelf,
                                      RvUint8 **ppBuf,
                                      RvUint32 *pBufSize,
                                      RvBool *pOutputEncountered);

static RvStatus StreamsConstruct(SigCompDecompressorDispatcher *pSelf,
								 RvUint8					   *pInBuf,
								 RvUint16					    inBufSize,
								 RvUint8					   *pOutBuf,
								 RvUint32						outBufSize);

static SigCompCompartmentHandlerMgr*
    SigCompMgrGetCompartmentHandler(RvSigCompMgrHandle hMgr);


static RvStatus DecompressMessagePostmortem(
						IN    SigCompDecompressorDispatcher *pSelf,
						IN    SigCompMsgParser				*pParser,
						IN	  RvUint32						 curTime,
						INOUT RvSigCompMessageInfo			*pMessageInfo,
						INOUT SigCompPerUdvmBlock		   **pPub,
						OUT   RvBool                        *pOutputEncountered,
						OUT   SigCompUdvmId					*pUdvmId,
						OUT   SigCompUdvmPostmortem         *pPostmortem); 

static RvStatus SigCompMsgParse(SigCompMsgParser *pSelf,
                                RvUint8          *msg,
                                RvUint16         msgsize);


static void SigCompNotifyOnPeerMsg(IN SigCompCompartment *pCompartment); 

#define GET_PER_UDVM_BLOCK(i) \
    ((SigCompPerUdvmBlock *)RA_GetElement(pSelf->perUdvmBlocks, i))


/**********************************************************************
* SigCompDecompressorDispatcherConstructSa
* ------------------------------------------------------------------------
* Description:
*  Constructs fresh instance of DecompressorDispatcher
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - points to the SigCompDecompressorDispatcher instance being
*           constructed
*   hManager - SigCompMgr handle
*   maxUdvmMachines - maximal number of pending (waiting for compartment id)
*     UDVM machines
*   timeout - maximal lifetime of pending UDVM machine expressed in ms.
*     The actual lifetime may be longer than this timeout, because timer
*     expiration is checked only in DecompressDispatcher methods
*
*   decompressionMemorySize - maximal size of decompression memory
*   cyclesPerBit - maximal cyclesPerBit
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/


UDVMAPI RvStatus SigCompDecompressorDispatcherConstructSa(
                        IN SigCompDecompressorDispatcher  *pSelf,
                        IN RvSigCompMgrHandle             hManager,
                        IN RvUint32                       maxUdvmMachines,
                        IN RvInt32                        timeout,
                        IN RvUint32                       decompressionMemorySize,
                        IN RvUint32                       cyclesPerBit)
{
    RvStatus crv;
    RvUint32 maxUdvmMemorySize;

    if(pSelf == 0 || maxUdvmMachines > MAX_UDVM_MACHINES)
    {
        return RV_ERROR_BADPARAM;
    }

    /* hManager == NULL - standalone mode */
    if(hManager != NULL)
    {
        SigCompMgr *pMgr = (SigCompMgr *)hManager;
        pSelf->pLogMgr = pMgr->pLogMgr;
        pSelf->pLogSource = pMgr->pLogSource;
    } else
    {
        pSelf->pLogMgr = 0;
        pSelf->pLogSource = 0;
    }

    DDLOG_ENTER(Construct);

    pSelf->cyclesPerBit				= cyclesPerBit;
    pSelf->decompressionMemorySize  = decompressionMemorySize;
    maxUdvmMemorySize  			    = RvMin(decompressionMemorySize, MAX_UDVM_MEMORY_SIZE);
    pSelf->firstPendingMachine		= NULL;
    pSelf->lastPendingMachine		= NULL;
    pSelf->hManager					= hManager;


    pSelf->timeout = (timeout + 500) / 1000;
    if(pSelf->timeout == 0)
    {
        pSelf->timeout = 1;
    }

    pSelf->curMessageId = 0;

    pSelf->maxUdvms = maxUdvmMachines;
	/* The UDVM block might be no more then 2^16 according to RFC3320, chapter 7 */
	/* Thus, the maximum element size is: the size of our internal use structure */
	/* plus no more than 2^16 (MAX_UDVM_MEMORY_SIZE) */
    pSelf->perUdvmBlocks =
        RA_Construct(sizeof(SigCompPerUdvmBlock) + maxUdvmMemorySize,
                     maxUdvmMachines, GET_RA_PRINT_FUNC(pSelf),
                     DDLOG_MANAGER, "perUdvmBlocks");

    if(pSelf->perUdvmBlocks == NULL)
    {
        DDLOG_ERROR_LEAVE(Construct, "unable to allocate UDVM machines");
        return RV_ERROR_UNKNOWN;
    }

    /* construct the lock/mutex of the manager itself */
    crv = RvMutexConstruct(pSelf->pLogMgr, &pSelf->lock);
    if (RV_OK != crv)
    {
        /* failed to construct lock */
        RvLogError(pSelf->pLogSource,(pSelf->pLogSource,
            "SigCompDecompressorDispatcherConstructSa - failed to construct lock of the manager (crv=%d)",
            crv));
        SigCompDecompressorDispatcherDestruct(pSelf);
        return(CCS2SCS(crv));
    }
    pSelf->pLock = &pSelf->lock;

#ifdef UDVM_DEBUG_SUPPORT
    pSelf->outputDebugString = 0;
    pSelf->cookie = 0;
#endif
    DDLOG_LEAVE(Construct);
    return RV_OK;
}

/**********************************************************************
* SigCompDecompressorDispatcherConstruct
* ------------------------------------------------------------------------
* Description:
*  Constructs fresh instance of DecompressorDispatcher
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   hManager - SigCompMgr handle
*
* ------------------------------------------------------------------------
* Returns:
*   RvStatus
*
*************************************************************************/

UDVMAPI RvStatus SigCompDecompressorDispatcherConstruct(IN RvSigCompMgrHandle hManager)
{
    SigCompMgr *pSCMgr = (SigCompMgr *)hManager;
    RvStatus   rv;

    if(hManager == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    RvLogEnter(pSCMgr->pLogSource,(pSCMgr->pLogSource, "SigCompDecompressorDispatcherConstruct"));

    rv = SigCompDecompressorDispatcherConstructSa(&pSCMgr->decompressorDispMgr,
                                                  hManager,
                                                  pSCMgr->maxTempCompartments,
                                                  pSCMgr->timeoutForTempCompartments,
                                                  pSCMgr->decompMemSize,
                                                  pSCMgr->cyclePerBit);

    if (rv != RV_OK) 
    {
        RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource, 
            "SigCompDecompressorDispatcherConstruct - object construction failed (rv=%d)",
            rv));
    }
    else 
    {
        RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource, 
            "SigCompDecompressorDispatcherConstruct - object constructed successfully (pDDMgr=%p)",
            &pSCMgr->decompressorDispMgr));
    }
    RvLogLeave(pSCMgr->pLogSource,(pSCMgr->pLogSource, "SigCompDecompressorDispatcherConstruct"));
    return rv;
}/* end of SigCompDecompressorDispatcherConstruct */


/**********************************************************************
* SigCompDecompressorDispatcherDestruct
* ------------------------------------------------------------------------
* Description:
*   Destructs the given instance of DecompressorDispatcher object and
*   releases all associated resources.
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - points to the instance of SigCompDecompressorDispatcher object
*
* ------------------------------------------------------------------------
* Returns:
*   void
*
*************************************************************************/

void SigCompDecompressorDispatcherDestruct(IN SigCompDecompressorDispatcher *pSelf)
{
    RvInt32 lockCount; /* dummy */

    lockCount = 0;

    if (NULL == pSelf)
    {
        return;
    }
    if (DDLOG_SOURCE != NULL) 
    {
        DDLOG_ENTER(Destruct);
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(!pSelf->pLock || !pSelf->pLogMgr)
		return;
#endif
/*SPIRENT_END */
    RvMutexLock(pSelf->pLock, pSelf->pLogMgr);
    
    if (NULL != pSelf->perUdvmBlocks)
    {
        RA_Destruct(pSelf->perUdvmBlocks);
    }

    /* release & destruct the lock/mutex of the manager itself */
    RvMutexRelease(pSelf->pLock, pSelf->pLogMgr, &lockCount);
    /* destruct the lock/mutex */
    RvMutexDestruct(pSelf->pLock, pSelf->pLogMgr);
    pSelf->pLock = NULL;

    if (DDLOG_SOURCE != NULL) 
    {
        RvLogInfo(pSelf->pLogSource,(pSelf->pLogSource, 
            "SigCompDecompressorDispatcherDestruct - object destructed (pDDMgr=%p)",
            pSelf));
        DDLOG_LEAVE(Destruct);
    }

    return;
} /* end of SigCompDecompressorDispatcherDestruct */


/************************************************************************
* SigCompDecompressorDispatcherDecompressMessage
* ------------------------------------------------------------------------
* Description:
* Accepts compressed message and decompresses it
*  After decompressing dispatcher may express its wish to save some state
*  information in the appropriate compartment by returning non-zero udvm id
*  to the caller.
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*      pSelf (in) - pointer to decompressor dispatcher object
*      pMessageInfo (inout) - pointer to RvSigCompMessageInfo structure that
*          contains input and output buffers
* OUTPUT:
*      pOutputEncountered (out) - will be set to 1 if ANY output was encountered
*          (even zero-length).
*      pUdvmId (out) - accepts message id to create association between udvm and
*          returned compartment. Will be set to NULL if no compartment needed
*
* Returns:
*      RV_OK - if successful, ERROR code otherwise
*
*************************************************************************/

RvStatus SigCompDecompressorDispatcherDecompressMessage(
                            IN  SigCompDecompressorDispatcher *pSelf,
                            IN  RvSigCompMessageInfo          *pMessageInfo,
                            OUT RvBool                        *pOutputEncountered,
                            OUT SigCompUdvmId                 *pUdvmId)
{
    RvStatus              rv;
    SigCompUdvmPostmortem *pPostmortem = &pSelf->postmortemInfo;

    DDLOG_ENTER(DecompressMessage);

    RvLogDebug(pSelf->pLogSource,(pSelf->pLogSource,
        "SigCompDecompressorDispatcherDecompressMessage - decompressing (compressedSize=%d)",
        pMessageInfo->compressedMessageBuffSize));

    RvMutexLock(pSelf->pLock, pSelf->pLogMgr);

    memset(pPostmortem, 0, sizeof(*pPostmortem));

    rv = SigCompDecompressorDispatcherDecompressMessagePostmortem(pSelf,
                                                                  pMessageInfo,
                                                                  pOutputEncountered,
                                                                  pUdvmId,
                                                                  pPostmortem);

    RvMutexUnlock(pSelf->pLock, pSelf->pLogMgr);

    if (rv != RV_OK) 
    {
        RvLogDebug(pSelf->pLogSource,(pSelf->pLogSource,
            "SigCompDecompressorDispatcherDecompressMessage - error decompressing (rv=%d udvmId=%d)",
            rv,
            *pUdvmId));
    }
    else 
    {
        RvLogDebug(pSelf->pLogSource,(pSelf->pLogSource,
            "SigCompDecompressorDispatcherDecompressMessage - finished decompressing (plainSize=%d udvmId=%d)",
            pMessageInfo->plainMessageBuffSize,
            *pUdvmId));
    }

    DDLOG_LEAVE(DecompressMessage);
    return rv;
} /* end of SigCompDecompressorDispatcherDecompressMessage */

/************************************************************************
* SigCompDecompressorDispatcherDecompressMessagePostmortem
* ------------------------------------------------------------------------
* Description:
* Accepts compressed message and decompresses it
*  After decompressing dispatcher may express its wish to save some state
*  information in the appropriate compartment by returning non-zero udvm id
*  to the caller.
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*      pSelf (in) - pointer to decompressor dispatcher object
*      pMessageInfo (inout) - pointer to RvSigCompMessageInfo structure that
*          contains input and output buffers
* OUTPUT:
*      pOutputEncountered (out) - will be set to 1 if ANY output was encountered
*          (even zero-length).
*      pUdvmId (out) - accepts message id to create association between udvm and
*          returned compartment. Will be set to NULL if no compartment needed
*
*      pPostmortem - filled with postmortem information
*
* Returns:
*      RV_OK - if successful, ERROR code otherwise
*
*************************************************************************/

RvStatus SigCompDecompressorDispatcherDecompressMessagePostmortem(
                        IN    SigCompDecompressorDispatcher *pSelf,
                        INOUT RvSigCompMessageInfo          *pMessageInfo,
                        OUT   RvBool                        *pOutputEncountered,
                        OUT   SigCompUdvmId                 *pUdvmId,
                        OUT   SigCompUdvmPostmortem         *pPostmortem)
{

    RvUint32             curTime;
    SigCompPerUdvmBlock  *pub = 0;
    RvStatus             rv   = RV_OK;
    RvStatus             crv  = RV_OK;
    SigCompMsgParser     parser;    /* Parses incoming compressed message */
    RvUint8              *pCompressedMessage   = pMessageInfo->pCompressedMessageBuffer;
    RvUint16             compressedMessageSize = (RvUint16) pMessageInfo->compressedMessageBuffSize;


    /* Sanity check */
    if(pSelf == NULL)
    {
        return RV_ERROR_BADPARAM;
    }


    /* Free expired machines */
    curTime = RvClockGet(0, 0);
    DecompressorDispatcherFreeExpiredMachines(pSelf, curTime);

    /* pass the logSource to facilitate logging inside the function */
    parser.pLogSource = pSelf->pLogSource;
    /* Parse compressed message */
    rv = SigCompMsgParse(&parser, pCompressedMessage, compressedMessageSize);

    /* If parsing failed - forward failure status to the caller */
    if(rv != RV_OK)
    {
        pPostmortem->failureReason = UDVM_MESSAGE_FORMAT;
        DDLOG_ERROR_LEAVE(DecompressMessagePostmortem, "message parsing failed");
        return rv;
    }

    /* Allocate new per-udvm block */
    crv = RA_Alloc(pSelf->perUdvmBlocks, (RA_Element *)&pub);

    if(crv != RV_OK)
    {
        pPostmortem->failureReason = UDVM_RESOURCES;
        DDLOG_ERROR_LEAVE(DecompressMessagePostmortem, "udvm allocation failed");
        return(CCS2SCS(crv));
    }

    ++pSelf->curMessageId;
    pub->pDispatcher = pSelf;
    
	rv = DecompressMessagePostmortem(
			pSelf,&parser,curTime,pMessageInfo,&pub,pOutputEncountered,pUdvmId,pPostmortem);

	if(rv != RV_OK && pub != NULL)
    {
        RA_DeAlloc(pSelf->perUdvmBlocks, (RA_Element) pub);
    }

    DDLOG_LEAVE(DecompressMessagePostmortem);
    return rv;
}

/************************************************************************
* FreeExpiredMachines
* ------------------------------------------------------------------------
* Description: Destroys and frees all expired udvm machines
*
* ------------------------------------------------------------------------
* Arguments:
*   pSelf - pointer to the instance of DecompressorDispatcher
*   curTime - current time in seconds
*
*************************************************************************/

static void DecompressorDispatcherFreeExpiredMachines(
                                    IN SigCompDecompressorDispatcher *pSelf,
                                    IN RvUint32                      curTime)
{

    SigCompPerUdvmBlock *cur = pSelf->firstPendingMachine;

    while(cur != NULL)
    {
        SigCompPerUdvmBlock *next;
        SigCompPerUdvmBlock *prev;

        if(cur->timestamp > curTime)
        {
            break;
        }

        next = cur->nextPendingMachine;
        prev = cur->prevPendingMachine;

        if(next != NULL)
        {
            next->prevPendingMachine = prev;
        } 
        else
        {
            pSelf->lastPendingMachine = prev;
        }

        if(prev != NULL)
        {
            prev->nextPendingMachine = next;
        } 
        else
        {
            pSelf->firstPendingMachine = next;
        }

        RvLogDebug(pSelf->pLogSource,(pSelf->pLogSource,
            "DecompressorDispatcherFreeExpiredMachines - temporary compartment expired (udvmId=%d)",
            cur->udvmid));
        PerUdvmBlockDestruct(cur);
        RA_DeAllocByPtr(pSelf->perUdvmBlocks, (RA_Element)cur);
        cur = next;
    }

    if(cur != NULL)
    {
        cur->prevPendingMachine = NULL;
    }

    pSelf->firstPendingMachine = cur;
}

/************************************************************************
* SigCompDecompressorDispatcherDeclareCompartment
* ------------------------------------------------------------------------
* Description:
*  Accepts compartment id associated with udvm id and performs various
*  post-compression operations:
*   1. saving/releasing state in associated compartment
*   2. forwarding returned and requested feedback item and a list of
*      returned parameters to the state handler and eventually to the
*      appropriate compressor
*   3. free associated resources
*
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - pointer to SigCompDecompressor object
*   pCompartment - pointer to compartment. if pCompartment == 0,
*                   compartment request was denied, just free resources
*   udvmId - udvm id that was returned by the call to DecompressMessage
*       method
*
* OUTPUT: none
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code - otherwise
*
*************************************************************************/

RvStatus SigCompDecompressorDispatcherDeclareCompartment(
                            IN SigCompDecompressorDispatcher *pSelf,
                            IN SigCompCompartment            *pCompartment,
                            IN SigCompUdvmId                 udvmId)
{
    RvUint32						udvmIdx = udvmId & UDVM_ID_MASK;
    SigCompPerUdvmBlock			   *pub;
    RvStatus						rv;
    
    /* Sanity check */
    if(pSelf == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    DDLOG_ENTER(DeclareCompartment);
    
    if(udvmIdx >= pSelf->maxUdvms || udvmId == 0)
    {
        RvLogError(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherDeclareCompartment - comp %p got unexpected udvmID = %d",pCompartment,udvmId));
        DDLOG_ERROR_LEAVE(DeclareCompartment, "bad param");
        return RV_ERROR_BADPARAM;
    }


    /* Check if this compartment id belongs to udvm that already doesn't exist */
    if(RA_ElementIsVacant(pSelf->perUdvmBlocks, udvmIdx))
    {
        RvLogWarning(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherDeclareCompartment - comp %p udvm doesn't exist - element is vacant",pCompartment));
        DDLOG_LEAVE(DeclareCompartment);
        return RV_ERROR_UNKNOWN;
    }

    pub = (SigCompPerUdvmBlock *) RA_GetElement(pSelf->perUdvmBlocks, udvmIdx);

    if(pub == NULL)
    {
        /* This really shouldn't happen */
        DDLOG_ERROR_LEAVE(DeclareCompartment, "null udvm");
        return RV_ERROR_UNKNOWN;
    }

    /* We found a udvm at the given index, but its' udvm id differs from our's */
    if(pub->udvmid != udvmId)
    {
        RvLogError(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherDeclareCompartment - comp %p got udvmID mismatch (pub->udvmid=%d udvmId=%d)",
            pCompartment,pub->udvmid, udvmId));
        DDLOG_ERROR_LEAVE(DeclareCompartment, "udvm doesn't exist");
        return RV_ERROR_UNKNOWN;
    }

    RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
        "SigCompDecompressorDispatcherDeclareCompartment - comp %p legal udvmID = %d",pCompartment,udvmId));

    /* Compartment id accepted */
    if(pCompartment != 0 && pSelf->hManager != 0)
    {
        SigCompCompartmentHandlerMgr *pCompartmentHandler =
            SigCompMgrGetCompartmentHandler(pSelf->hManager);

        pCompartment->nIncomingMessages++;
        pCompartment->lastIncomingMessageTime = SigCompDynZTimeSecGet();
        rv = SigCompUdvmFinish(&pub->udvmMachine,
                               pCompartmentHandler,
                               pCompartment);

        RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherDeclareCompartment - comp %p, udvm finished %d (rv=%d)",
            pCompartment,pub->udvmid,
            rv));

        /* Forward returned feedback */
        if(rv == RV_OK && pub->returnedFeedbackItemSize > 0)
        {
            rv = SigCompCompartmentHandlerForwardReturnedFeedbackItem(pCompartmentHandler,
                                                                      pCompartment,
                                                                      (RvUint16)pub->returnedFeedbackItemSize,
                                                                     pub->returnedFeedbackItem);
            if (rv != RV_OK) 
            {
                RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
                    "SigCompDecompressorDispatcherDeclareCompartment - comp %p, failed to forward returned feedback item (rv=%d)",
                    pCompartment,rv));
            }

        }

        RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherDeclareCompartment - forward returned feedback (hCompartment=0x%p returnedFeedbackItemSize=%d)",
            pCompartment,
            (RvUint16)pub->returnedFeedbackItemSize));

		/* Notify the application of a compartment incoming message */ 
		SigCompNotifyOnPeerMsg(pCompartment); 
    }

    /* Delete this pub from the list of pending machines */
    if(pub->prevPendingMachine == NULL)
    {
        pSelf->firstPendingMachine = pub->nextPendingMachine;
    } 
    else
    {
        pub->prevPendingMachine->nextPendingMachine = pub->nextPendingMachine;
    }

    if(pub->nextPendingMachine == NULL)
    {
        pSelf->lastPendingMachine = pub->prevPendingMachine;
    } 
    else
    {
        pub->nextPendingMachine->prevPendingMachine = pub->prevPendingMachine;
    }

    RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
        "SigCompDecompressorDispatcherDeclareCompartment - udvm gracefully closed (udvmId=%d)",
        pub->udvmid));
    PerUdvmBlockDestruct(pub);
    RA_DeAlloc(pSelf->perUdvmBlocks, (RA_Element) pub);
    DDLOG_LEAVE(DeclareCompartment);
    return RV_OK;
} /* end of SigCompDecompressorDispatcherDeclareCompartment */

/**********************************************************************
* SigCompDecompressorDispatcherGetResources
* ------------------------------------------------------------------------
* Description:
*   Returns resource consumption statistics of
*   SigCompDecompressorDispatcher object
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - points to the instance of object
*
* OUTPUT:
*   pResources - resource structure to be filled
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

RvStatus SigCompDecompressorDispatcherGetResources(
                        IN  SigCompDecompressorDispatcher             *pSelf,
                        OUT RvSigCompDecompressorDispatcherResources  *pResources)
{
    RvStatus rv;
    RV_Resource resource;
    RvSigCompResource *pResource;

    DDLOG_ENTER(GetResources);

    memset(pResources, 0, sizeof(*pResources));
    pResource = &pResources->udvmRA;

    if(pSelf->perUdvmBlocks == NULL)
    {
        DDLOG_LEAVE(GetResources);
        return RV_OK;
    }

    rv = RA_GetResourcesStatus(pSelf->perUdvmBlocks, &resource);
    if(rv != RV_OK)
    {
        RvLogError(DDLOG_SOURCE,(DDLOG_SOURCE,
            "SigCompDecompressorDispatcherGetResources - error getting resources (rv=%d)",
            rv));
        DDLOG_LEAVE(GetResources);
        return rv;
    }

    pResource->currNumOfUsedElements = resource.numOfUsed;
    pResource->maxUsageOfElements = resource.maxUsage;
    pResource->numOfAllocatedElements = resource.maxNumOfElements;

    DDLOG_LEAVE(GetResources);
    return RV_OK;
} /* end of SigCompDecompressorDispatcherGetResources */


static RvStatus StreamsConstruct(SigCompDecompressorDispatcher *pSelf,
								 RvUint8					   *pInBuf,
								 RvUint16					    inBufSize,
								 RvUint8					   *pOutBuf,
								 RvUint32						outBufSize)
{
	if(pInBuf == NULL || pOutBuf == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    pSelf->inputStream.pCur  = pInBuf;
    pSelf->inputStream.pLast = pInBuf + inBufSize;
	
	pSelf->outputStream.pCur = pSelf->outputStream.outputArea = pOutBuf;
    pSelf->outputStream.outputAreaSize	   = outBufSize;
    pSelf->outputStream.bOutputEncountered = 0;
	
    return RV_OK;
}

/*************************************************************************/

/*************************************************************************
* OutputStreamGetOutput
* ------------------------------------------------------------------------
* Description:
*   Returns output from decompression process as raw memory buffer
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - points to the OutputStream object
*
* OUTPUT:
*   ppBuf - will be filled with pointer to output buffer
*   pBufSize - pointer to the size of *ppBuf
*   pOutputEncountered - *pOutput will be set to 1 if ANY output
*       (even zero length) was encountered during decompression process
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*************************************************************************/

static RvStatus OutputStreamGetOutput(IN  SigCompOutputStream *pSelf,
                                      OUT RvUint8             **ppBuf,
                                      OUT RvUint32            *pBufSize,
                                      OUT RvBool              *pOutputEncountered)
{
    if(pSelf == 0)
    {
        return RV_ERROR_UNKNOWN;
    }

    if(ppBuf != 0)
    {
        *ppBuf = pSelf->outputArea;
    }

    if(pBufSize != 0)
    {
        *pBufSize = (RvUint32)(pSelf->pCur - pSelf->outputArea);
    }

    if(pOutputEncountered != 0)
    {
        *pOutputEncountered = pSelf->bOutputEncountered;
    }

    return RV_OK;
} /* end of OutputStreamGetOutput */


/*   drawing taken from RFC-3320

   0   1   2   3   4   5   6   7       0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
   | 1   1   1   1   1 | T |  len  |   | 1   1   1   1   1 | T |   0   |
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
   |                               |   |                               |
   :    returned feedback item     :   :    returned feedback item     :
   |                               |   |                               |
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
   |                               |   |           code_len            |
   :   partial state identifier    :   +---+---+---+---+---+---+---+---+
   |                               |   |   code_len    |  destination  |
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
   |                               |   |                               |
   :   remaining SigComp message   :   :    uploaded UDVM bytecode     :
   |                               |   |                               |
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
                                       |                               |
                                       :   remaining SigComp message   :
                                       |                               |
                                       +---+---+---+---+---+---+---+---+


   0   1   2   3   4   5   6   7       0   1   2   3   4   5   6   7
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
   | 0 |  returned_feedback_field  |   | 1 | returned_feedback_length  |
   +---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
                                       |                               |
                                       :    returned_feedback_field    :
                                       |                               |
                                       +---+---+---+---+---+---+---+---+

*/

#define TBIT_MASK 0x4;
#define PSTATE_LEN_MASK 0x3;



/*************************************************************************
* SigCompMsgParse
* ------------------------------------------------------------------------
* Description:
*   Parses SigComp message given by 'msg' parameter and fills pSelf
*   structure
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelf - points to structure to be filled with various SigComp message
*       fields
*   msg - points to compressed message
*   msgsize - size of the area pointed by msg
*
*************************************************************************/

static RvStatus SigCompMsgParse(IN SigCompMsgParser *pSelf,
                                IN RvUint8          *msg,
                                IN RvUint16         msgsize)
{

    RvUint8 *curpos = msg;
    RvUint8 *lastPos = msg + msgsize;
    RvUint8 firstByte;
    RvUint8 rfiFirst;
    RvUint32 psiLen;
    RvBool  tbit;
    RvStatus rv;

#define CHECKPOS(i) if((curpos + (i)) >= lastPos) {return RV_ERROR_UNKNOWN;}

    /* Sanity checks */
    if(pSelf == NULL || msg == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    CHECKPOS(0);
    firstByte = *curpos++;
    if(firstByte < (RvUint8)0xF8)
    {
        /* Not a SigComp message, shouldn't happen */
        RvLogWarning(pSelf->pLogSource,(pSelf->pLogSource,
            "SigCompMsgParse - not a SigComp message (firstByte=0x%.2X)",
            firstByte));
        return RV_ERROR_UNKNOWN;
    }

    memset(pSelf, 0, sizeof(*pSelf));
    tbit = firstByte & TBIT_MASK;
    psiLen = firstByte & PSTATE_LEN_MASK;
    CHECKPOS(0);
    rfiFirst = *curpos;

    if(tbit)
    {
        if(rfiFirst <= 127)
        {
            pSelf->returnedFeedbackItemLen = 1;
            pSelf->pReturnedFeedbackItem = curpos++;
        } else
        {
            size_t len = rfiFirst & 0x7F;

            pSelf->returnedFeedbackItemLen = (RvUint32)len;
            pSelf->pReturnedFeedbackItem = ++curpos;
            curpos += len;
        }
    } else
    {
        /* No feedback item in this message */
        pSelf->returnedFeedbackItemLen = 0;
        pSelf->pReturnedFeedbackItem = NULL;
    }

    /* partial state identifier found */
    if(psiLen)
    {
        pSelf->stateIdLen = 3 + 3 * psiLen;
        pSelf->pStateId = curpos;
        curpos += pSelf->stateIdLen;
    } else
    {
        size_t dest;
        size_t codeLen;
        
        CHECKPOS(1);
        codeLen = (*curpos << 4) + (*(curpos + 1) >> 4);

        curpos++;
        dest = (*curpos++) & 0x0F;
        if(!dest)
        {
            RvLogWarning(pSelf->pLogSource,(pSelf->pLogSource,
                "SigCompMsgParse - not a SigComp message (dest=0x%.2X)",
                dest));
            return RV_ERROR_UNKNOWN;
        }
        dest = (dest + 1) * 64;
        pSelf->codeDestination = (RvUint16) dest;
        pSelf->pCode		   = curpos;
        pSelf->codeLen		   = (RvUint32)codeLen;
        curpos				  += codeLen;
    }

    CHECKPOS(-1);
    pSelf->pInputArea = curpos;
    pSelf->inputAreaSize = (RvUint16) (msgsize - (curpos - msg));

    /* There should be partial state identifier specified or code */
    if ((pSelf->stateIdLen >= 6) || (pSelf->codeLen > 0)) 
    {
        rv = RV_OK;
    }
    else 
    {
    	rv = RV_ERROR_UNKNOWN;
        RvLogWarning(pSelf->pLogSource,(pSelf->pLogSource,
            "SigCompMsgParse - unexpected value (stateIdLen=%d codeLen=%d)",
            pSelf->stateIdLen, pSelf->codeLen));
    }

    return rv;
#undef CHECKPOS
} /* end of SigCompMsgParse */


static SigCompCompartmentHandlerMgr* SigCompMgrGetCompartmentHandler(
                                                  RvSigCompMgrHandle hMgr)
{
    SigCompMgr *pMgr = (SigCompMgr *)hMgr;

    if(pMgr == 0)
    {
        return 0;
    }

    return &pMgr->compartmentHandlerMgr;
}


static void PerUdvmBlockDestruct(IN SigCompPerUdvmBlock *pSelf)
{
    SigCompUdvmDestruct(&pSelf->udvmMachine);

    RvLogDebug(pSelf->pDispatcher->pLogSource,(pSelf->pDispatcher->pLogSource,
        "PerUdvmBlockDestruct - udvm destructed (udvmMachine=%p)",
        (&pSelf->udvmMachine)));

}



/************************************************************************
* UdvmReadBytes, UdvmWriteBytes, UdvmRequire
* ------------------------------------------------------------------------
* Description:
*   Implements abstract UDVM read/write methods based on InputStream and
*   OutputStream objects. All those methods just forwards requests to the
*   appropriate InputStream/OutputStream objects
*
* ------------------------------------------------------------------------
* Arguments:
*
*************************************************************************/

#ifdef UDVM_DEBUG_SUPPORT

void DecompressorDispatcherSetOutputDebugStringCb(
                                    SigCompDecompressorDispatcher *pSelf,
                                    OutputDebugStringCb           cb,
                                    void                          *cookie)
{
    pSelf->outputDebugString = cb;
    pSelf->cookie = cookie;
}
#endif


/************************************************************************
* DecompressMessagePostmortem
* ------------------------------------------------------------------------
* Description:
* Accepts compressed message and decompresses it
*  After decompressing dispatcher may express its wish to save some state
*  information in the appropriate compartment by returning non-zero udvm id
*  to the caller.
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*      pSelf (in) - pointer to decompressor dispatcher object
*      pMessageInfo (inout) - pointer to RvSigCompMessageInfo structure that
*          contains input and output buffers
* OUTPUT:
*      pOutputEncountered (out) - will be set to 1 if ANY output was encountered
*          (even zero-length).
*      pUdvmId (out) - accepts message id to create association between udvm and
*          returned compartment. Will be set to NULL if no compartment needed
*
*      pPostmortem - filled with postmortem information
*
* Returns:
*      RV_OK - if successful, ERROR code otherwise
*
*************************************************************************/
static RvStatus DecompressMessagePostmortem(
						IN    SigCompDecompressorDispatcher *pSelf,
						IN    SigCompMsgParser				*pParser,
						IN	  RvUint32						 curTime,
						INOUT RvSigCompMessageInfo			*pMessageInfo,
						INOUT SigCompPerUdvmBlock		   **pPub,
						OUT   RvBool                        *pOutputEncountered,
						OUT   SigCompUdvmId					*pUdvmId,
						OUT   SigCompUdvmPostmortem         *pPostmortem)
{
	RvStatus			  rv; 
	SigCompUdvmStatus     ustatus;
	RvUint32              udvmId;
	RvBool                bShouldWait;
	RvUint8              *pCompressedMessage = pMessageInfo->pCompressedMessageBuffer;
	
	SigCompUdvmParameters udvmParameters;
	RvUint32              initCycles;
    RvUint32              dms;
    RvUint32              ums;
    RvUint32              msgSize;

    msgSize = pMessageInfo->compressedMessageBuffSize;
    dms = pSelf->decompressionMemorySize;

    if(pMessageInfo->transportType & RVSIGCOMP_STREAM_TRANSPORT) {
        ums = dms >> 1;
    } else {
        if(dms <= msgSize) {
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }
        ums = dms - msgSize;
        ums = RvMin(ums, MAX_UDVM_MEMORY_SIZE);
    }

	rv = StreamsConstruct(pSelf,pParser->pInputArea,pParser->inputAreaSize,
		pMessageInfo->pPlainMessageBuffer,pMessageInfo->plainMessageBuffSize);
	if(rv != RV_OK)
	{
		DDLOG_ERROR1(DecompressMessagePostmortem, "input&output streams construction failed");
		return rv; 
	}


	udvmParameters.cyclesPerBit		 = pSelf->cyclesPerBit;
	udvmParameters.pUdvmMemory		 = (*pPub)->decompressionMemory;
    udvmParameters.udvmMemorySize    = ums;
	
	
	if(pParser->stateIdLen != 0)
	{
		RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - creating udvm from state (stateIdLen=%d stateID=0x%X 0x%X 0x%X)",
			pParser->stateIdLen,pParser->pStateId[0],pParser->pStateId[1],pParser->pStateId[2]));
		rv = SigCompUdvmCreateFromState(&(*pPub)->udvmMachine,
			pSelf->hManager,
			&udvmParameters,
			pParser->pStateId,
			pParser->stateIdLen);
		
	} 
	else
	{
		RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - creating udvm from code (codeLen=%d)",
			pParser->codeLen));
		rv = SigCompUdvmCreateFromCode(&(*pPub)->udvmMachine,
			pSelf->hManager,
			&udvmParameters,
			pParser->pCode, 
			pParser->codeLen,
			pParser->codeDestination);
		
	}
	
	if(rv != RV_OK)
	{
		RvLogWarning(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - udvm construction failed (rv=%d)",
			rv));
		return rv;
	}
	
#ifdef UDVM_DEBUG_SUPPORT
	SigCompUdvmSetOutputDebugStringCb(&(*pPub)->udvmMachine,
		pSelf->outputDebugString,
		pSelf->cookie);
#endif
	
	
	/* Compute initial cycles to run */
	initCycles = (RvUint32)(8 * (pParser->pInputArea - pCompressedMessage) + 1000) *
		pSelf->cyclesPerBit;
	
	RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
		"DecompressMessagePostmortem - start running udvm %d with initCycles=%d",
		(*pPub)->udvmid, initCycles));
	
	rv = SigCompUdvmRunPostmortem(&(*pPub)->udvmMachine,initCycles,&ustatus,pPostmortem);
	
	if(rv != RV_OK)
	{
		RvLogWarning(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - udvm failure returning from SigCompUdvmRunPostmortem"));
		return rv;
	}
	
	/* ustatus == UDVM_RUNNING - allocated cycles were exceeded */
	if(ustatus == UDVM_RUNNING)
	{
		RvLogWarning(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - udvm failure - allocated cycles were exceeded"));
		rv = RV_ERROR_DECOMPRESSION_FAILED;
		return rv;
	}
	
	/* if feedback item exists - copy it into per-udvm area */
	if(0 != ((*pPub)->returnedFeedbackItemSize = pParser->returnedFeedbackItemLen))
	{
		memcpy((*pPub)->returnedFeedbackItem,
			pParser->pReturnedFeedbackItem, 
			pParser->returnedFeedbackItemLen);
		
		RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - copy feedback item (returnedFeedbackItemLen=%d)",
			pParser->returnedFeedbackItemLen));
	}
	
	udvmId = 0;
	
	/* Should wait for compartment id */
	bShouldWait = (ustatus == UDVM_WAITING) || ((*pPub)->returnedFeedbackItemSize > 0);
	
	if(bShouldWait)
	{
	/* Insert current udvm in the list of pending UDVM machine and generate
	* appropriate udvmid from message id
		*/
		udvmId = pSelf->curMessageId;
		udvmId <<= UDVM_ID_BITS;
		udvmId += RA_GetByPointer(pSelf->perUdvmBlocks, (RA_Element) (*pPub));
		(*pPub)->udvmid    = udvmId;
		(*pPub)->timestamp = curTime + pSelf->timeout;
		if(pSelf->lastPendingMachine != NULL)
		{
			pSelf->lastPendingMachine->nextPendingMachine = (*pPub);
		}
		(*pPub)->nextPendingMachine = NULL;
		(*pPub)->prevPendingMachine = pSelf->lastPendingMachine;
		pSelf->lastPendingMachine   = (*pPub);
		if(pSelf->firstPendingMachine == NULL)
		{
			pSelf->firstPendingMachine = (*pPub);
		}
		
		RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - expecting compartment (udvmId=%d)",
			udvmId));
	} 
	else
	{
	/* Destruct current udvm machine and free resources
	*
		*/
		RvLogDebug(DDLOG_SOURCE,(DDLOG_SOURCE,
			"DecompressMessagePostmortem - no need for compartment, udvm destructed (udvmId=%d)",
			(*pPub)->udvmid));
		PerUdvmBlockDestruct((*pPub));
		RA_DeAlloc(pSelf->perUdvmBlocks, (RA_Element) (*pPub));
		*pPub = NULL;
	}
	
	*pUdvmId = udvmId;
	rv = OutputStreamGetOutput(&pSelf->outputStream, 
		0,
		&pMessageInfo->plainMessageBuffSize,
		pOutputEncountered);
	
	DDLOG_LEAVE(DecompressMessagePostmortem);
	
	return rv;
}


/*************************************************************************
* SigCompNotifyOnPeerMsg
* ------------------------------------------------------------------------
* General: Notifies the application of a message, related to the given 
*		   compartment, has been received and decompressed 
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSigCompMgr - Pointer to the SigComp manager 
*************************************************************************/
static void SigCompNotifyOnPeerMsg(IN SigCompCompartment *pCompartment)
{
	RvSigCompCompartmentOnPeerMsgEv onPeerMsgCb;
	
	onPeerMsgCb = pCompartment->pCompressionAlg->evHandlers.pfnCompartmentOnPeerMsgEvHandler;
	if(onPeerMsgCb) {
		RvSigCompCompressionInfo compressionInfo;
		
		compressionInfo.pReturnedFeedbackItem    = pCompartment->returnedFeedbackItem;
		compressionInfo.returnedFeedbackItemSize = pCompartment->rtFeedbackItemLength;
		compressionInfo.bResetContext            = pCompartment->bSendBytecodeInNextMessage;
		
		compressionInfo.bStaticDic3485Mandatory  = pCompartment->pCHMgr->pSCMgr->bStaticDic3485Mandatory;
		
		compressionInfo.remoteSMS = pCompartment->remoteSMS;
		compressionInfo.remoteDMS = pCompartment->remoteDMS;
		compressionInfo.remoteCPB = pCompartment->remoteCPB;
		compressionInfo.pRemoteStateIds = pCompartment->remoteIdList;
		
		compressionInfo.localCPB = pCompartment->localCPB;
		compressionInfo.localDMS = pCompartment->localDMS;
		compressionInfo.localSMS = pCompartment->localSMS;
		
		
		compressionInfo.bSendLocalCapabilities = pCompartment->bSendLocalCapabilities;
		compressionInfo.bSendLocalStatesIdList = pCompartment->bSendLocalStatesIdList;
		
		compressionInfo.nIncomingMessages = pCompartment->nIncomingMessages;
		compressionInfo.lastIncomingMessageTime = pCompartment->lastIncomingMessageTime;
		SigCompCallbackCompartmentOnPeerMsgEv(
			pCompartment->pCHMgr->pSCMgr,pCompartment,&compressionInfo); 
	}
}


