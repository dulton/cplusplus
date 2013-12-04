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
 *                              <RvSigCompTypes.h>
 *
 * The RvSipSigCompTypes.h file contains all type definitions for the SigComp module.
 *
 * includes:
 * 1.Handle Type definitions
 * 2.SigComp Compartment Type definitions
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Elly Amitai                 20031100
 *    Gil Keini                   20040111
 *********************************************************************************/

#ifndef RV_SIGCOMP_TYPES_H
#define RV_SIGCOMP_TYPES_H

#if defined(__cplusplus)
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_ADS_DEF.h"
#include "RV_SIGCOMP_DEF.h"

/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

#define RVSIGCOMP_MAX_NUM_ALGORITHMS   (10)
#define RVSIGCOMP_MAX_NUM_DICTIONARIES ( 3)
#define RVSIGCOMP_STATE_ID_LENGTH      (20)
#define RVSIGCOMP_ALGO_NAME            (25)
#define RVSIGCOMP_SHA1_HASH_SIZE       (20)

/* Various transport properties, may be or'ed */
#define RVSIGCOMP_STREAM_TRANSPORT      1
#define RVSIGCOMP_MESSAGE_TRANSPORT     0

#define RVSIGCOMP_RELIABLE_TRANSPORT    2
#define RVSIGCOMP_UNRELIABLE_TRANSPORT  0

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#define RVSIGCOMP_DEFAULT_SMS           8192
#else
#define RVSIGCOMP_DEFAULT_SMS           4096
#endif
/* SPIRENT_END */

	
RV_DECLARE_HANDLE(RvSigCompMgrHandle);

RV_DECLARE_HANDLE(RvSigCompCompartmentHandle);

typedef RvUint8 RvSigCompStateID[RVSIGCOMP_STATE_ID_LENGTH]; /* a partial stateID can be 6/9/12 bytes long */

/* SHA1 related stuff */
/********************************************************************************************
 * RvSigCompShaStatus
 * -----------------------------------------------------------------------------
 * The enum is used for indicating the last SHA1 operation outcome status
 ********************************************************************************************/
typedef enum 
{   RVSIGCOMP_SHA_SUCCESS = 0x00,
	RVSIGCOMP_SHA_NULL,             /* Null pointer parameter    */
	RVSIGCOMP_SHA_INPUT_TOO_LONG,   /* input data too long       */
	RVSIGCOMP_SHA_STATE_ERROR       /* called Input after Result */
} RvSigCompShaStatus;

/********************************************************************************************
 * RvSigCompShaStatus
 * -----------------------------------------------------------------------------
 * This structure will hold context information for the SHA-1 hashing operation 
 ********************************************************************************************/
typedef struct
{
    RvUint32		   intermediateHash[RVSIGCOMP_SHA1_HASH_SIZE/4]; 
	/* Message Digest                   */
    RvUint32		   lengthLow;               /* Message length in bits           */
    RvUint32		   lengthHigh;              /* Message length in bits           */
    RvUint16		   messageBlockIndex;       /* Index into message block array   */
    RvUint8			   messageBlock[64];        /* 512-bit message blocks           */
    RvUint32		   isComputed;              /* Is the digest computed?          */
    RvSigCompShaStatus isCorrupted;             /* Is the message digest corrupted? */
} RvSigCompSHA1Context;



/********************************************************************************************
 * RvSigCompDictionary
 * -----------------------------------------------------------------------------
 * A structure for holding information about a static dictionary.
 * This structure is set/filled with values by the 
 * "SigCompStaticDictionaryXXXInit" function (supplied as part of the 
 * dictionary files).
 *
 * minimalAccessLength
 * --------------------
 * Minimal access length to the state containing the dictionary. 
 * Remark: Can be 6/9/12 according to RFC3320
 *
 * dictionarySize
 * --------------
 * The size of the dictionary in bytes. 
 *
 * dictionary
 * ----------
 * A pointer to a dictionary (content). 
 ********************************************************************************************/
typedef struct {
    RvUint32 minimalAccessLength; 
    RvUint32 dictionarySize;      
    RvUint8  *dictionary;  
} RvSigCompDictionary;


