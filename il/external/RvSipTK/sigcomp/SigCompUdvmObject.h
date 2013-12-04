/***********************************************************************
Filename   : SigCompUdvmObject.h
Description: UDVM machine object header file
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

#ifndef SIG_COMP_UDVM_OBJECT
#define SIG_COMP_UDVM_OBJECT

#if defined(__cplusplus)
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                   Additional header-files                             */
/*-----------------------------------------------------------------------*/
#include "RvSigComp.h"
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"
#include "SigCompCompartmentHandlerObject.h"
   
/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define MAX_SAVE_STATES 4
#define MAX_FREE_STATES 4
#define STATE_IDENTIFIER_SIZE 20

RV_DECLARE_HANDLE(SigCompRetParamsHandle);


/*
 *	Data structures responsible for state information bookkeeping
 */

typedef enum
{
    SAVE_STATE_REQUEST,
    FREE_STATE_REQUEST
} SigCompStateRequestType;

/**********************************************************************
* SigCompStateRequest
*
* ------------------------------------------------------------------------
* Description: 
* Common part for FreeState and SaveState requests
*  Holds request type and pointer to the next request
* 
*************************************************************************/
typedef struct _SigCompStateRequest
{
    SigCompStateRequestType     requestType;
    struct _SigCompStateRequest *pNext;
} SigCompStateRequest;

/**********************************************************************
* SigCompSaveStateRequest
* ------------------------------------------------------------------------
* Description: 
*
* Holds information pertinent to saved state. 
* Pay attention: only metainformation is saved. State value itself is 
* holded in UDVM memory (see STATE-CREATE instruction in RFC3320, section 9.4.6)
*	STATE-CREATE (%state_length, %state_address, %state_instruction,
*		%minimum_access_length, %state_retention_priority)
* 
*************************************************************************/

typedef struct
{
    SigCompStateRequest base;
    RvUint16            stateLength;
    RvUint16            stateAddress;
    RvUint16            stateInstruction;
    RvUint16            minimumAccessLength;
    RvUint16            stateRetentionPriority;
    RvUint8             *stateIdentifier;
} SigCompSaveStateRequest;

/**********************************************************************
* SigCompFreeStateRequest
* ------------------------------------------------------------------------
* Description: 
* Holds information pertinent to released state. 
* Pay attention: only metainformation is saved. Partial identifier itself is holded
*  in UDVM memory (see STATE-FREE instruction, RFC3320, section 9.4.7)
*  STATE-FREE (%partial_identifier_start, %partial_identifier_length)
* 
*************************************************************************/

typedef struct
{
    SigCompStateRequest base;
    RvUint16            partialIdentifierStart;
    RvUint16            partialIdentifierLength;
} SigCompFreeStateRequest;

typedef enum
{
    UDVM_UNKNOWN,               /* no reason was specified          */
    UDVM_OK,
    UDVM_MEM_ACCESS,            /* memory access violation          */
    UDVM_INPUT,                 /* input problem                    */
    UDVM_RESOURCES,             /* resource allocation problem      */
    UDVM_ZERO_DIVISION,         /* Divide by zero condition         */
    UDVM_CPB_EXCEEDED,          /* Cycles per bit exceeded          */
    UDVM_ILLEGAL_OPERAND,       /* Illegal operand for instruction  */
	UDVM_STACK_UNDERFLOW,       /* Stack underflow condition        */
    UDVM_HUFFMAN_MATCH,         /* match wasn't found in INPUT-HUFFMAN */
    UDVM_HUFFMAN_BITS ,
    UDVM_STATE,                 /* State rules violation */
    UDVM_STATE_ACCESS,          /* State access error    */ 
    UDVM_MESSAGE_FORMAT         /* Illegal message format */          
} SigCompFailureReason;


/*
 *	Few additional UDVM commands were implemented for debugging support:
 *  WATCHPOINT, BREAK and TRACE instructions. They are enabled in 
 *  UDVM_DEBUG_SUPPORT mode. Following data structures was defined to 
 *  support this commands
 */
#ifdef UDVM_DEBUG_SUPPORT

#define UDVM_MAX_BREAKPOINTS 10
#define UDVM_MAX_WATCHPOINTS 10

/**********************************************************************
* SigCompUdvmWatchpoint
* ------------------------------------------------------------------------
* Description: 
* Single watchpoint definition
*************************************************************************/

typedef struct
{
    RvUint16 pc;
    RvUint16 index;
    RvUint16 startAddr;
    RvUint16 size;
    RvBool   bEnabled;
} SigCompUdvmWatchpoint;

/**********************************************************************
* SigCompUdvmBreakpoint
* ------------------------------------------------------------------------
* Description: 
* Single breakpoint definition
*************************************************************************/

