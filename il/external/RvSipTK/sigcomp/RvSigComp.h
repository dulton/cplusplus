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
 *                              <RvSipSigComp.h>
 *
 * The SigComp Compartment functions of the RADVISION SIP stack enable you to
 * create and manage SigComp Compartment objects, attach/detach to/from
 * compartments and control the compartment parameters.
 *
 *
 * SigComp Compartment API functions are grouped as follows:
 * The SigComp Compartment Manager API
 * ------------------------------------
 * The SigComp Compartment manager is in charge of all the compartments. It is used
 * to create new compartments.
 *
 * The SigComp Compartment API
 * ----------------------------
 * A compartment represents a SIP SigComp Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transactions that is identify by a compartment ID when sending request
 * through a compressor entity. Using the API, the user can initiate compartments,
 * or destruct it. Functions to set and access the compartment fields are also
 * available in the API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofer Goren                  20031100
 *    Gil Keini                   20040114
 *********************************************************************************/

#ifndef RV_SIGCOMP_H
#define RV_SIGCOMP_H

#if defined(__cplusplus)
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSigCompTypes.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID (1)

/*-----------------------------------------------------------------------*/
/*                       SIGCOMP MANAGER API                             */
/*-----------------------------------------------------------------------*/

/************************************************************************************
* RvSigCompInitCfg
* ----------------------------------------------------------------------------------
* General: Initializes the RvSigCompCfg structure. This structure is given to the
*          RvSigCompConstruct function and it is used to initialize the SigComp module.
*          The RvSigCompInitCfg function relates to two types of parameters found in
*          the RvSigCompCfg structure:
*          (A) Parameters that influence the value of other parameters
*          (B) Parameters that are influenced by the value of other parameters.
*          The RvSigCompInitCfg will set all parameters of type A to default values
*          and parameters of type B to -1.
*
*          When calling the RvSigCompConstruct function all the B type parameters
*          will be calculated using the values found in the A type parameters.
*          if you change the A type values the B type values will be changed
*          accordingly.
*
* Return Value: (-)
* ----------------------------------------------------------------------------------
* Arguments:
* Input:  sizeOfCfg - The size of the configuration structure in bytes
*
* output: pSigCompCfg - The configuration structure containing the RADVISION SigComp default values.
***********************************************************************************/
RVAPI void RvSigCompInitCfg(IN  RvUint32     sizeOfCfg,
                            OUT RvSigCompCfg *pSigCompCfg);

/*************************************************************************
* SigCompConstruct
* ------------------------------------------------------------------------
* General: SigComp module constructor, called on initialization of the module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    sizeOfCfg   - The size of the configuration structure in bytes
*           pSigCompCfg - A pointer to a 'SipSigCompCfg' configuration structure
*
* Output    hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompConstruct(IN  RvUint32            sizeOfCfg,
                                             IN  RvSigCompCfg       *pSigCompCfg,
                                             OUT RvSigCompMgrHandle *hSigCompMgr);

/*************************************************************************
* RvSigCompDestruct
* ------------------------------------------------------------------------
* General: The SigComp module destructor, called on termination of the module.
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RVAPI void RVCALLCONV RvSigCompDestruct(IN RvSigCompMgrHandle hSigCompMgr);


/************************************************************************************
* RvSigCompGetResources
* ----------------------------------------------------------------------------------
* General: Gets the status of resources used by the RADVISION SigComp module.
*
* Return Value: RvStatus
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   hSigCompMgr       - Handle to the RADVISION SigComp module object.
*
* Output:  pSCMResources     - The resources in use by the whole SigComp module
***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetResources(
                                   IN  RvSigCompMgrHandle       hSigCompMgr,
                                   OUT RvSigCompModuleResources *pSCMResources);


/*************************************************************************
* RvSigCompCloseCompartment
* ------------------------------------------------------------------------
* General: Close session and delete it's memory compartment
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           hCompartment - a handle to the memory compartment
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCloseCompartment(
                                   IN RvSigCompMgrHandle         hSigCompMgr,
                                   IN RvSigCompCompartmentHandle hCompartment);


/*************************************************************************
* RvSigCompDeclareCompartment
* ------------------------------------------------------------------------
* General: The SIP stack declares to which memory compartment ("location")
*          the decompressed message (state variables) should be correlated.
*          The message is identified by its unique ID (number).
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           unqID		   - a unique ID (number) of the message processed 
*						     by the stack
*			algoName       - The algorithm name which will affect the 
*						     compartment compression manner. The algorithm
*						     name MUST be equal to the set algorithm name 
*						     during algorithm initialization. If this
*						     parameter value is NULL the default algo is used
*           *phCompartment - pointer to a handle of the memory compartment
*                               Pass zero (0) value for new compartment
*                               Pass one (1) value if message is not "legal"
*
* Output:   *phCompartment - pointer to a handle of the memory compartment
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDeclareCompartmentEx(
                                    IN RvSigCompMgrHandle            hSigCompMgr,
                                    IN RvInt32                       unqID,
                                    IN RvChar                        *algoName,
                                    INOUT RvSigCompCompartmentHandle *phCompartment);

/* Keep the old API */
#define RvSigCompDeclareCompartment(hSigCompMgr, unqID, phCompartment) \
    RvSigCompDeclareCompartmentEx(hSigCompMgr, unqID, 0, phCompartment)