/********************************************************************************************
 * RvSigCompMessageInfo
 * -----------------------------------------------------------------------------
 * RvSigCompMessageInfo contains a set parameters, to be used in compression
 * or decompression requests.
 *
 * pPlainMessageBuffer
 * --------------------
 * Pointer to a plain message buffer which may be an output of a decompression 
 * session or an input for compression session. 
 *
 * plainMessageBuffSize
 * ---------------------
 * The size of a plain message buffer. When sent to the decompression function,
 * it will hold the size of the allocated buffer and, on return, will hold the 
 * number of used bytes (smaller or equal to the value on input).
 *
 * pCompressedMessageBuffer
 * ------------------------
 * Pointer to a compressed message buffer which may be an input of a decompression 
 * session or an output for compression session. 
 *
 * compressedMessageBuffSize
 * -------------------------
 * The size of a compressed message buffer. When sent to the compression function, 
 * it will hold the size of the allocated buffer and, on return, will hold the number
 * of used bytes (smaller or equal to the value on input).
 *
 * transportType
 *--------------
 * Type of transport (message, stream, reliable). My be OR between RVSIGCOMP_xxx_TRANSPORT
 *  constants
 ********************************************************************************************/
typedef struct {
    RvUint8  *pPlainMessageBuffer;
    RvUint32  plainMessageBuffSize;
    RvUint8  *pCompressedMessageBuffer;
    RvUint32  compressedMessageBuffSize;
    RvInt     transportType;
} RvSigCompMessageInfo;

/********************************************************************************************
 * RvSigCompCompressionInfo
 * -----------------------------------------------------------------------------
 * A structure for transferring context information to the compression function.
 *
 * pReturnedFeedbackItem
 * ---------------------
 * A pointer to the returned feedback item (RFI) buffer. 
 * This buffer is unique to a sigComp manager/instance.
 *
 * returnedFeedbackItemSize
 * ------------------------
 * The size of the RFI buffer in bytes.
 *
 * bResetContext
 * -------------
 * A flag denoting that the algorithm's context is "new" or has been reset
 * since the last call.
 *
 * bStaticDic3485Mandatory
 * -----------------------
 * A flag denoting the existence of SIP/SDP static dictionary (see RFC-3485).
 * This has to be set (RV_TRUE) for a SIP implementation.
 * Other protocol stacks may un-set this flag (RV_FALSE).
 *
 * remoteSMS
 * ---------
 * The value of remote/peer end-point (EP) state memory size (SMS).
 * 
 * remoteDMS
 * ---------
 * The value of peer EP decompression memory size (DMS).
 *
 * remoteCPB
 * ---------
 * The value of peer EP cycles per bit (CPB).
 *
 * pRemoteStateIds
 * ---------------
 * A pointer to a buffer containing the peer list of "locally available state IDs".
 * It is used just for reading the content by the compressor.
 * The list of state identifiers is terminated by a byte in the position
 * where the next length field would be expected, that is set to a value
 * below 6 or above 20. (see RFC-3320  section 9.4.9 bottom of p. 55) 
 *
 * localSMS
 * --------
 * The value of local compartment's SMS.
 *
 * localDMS
 * --------
 * The value of local DMS.
 *
 * localCPB
 * --------
 * The value of local CPB.
 *
 * localStatesSize
 * ---------------
 * The size of "pLocalStateIds" buffer (allocated), in bytes.
 * It is used to prevent buffer over-run when writing to this buffer.
 *
 * pLocalStateIds
 * --------------
 * The local equivalent of "pRemoteStateIds".
 *
 * bSendLocalCapabilities
 * ----------------------
 * A flag denoting a request to send local capabilities to peer. 
 * It is set by the decoder and reset by the encoder.
 *
 * bSendLocalStatesIdList
 * ----------------------
 * A flag denoting a request to send the local states ID list to peer. 
 * It is set by the decoder and reset by the encoder.
 ********************************************************************************************/
typedef struct {
	RvUint8   *pReturnedFeedbackItem;     
	RvUint32  returnedFeedbackItemSize; /* if 0 - no returned feedback item */
	RvBool    bResetContext;            
	
    RvBool    bStaticDic3485Mandatory; /* do all remote EPs (and local) support RFC-3485 ? */

    RvUint32  remoteSMS;
	RvUint32  remoteDMS;
	RvUint32  remoteCPB;
	RvUint8   *pRemoteStateIds;

	RvUint32  localSMS;
	RvUint32  localDMS;
	RvUint32  localCPB;
	RvUint32  localStatesSize;
	RvUint8   *pLocalStateIds;
    
    RvUint    nIncomingMessages;
    RvUint32  lastIncomingMessageTime;

	RvBool    bSendLocalCapabilities;
	RvBool    bSendLocalStatesIdList;
} RvSigCompCompressionInfo;

