/***********************************************************************
Filename   : SigCompCompartmentHandlerObject.h
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


#ifndef SIGCOMP_COMPARTMENT_HANDLER_OBJECT_H
#define SIGCOMP_COMPARTMENT_HANDLER_OBJECT_H

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
#include "RvSigCompTypes.h"
#include "SigCompStateHandlerObject.h"
    
/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
typedef struct {
    struct _SigCompMgr *pSCMgr;
    HRA                hCompartmentRA;
    HRA                hAlgContextRA;
    RLIST_POOL_HANDLE  hStateListPool;
    RvUint32           maxStateMemoryPerCompartment; 
    RvMutex            lock;        /* A lock object */
    RvMutex            *pLock;
} SigCompCompartmentHandlerMgr;

typedef struct {
    RvUint32 dataSize; /* the size of the data in the state in bytes */
    HRA      curRA;
    RvUint8  *pData;
} SigCompAlgContext;

  
typedef struct {
    SigCompCompartmentHandlerMgr *pCHMgr;
    HRA                  curRA;
    RLIST_HANDLE         hStatesList; /* list of states related to the compartment */
    RvUint32             availableStateMemory; /* amount of memory available for the compartment*/
    RvUint16             counter; /* for determining order of state creation (see SigCompCompartmentState) */
    RvMutex              lock;      /* A lock object */
    /* compression algorithm related */
    RvSigCompAlgorithm   *pCompressionAlg; /* the algorithm to be used for compression */
    SigCompAlgContext    algContext; /* the context for the compression algorithm */
    RvUint8              requestedFeedbackItem[128]; /* return this item to remote EP (RFC-3320 section 7.1) */
    RvUint32             rqFeedbackItemLength;       /* the actual length of the requested feedback item */
    RvUint8              returnedFeedbackItem[128];  /* this item was returned by the remote EP (RFC-3320 section 7.1) */
    RvUint32             rtFeedbackItemLength;       /* the actual length of the returned feedback item */
    /* flags for controlling the content of the sent message */
    RvBool               bRequestedFeedbackItemReady; /* is feedback item waiting to be sent ? */
    RvBool               bReturnedFeedbackItemReady; /* is feedback item waiting ? */
    RvBool               bSendBytecodeInNextMessage;
    RvBool               bSendLocalCapabilities;
    RvBool               bSendLocalStatesIdList; /* when set to FALSE the message must not contain local states list (I-bit) */
    RvBool               bKeepStatesInCompartment; /* if set to FALSE, then the compartment may be cleared from its states (S-bit)*/
    /* local EP information - compartment specific */
    RvBool               bLocalCapabilitiesHaveChanged;
    RvUint32             localDMS;  /* Decompression Memory Size */
    RvUint32             localSMS;  /* State Memory Size         */
    RvUint32             localCPB;  /* Cycles Per Bit            */
    /* remote EP information */
    RvBool               bRemoteCapabilitiesHaveChanged;
    RvUint32             remoteSigCompVersion;
    RvUint32             remoteDMS; /* Decompression Memory Size */
    RvUint32             remoteSMS; /* State Memory Size         */
    RvUint32             remoteCPB; /* Cycles Per Bit            */
    RvUint8              remoteIdList[512]; /* = ((39 local states) * 
                                                  (1 byte for length + 12 bytes for max partial ID) 
                                                  + 5 bytes to round to a whole number) */
    RvUint               nIncomingMessages;  /* Number of incoming messages so far */
    RvUint32             lastIncomingMessageTime;    /* Time of last message arrival */
} SigCompCompartment;


typedef struct {
    RLIST_HANDLE hStatesList; /* list of states related to the compartment */
    RvUint16     stateRetentionPriority; /* RFC-3320, section 6.2.5. page 22 */
    RvUint16     counter; /* for determining order of state creation (see reference above) */
    SigCompState *pState; /* pointer to the state itself */
} SigCompCompartmentState;


/*-----------------------------------------------------------------------*/
/*                    Whole Module Functions                             */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompCompartmentHandlerConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the compartment handler ,there 
*          is one compartment handler per sigComp entity and therefore it is 
*          called upon initialization of the sigComp module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:     hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RvStatus SigCompCompartmentHandlerConstruct(IN RvSigCompMgrHandle hSigCompMgr);

/*************************************************************************
* SigCompCompartmentHandlerDestruct
* ------------------------------------------------------------------------
* General: destructor of the compartment handler, will be called 
*          when destructing the sigComp module
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler
*************************************************************************/
void SigCompCompartmentHandlerDestruct(IN SigCompCompartmentHandlerMgr *pCHMgr);