typedef struct
{
    RvUint16 pc;
    RvBool   bEnabled;
} SigCompUdvmBreakpoint;

typedef void (*OutputDebugStringCb) (void *cookie,
                                     const char *debugString);


#endif /* UDVM_DEBUG_SUPPORT */

/**********************************************************************
* SigCompUdvm
* ------------------------------------------------------------------------
* Description: 
*   main UDVM structure
*************************************************************************/

typedef struct
{
    RvSigCompMgrHandle hManager;
	RvLogSource        *pLogSource;

    SigCompFailureReason reason;

    /* UDVM memory bookkeeping */
    RvUint8 *pUdvmMemory;
    RvUint32 udvmMemorySize;

    RvUint16 cyclesPerBit;
    RvUint32 totalCycles;
    RvUint32 targetCycles;
    RvUint32 totalInstructions;

    RvUint16 pc;                /* Program counter */

    /* State requests order bookkeeping */
    SigCompStateRequest *pFirstStateRequest;
    SigCompStateRequest *pLastStateRequest;

    /* Free state requests bookkeeping */
    RvInt nextFreeState;
    SigCompFreeStateRequest freeStateRequestList[MAX_FREE_STATES];

    /* Save state requests bookkeeping */
    RvInt nextSaveState;
    SigCompSaveStateRequest saveStateRequestList[MAX_SAVE_STATES];


    /* End-Message instruction bookkeeping 
     * This instruction is divided in time. 
     * Part of it is executed during the normal course of execution and
     *  other part on accepting valid compartment identifier
     * This fields are used to save UDVM state inbetween 
     */
    RvUint16 requestedFeedbackLocation;
    RvUint16 returnedParametersLocation;

    /*
     *  Input bits bookkeeping
     */
    RvUint32 bitsBuffer;/* buffered bits. This buffer can contain upto 23 bits */
    RvInt    curBits;   /* exact number of bits in the bitsBuffer */
    RvBool   pbit;      /* saved value of pbit */

#ifdef UDVM_DEBUG_SUPPORT
    /* Debugging support */
    SigCompUdvmBreakpoint breakPoints[UDVM_MAX_BREAKPOINTS];
    RvUint16 nBreakPoints;

    SigCompUdvmWatchpoint watchPoints[UDVM_MAX_WATCHPOINTS];
    RvUint16 nWatchPoints;

    RvBool bTraceOn;

    OutputDebugStringCb outputDebugStringCb;
    void *outputDebugStringCookie;

#endif                          /* UDVM_DEBUG_SUPPORT */
} SigCompUdvm;

typedef enum
{
    UDVM_RUNNING,               /* END-MESSAGE instruction wasn't seen yet */
    UDVM_FINISHED,              /* UDVM finished all it's work and can be 
                                   safely destroyed */
    UDVM_WAITING               /* END-MESSAGE was reached, waiting for compartment
                                   ID to finalize */
#ifdef UDVM_DEBUG_SUPPORT
    ,UDVM_BREAK                 /* BREAK instruction was reached */
#endif
} SigCompUdvmStatus;


/***********************************************************************
* SigCompUdvmParameters
* ----------------------------------------------------------------------
* Description: 
*   Holds UDVM initialization parameters
*************************************************************************/

typedef struct
{
    RvUint8  *pUdvmMemory;
    RvUint32 udvmMemorySize;
    RvUint   cyclesPerBit;
} SigCompUdvmParameters;

typedef struct
{
    RvBool   bIndirect;
    RvUint16 value;
} SigCompDecodedOperand;

/**********************************************************************
* SigCompUdvmPostmortem
* ------------------------------------------------------------------------
* Description: 
* Holds postmortem information
*************************************************************************/

typedef struct
{
    SigCompFailureReason failureReason;
    RvUint16 pc;
    RvUint32 totalCycles;
    RvUint32 totalInstructions;
    RvBool   bDecodeInstruction;
    RvUint8  lastInstruction;
    SigCompDecodedOperand decodedOperands[10];
} SigCompUdvmPostmortem;



/*-----------------------------------------------------------------------*/
/*                    Whole Module Functions                             */
/*-----------------------------------------------------------------------*/

/**********************************************************************
* SigCompUdvmCreateFromState
* ------------------------------------------------------------------------
* Description: 
*  Creates fresh instance of UDVM machine from the previously saved state 
*  given by partial state identifier and prepare it for running
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*  pSelf - points to this instance of UDVM machine
*  pPartialState - point to the partial state identifier
*  partialStateLength - size of the memory area pointed by pPartialState
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successful, error code if failed
*     
*************************************************************************/