typedef struct RvSigCompAlgorithm_ RvSigCompAlgorithm;


/***************************************************************************
 * RvSigCompCompression
 * ------------------------------------------------------------------------
 * General: A prototype for a compression function to be used as a callback.
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback registration
 *                         process.
 *         filter        - The filter that this message is using (info, error..)
 *         formattedText - The text to be printed to the log. The text
 *                          is formatted as follows:
 *                          <filer> - <module> - <message>
 *                          for example:
 *                          "INFO  - STACK - Stack was constructed successfully"
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompCompression)(
    IN    RvSigCompMgrHandle         hSigCompMgr,
    IN    RvSigCompCompartmentHandle hCompartment,
    IN    RvSigCompCompressionInfo   *pCompressionInfo,
    IN    RvSigCompMessageInfo       *pMsgInfo,
    IN    RvSigCompAlgorithm         *pAlgStruct,
    INOUT RvUint8					 *pInstanceContext,
    INOUT RvUint8                    *pAppContext);

/***************************************************************************
 * RvSigCompAlgorithmInitializedEv
 * ------------------------------------------------------------------------
 * General: A prototype for algorithm initialization function to be triggered
 *			as a callback. Notifies that an algorithm was initialized by the  
 *			SigComp module during initialization process. 
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - The SigComp manager handle 
 *        pAlgorithm  - Pointer to the initialized algorithm structure. This
 *						structure includes details and characteristics of 
 *						the initialized algorithm
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompAlgorithmInitializedEv)(
					IN RvSigCompMgrHandle  hSigCompMgr,
					IN RvSigCompAlgorithm *pAlgorithm);

/***************************************************************************
 * RvSigCompAlgorithmEndedEv
 * ------------------------------------------------------------------------
 * General: A prototype for algorithm decease function to be triggered
 *			as a callback. Notifies that an algorithm was dereferenced by   
 *			the SigComp module. 
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - The SigComp manager handle 
 *        pAlgorithm  - Pointer to the ended algorithm structure. This
 *						structure includes details and characteristics of 
 *						the ended algorithm
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompAlgorithmEndedEv)(
					IN RvSigCompMgrHandle  hSigCompMgr,
					IN RvSigCompAlgorithm *pAlgorithm);

/***************************************************************************
 * RvSigCompCompartmentCreatedEv
 * ------------------------------------------------------------------------
 * General: A prototype for new compartment creation function to be triggered
 *			as a callback. Notifies that a compartment was created by   
 *			the SigComp module. 
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr  - The SigComp manager handle 
 *		  hCompartment - Handle to the created compartment
 *        pAlgorithm   - Pointer to the compartment algorithm structure. This
 *					  	 structure includes details and characteristics of 
 *						 the compartment algorithm
 *        pCompContext - Pointer to the compartment context buffer. This 
 *						 buffer is allocated by the SigComp module during 
 *					     a compartment construction according to it's 
 *						 related algorithm details. The length of this 
 *						 buffer matches the pAlgorithm->contextSize value.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompCompartmentCreatedEv)(
							 IN    RvSigCompMgrHandle          hSigCompMgr,
							 IN    RvSigCompCompartmentHandle  hCompartment,
							 IN    RvSigCompAlgorithm         *pAlgorithm,
							 INOUT void						  *pCompContext);

/***************************************************************************
 * RvSigCompCompartmentDestructedEv
 * ------------------------------------------------------------------------
 * General: A prototype for new compartment termination function to be 
 *			triggered as a callback. Notifies that a compartment was     
 *			terminated by the SigComp module. 
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr  - The SigComp manager handle 
 *		  hCompartment - Handle to the terminated compartment
 *        pAlgorithm   - Pointer to the compartment algorithm structure. This
 *					  	 structure includes details and characteristics of 
 *						 the compartment algorithm
 *		  pCompContext - Pointer to the compartment context buffer. This 
 *						 buffer is allocated by the SigComp module during 
 *					     a compartment construction according to it's 
 *						 related algorithm details. The length of this 
 *						 buffer matches the pAlgorithm->contextSize value.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompCompartmentDestructedEv)(
							 IN    RvSigCompMgrHandle          hSigCompMgr,
							 IN    RvSigCompCompartmentHandle  hCompartment,
							 IN    RvSigCompAlgorithm         *pAlgStruct,
							 INOUT void						  *pCompContext);

/***************************************************************************
 * RvSigCompCompartmentOnPeerMsgEv
 * ------------------------------------------------------------------------
 * General: A prototype for new compartment notification function to be 
 *			triggered as a callback. Notifies that a message which is related
 *			to a compartment was received and decompressed successfully.
 *          The application can set this kind of callback by using the 
 *          function SigCompAlgXXXInit function 
 *          (supplied as part of the compression algorithm files).
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr  - The SigComp manager handle 
 *		  hCompartment - Handle to the terminated compartment
 *        pAlgorithm   - Pointer to the compartment algorithm structure. This
 *					  	 structure includes details and characteristics of 
 *						 the compartment algorithm
 *		  pCompContext - Pointer to the compartment context buffer. This 
 *						 buffer is allocated by the SigComp module during 
 *					     a compartment construction according to it's 
 *						 related algorithm details. The length of this 
 *						 buffer matches the pAlgorithm->contextSize value.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *RvSigCompCompartmentOnPeerMsgEv)(
						   IN    RvSigCompMgrHandle          hSigCompMgr,
						   IN    RvSigCompCompartmentHandle  hCompartment,
						   IN    RvSigCompAlgorithm         *pAlgStruct,
						   IN    RvSigCompCompressionInfo   *pCompInfo,
						   INOUT void					    *pCompContext);

/* RvSigCompAlgoEvHandlers
 * ------------------------------------------------------------------------
 * Structure with function pointers to the SigComp module's algorithm 
 * call-back.
 * The application can set those callbacks by using the function 
 * SigCompAlgXXXInit() function (supplied as part of the compression  
 * algorithm files) by assigning the suitable functions pointers into 
 * RvSigCompAlgoEvHandlers within the structure RvSigCompAlgorithm.
 * 
 * pfnAlgorithmInitializedEvHandler - Notifies of a new initialized algorithm
 * pfnAlgorithmEndedEvHandler       - Notifies of a deceased algorithm
 * pfnCompartmentCreatedEvHandler   - Notifies of a newly created compartment
 * pfnCompartmentDestructedEv       - Notifies of a terminated compartment
 */
