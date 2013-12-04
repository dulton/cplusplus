/***********************************************************************
Filename   : SigCompCommon.h
Description: SigComp common functions header file
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


#ifndef SIGCOMP_COMMON_H
#define SIGCOMP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
    
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RvSigCompTypes.h"
#include "SigCompMgrObject.h"
#include "SigCompStateHandlerObject.h"


/*-----------------------------------------------------------------------*/
/*                              MACROS                                  */
/*-----------------------------------------------------------------------*/
/***************************************************************************
* CCS2SCS
* ------------------------------------------------------------------------
* General: converts a Common Core Status to SigComp Status
* Return Value:RvStatus
* ------------------------------------------------------------------------
* Arguments:
***************************************************************************/
#define CCS2SCS(_e) SigCompCommonCoreConvertStatus(_e)

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/

/* CRC16 related stuff */
#define PPPINITFCS16 0xFFFF /* Initial FCS value */
#define PPPGOODFCS16 0xF0B8 /* Good final FCS value */
      
 
RvSigCompShaStatus SHA1Input(
					IN RvSigCompSHA1Context   *context,
                    IN const RvUint8	      *message_array,
                    IN RvUint32				   length);


RvSigCompShaStatus SHA1Result(IN  RvSigCompSHA1Context *context,
                     OUT RvUint8     messageDigest[RVSIGCOMP_SHA1_HASH_SIZE]);


/*-----------------------------------------------------------------------*/
/*                    Common Functions                                   */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* SigCompCommonCoreConvertStatus
* ------------------------------------------------------------------------
* General: Converts a core status to SigComp status.
*
* Return Value: a SIP/SigComp status code.
* ------------------------------------------------------------------------
* Arguments:
* Input:     coreStatus - a core status code
***************************************************************************/
RvStatus RVCALLCONV SigCompCommonCoreConvertStatus(IN RvStatus coreStatus);



/***************************************************************************
* SigCompCommonEntityLock
* ------------------------------------------------------------------------
* General: Check if vacant and locks SigComp internal objects
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pMutex    - pointer to the lock object
*            *pLogMgr   - pointer to the log manager
*            *pLogSrc   - pointer to the log source
*            raH        - the RA from which the element ahs been allocated
*            element    - the element to be locked
***************************************************************************/
RvStatus SigCompCommonEntityLock(IN RvMutex     *pMutex,
                                 IN RvLogMgr    *pLogMgr,
                                 IN RvLogSource *pLogSrc,
                                 IN HRA         raH,
                                 IN RA_Element  element);


/***************************************************************************
* SigCompCommonEntityUnlock
* ------------------------------------------------------------------------
* General: Unlocks SigComp internal objects
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pMutex    - pointer to the lock object
*            *pLogMgr   - pointer to the log manager
***************************************************************************/
void SigCompCommonEntityUnlock(IN RvMutex     *pMutex,
                               IN RvLogMgr    *pLogMgr);


/*************************************************************************
* SigCompFastHash
* ------------------------------------------------------------------------
* General: A generic fast hashing function for hash-tables.
*   Hash a variable-length key into a 32-bit value
*   Originally by Bob Jenkins, December 1996, Public Domain.
*   Use for hash table lookup, or anything where one collision in 2^32 is
*   acceptable.  Do NOT use for cryptographic purposes.
*
*   Returns a 32-bit value.  Every bit of the key affects every bit of
*   the return value.  Every 1-bit and 2-bit delta achieves avalanche.
*   About 36+6len instructions.
*
*   The best hash table sizes are powers of 2.  There is no need to do
*   mod a prime (mod is so slow!).  If you need less than 32 bits,
*   use FAST_HASH_MASK.  For example, if you need only 10 bits, do
*   h = (h & FAST_HASH_MASK(10));
*   In which case, the hash table should have FAST_HASH_SIZE(10) elements.
  
* Return Value: RvUint32
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *k - pointer the key
*           length - the size of the key, in bytes
*           initval - can be any 4-byte value such as the previous hash, 
*                     or an arbitrary value.
*************************************************************************/

