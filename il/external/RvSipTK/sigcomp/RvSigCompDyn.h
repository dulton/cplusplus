#ifndef RV_SIGCOMP_DYN_STATE_H
#define RV_SIGCOMP_DYN_STATE_H

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
 *                              <RvSigCompDynState.h>
 *
 * The RvSigCompDynState.h file contains all type definitions for the SigComp 
 * dynamic compression functionality.
 *
 * includes:
 * 1.Handle Type definitions
 * 2.SigComp Compartment Type definitions
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sasha G					    July 2006    
 *********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSigCompTypes.h"
	
/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define MAX_STATES_TO_REMOVE 4
#define DYNCOMP_RHEADER_SIZE 5
#define RVSIGCOMP_Z_FAKETIME 1

typedef RvUint32 RvSigCompZTime; 
typedef RvUint32 RvSigCompZTimeSec;

typedef RvUint16  RvSigCompDynZid;
typedef RvUint16  RvSigCompDynPrio;

typedef void RvSigCompDynZStream;

typedef struct {
    RvSigCompDynZid  zid;
    RvSigCompDynPrio prio;
    RvInt            wrapCnt;
} RvSigCompDynFid;


#define PRIO(z) ((z)->fid.prio)
#define ZID(z)  ((z)->fid.zid)

typedef struct _RvSigCompBytecodeDescr {
    RvInt minAccessLen;
    RvInt returnedParamsLocation;
    RvInt circularBufferStart;
    RvInt circularBufferSize;
    RvInt codeStart;
    RvInt stateStart;
    RvInt sipDictOffset;
    RvInt sipDictSize;
    RvInt stateLng;
    RvUint8 *byteCode;
    RvSize_t bytecodeSize;
} RvSigCompBytecodeDescr;

/***************************************************************************
 * RvSigCompDynZStreamAllocCB
 * ------------------------------------------------------------------------
 * General: A prototype for a new Z steam allocation callback. This callback
 *			will be triggered during SigComp Dynamic compression 
 *			processing. This function enable the dynamic compression algorithm
 *			to allocate new Z stream to be referred as a compartment 
 *			applicative context.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback registration
 *                         process.
 *		   ppStream      - The new Z stream to be allocated by the compression 
 *						   algorithm. 
 ***************************************************************************/
typedef RvStatus (*RvSigCompDynZStreamAllocCB)(IN  void					*ctx, 
											   OUT RvSigCompDynZStream **ppStream);


/***************************************************************************
 * RvSigCompDynZStreamFreeCB
 * ------------------------------------------------------------------------
 * General: A prototype for a Z steam release callback. This callback
 *			will be triggered during the SigComp Dynamic compression 
 *			processing. This function enable the dynamic compression algorithm
 *			to release a Z stream which is referred as a compartment 
 *			applicative context.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback registration
 *                         process.
 *		   ppStream      - The new Z stream to be released by the compression 
 *						   algorithm. 
 ***************************************************************************/
typedef void    (*RvSigCompDynZStreamFreeCB)(IN void *ctx, INOUT RvSigCompDynZStream *pStream);

/***************************************************************************
 * RvSigCompDynZStreamDestructCB
 * ------------------------------------------------------------------------
 * General: A prototype for a Z steam destruction callback. This callback
 *			will be triggered during the SigComp Dynamic compression 
 *			processing. This function enable the dynamic compression algorithm
 *			to destruct a Z stream which is referred as a compartment 
 *			applicative context.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback registration
 *                         process.
 *		   ppStream      - The new Z stream to be released by the compression 
 *						   algorithm. 
 ***************************************************************************/
typedef RvStatus (*RvSigCompDynZStreamDestructCB)(IN    void				*ctx, 
												  INOUT RvSigCompDynZStream *pstream);

/***************************************************************************
 * RvSigCompDynZStreamCompressCB
 * ------------------------------------------------------------------------
 * General: A prototype for a Z steam compression callback. This callback
 *			will be triggered during the SigComp Dynamic compression 
 *			processing. This function enable the dynamic compression algorithm
 *			to compress a Z stream which is referred as a compartment 
 *			applicative context.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback registration
 *                         process.
 *		   ppStream      - The new Z stream to be released by the compression 
 *						   algorithm. 
 ***************************************************************************/