typedef struct 
{
	RvSigCompAlgorithmInitializedEv   pfnAlgorithmInitializedEvHandler; 
	RvSigCompAlgorithmEndedEv         pfnAlgorithmEndedEvHandler; 
	RvSigCompCompartmentCreatedEv     pfnCompartmentCreatedEvHandler;
	RvSigCompCompartmentDestructedEv  pfnCompartmentDestructedEvHandler;
	RvSigCompCompartmentOnPeerMsgEv   pfnCompartmentOnPeerMsgEvHandler;
} RvSigCompAlgoEvHandlers;

/********************************************************************************************
 * RvSigCompCompressionInfo
 * -----------------------------------------------------------------------------
 * A structure for transferring context information to the compression function.
 *
 * algorithmName
 * -------------
 * The algorithm name in ASCII, to be used in debugging.
 *
 * contextSize
 * -----------
 * The size in bytes of the requested context buffer.
 * 
 * bSharedCompression
 * ------------------
 * A flag denoting "shared compression" mechanism.
 *
 * sharedStateID
 * -------------
 * The ID of the shared state.
 *
 * dictionaryHandle
 * ----------------
 * A handle (stateID) to a dictionary if used.
 *
 * pfnCompressionFunc
 * ------------------
 * A pointer to a compression function (callback).
 *
 * pDecompBytecode
 * ---------------
 * A pointer to a buffer containing the decompression UDVM bytecode.
 *
 * decompBytecodeSize
 * ------------------
 * The size of the above buffer in bytes.
 *
 * decompCPB
 * ---------
 * The CPB required to run the decompression bytecode.
 *
 * decompSMS
 * ---------
 * The minimal SMS required by the compression algorithm, 
 * which manages the states in the peer SigComp compartment.
 *
 * decompDMS
 * ---------
 * The minimal DMS required by the decompression bytecode to work.
 *
 * decompVer
 * ---------
 * The minimal SigComp version required by the algorithm.
 *
 * decompBytecodeAddr
 * ------------------
 * Specifies the starting memory address to which the bytecode is copied. 
 * This value is required when building the SigComp message header.
 *
 * decompBytecodeInst
 * ------------------
 * Specifies the start memory address from which to run the bytecode.
 * This value is required when building the SigComp message header.
 ********************************************************************************************/