RvUint32 SigCompFastHash(IN void     *key, 
                         IN RvUint32 length, 
                         IN RvUint32 initval);



/*************************************************************************
* SigCompSHA1Calc
* ------------------------------------------------------------------------
* General: One-call SHA1 hash calculation.
*           Calculates the handle of the provided state, using SHA1.
*           This code implements the Secure Hashing Algorithm 1 as defined 
*           in FIPS PUB 180-1 published April 17, 1995.
*           Many of the variable names in this code, especially the
*           single character names, were used because those were the names
*           used in the publication.
* Caveats:
*           SHA-1 is designed to work with messages less than 2^64 bits
*           long. Although SHA-1 allows a message digest to be generated
*           for messages of any number of bits less than 2^64, this
*           implementation only works with messages with a length that is
*           a multiple of the size of an 8-bit character.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pStateData - pointer the state's data buffer
*            dataSize - the size of the data buffer, in bytes
* Output:    stateID - a stateID variable (RvUint8 [20], into which the result
*                       will be inserted.
*************************************************************************/
RvStatus SigCompSHA1Calc (IN RvUint8         *pStateData,
                          IN RvUint32        dataSize,
                          OUT RvSigCompStateID stateID);

/*************************************************************************
* SigCompCrc16Calc
* ------------------------------------------------------------------------
* General: Calculates the CRC16 of the provided state
*           Based on 16-bit FCS computation method presented in RFC-1662
*           section C.2.
*
* Note:    CRC16 of non-continuous buffers can be done in parts by sending each part
*           separately and zeroing the crc16 argument only the first time.
*           The result will be "accumulated" in this argument.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pStateData - pointer the state's data buffer
*            dataSize - the size of the data buffer, in bytes
*           *crc16 - a pointer to a (16 bit) variable, into which the result
*                       will be inserted. The variable must be zeroed.
*************************************************************************/

RvStatus  SigCompCrc16Calc(IN    RvUint8  *pStateData,
                           IN    RvUint32 dataSize,
                           INOUT RvUint16 *crc16);

/*************************************************************************
* SigCompCommonSHA1ProcessMessageBlock
* ------------------------------------------------------------------------
* General:
*   This function will process the next 512 bits of the message
*   stored in the messageBlock array.
* Comment:
*   Many of the variable names in this code, especially the
*   single character names, were used because those were the
*   names used in the publication.
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *context - The SHA context to update
*
*************************************************************************/
void SigCompCommonSHA1ProcessMessageBlock(IN RvSigCompSHA1Context *context); 


/*************************************************************************
* SigCompCommonSHA1PadMessage
* ------------------------------------------------------------------------
* General:
*   According to the standard, the message must be padded to an even
*   512 bits. The first padding bit must be a '1'. The last 64
*   bits represent the length of the original message. All bits in
*   between should be 0. This function will pad the message
*   according to those rules by filling the messageBlock array
*   accordingly. It will also call the ProcessMessageBlock function
*   provided appropriately. When it returns, it can be assumed that
*   the message digest has been computed.
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *context - The SHA context to update
*
*************************************************************************/
void SigCompCommonSHA1PadMessage(IN RvSigCompSHA1Context *context); 

/*************************************************************************
* SigCompAlgorithmFind
* ------------------------------------------------------------------------
* General:
*   Find the algorithm data structure which is related to the given 
*	algorithm name
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompMgr - A handle to the sigComp manager
*			algoName    - The searched algorithm name
* Output:   ppAlgorithm - The found algorithm structure
*************************************************************************/
RvStatus SigCompAlgorithmFind(IN  SigCompMgr          *pSigCompMgr,
							  IN  RvChar			  *algoName,
							  OUT RvSigCompAlgorithm **ppAlgorithm); 

#ifdef __cplusplus
}
#endif

#endif /* SIGCOMP_COMMON_H */