typedef RvStatus (*RvSigCompDynZStreamCompressCB)(
									  IN  void				  *ctx, 
                                      IN  RvSigCompDynZStream *newStream, 
                                      IN  RvSigCompDynZStream *baseStream, 
                                      IN  RvUint8			  *plain,
                                      IN  RvSize_t			   plainSize,
                                      OUT RvUint8			  *compressed,
                                      OUT RvSize_t			  *pCompressedSize,
									  OUT RvSigCompStateID	   sid);

/********************************************************************************************
* RvSigCompDynZStreamVT
* -----------------------------------------------------------------------------
* A virtual table structure for keeping information of the current status of a 
* dynamic compression compartment. The Z stream is being "maintained" by the dynamic 
* compression algorithm and functions as an applicative context of an internal SigComp 
* compartment. 
*
* ctx       - A general context of the Z stream virtual table 
* bytecode  - Pointer to a bytecode which decompresses the dynamic compression algorithm msgs
* bytecodeSize - 
*			  The size of the bytecode buffer
* codeStart - The offset on the bytecode, which contains the first UDVM instruction to be
*			   executed during decompression 
* stateSize - The total dynamic compression state which is stored and updated in accordance 
*			   with a compartment
* minAccessLen - 
*			   The minimum length of state id used for locating and accessing it 
* localStateSize -
*             Estimation on memory size required to save compressor state. 
*             It may differ significantly from stateSize and usually much larger than former.
* returnedParamsLocation -
*             
* pfnZsalloc - 
*			   Algorithm callback used for allocating new Z stream. 
* pfnZsfree - Algorithm callback used for releasing a Z stream.
* pfnZsend  - Algorithm callback used for destructing a Z stream.
* pfnZscompress - 
*			   Algorithm callback used for compression a Z stream.
********************************************************************************************/
typedef struct {
    const RvChar                 *algoName;
    void						 *ctx;
    RvUint8						 *bytecode;
    RvSize_t					  bytecodeSize;
    RvUint16					  codeStart;
    RvSize_t					  stateSize;
    RvSize_t					  minAccessLen;
    RvSize_t                      localStateSize;
    RvUint16                      returnedParamsLocation;
    RvSigCompDynZStreamAllocCB    pfnZsalloc;
    RvSigCompDynZStreamFreeCB     pfnZsfree;
    RvSigCompDynZStreamDestructCB pfnZsend;
    RvSigCompDynZStreamCompressCB pfnZscompress;
} RvSigCompDynZStreamVT;

/********************************************************************************************
 * RvSigCompDynZ
 * -----------------------------------------------------------------------------
 * The Z structure is used for keeping information of the current status of a 
 * dynamic compression compartment. Each Z structure represents a single Z stream 
 * structure. The Z stream is being "maintained" by the dynamic 
 * compression algorithm and functions as an applicative context of an internal SigComp 
 * compartment. 
 *
 * dynState		- Pointer to the dynamic state which is related to the Z stream. The 
 *				  dynamic state is mostly used internally in the SigComp module
 * includesBytecode - 
 *			  	  Saved state includes bytecode also +
 * persistent	- Persistent state, e.g SIP dictionary, for example + 
 * marked       - Transient mark used in some functions + 
 * mayBeDeleted - Indication if the state may be deleted 
 * sid			- SHA-1 based stated id
 * fid			- Full state id 
 * dfid			- Dad's fid 
 * nRequests    - Number of states we asked to delete
 * removeRequests - 
 *				  States we asked to delete
 * createsState - if 1, this message creates state at peer
 * sendTime     - Send time in units of compressed messages in this compartment
 * ackedTime    - Acknowledgment time in units of compressed messages in this compartment 
 * sendTimeSec  - Send time in seconds
 * ackedTimeSec - Acknowledge time in seconds
 * pZdad        - Pointer to dad's Z stream
 * pZnext       - Pointer to next Z stream
 * pZfirstDependent - 
 *				  Points to the first unacknowledged dependent of this state 
 * pZnextDependent - 
 *				  Points to the next unacknowledged dependent in the dependents chain 
 *
 ********************************************************************************************/