struct RvSigCompAlgorithm_ {
    RvChar   algorithmName[RVSIGCOMP_ALGO_NAME];  
    RvUint32 contextSize;        
    RvBool   bSharedCompression; 
    RvSigCompStateID  sharedStateID;       
    RvSigCompStateID  dictionaryHandle;   
    RvSigCompCompression pfnCompressionFunc; 
	
    RvUint8  *pDecompBytecode;   
    RvUint32 decompBytecodeSize; 
    RvUint32 decompCPB;          
    RvUint32 decompSMS;          
    RvUint32 decompDMS;          
    RvUint32 decompVer;          
    RvUint32 decompBytecodeAddr; 
    RvUint32 decompBytecodeInst; 

	RvSigCompAlgoEvHandlers evHandlers;
};

/* Defines the log filters to be used by the sigComp module. */
typedef enum {
        RVSIGCOMP_LOG_NONE_FILTER    = 0x00,
        RVSIGCOMP_LOG_DEBUG_FILTER   = 0x01,
        RVSIGCOMP_LOG_INFO_FILTER    = 0x02,
        RVSIGCOMP_LOG_WARN_FILTER    = 0x04,
        RVSIGCOMP_LOG_ERROR_FILTER   = 0x08,
        RVSIGCOMP_LOG_EXCEP_FILTER   = 0x10,
        RVSIGCOMP_LOG_ENTER_FILTER   = 0x20,
		RVSIGCOMP_LOG_LEAVE_FILTER   = 0x40,
        RVSIGCOMP_LOG_LOCKDBG_FILTER = 0x80
} RvSigCompLogFilters;

/***************************************************************************
 * RvSigCompLogEntryPrint
 * ------------------------------------------------------------------------
 * General: Notifies the application each time a line should be printed to
 *          the log. The application can decide whether to print the line
 *          to the screen, file or other output device. You set this
 *          callback in the RvSigCompCfg structure before initializing
 *          the module. If you do not implement this function a default logging
 *          will be used and the line will be written to the SigComp.txt file.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  context       - The context that was given in the callback 
 *                         registration process.
 *         filter        - The filter that this message is using (info, error..)
 *         formattedText - The text to be printed to the log. The text
 *                         is formatted as follows:
 *                         <filer> - <module> - <message>
 *                         for example:
 *                         "INFO  - SIGCOMP - SigComp was constructed successfully"
 ***************************************************************************/
typedef void (RVCALLCONV * RvSigCompLogEntryPrint)(
                                        IN void*               context,
                                        IN RvSigCompLogFilters filter,
                                        IN const RvChar       *formattedText);

