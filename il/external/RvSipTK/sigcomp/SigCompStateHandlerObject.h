/***********************************************************************
Filename   : SigCompStateHandlerObject.h
Description: SigComp main internal interface header file
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


#ifndef SIGCOMP_STATE_HANDLER_OBJECT_H
#define SIGCOMP_STATE_HANDLER_OBJECT_H

#if defined(__cplusplus)
extern "C" {
#endif 
    
/*-----------------------------------------------------------------------*/
/*                   Additional header-files                             */
/*-----------------------------------------------------------------------*/
#include "rvmutex.h"
#include "AdsHash.h"
#include "AdsRa.h"
#include "AdsRlist.h"
#include "rpool_API.h"
#include "RvSigCompTypes.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
#define SIGCOMP_MINIMAL_STATE_ID_LENGTH (6)
typedef RvUint8 SigCompMinimalStateID[SIGCOMP_MINIMAL_STATE_ID_LENGTH]; /* minimal stateID for the hash-table key */

    
typedef struct {
    struct _SigCompMgr *pSCMgr; /* pointer to the parent */
    HASH_HANDLE hHashTable;  /* A hash table for holding pointers to states */
    HRA      smallBufRA;
    RvUint32 smallBufSize;   /* size of buffers in small pool   */
    RvUint32 smallBufAmount; /* amount of buffers in small pool */
    HRA      mediumBufRA;
    RvUint32 mediumBufSize;  /* size of buffers in medium pool   */
    RvUint32 mediumBufAmount;/* amount of buffers in medium pool */
    HRA      largeBufRA;
    RvUint32 largeBufSize;   /* size of buffers in large pool   */
    RvUint32 largeBufAmount; /* amount of buffers in large pool */
    HRA      statesRA;       /* a pool of state structures      */
    RvLogSource *pLogSrc;    /* The module log-id. Used for printing messages. */
    RvLogMgr    *pLogMgr;    /* A pointer to the common core log manager       */
    RvMutex     lock;        /* A lock object */
    RvMutex     *pLock;
} SigCompStateHandlerMgr;
  
typedef struct {
    /* information described in RFC-3320 p.14 */
    RvSigCompStateID stateID;     /* The SHA1 hash of the state content (20 bytes) */
    RvUint16 stateAddress;      /* (see p. 14 in RFC-3320) */
    RvUint16 stateInstruction;  /* (see p. 14 in RFC-3320) */
    RvUint16 minimumAccessLength; /* can have the following values 6/9/12/20  (see p. 14 in RFC-3320) */
    RvUint16 stateDataSize;     /* the size of the data in the state in bytes (state_length in RFC-3320, p.14) */
    RvUint8  *pData;            /* content buffer with length = state_length bytes (state_value in RFC-3320, p.14) */
    /* additional information needed for implementation */
    RvUint16 isUsed;            /* a counter for usage by compartments */
    RvUint16 retentionPriority; /* the priority for retention */
    RvUint32 stateBufSize;      /* the size of the buffer (S/M/L) in bytes */
    HRA      dataBufRA;         /* the RA from which the data buffer has been allocated */
    HRA      statesRA;          /* the RA from which the state has been allocated */
} SigCompState;


/* function prototype for comparing state's linear buffer data with the 
    UDVM's internal (cyclic buffer) representation of the data 
    Return value: RV_TRUE if the data is identical */
typedef RvBool (*SigCompUdvmStateCompare)(void *ctx, RvUint8 *pStateData, RvUint32 dataLength);


/*-----------------------------------------------------------------------*/
/*                    Whole Module Functions                             */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompStateHandlerConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the state handler ,there 
*          is one state handler per sigComp entity and therefore it is 
*          called upon initialization of the sigComp module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:     hSigCompMgr - A handle to the SigCompMgr context structure
*************************************************************************/
RvStatus SigCompStateHandlerConstruct(IN RvSigCompMgrHandle hSigCompMgr);

/*************************************************************************
* SigCompStateHandlerDestruct
* ------------------------------------------------------------------------
* General: destructor of the state handler, will be called 
*          when destructing the sigComp module
* Return Value: void
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a pointer to the state handler
*************************************************************************/
void SigCompStateHandlerDestruct(IN SigCompStateHandlerMgr *pSHMgr);