/*************************************************************************
* SigCompCompartmentHandlerGetResources
* ------------------------------------------------------------------------
* General: retrieves the resource usage of the CompartmentHandler
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:     *pCHMgr - a pointer to the compartment handler
*
* Output:    *RvSigCompCompartmentHandlerResources - a pointer a resource structure
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetResources(
                     IN  SigCompCompartmentHandlerMgr         *pCHMgr,
                     OUT RvSigCompCompartmentHandlerResources *pResources);

/*************************************************************************
* SigCompCompartmentHandlerCreateComprtment
* ------------------------------------------------------------------------
* General: allocates a new compartment from the pool
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler
*
* Output:   **ppCompartment - a pointer to pointer to the compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerCreateComprtmentEx(
                     IN  SigCompCompartmentHandlerMgr *pCHMgr,
                     IN  RvChar                       *algoName,
                     OUT SigCompCompartment           **ppCompartment);

#define SigCompCompartmentHandlerCreateComprtment(compartmentMgr, pCompartment) \
    SigCompCompartmentHandlerCreateComprtmentEx(compartmentMgr, 0, pCompartment)


/*************************************************************************
* SigCompCompartmentHandlerGetComprtment
* ------------------------------------------------------------------------
* General: gets the compartment from its handle
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler Mgr
*           hCompartment - The handle of the compartment
*
* Output:   **ppCompartment - a pointer to the this compartment in memory
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetComprtment(
                           IN  SigCompCompartmentHandlerMgr *pCHMgr,
                           IN  RvSigCompCompartmentHandle   hCompartment, 
                           OUT SigCompCompartment           **ppCompartment);

/*************************************************************************
* SigCompCompartmentHandlerSaveComprtment
* ------------------------------------------------------------------------
* General: saves the given compartment, this function also "unlocks"
*          the compartment
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the session's compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerSaveComprtment(
                           IN SigCompCompartmentHandlerMgr *pCHMgr,
                           IN SigCompCompartment           *pCompartment);

/*************************************************************************
* SigCompCompartmentHandlerDeleteComprtment
* ------------------------------------------------------------------------
* General: deletes the given compartment, this function is called when the
*          application asks to close a compartment
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler
*           hCompartment - The handle of the session's compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerDeleteComprtment(
                           IN SigCompCompartmentHandlerMgr *pCHMgr,
                           IN RvSigCompCompartmentHandle   hCompartment);

/*************************************************************************
* SigCompCompartmentHandlerGetComprtmentSize
* ------------------------------------------------------------------------
* General: get decompression bytecode
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           hCompartment - The handle to the compartment
*
* Output    *pCompartmentSize - the size of the given state
*************************************************************************/
RvStatus SigCompCompartmentHandlerGetComprtmentSize(
                           IN  SigCompCompartmentHandlerMgr *pCHMgr,
                           IN  RvSigCompCompartmentHandle   hCompartment,
                           OUT RvUint32                     *pCompartmentSize);

/*************************************************************************
* SigCompCompartmentHandlerAddStateToCompartment
* ------------------------------------------------------------------------
* General: this function adds the given state to the given compartments list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           stateID - The ID of the state
*           statePriority - provided in the END_MESSAGE UDVM command (RFC-3320 section 9.4.9)
*           stateMinAccessLength - same as above parameter
*           stateAddress - same as above parameter
*           stateInstruction - same as above parameter
*           stateDataSize - The size of the content buffer in bytes
*           pfnComparator - state data compare callback function
*           *pComparatorCtx - the above function's context
*
* Output:   **ppStateData - pointer to pointer to the state's content buffer
*************************************************************************/
RvStatus SigCompCompartmentHandlerAddStateToCompartment(
                                   IN  SigCompCompartmentHandlerMgr  *pCHMgr,
                                   IN  SigCompCompartment            *pCompartment,
                                   IN  RvSigCompStateID                stateID,
                                   IN  RvUint16                      statePriority,
                                   IN  RvUint16                      stateMinAccessLength,
                                   IN  RvUint16                      stateAddress,
                                   IN  RvUint16                      stateInstruction,
                                   IN  RvUint16                      stateDataSize,
                                   IN  SigCompUdvmStateCompare       pfnComparator,
                                   IN  void                          *pComparatorCtx,
                                   OUT RvUint8                       **ppStateData);
                                                        