/*************************************************************************
* RvSigCompCompressMessageEx
* ------------------------------------------------------------------------
* General:  Compress a message. This function should be called before 
*           sending the message. If the compartment handle is NULL, 
*           a new compartment should be created. Otherwise, use the 
*           handle to access the compartment.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*			algoName    - The algorithm name which will affect the 
*						  compression manner. This parameter is influencing 
*						  only when *phCompartment equals NULL. The algorithm
*						  name MUST be equal to the set algorithm name  
*						  during algorithm initialization. If this
*						  parameter value is NULL the default algo is used
*           *phCompartment - a pointer to a handle to the memory compartment
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the plain message + its size
* Output:   *phCompartment - return a handle to the compartment used
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the compressed message + its size
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCompressMessageEx(
                                   IN    RvSigCompMgrHandle         hSigCompMgr,
                                   IN    RvChar                     *algoName,
                                   INOUT RvSigCompCompartmentHandle *phCompartment,
                                   INOUT RvSigCompMessageInfo       *pMessageInfo);

/* Keep the old API */ 
#define RvSigCompCompressMessage(hSigCompMgr, phCompartment, pMessageInfo) \
    RvSigCompCompressMessageEx(hSigCompMgr, 0, phCompartment, pMessageInfo)

/*************************************************************************
* RvSigCompDecompressMessage
* ------------------------------------------------------------------------
* General:  Decompress a message. This function should be called 
*           after receiving a compressed message.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the plain message + its size
*
* Output:   *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the compressed message + its size
*           *pUnqId - a unique identifier (number) for use in the
*                        SigCompDeclareCompartment() function which must follow
*                        from the stack to the SigComp
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDecompressMessage(
                               IN    RvSigCompMgrHandle   hSigCompMgr,
                               INOUT RvSigCompMessageInfo *pMessageInfo,
                               OUT   RvUint32             *pUnqId);

/***************************************************************************
 * RvSigCompDictionaryAdd
 * ------------------------------------------------------------------------
 * General: adds an a dictionary to the list of dictionaries
 *          The number of dictionaries is limited to RVSIGCOMP_MAX_NUM_DICTIONARIES
 *          which is defined in RvSigCompTypes header-file, and also by the
 *          maximum amount of memory allocated to the dictionaries which is
 *          a configuration parameter (RvSigCompCfg.maxMemForDictionaries)
 *
 * Return Value: RV_OK            - On success.
 *               RV_ERROR_UNKNOWN - On failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the sigComp manager
 *        pDictionary - the dictionary to add
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDictionaryAdd(IN RvSigCompMgrHandle  hSigCompMgr,
                                                 IN RvSigCompDictionary *pDictionary);

/***************************************************************************
 * RvSigCompAlgorithmAdd
 * ------------------------------------------------------------------------
 * General: Adds an algorithm to the list of algorithms.
 *          The number of algorithms is limited to RVSIGCOMP_MAX_NUM_ALGORITHMS
 *          which is defined in the RvSigCompTypes header-file. 
 * NOTE: More than a single Algorithm might be added to the SigComp module.
 *       In this case the preceding algorithms are preferable when the 
 *       SigComp module determines, which algorithm will be used.
 *
 * Return Value: RV_OK            - Success.
 *               RV_ERROR_UNKNOWN - Failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the sigComp manager
 *        pAlgorithm  - the algorithm to add
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompAlgorithmAdd(IN RvSigCompMgrHandle hSigCompMgr,
                                                IN RvSigCompAlgorithm *pAlgorithm);


/***************************************************************************
 * RvSigCompGetState
 * ------------------------------------------------------------------------
 * General: Retrieves a state (such as dictionary) given the partial ID. 
 *          To be used by the SigComp compression algorithm.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSigCompMgr - a handle to the sigComp manager
 *         partialIdLength - length of partialID in bytes (6..20)
 *         *pPartialID  - pointer to the partial ID (derived from SHA1 of the state)
 *         **ppStateData  - to be filled with the pointer to the state's data
 *
 * Output: **ppStateData - filled with the pointer to the state's data
 *         *pStateAddress  - part of the state information
 *         *pStateInstruction  - part of the state information
 *         *pMinimumAccessLength  - part of the state information
 *         *pStateDataSize  - part of the state information
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetState(IN    RvSigCompMgrHandle  hSigCompMgr,
                                            IN    RvUint8             partialIdLength,
                                            IN    RvUint8             *pPartialID,
                                            INOUT RvUint8             **ppStateData,
                                            OUT   RvUint16            *pStateAddress,
                                            OUT   RvUint16            *pStateInstruction,
                                            OUT   RvUint16            *pMinimumAccessLength,
                                            OUT   RvUint16            *pStateDataSize);


/***************************************************************************
 * RvSigCompGetLocalStatesIdList
 * ------------------------------------------------------------------------
 * General: The compressor may call this function to get a list of the available
 *          states IDs, in order to inform the remote side of these 
 *          states (dictionaries). This list is stored as part of the 
 *          SigComp manager.
 *
 *          Structure of the local states list is:
 *          +---+---+---+---+---+---+---+---+
 *          | length_of_partial_state_ID_1  |
 *          +---+---+---+---+---+---+---+---+
 *          |                               |
 *          :  partial_state_identifier_1   :
 *          |                               |
 *          +---+---+---+---+---+---+---+---+
 *          :                               :
 *          +---+---+---+---+---+---+---+---+
 *          | length_of_partial_state_ID_n  |
 *          +---+---+---+---+---+---+---+---+
 *          |                               |
 *          :  partial_state_identifier_n   :
 *          |                               |
 *          +---+---+---+---+---+---+---+---+
 *          | 0   0   0   0   0   0   0   0 |
 *          +---+---+---+---+---+---+---+---+
 *
 *
 * Return Value: RV_OK            - On success.
 *               RV_ERROR_UNKNOWN - On failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSigCompMgr - a handle to the sigComp manager
 *         *pList - pointer to a list to be filled with data
 *
 * Output: *pList - pointer to a list to filled with data
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetLocalStatesIdList(
                                                 IN    RvSigCompMgrHandle  hSigCompMgr,
                                                 INOUT RvSigCompLocalStatesList *pList);

/***************************************************************************
 * RvSigCompCompartmentClearBytecodeFlag
 * ------------------------------------------------------------------------
 * General: This function resets the specified compartment compression bytecode flag
 *          It may be used for error-recovery purposes (to force resending bytecode)
 *
 * Return Value: RV_OK            - On success.
 *               RV_ERROR_UNKNOWN - On failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the SigComp manager
 *        hCompartment  - a handle to the sigComp compartment
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCompartmentClearBytecodeFlag(
                                          IN RvSigCompMgrHandle         hSigCompMgr,
                                          IN RvSigCompCompartmentHandle hCompartment);

/************************************************************************************
 * RvSigCompSetNewLogFilters
 * ----------------------------------------------------------------------------------
 * General: set the new log filters for the specified module in run time.
 *          note that the new filters are not only the ones that we want to change,
 *          but they are a new set of filters that the module is going to use.
 * Return Value:
 *          RV_OK             - log filters were set successfully
 *          RV_ERROR_NULLPTR  - the handle to the stack was NULL
 *          RV_ERROR_BADPARAM - the register id of this module is not used
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hSigCompMgr    - Handle to stack instance
 *          filters        - the new set of filters
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompSetNewLogFilters(IN RvSigCompMgrHandle  hSigCompMgr,
                                                    IN RvInt32             filters);


/************************************************************************************
 * RvSigCompIsLogFilterExist
 * ----------------------------------------------------------------------------------
 * General: Checks the existence of a filter in the SigComp module.
 * Return Value:
 *          RV_TRUE        - the filter exists
 *          RV_False       - the filter does not exist
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to sigcomp manager
 *          filter         - the filter that we want to check existence for
 * Output:  none
 ***********************************************************************************/