/*************************************************************************
* SigCompStateHandlerGetResources
* ------------------------------------------------------------------------
* General: retrieves the resource usage of the StateHandler
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:     *pSHMgr - a pointer to the state handler
*
* Output:    *pResources - a pointer a resource structure
*************************************************************************/
RvStatus SigCompStateHandlerGetResources(IN  SigCompStateHandlerMgr         *pSHMgr,
                                         OUT RvSigCompStateHandlerResources *pResources);

/*************************************************************************
* SigCompStateHandlerCreateState
* ------------------------------------------------------------------------
* General: creates a new state
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*            requestedSize - the requested size of the state in bytes
* Output:   **pState - a pointer to the this (empty) state in memory
*************************************************************************/
RvStatus SigCompStateHandlerCreateState(IN  SigCompStateHandlerMgr *pSHMgr,
                                        IN  RvUint32               requestedSize,
                                        OUT SigCompState           **pState);

/*************************************************************************
* SigCompStateHandlerCreateStateForDictionary
* ------------------------------------------------------------------------
* General: creates a new state for a dictionary.
*           The difference from the above "normal" creteState function
*           is that the dictionary-state data pointer is to an
*           "external" memory which is handled by the application
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*            requestedSize - the requested size of the state in bytes
*
* Output:   **pState - a pointer to the this (empty) state in memory
*************************************************************************/
RvStatus  SigCompStateHandlerCreateStateForDictionary(
                                         IN  SigCompStateHandlerMgr *pSHMgr,
                                         IN  RvUint32               requestedSize,
                                         OUT SigCompState           **ppState);

/*************************************************************************
* SigCompStateHandlerSaveState
* ------------------------------------------------------------------------
* General: saves the given state.
*           Note the usage of pState !
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           *pfnComparator - comparator function for UDVM cyclic buffer
*                            pass NULL to use memcmp (ansi function)
*           *pComparatorCtx - comparator function context
*                            pass NULL when using memcmp
*           **ppState - a pointer to the state structure.
*                       if the state already exists, the new state will be deleted
*                       and a pointer to the existing (identical) state will be returned.
*
* Output:   **ppState - a pointer to the state structure
*************************************************************************/
RvStatus SigCompStateHandlerSaveState(IN    SigCompStateHandlerMgr  *pSHMgr,
                                      IN    SigCompUdvmStateCompare pfnComparator,
                                      IN    void                    *pCtx,
                                      INOUT SigCompState            **ppState);



/*************************************************************************
* SigCompStateHandlerDeleteState
* ------------------------------------------------------------------------
* General: deletes the given state, this function is called when the
*          application asks to close a compartment ,per each state
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *pState - The pointer to the state
*           bIsHashed - has this state been hashed / used
*************************************************************************/
RvStatus SigCompStateHandlerDeleteState(IN SigCompStateHandlerMgr *pSHMgr,
                                        IN SigCompState           *pState,
                                        IN RvBool                 bIsHashed);
                                        
/*************************************************************************
* SigCompStateHandlerGetState
* ------------------------------------------------------------------------
* General: gets the given state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pSHMgr - a handle to the state handler
*           pPartialID - pointer to the ID of the state, can be a partial ID (6/9/12 bytes)
*           partialIdLength - length in bytes of partial ID (6/9/12/20 bytes)
*           bIgnoreAccessLength - TRUE if the GetState operation must ignore
*                                   the state's minimumAccessLength
*
* Output:   **ppState - a pointer to pointer to the state
*************************************************************************/
RvStatus  SigCompStateHandlerGetState(IN  SigCompStateHandlerMgr *pSHMgr,
                                      IN  RvUint8                *pPartialID,
                                      IN  RvUint32               partialIdLength,
                                      IN  RvBool                 bIgnoreAccessLength,
                                      OUT SigCompState           **ppState);


/*************************************************************************
* SigCompStateHandlerStateSetPriority
* ------------------------------------------------------------------------
* General: set state priority
*
* Return Value: RvStatus 
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           priority - state retention priority [0..65535]
*           *pState - pointer to the state for which the priority will be modified
*
* Output:   *pState - pointer to the state for which the priority will be modified
*************************************************************************/
RvStatus  SigCompStateHandlerStateSetPriority(IN    SigCompStateHandlerMgr *pSHMgr,
                                              IN    RvUint32               priority,
                                              INOUT SigCompState           *pState);