typedef struct _RvSigCompDynZ     RvSigCompDynZ;
typedef struct _RvSigCompDynState RvSigCompDynState; 

struct _RvSigCompDynZ {
    RvSigCompDynZ      *pZdad;
    RvSigCompDynZ      *pZnext;   
    RvSigCompDynZ      *pZfirstDependent;       
    RvSigCompDynZ      *pZnextDependent;   

    RvBool			   includesBytecode;   
    RvBool			   persistent;         
    RvBool			   marked;             
    RvBool			   mayBeDeleted;     /* State may be deleted at peer */
    RvBool             servedAsActive;   /* True if this state was ever basis of compression */
    RvInt              nDeleteCount;     /* Number of time delete request was sent */

    RvSigCompStateID   sid;              
    RvSigCompDynFid	   fid;             
    RvSigCompDynFid    dfid;              

    RvSize_t		   nRequests;         
    RvSigCompDynFid	   removeRequests[4];  
    
    RvBool			   createsState;       
    
    RvSigCompZTime  sendTime;           
    RvSigCompZTime  ackedTime;          

    RvSigCompZTimeSec  sendTimeSec;      
    RvSigCompZTimeSec  ackedTimeSec;     
    RvSigCompZTimeSec  lastDependentTime;


    RvSize_t			 stateSize;                    
    RvBool				 acked;  
    RvUint8             *plain;
    RvSize_t             plainSize;  
    RvSigCompDynZStream *pZ;                            
};

typedef RvSigCompDynZ *RvSigCompDynZList; /* A type for Z structure list */ 

/********************************************************************************************
 * RvSigCompDynState
 * -----------------------------------------------------------------------------
 * The DynState structure is used for keeping information of the current status of a 
 * dynamic compression compartment. The DynState is being "maintained" internally by the 
 * SigComp module utility for dynamic compression
 *
 * hSigCompMgr   - Handle to the SigComp module manager 
 * maxTripTime   - estimation on maximal trip time (in seconds)
 * peerInfoKnown - RV_TRUE, if peer information already accepted 
 * peerCompSize  - peer compartment size 
 * reliableTransport - if true - use optimizations for reliable transport (TCP, for example)
 * curTime       - Actually, this isn't time per-se, it's just some counter that is 
 *				   incremented for each message compressed by this compressor
 * curTimeSec    - Current time in seconds
 * lastPeerMessageTime - 
 *				   Time of arrival of the last message from peer 
 * lastPeerMessageNum - 
 *				   Sequential number of the last message from peer 
 * states        -  List of states 's' such that each of them may be present 
 *					at peer compartment. Pay attention: cumulative size of these
 *					states may exceed total compartment size. In these case we know
 *					that there are states in this list that aren't present at peer compartment,
 *					but we don't know which exactly. The states in this list are kept in the order
 *					of increasing priority. 
 * curZid		 -  zid that will be used for next message
 ********************************************************************************************/
struct _RvSigCompDynState {
    RvSigCompMgrHandle hSigCompMgr;
    RvUint32           maxTripTime;    /* Estimation on maximal trip time */
    RvBool			   peerInfoKnown; 
    RvSize_t		   peerCompSize;  
    RvBool             reliableTransport;

    RvSigCompZTime	   curTime;   
    RvSigCompZTimeSec  curTimeSec; 

    RvSigCompZTimeSec  lastPeerMessageTime; 
    RvUint32		   lastPeerMessageNum;  

    RvInt              maxCachedZ;
    RvInt              nCachedZ;

    RvInt                nStates;     /* current history size */
    RvInt                peekStates;  /* Peek history size */
    RvInt                maxStates;   /* Maximal history size to keep */

