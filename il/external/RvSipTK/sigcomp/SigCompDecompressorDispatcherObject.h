/***********************************************************************
Filename   : SigCompDecompressorDispatcherObject.h
Description: Header file for SigCompDecompressorDispatcherObject
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

#ifndef SIGCOMP_DECOMPRESSOR_DISPATCHER
#define SIGCOMP_DECOMPRESSOR_DISPATCHER

#define UDVM_STANDALONE 1

#define DECOMPRESS_DEBUG 1
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"
#include "rvmutex.h"
#include "AdsRa.h"
#include "SigCompUdvmObject.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_OUTPUT_SIZE 0x10000  
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#define MAX_UDVM_MACHINES 16384
#else
#define MAX_UDVM_MACHINES 1024
#endif
//SPIRENT END

#define MAX_FEEDBACK_ITEM_SIZE 127

typedef RvUint32 SigCompUdvmId;

/**********************************************************************
* SigCompPerUdvmBlock
* ------------------------------------------------------------------------
* Description: 
* SigCompPerUdvmBlock holds UDVM machine itself and some bookkeeping information 
*  related to it: decompression memory, timestamp, id.
*  Implementation note: It's important that 'decompressionMemory' field will be the
*  last field in this structure (it's actual size is unknown till runtime)
*************************************************************************/

typedef struct _SigCompPerUdvmBlock
{
    SigCompUdvm   udvmMachine;
    SigCompUdvmId udvmid;
    struct _SigCompDecompressorDispatcher *pDispatcher;
    RvUint32 timestamp;
    /* points to the next pending machine */
    struct _SigCompPerUdvmBlock *nextPendingMachine;    
    /* points to the previous pending machine */
    struct _SigCompPerUdvmBlock *prevPendingMachine;    
    RvUint32 returnedFeedbackItemSize;
    RvUint8  returnedFeedbackItem[MAX_FEEDBACK_ITEM_SIZE];
    RvUint32 udvmMemorySize;
    RvUint8 decompressionMemory[1];     /* Real size will be accepted at runtime */
} SigCompPerUdvmBlock;


typedef HRA SigCompPerUdvmBlockList;


/**********************************************************************
* SigCompOutputStream
* ------------------------------------------------------------------------
* Description: 
*   manages UDVM output
*************************************************************************/

typedef struct
{
    RvUint8 *pCur;
    /* Set to 1 if OUTPUT instruction was encountered during UDVM run even
    *  if this output was empty.
    */
    RvBool   bOutputEncountered;
    RvUint32 outputAreaSize;
    RvUint8 *outputArea;
} SigCompOutputStream;

/**********************************************************************
* SigCompInputStream
*
* ------------------------------------------------------------------------
* Description: 
*    manages UDVM input 
*************************************************************************/
typedef struct
{
    RvUint8 *pCur;         /* point to the next byte to read */
    RvUint8 *pLast;        /* points to the byte just beyond the last byte */
} SigCompInputStream;


/**********************************************************************
* SigCompDecompressorDispatcher
* ------------------------------------------------------------------------
* Description: 
*  Holds SigCompDecompressorDispatcher
*  Important assumption: in any given moment only one UDVM machine can be
*  in the running state, on the other hand, there can me some UDVM machine in the
*  pending (waiting for compartment identifier) state;
* 
*************************************************************************/