/********************************************************************************************
 * RvSigCompCfg
 * --------------------------------------------------------------------------
 * RvSigCompCfg contains the configuration parameters and default values 
 * of the SigComp module.Calling the RvSigCompInitCfg function will 
 * initiallize the configuration structure to the default values.
 *
 * maxOpenCompartments 
 * -------------------
 * The maximum number of concurrently open compartments.
 * Usually each compartment is correlated to a call-leg.
 * Minimal value: 1.
 * Default value: DEFAULT_SC_OPEN_COMPARTMENTS (10)
 *
 * maxTempCompartments 
 * -------------------
 * The maximum number of concurrent temporary compartments. 
 * A temporary compartment is correlated to an incoming 
 * processed message. The temporary compartment information
 * is saved to a regular/persistent compartment after 
 * authorization by the SIP Stack using the RvSigCompDeclareCompartment
 * function. The temporary compartment may be deleted by passing
 * RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID as the compartment handle. Or, it 
 * will be deleted if there was no call to the 
 * RvSigCompDeclareCompartment function during a period defined 
 * in the timeoutForTempCompartments parameter (see below).
 * Minimal value: 1.
 * Default value: DEFAULT_SC_TEMP_COMPARTMENTS (10)
 *
 * maxStatesPerCompartment
 * -----------------------
 * The maximum number of states associated with a compartment.
 * This number is used to define the list object that keeps 
 * track of the states in the compartment.
 * Minimal value: 1.
 * Default value: DEFAULT_SC_MAX_STATES_PER_COMP (16)
 * Maximum value allowed: 512
 *
 * timeoutForTempCompartments 
 * --------------------------
 * The timeout value in mSec that defined the "life" 
 * of a temporary compartment.
 * If this time length is exceeded without a call to the 
 * RvSigCompDeclareCompartment function, the temporary
 * compartment, the related UDVM instance, and information 
 * are all removed/deleted from the memory.
 * Minimal value: 10 mSec.
 * Default value: DEFAULT_SC_TIMEOUT (5000)
 *
 * bStaticDic3485Mandatory 
 * -----------------------
 * A flag denoting if all the remote EPs support RFC-3485
 * (SIP/SDP static dictionary). This RFC is mandatory for all
 * SIP endpoints.
 * Default value: RV_TRUE
 *
 * decompMemSize 
 * -------------
 * The Deompression Memory Size (DMS) of the local machine.
 * DMS is per SigComp message and the memory is reclaimed
 * after the message has been decompressed.
 * From <RFC5049> p. 3 section 2.1:
 * Minimum value for ANY/SigComp: 2048 bytes, as specified in section
 * 3.3.1 of [RFC-3320].
 * Minimum value for SIP/SigComp: 8192 bytes.
 * Default value: DEFAULT_SC_DECMOP_MEM_SIZE (8192)
 * 
 * stateMemSize
 * ------------
 * The State Memory Size (SMS) of the local machine. This memory
 * limitation is per-compartment. An endpoint MAY offer different 
 * SMSs for different compartments as long as the SMS is not less
 * than 2048 bytes.
 * From <RFC5049> p. 3 section 2.2:
 * Minimum value for ANY/SigComp: 0 (zero) bytes, as specified in
 * section 3.3.1 of [RFC-3320].
 * Minimum value for SIP/SigComp: 2048 bytes.
 * Default value: DEFAULT_SC_STATE_MEM_SIZE (2048)
 *
 * cyclePerBit
 * -----------
 * The Cycles Per Bit (CPB) value allocated by the local machine. This 
 * measure is the UDVM equivalent to MIPS/FLOPS of a regular CPU, and
 * denotes the maximum computing power allocated for decoding a message.
 * More complex decoding algorithms use more cycles per bit. CPB is per 
 * compartment. An endpoint MAY offer different CPB for  different 
 * compartments as long as the CPB is not less than 16 cycles.
 * From <RFC5049> p. 4 section 2.3:
 * Minimum value for ANY/SigComp: 16, as specified in section 3.3.1 of
 * [RFC-3320].
 * Minimum value for SIP/SigComp: 16
 * Default value: DEFAULT_SC_CYCLES (16)
 *
 * smallBufPoolSize 
 * ----------------
 * The size in bytes of buffers in the pool of "small" HPAGEs.
 * Minimum value: 8 bytes
 * Default value: DEFAULT_SC_SMALL_BUF_SIZE (64)
 *
 * smallPoolAmount 
 * ---------------
 * The number of "pages" in the pool of "small" HPAGEs
 * Minimum value: 1 element
 * Default value: 8 elements per compartment (persistant + temporary)
 * 
 * mediumBufPoolSize
 * -----------------
 * The size in bytes of buffers in the pool of "medium" HPAGEs
 * Minimum value: 8 bytes
 * Default value: DEFAULT_SC_SMALL_BUF_SIZE (512)
 *
 * mediumPoolAmount 
 * ----------------
 * The number of "pages" in the pool of "medium" HPAGEs
 * Minimum value: 0 elements
 * Default value: 2 elements per compartment (persistant + temporary)
 *
 * largeBufPoolSize
 * ----------------
 * The size in bytes of buffers in the pool of "large" HPAGEs
 * Minimum value: 8 bytes
 * Default value: DEFAULT_SC_SMALL_BUF_SIZE (8192)
 *
 * largePoolAmount 
 * ---------------
 * The number of "pages" in the pool of "large" HPAGEs
 * Minimum value: 0 elements
 * Default value: 1 element per compartment (persistent + temporary)
 *
 * logHandle
 * ---------
 * The LOG handle to use for logging messages.
 * Default value: NULL
 *
 * pfnLogEntryPrint
 * ----------------
 * See the RvSigCompLogEntryPrint callback function for details.
 * Default value: NULL
 *
 * logEntryPrintContext
 * --------------------
 * See the RvSigCompLogEntryPrint callback function for details.
 * Default value: NULL
 *
 * logFilter
 * ---------
 * See the RvSigCompLogFilters typedef for details.
 * See the RvSigCompLogEntryPrint callback function for more details.
 * 
 * dynMaxTripTime
 * ----------------
 * This value is taken into account when using dynamic compression algorithms. This 
 * integer value limits the lifetime (in seconds) of dynamic compression states 
 * (used only for compression), which were NOT acknowledged during a SigComp session. 
 *
 * dynMaxStatesNo
 * ---------------
 * This value is taken into account when using dynamic compression algorithms. This 
 * integer value limits the number of dynamic compression states (used only for 
 * compression), which a single compartment can keep.
 *
 * dynMaxTotalStatesSize
 * ---------------------
 * This value is taken into account when using dynamic compression algorithms. This 
 * integer value limits the total size of dynamic compression states (used only for 
 * compression), which a single compartment can keep.
 *
 ********************************************************************************************/