RVAPI RvBool RVCALLCONV RvSigCompIsLogFilterExist(
                                         IN RvSigCompMgrHandle   hSigCompMgr,
                                         IN RvSigCompLogFilters  filter);

/*-----------------------------------------------------------------------*/
/*                       SIGCOMP UTILS API                               */
/*-----------------------------------------------------------------------*/


/*************************************************************************
* RvSigCompStreamToMessage
* ------------------------------------------------------------------------
* General: convert stream-based data into message-based format
*           Handles the 0xFF occurrence as per RFC-3320 section 4.2.2.
*           Usually, the end result (message data)  will be 2 to a few 
*           bytes smaller. To be used by the SIP Stack only when working
*           over a TCP connection.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    streamDataSize - the size in bytes of the data in the buffer below
*           *pStreamData - a pointer to a buffer containing the stream data
*           *pMessageDataSize - the size in bytes of the buffer below
*
* Output:   *pMessageDataSize - the size in bytes of the data (used) in the buffer below
*           *pMessageData - a pointer to a buffer containing the message data
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompStreamToMessage(
                                   IN    RvUint32 streamDataSize,
                                   IN    RvUint8  *pStreamData,
                                   INOUT RvUint32 *pMessageDataSize,
                                   OUT   RvUint8  *pMessageData);
  
/*************************************************************************
* RvSigCompMessageToStream
* ------------------------------------------------------------------------
* General: convert message-based data into stream-based format
*           Handles the 0xFF occurrence as per RFC-3320 section 4.2.2.
*           Usually, the end result (stream data)  will be 2 to a few 
*           bytes bigger. To be used by the SIP Stack only when
*           working over a TCP connection.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    messageDataSize - the size in bytes of the data in the buffer above
*           *pMessageData - a pointer to a buffer containing the message data
*           *pStreamDataSize - the size in bytes of the buffer below
*
* Output:   *pStreamDataSize - the size in bytes of the data (used) in the buffer below
*           *pStreamData - a pointer to a buffer containing the stream data
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompMessageToStream(
                                   IN    RvUint32 messageDataSize,
                                   IN    RvUint8  *pMessageData,
                                   INOUT RvUint32 *pStreamDataSize,
                                   OUT   RvUint8  *pStreamData);
  

/*************************************************************************
* RvSigCompSHA1
* ------------------------------------------------------------------------
* General: Calculates the hash of the provided data, using SHA1.
*           This code implements the Secure Hashing Algorithm 1 as defined
*           in FIPS PUB 180-1 published April 17, 1995.
* Caveats:
*           SHA-1 is designed to work with messages less than 2^64 bits
*           long. Although SHA-1 allows a message digest to be generated
*           for messages of any number of bits less than 2^64, this
*           implementation only works with messages with a length that is
*           a multiple of the size of an 8-bit character.
*          This function is a utility function that may be used by the 
*          application for calculating SHA-1 signatures.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pData - pointer the data buffer
*            dataSize - the size of the data buffer, in bytes
*            hash - a pointer to an array, into which the 20 byte result will be inserted
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompSHA1 (IN  RvUint8  *pData,
                                         IN  RvUint32 dataSize,
                                         OUT RvUint8  hash[20]);



/*************************************************************************
* RvSigCompGenerateHeader
* ------------------------------------------------------------------------
* General: builds the sigComp header as described in RFC-3320 section 7
*          and figure 3 (p.24). This function is to be used by the 
*          compression algorithm.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hCompartment  - A handle to the compartment
*           *pBytecode    - a pointer to a buffer containing the bytecode
*           bytecodeSize  - size of bytecode in bytes
*           bytecodeStart - zero (0) indicates stateID of the bytecode is sent
*                           non-zero indicates starting address for bytecode
*           *pBuffer      - a pointer to the output buffer to contain the header
*           *pBufferSize  - on input, will contain the allocated size of the buffer
*
* Output:   *pBuffer      - a pointer to the output buffer to contain the header
*           *pBufferSize  - on output, will contain the number of used bytes
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGenerateHeader(
                                    IN    RvSigCompCompartmentHandle hCompartment,
                                    IN    RvUint8                    *pBytecode,
                                    IN    RvUint32                   bytecodeSize,
                                    IN    RvUint32                   bytecodeStart,
                                    INOUT RvUint8                    *pBuffer,
                                    INOUT RvUint32                   *pBufferSize);

/*************************************************************************
* RvSigCompSHA1Reset
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function will initialize the SHA1Context in preparation
*           for computing a new SHA1 message digest.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The context to reset/initialize
*
*************************************************************************/
RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Reset(
										IN RvSigCompSHA1Context *pContext); 

/*************************************************************************
* RvSigCompSHA1Input
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function accepts an array of octets as the next portion
*           of the message.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The SHA context to update
*           *message_array - An array of characters representing the next portion of the message.
*           length -  The length of the message in message_array
*
*************************************************************************/
RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Input(
						   IN RvSigCompSHA1Context *pContext,
                           IN const RvUint8		   *message_array,
                           IN RvUint32			    length); 

/*************************************************************************
* RvSigCompSHA1Result
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function will return the 160-bit message digest into the
*           messageDigest[] array provided by the caller.
*           NOTE: The first octet of hash is stored in the 0th element,
*           the last octet of hash in the 19th element.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The context to use to calculate the SHA-1 hash
*           *messageDigest[SHA1HASHSIZE] - Where the digest is returned
*
*************************************************************************/
RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Result(
						IN  RvSigCompSHA1Context *pContext,
                        OUT RvUint8				  messageDigest[RVSIGCOMP_SHA1_HASH_SIZE]); 


#if defined(__cplusplus)
}
#endif

#endif /* RV_SIGCOMP_H */