    RvSigCompDynZList    states;      /* List of states 's' such that each of them may be present 
                                       * at peer compartment. Pay attention: cumulative size of these
                                       * states may exceed total compartment size. In these case we know
                                       * that there are states in this list that aren't present at peer compartment,
                                       * but we don't know which exactly. The states in this list are kept in the order
                                       * of increasing priority
                                       */

    RvSize_t               totalSize;  /* Total memory consumption of this compartment */
    RvSize_t               maxTotalSize;
    RvSize_t               peekTotalSize;

    RvSigCompDynZid        curZid;  

    RvSize_t			   curPlain;
    RvSize_t			   curCompressed;
#ifdef RV_SIGCOMP_STATISTICS
    double				   curRatio;
#endif

    RvSigCompDynZ		  *activeState;
    RvSigCompDynZ		  *initState;
    RvSigCompDynZStreamVT  vt;
};


/*-----------------------------------------------------------------------*/
/*                       DYNAMIC COMPRESSION API                         */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* RvSigCompDynZTimeSecSet
* ------------------------------------------------------------------------
* General: Set the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RVAPI void RvSigCompDynZTimeSecSet(RvSigCompZTimeSec time);

/*************************************************************************
* RvSigCompDynZTimeSecGet
* ------------------------------------------------------------------------
* General: Retrieve the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RVAPI RvSigCompZTimeSec RvSigCompDynZTimeSecGet();

/*************************************************************************
 *				Dynamic Compression Utility Functions					 *
 *************************************************************************/ 

/*************************************************************************
* RvSigCompDynCompress
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm.
*		   This function performs a dynamic compression using given 
*		   algorithm callbacks (which resides within the application context) 
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pCompressionInfo - The compression info structure which 
*							contains essentials details for compression
*		 pMsgInfo         - Details of the message to be compressed
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 pInstanceContext - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*		 pAppContext      - General application context (currently it's 
*							useless - always NULL)
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynCompress(  
                        IN    RvSigCompMgrHandle         hSigCompMgr,
                        IN    RvSigCompCompartmentHandle hCompartment,
                        IN    RvSigCompCompressionInfo   *pCompressionInfo,
                        IN    RvSigCompMessageInfo       *pMsgInfo,
                        IN    RvSigCompAlgorithm         *pAlgStruct,
                        INOUT RvUint8                    *pInstanceContext,
                        INOUT RvUint8                    *pAppContext); 

/*************************************************************************
* RvSigCompDynStateConstruct
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm
*		   This function performs a compartment context construction using  
*		   given algorithm callbacks (which resides within the application  
*		   context)
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 instance		  - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynStateConstruct(
							 IN    RvSigCompMgrHandle    hSigCompMgr,
							 IN    RvSigCompDynZStream  *initStream,
							 INOUT RvSigCompDynState    *instance); 

/*************************************************************************
* RvSigCompDynStateDestruct
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm
*		   This function performs a compartment context destruction using  
*		   given algorithm callbacks (which resides within the application  
*		   context)
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 instance		  - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynStateDestruct(
							   IN    RvSigCompMgrHandle          hSigCompMgr,
							   IN    RvSigCompCompartmentHandle  hCompartment,
							   IN    RvSigCompAlgorithm         *pAlgStruct,
							   INOUT void						*instance); 

/*************************************************************************
* RvSigCompDynOnPeerMsg
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm.
*		   This function performs an update of a compartment context (which
*		   is used in dynamic compression) regarding an incoming message.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pInfo			  - The compression info structure which 
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 pInstanceContext - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynOnPeerMsg(
							IN    RvSigCompMgrHandle          hSigCompMgr,
							IN    RvSigCompCompartmentHandle  hCompartment,
							IN    RvSigCompAlgorithm         *pAlgStruct,
                            IN    RvSigCompCompressionInfo   *pInfo,
							INOUT void		  			     *instance); 

#ifdef __cplusplus
}
#endif

#endif /* RV_SIGCOMP_DYN_STATE_H */ 