typedef struct {
    RvUint32 maxOpenCompartments; 
    RvUint32 maxTempCompartments; 
    RvUint32 maxStatesPerCompartment; 
    RvUint32 timeoutForTempCompartments; 
    RvBool   bStaticDic3485Mandatory; 
    /* The following 3 variables are derived from the machine/host CPU/memory capabilities */
    RvUint32 decompMemSize; /* 8192 is the minimum for SIP/SigComp > */
    RvUint32 stateMemSize;  /* 4096 is the minimum for SIP/SigComp according to 3GPP TS 24.229 */
    RvUint32 cyclePerBit;   /*   16 is the minimum for SIP/SigComp > */
    /* there will be three pools of HPAGEs for the states, their parameters are as follow:*/
    RvUint32 smallBufPoolSize;  /* size of buffers in small pool */
    RvUint32 smallPoolAmount;   /* amount of buffers in small pool */
    RvUint32 mediumBufPoolSize; /* size of buffers in medium pool */
    RvUint32 mediumPoolAmount;  /* amount of buffers in medium pool */
    RvUint32 largeBufPoolSize; /* size of buffers in large pool */
    RvUint32 largePoolAmount;   /* amount of buffers in large pool */
    /* Log related variables */
    RV_LOG_Handle logHandle; /* The logHandle is pointer to a log manager and not the log manager itself */
    RvSigCompLogEntryPrint  pfnLogEntryPrint;
    void				   *logEntryPrintContext;
    RvUint32				logFilter;
	/* New Version Parameters */ 
	RvUint32				dynMaxTripTime; /* In seconds */ 
	RvUint32				dynMaxStatesNo; 
	RvUint32				dynMaxTotalStatesSize; 
} RvSigCompCfg;

/********************************************************************************************
 * RvSigCompLocalStatesList
 * --------------------------------------------------------------------------
 * A structure holding the list of local states by their partial IDs.
 *
 * nLocalStatesSize 
 * ----------------
 * The size of the buffer in bytes.
 *
 * pLocalStates
 * ------------
 * A pointer to a buffer that holds the list of local states IDs.
 ********************************************************************************************/
typedef struct
{
    RvUint32        nLocalStatesSize; 
    const RvUint8  *pLocalStates;     
} RvSigCompLocalStatesList;


/********************************************************************************************
 * RvSigCompResource
 * --------------------------------------------------------------------------
 * A structure holding information about the memory resources. 
 * This is a generic type used by other SigComp specific types.
 *
 * numOfAllocatedElements 
 * ----------------------
 * The number of allocated elements in a structure,
 * such as elements in a list, cells in an array, etc.
 *
 * currNumOfUsedElements
 * ---------------------
 * The number of elements currently used.
 * 
 * maxUsageOfElements
 * ------------------
 * The maximum number of used elements reached during operation.
 ********************************************************************************************/