typedef struct _SigCompDecompressorDispatcher
{
    RvSigCompMgrHandle      hManager;
    RvLogMgr               *pLogMgr;
    RvLogSource            *pLogSource;
    RvMutex                lock;           /* A lock object */
    RvMutex                *pLock;
    
    RvUint32 maxUdvms;
    SigCompPerUdvmBlockList perUdvmBlocks; /* holds data that is unique for each 
                                            * UDVM
                                            */
    SigCompOutputStream     outputStream;      /* Manages output from udvm */
    SigCompInputStream      inputStream;        /* Manages udvm input */

    /* Pending UDVM management related fields */
    SigCompPerUdvmBlock    *firstPendingMachine;
    SigCompPerUdvmBlock    *lastPendingMachine;
    RvInt32 timeout;

    /* Size of decompression memory */
    RvInt32                 decompressionMemorySize;

    /* Message id management */
    RvUint32                curMessageId;

    RvInt32                 cyclesPerBit;
    RvInt32                 cyclesTotal;
    
    SigCompUdvmPostmortem   postmortemInfo;
    
#ifdef UDVM_DEBUG_SUPPORT
    OutputDebugStringCb outputDebugString;
    void *cookie;
#endif                          /* UDVM_DEBUG_SUPPORT */
} SigCompDecompressorDispatcher;


#if defined(UDVM_STANDALONE) && (UDVM_STANDALONE > 0)
#define UDVMAPI RVAPI
#else
#define UDVMAPI
#endif


/**********************************************************************
* SigCompDecompressorDispatcherConstruct
* ------------------------------------------------------------------------
* Description: 
*  Constructs fresh instance of DecompressorDispatcher
* ------------------------------------------------------------------------
* Arguments:
*   hManager - SigCompMgr handle
*
* ------------------------------------------------------------------------
* Returns:
*	RvStatus
*     
*************************************************************************/

UDVMAPI RvStatus SigCompDecompressorDispatcherConstruct(IN RvSigCompMgrHandle hManager);

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
                        IN RvUint32                       cyclesPerBit);


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
*	pSelf - points to the instance of SigCompDecompressorDispatcher object 
*
* OUTPUT: none
* ------------------------------------------------------------------------
* Returns:
*	void
*     
*************************************************************************/

UDVMAPI void SigCompDecompressorDispatcherDestruct(
	                                IN SigCompDecompressorDispatcher *pSelf);

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
*      RV_OK - if successfull, ERROR code otherwise
*     
*************************************************************************/

UDVMAPI RvStatus SigCompDecompressorDispatcherDecompressMessage(
    IN  SigCompDecompressorDispatcher *pSelf,
    IN  RvSigCompMessageInfo          *pMessageInfo,
    OUT RvBool                        *pOutputEncountered,
    OUT SigCompUdvmId                 *pUdvmId);


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
*      RV_OK - if successfull, ERROR code otherwise
*     
*************************************************************************/

UDVMAPI RvStatus SigCompDecompressorDispatcherDecompressMessagePostmortem(
	IN  SigCompDecompressorDispatcher *pSelf,
    IN  RvSigCompMessageInfo          *pMessageInfo,
    OUT RvBool                        *pOutputEncountered,
    OUT SigCompUdvmId                 *pUdvmId,
    OUT SigCompUdvmPostmortem         *pPostmortem);


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
*                       compartment request was denied, just free resources
*   udvmId - udvm id that was returned by the call to DecompressMessage 
*       method
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successfull, error code - otherwise
*     
*************************************************************************/

UDVMAPI RvStatus SigCompDecompressorDispatcherDeclareCompartment(
	IN SigCompDecompressorDispatcher *pSelf,
	IN SigCompCompartment            *pCompartment,
	IN SigCompUdvmId                 udvmId);

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
*	pSelf - points to the instance of object 
*
* OUTPUT:
*	pResources - resource structure to be filled
* ------------------------------------------------------------------------
* Returns:
*	RV_OK if successfull, error code if failed
*     
*************************************************************************/

RvStatus SigCompDecompressorDispatcherGetResources(
    IN SigCompDecompressorDispatcher            *pSelf,
    IN RvSigCompDecompressorDispatcherResources *pResources);


#ifdef UDVM_DEBUG_SUPPORT
void DecompressorDispatcherSetOutputDebugStringCb(
    IN SigCompDecompressorDispatcher *pSelf,
    IN OutputDebugStringCb           cb,
    IN void                          *cookie);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_DECOMPRESSOR_DISPATCHER */