/*************************************************************************
* SigCompCompartmentHandlerRemoveStateFromCompartment
* ------------------------------------------------------------------------
* General: this function removes the given state from the given compartments list
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - The ID of the compartment
*           pPartialStateID - pointer to the partial (6/9/12/20) ID of the state
*           partialStateIdLength - length of the partial ID, can be one of: 6/9/12/20 bytes
*************************************************************************/
RvStatus SigCompCompartmentHandlerRemoveStateFromCompartment(
                                    IN SigCompCompartmentHandlerMgr *pCHMgr,
                                    IN SigCompCompartment           *pCompartment,
                                    IN RvUint8                      *pPartialStateID,
                                    IN RvUint32                     partialStateIdLength);

/*************************************************************************
* SigCompCompartmentHandlerPeerEPCapabilitiesUpdate
* ------------------------------------------------------------------------
* General: this function updates the algorithm structure with the
*          capabilities of the peer endpoint + chooses according to these
*          capabilities the appropriate compressing algorithm
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           remoteEpCapabilities - a byte holding the peer End Point (EP) capabilities:
*                            decompression memory size
*                            state memory size
*                            cycles per bit
*           sigcompVersion - version of the remote EP SigComp
*           *pLocalIdListSize - (in) requested size of list
*
* Output:   *pLocalIdListSize - (out) allocated size of list area (may be different than the requested value)
*           **ppLocalIdList - will contain a pointer to an area to hold the remote EP list of 
*                               locally available states (see RFC-3320 section 9.4.9. on structure of list)
*************************************************************************/
RvStatus SigCompCompartmentHandlerPeerEPCapabilitiesUpdate(
                           IN    SigCompCompartmentHandlerMgr *pCHMgr,
                           IN    SigCompCompartment           *pCompartment,
                           IN    RvUint8                      remoteEpCapabilities,
                           IN    RvUint8                      sigcompVersion,
                           INOUT RvUint32                     *pLocalIdListSize,
                           OUT   RvUint8                      **ppLocalIdList);

/*************************************************************************
* SigCompCompartmentHandlerClearBytecodeFlag
* ------------------------------------------------------------------------
* General: clears (reset) the resendBytecode flag of the given compartment,
*           This function may be used for recovery of connection with the remote peer
*           
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCHMgr - a handle to the compartment handler
*           hCompartment - The handle of the compartment
*************************************************************************/
RvStatus SigCompCompartmentHandlerClearBytecodeFlag(
                                 IN SigCompCompartmentHandlerMgr *pCHMgr,
                                 IN RvSigCompCompartmentHandle   hCompartment);


/*************************************************************************
* SigCompCompartmentHandlerForwardRequestedFeedback
* ------------------------------------------------------------------------
* General: this function handles the forward requested feedback.
*           It is called from the decompressor in response to directions embedded
*           in the incoming message (after the END_MESSAGE command).
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           bRemoteStateListIsNeeded - reflects the S_bit in the header 
*                                   of the Requested Feedback Data (RFD)
*           bLocalStatesListIsNeeded - reflects the I_bit in the header of the RFD
*           requestedFeedbackItemSize - range = 0..127
*                                       0 means no requested feedback found
*           *pRequestedFeedbackItem - the content of the item to be returned to 
*                                       the remote EP
*************************************************************************/
RvStatus SigCompCompartmentHandlerForwardRequestedFeedback(
                   IN SigCompCompartmentHandlerMgr *pCHMgr,
                   IN SigCompCompartment           *pCompartment,
                   IN RvBool                       bRemoteStateListIsNeeded,
                   IN RvBool                       bLocalStatesListIsNeeded,
                   IN RvUint16                     requestedFeedbackItemSize,
                   IN RvUint8                      *pRequestedFeedbackItem);

/*************************************************************************
* SigCompCompartmentHandlerForwardReturnedFeedbackItem
* ------------------------------------------------------------------------
* General: this function handles the forward requested feedback.
*           It is called from the decompressor in response to directions embedded
*           in the incoming message (after the END_MESSAGE command).
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCHMgr - a handle to the compartment handler
*           *pCompartment - pointer to the compartment
*           returnedFeedbackItemSize - range = 0..127
*           *pReturnedFeedbackItem - the content of the item returned from remote EP
*************************************************************************/
RvStatus SigCompCompartmentHandlerForwardReturnedFeedbackItem(
                           IN SigCompCompartmentHandlerMgr *pCHMgr,
                           IN SigCompCompartment           *pCompartment,
                           IN RvUint16                     returnedFeedbackItemSize,
                           IN RvUint8                      *pReturnedFeedbackItem);




                                                           
#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_COMPARTMENT_HANDLER_OBJECT_H */