typedef struct
{
    RvUint32 numOfAllocatedElements;
    RvUint32 currNumOfUsedElements;
    RvUint32 maxUsageOfElements;
} RvSigCompResource;

/********************************************************************************************
 * RvSigCompMgrResources
 * --------------------------------------------------------------------------
 * A structure holding information about the resources of the 
 * SigComp manager object.
 *
 * dictionaries 
 * ------------
 * A resource structure for information about the list of dictionaries.
 *
 * algorithms
 * ----------
 * A resource structure for information about the list of algorithms.
 ********************************************************************************************/
typedef struct {
    RvSigCompResource   dictionaries;
    RvSigCompResource   algorithms;
} RvSigCompMgrResources;

/********************************************************************************************
 * RvSigCompStateHandlerResources
 * --------------------------------------------------------------------------
 * A structure holding information about the resources of the state 
 * handler object.
 *
 * smallPool 
 * ---------
 * A resource structure for information about the pool of "small" pages defined as RA.
 * For an explanation about small/medium/large pools, see the configuration structure.
 *
 * mediumPool
 * ----------
 * A resource structure for information about the pool of "medium" pages defined as RA.
 * For an explanation about small/medium/large pools, see the configuration structure.
 *
 * largePool
 * ---------
 * A resource structure for information about the pool of "large" pages defined as RA.
 * For an explanation about small/medium/large pools, see the configuration structure.
 * 
 * stateStructsRA
 * --------------
 * A resource structure for information about the RA of state elements.
 *
 * hashTableKeys
 * -------------
 * A resource structure for information about the keys used in the hash table (of the states).
 *
 * hashTableElements
 * -----------------
 * A resource structure for information about the elements of the hash table
 * (used to hash the states).
 ********************************************************************************************/
typedef struct {
    RvSigCompResource   smallPool;
    RvSigCompResource   mediumPool;
    RvSigCompResource   largePool;
    RvSigCompResource   stateStructsRA;
    RvSigCompResource   hashTableKeys;
    RvSigCompResource   hashTableElements;
} RvSigCompStateHandlerResources;

/********************************************************************************************
 * RvSigCompCompartmentHandlerResources
 * --------------------------------------------------------------------------
 * A structure holding information about the resources of the compartment handler object.
 *
 * compratmentsStructRA 
 * --------------------
 * A resource structure for information about the RA of sigcomp compartment structures.
 ********************************************************************************************/
typedef struct {
    RvSigCompResource   compratmentsStructRA;
} RvSigCompCompartmentHandlerResources;

/********************************************************************************************
 * RvSigCompDecompressorDispatcherResources
 * --------------------------------------------------------------------------
 * A structure holding information about the resources of the decompressor dispatcher object.
 *
 * udvmRA 
 * ------
 * A resource structure for information about the RA of UDVM instances.
 ********************************************************************************************/
typedef struct {
    RvSigCompResource   udvmRA;
} RvSigCompDecompressorDispatcherResources;

/********************************************************************************************
 * RvSigCompModuleResources
 * --------------------------------------------------------------------------
 * A structure holding information about the resources of the whole SigComp 
 * module. This is comprised of the resource structures of each object in the module defined above.
 *
 * sigCompMgrResources 
 * -------------------
 * A resource structure defined above.
 * 
 * stateHandlerResources
 * ---------------------
 * A resource structure defined above.
 *
 * compartmentHandlerResources
 * ---------------------------
 * A resource structure defined above.
 *
 * decompressorDispatcherResources
 * -------------------------------
 * A resource structure defined above.
 ********************************************************************************************/
typedef struct {
    RvSigCompMgrResources                    sigCompMgrResources;
    RvSigCompStateHandlerResources           stateHandlerResources;
    RvSigCompCompartmentHandlerResources     compartmentHandlerResources;
    RvSigCompDecompressorDispatcherResources decompressorDispatcherResources;
} RvSigCompModuleResources;

#if defined(__cplusplus)
}
#endif

#endif /* end of: RV_SIGCOMP_TYPES_H */