RvStatus SigCompUdvmCreateFromState(
    IN SigCompUdvm           *pSelf,
    IN RvSigCompMgrHandle    hMgr,
    IN SigCompUdvmParameters *pParameters,
    IN RvUint8               *pPartialState,
    IN RvUint                 partialStateLength);

/**********************************************************************
* SigCompUdvmCreateFromCode
* ------------------------------------------------------------------------
* Description: 
*  Creates fresh instance of UDVM machine from the code sent inside SigComp 
*  message and prepare it to run
* 
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*	pSelf - points to the instance of SigCompUdvm object 
*   pCode - points to the start of code buffer
*   codeLen - size of the code buffer
*   destination - start address in UDVM memory for code loading
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successful, error code if failed
*     
*************************************************************************/

RvStatus SigCompUdvmCreateFromCode(
    IN SigCompUdvm           *pSelf,
    IN RvSigCompMgrHandle    hMgr,
    IN SigCompUdvmParameters *pParameters,
    IN RvUint8               *pCode,
    IN RvUint32               codeLen,
    IN RvUint32               destination);

/**********************************************************************
* SigCompUdvmRun
* ------------------------------------------------------------------------
* Description: 
*
* Run UDVM for the specified number of cycles. Cycles quota may be 
*  enlarged by consuming additional input
* 
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*	pSelf - points to the instance of SigCompUdvm object 
*   nCycles - initial cycles quota 
*
* OUTPUT:
*   pStatus - returns the status of UDVM:
*     UDVM_RUNNING - end-message instruction wasn't encountered 
*        (UDVM needs more cycles to decompress this message)
*     UDVM_OK - udvm finished, no state
*	   UDVM_WAITING,
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successful, error code if failed
*     
*************************************************************************/

RvStatus SigCompUdvmRun(IN SigCompUdvm        *pSelf,
                        IN RvInt32            nCycles,
                        OUT SigCompUdvmStatus *pStatus);


/**********************************************************************
* SigCompUdvmRunPostmortem
* ------------------------------------------------------------------------
* Description: 
*
* Run UDVM for the specified number of cycles. Cycles quota may be 
*  enlarged by consuming additional input
* 
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*	pSelf - points to the instance of SigCompUdvm object 
*   nCycles - number of cycles to run 
*    (-1 run till the end or till cycles per bit exhausted)
*
* OUTPUT: 
*   pPostmortem - pointer to SigCompPostmortem structure to be filled
*     with postmortem info
*   pStatus - returns the status of UDVM:
*     UDVM_RUNNING - end-message instruction wasn't encountered 
*        (UDVM needs more cycles to decompress this message)
*     UDVM_OK - udvm finished, no state
*	  UDVM_WAITING - udvm is waiting for valid compartment id to finish

*
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successful, error code if failed
*     
*************************************************************************/

RvStatus SigCompUdvmRunPostmortem(
    IN  SigCompUdvm           *pSelf,
    IN  RvInt32               nCycles,
    OUT SigCompUdvmStatus     *pStatus,
    OUT SigCompUdvmPostmortem *pPostmortem);

/**********************************************************************
* SigCompUdvmFinish
* ------------------------------------------------------------------------
* Description: 
*    Finalize UDVM run. This function will perform all actions related to the
*    second part of End-Message instruction such as:
*     1. Freeing and saving states
*     2. Forwarding returned parameters:
*          * cycles-per-bit of the peer endpoint
*          * decompression_memory_size of the peer endpoint
*          * state_memory_size of the peer
*          * list of partial state IDs that are locally available at the peer 
*            endpoint
*     3. Forwarding requestedfeedback specification:
*          S (state not needed) and I (local state items not needed) bits and
*          requested feedback item that will traverse as is in the opposite 
*          direction in the next compressed message to this peer
* 
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*	pSelf - points to the instance of the SigCompUdvm object 
*   hCompartmentId  - handle to compartment 
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successful, error code if failed
*     
*************************************************************************/


RvStatus SigCompUdvmFinish(IN SigCompUdvm                *pSelf,
                           IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
                           IN SigCompCompartment *pCompartment
                           );

/**********************************************************************
* SigCompUdvmDestruct
* ------------------------------------------------------------------------
* Description: 
*   Destructs UDVM machine given by pSelf pointer
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*	pSelf - points to the instance of SigCompUdvm object 
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*	void
*     
*************************************************************************/

void SigCompUdvmDestruct(IN SigCompUdvm *pSelf);




#ifdef UDVM_DEBUG_SUPPORT
void SigCompUdvmSetOutputDebugStringCb(SigCompUdvm          *pSelf,
                                       OutputDebugStringCb  cb,
                                       void                 *cookie);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* SIG_COMP_UDVM_OBJECT */