/*************************************************************************
* SigCompStateHandlerStateGetPriority
* ------------------------------------------------------------------------
* General: set state priority
*
* Return Value: RvStatus 
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *pState - pointer to the state
*
* Output:   *pPriority - pointer to a variable to be filled with 
*                        the state retention priority [0..65535]
*************************************************************************/
RvStatus  SigCompStateHandlerStateGetPriority(IN  SigCompStateHandlerMgr *pSHMgr,
                                              OUT RvUint32               *pPriority,
                                              IN  SigCompState           *pState);


/*************************************************************************
* SigCompStateHandlerStateGetID
* ------------------------------------------------------------------------
* General: get state ID
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*
* Output:   *pStateID - pointer to state ID
*************************************************************************/
RvStatus SigCompStateHandlerStateGetID(IN  SigCompStateHandlerMgr *pSHMgr,
                                       IN  SigCompState           *pState, 
                                       OUT RvSigCompStateID         *pStateID);

/*************************************************************************
* SigCompStateHandlerStateSetID
* ------------------------------------------------------------------------
* General: sets the state ID
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*
* Output    *pState - stateID field updated
*************************************************************************/
RvStatus SigCompStateHandlerStateSetID(IN SigCompStateHandlerMgr *pSHMgr,
                                       IN SigCompState           *pState);

/*************************************************************************
* SigCompStateHandlerStateGetSize
* ------------------------------------------------------------------------
* General: get state size
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*
* Output:   *pStateBufSize - return the allocated buffer size
*           *pStateDataSize - return the actual data size
*************************************************************************/
RvStatus SigCompStateHandlerStateGetSize(IN  SigCompStateHandlerMgr *pSHMgr,
                                         IN  SigCompState           *pState, 
                                         OUT RvUint32               *pStateBufSize,
                                         OUT RvUint32               *pStateDataSize);

/*************************************************************************
* SigCompStateHandlerStateReadData
* ------------------------------------------------------------------------
* General: Reads the raw data from the state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *state - pointer to the state
*           startLocation - byte location to start reading from
*           dataLength - number of bytes to read
*
* Output:   *pData - pointer allocated by the caller. 
*                    Will be filled with the state's data
*************************************************************************/
RvStatus SigCompStateHandlerStateReadData(IN  SigCompStateHandlerMgr *pSHMgr,
                                          IN  SigCompState           *pState, 
                                          IN  RvUint32               startLocation, 
                                          IN  RvUint32               dataLength,
                                          OUT RvUint8                *pData);

/*************************************************************************
* SigCompStateHandlerStateWriteData
* ------------------------------------------------------------------------
* General: Writes the raw data into the state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a handle to the state handler
*           *pState - pointer to the state
*           startLocation - byte location to start writing to
*           dataLength - number of bytes to write
*           *pData - raw data
*************************************************************************/
RvStatus SigCompStateHandlerStateWriteData(IN SigCompStateHandlerMgr *pSHMgr,
                                           IN SigCompState           *pState, 
                                           IN RvUint32               startLocation, 
                                           IN RvUint32               dataLength,
                                           IN RvUint8                *pData);



/*************************************************************************
* SigCompStateHandlerAlgContextDataAlloc
* ------------------------------------------------------------------------
* General: allocates an area for the algorithm-context data
*           Should be called once per compartment/algorithm
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a pointer to the state handler
*           requestedSize - the requested size in bytes
*
* Output:   **ppContextData - a pointer to pointer of the context's data
*           *pCurRA - the RA from which the buffer was allocated
*************************************************************************/
RvStatus SigCompStateHandlerAlgContextDataAlloc(
                            IN  SigCompStateHandlerMgr *pSHMgr,
                            IN  RvUint32               requestedSize,
                            OUT RvUint8                **ppContextData,
                            OUT HRA                    *pCurRA);


/*************************************************************************
* SigCompStateHandlerAlgContextDataDealloc
* ------------------------------------------------------------------------
* General: deallocates the algorithm-context data buffer, this function is called when the
*          application asks to close a compartment, or when changing algorithms
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pSHMgr - a pointer to the state handler
*           *pContext - a pointer to the context
*           curRA - the RA from which the buffer was allocated
*************************************************************************/
RvStatus SigCompStateHandlerAlgContextDataDealloc(
                            IN SigCompStateHandlerMgr *pSHMgr,
                            IN RvUint8                *pContextData,
                            IN HRA                    curRA);




#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_STATE_HANDLER_OBJECT_H */























