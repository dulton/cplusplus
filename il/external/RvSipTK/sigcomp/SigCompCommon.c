/***********************************************************************
* Filename   : SigCompCommon.c
* Description: SigComp common functions source file
*
*    Author                         Date
*    ------                        ------
*    Gil Keini                    20040114
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
#include "rvstrutils.h"
#include "SigCompCommon.h"
#include "RvSigComp.h"
#include "SigCompMgrObject.h"

/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/

/**************************/
/* Hash related functions */
/**************************/
#define FAST_HASH_SIZE(n) ((RvUint32)1<<(n))
#define FAST_HASH_MASK(n) (FAST_HASH_SIZE(n)-1)
#define SIGCOMP_HASH_MIX(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c>>13); \
    b -= c; b -= a; b ^= (a<<8);  \
    c -= a; c -= b; c ^= (b>>13); \
    a -= b; a -= c; a ^= (c>>12); \
    b -= c; b -= a; b ^= (a<<16); \
    c -= a; c -= b; c ^= (b>>5);  \
    a -= b; a -= c; a ^= (c>>3);  \
    b -= c; b -= a; b ^= (a<<10); \
    c -= a; c -= b; c ^= (b>>15); \
}

/***********************/
/* SHA-1 related stuff */
/***********************/

/* Constants defined in SHA-1 */
static const RvUint32 K[] = {
    0x5A827999,
    0x6ED9EBA1,
    0x8F1BBCDC,
    0xCA62C1D6
};

/* Define the SHA1 circular left shift macro */
#define SHA1_CIRCULAR_SHIFT(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))

/***********************/
/* CRC16 related stuff */
/***********************/

/* FCS lookup table as calculated by a table generator.*/
static RvUint16 fcstab[256] = {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

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
RvStatus RVCALLCONV SigCompCommonCoreConvertStatus(IN RvStatus coreStatus)
{
    RvStatus internalStatus = RvErrorGetCode(coreStatus);
    if(coreStatus == RV_OK)
    {
        return RV_OK;
    }
    
    switch(internalStatus)
    {
    case RV_ERROR_UNKNOWN:
    case RV_ERROR_OUTOFRESOURCES:
    case RV_ERROR_BADPARAM:
    case RV_ERROR_NULLPTR:
    case RV_ERROR_OUTOFRANGE:
    case RV_ERROR_DESTRUCTED:
    case RV_ERROR_NOTSUPPORTED:
    case RV_ERROR_UNINITIALIZED:
    case RV_ERROR_TRY_AGAIN:
        return internalStatus;
    default:
        return RV_ERROR_UNKNOWN;
    }
} /* end of SigCompCommonCoreConvertStatus */


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
                                 IN RA_Element  element)
{
#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
    RV_UNUSED_ARG(pLogMgr);
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
#if  (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

    if (NULL == pMutex)
    {
        return(RV_ERROR_NULLPTR);
    }

    RvMutexLock(pMutex,pLogMgr);
    
    if (RA_ElemPtrIsVacant(raH, element) == RV_TRUE)
    {
        /* The element has been released */
        RvLogWarning(pLogSrc,(pLogSrc,
            "SigCompCommonEntityLock - tried to lock a non allocated entity (RA_ElemPtrIsVacant returned RV_TRUE)"));
        RvMutexUnlock(pMutex,pLogMgr);
        return(RV_ERROR_DESTRUCTED);
    }
    
    return(RV_OK);
} /* end of SigCompCommonEntityLock */



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
                               IN RvLogMgr    *pLogMgr)
{
#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
    RV_UNUSED_ARG(pLogMgr);
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/

    if (NULL == pMutex)
    {
        return;
    }

    RvMutexUnlock(pMutex,pLogMgr);
} /* end of SigCompCommonEntityUnlock */



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
                         IN RvUint32 initval)
{
    RvUint32 a,b,c,len;
    RvUint8 *k = (RvUint8 *)key;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value      */
    c = initval;         /* the previous hash value                   */

    /*---------------------------------------- handle most of the key */
    while (len >= 12)
    {
        a += (k[0] +((RvUint32)k[1]<<8) +((RvUint32)k[2]<<16) +((RvUint32)k[3]<<24));
        b += (k[4] +((RvUint32)k[5]<<8) +((RvUint32)k[6]<<16) +((RvUint32)k[7]<<24));
        c += (k[8] +((RvUint32)k[9]<<8) +((RvUint32)k[10]<<16)+((RvUint32)k[11]<<24));
        SIGCOMP_HASH_MIX(a,b,c);
        k += 12;
        len -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    /*lint -e616*/
    c += length;
    switch(len)              /* all the case statements fall through */
    {
    case 11: c+=((RvUint32)k[10]<<24);
    case 10: c+=((RvUint32)k[9]<<16);
    case 9 : c+=((RvUint32)k[8]<<8);
        /* the first byte of c is reserved for the length */
    case 8 : b+=((RvUint32)k[7]<<24);
    case 7 : b+=((RvUint32)k[6]<<16);
    case 6 : b+=((RvUint32)k[5]<<8);
    case 5 : b+=k[4];
    case 4 : a+=((RvUint32)k[3]<<24);
    case 3 : a+=((RvUint32)k[2]<<16);
    case 2 : a+=((RvUint32)k[1]<<8);
    case 1 : a+=k[0];
    default:
        /* case 0: nothing left to add */
        break;
    }
    /*lint +e616*/
    SIGCOMP_HASH_MIX(a,b,c);
    /*-------------------------------------------- report the result */
    return(c);
}


/*************************************************************************
* SigCompSHA1Calc
* ------------------------------------------------------------------------
* General: Calculates the handle of the provided state, using SHA1.
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
* Input:     *pStateData - pointer the state's data buffer
*            dataSize - the size of the data buffer, in bytes
* Output:    stateID - a stateID variable (RvUint8 [20], into which the result
*                       will be inserted.
*************************************************************************/

RvStatus  SigCompSHA1Calc (IN  RvUint8        *pStateData,
                           IN  RvUint32       dataSize,
                           OUT RvSigCompStateID stateID)
{
    RvSigCompSHA1Context sha;
    RvSigCompShaStatus   err;

    err = RvSigCompSHA1Reset(&sha);
    if (err)
    {
        /* SHA1Reset Error */
        return(RV_ERROR_UNKNOWN);
    }

    err = RvSigCompSHA1Input(&sha,(const RvUint8 *) pStateData, dataSize);
    if (err)
    {
        /* SHA1Input Error */
        return(RV_ERROR_UNKNOWN);
    }

    err = RvSigCompSHA1Result(&sha, stateID);
    if (err)
    {
        /* SHA1Result Error, could not compute message digest. */
        return(RV_ERROR_UNKNOWN);
    }

    return(RV_OK);
}



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
*           dataSize - the size of the data buffer, in bytes
*           *crc16 - a pointer to a (16 bit) variable, into which the result
*                       will be inserted. The variable must be zeroed.
*************************************************************************/

RvStatus  SigCompCrc16Calc(IN    RvUint8  *pStateData,
                           IN    RvUint32 dataSize,
                           INOUT RvUint16 *crc16)
{
    RvUint8 *pTempLocation = pStateData;

    /* this function assumes the RvUint16 is exactly 16 bits long */

    while (dataSize--)
    {
        *crc16 = (RvUint16) ((*crc16 >> 8) ^ fcstab[(*crc16 ^ *pTempLocation++) & 0xFF]);
    }
    return (RV_OK);
}

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
void SigCompCommonSHA1ProcessMessageBlock(IN RvSigCompSHA1Context *context)
{
    RvInt32  t;             /* Loop counter         */
    RvUint32 temp;          /* Temporary word value */
    RvUint32 W[80];         /* Word sequence        */
    RvUint32 A, B, C, D, E; /* Word buffers         */

    /* Initialize the first 16 words in the array W */
    for(t = 0; t < 16; t++)
    {
        W[t]  = context->messageBlock[t * 4] << 24;
        W[t] |= context->messageBlock[t * 4 + 1] << 16;
        W[t] |= context->messageBlock[t * 4 + 2] << 8;
        W[t] |= context->messageBlock[t * 4 + 3];
    }

    for(t = 16; t < 80; t++)
    {
        W[t] = SHA1_CIRCULAR_SHIFT(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }

    A = context->intermediateHash[0];
    B = context->intermediateHash[1];
    C = context->intermediateHash[2];
    D = context->intermediateHash[3];
    E = context->intermediateHash[4];

    for(t = 0; t < 20; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5,A) +
            ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30,B);
        B = A;
        A = temp;
    }

    for(t = 20; t < 40; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30,B);
        B = A;
        A = temp;
    }

    for(t = 40; t < 60; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5,A) +
            ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30,B);
        B = A;
        A = temp;
    }

    for(t = 60; t < 80; t++)
    {
        temp = SHA1_CIRCULAR_SHIFT(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1_CIRCULAR_SHIFT(30,B);
        B = A;
        A = temp;
    }

    context->intermediateHash[0] += A;
    context->intermediateHash[1] += B;
    context->intermediateHash[2] += C;
    context->intermediateHash[3] += D;
    context->intermediateHash[4] += E;
    context->messageBlockIndex = 0;
}

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
void SigCompCommonSHA1PadMessage(IN RvSigCompSHA1Context *context)
{
/* Check to see if the current message block is too small to hold
   the initial padding bits and length. If so, we will pad the
   block, process it, and then continue padding into a second block. 
*/
    if (context->messageBlockIndex > 55)
    {
        context->messageBlock[context->messageBlockIndex++] = 0x80;

        while(context->messageBlockIndex < 64)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }
        SigCompCommonSHA1ProcessMessageBlock(context);
        while(context->messageBlockIndex < 56)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }
    }
    else
    {
        context->messageBlock[context->messageBlockIndex++] = 0x80;
        while(context->messageBlockIndex < 56)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }
    }
    /* Store the message length as the last 8 octets     */
    context->messageBlock[56] = (RvUint8)(context->lengthHigh >> 24);
    context->messageBlock[57] = (RvUint8)(context->lengthHigh >> 16);
    context->messageBlock[58] = (RvUint8)(context->lengthHigh >> 8);
    context->messageBlock[59] = (RvUint8)(context->lengthHigh);
    context->messageBlock[60] = (RvUint8)(context->lengthLow >> 24);
    context->messageBlock[61] = (RvUint8)(context->lengthLow >> 16);
    context->messageBlock[62] = (RvUint8)(context->lengthLow >> 8);
    context->messageBlock[63] = (RvUint8)(context->lengthLow);
    SigCompCommonSHA1ProcessMessageBlock(context);
} /* end of SigCompCommonSHA1PadMessage */

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
							  OUT RvSigCompAlgorithm **ppAlgorithm)
{
	
    SigCompMgr *self = pSigCompMgr;
    RLIST_ITEM_HANDLE cur = 0;
    RLIST_POOL_HANDLE hListPool = self->hAlgorithmListPool;
    RLIST_HANDLE hList = self->hAlgorithmList;
    RvSigCompAlgorithm *curAlgo = 0;
	
    RvMutexLock(self->pLock, self->pLogMgr);
	
    if(algoName == 0) {
        /* Algorithm name wasn't specified, pick default algorithm if defined*/
        if(self->defaultAlgorithm) {
            *ppAlgorithm = self->defaultAlgorithm;
        } else {
            /* If default algo wasn't defined, pick the first algorithm on the list */
            RLIST_GetHead(hListPool, hList, &cur);
            *ppAlgorithm = (RvSigCompAlgorithm *)cur;
        }
        
        RvMutexUnlock(self->pLock, self->pLogMgr);
        return RV_OK;
    }
	
	
    RLIST_GetHead(hListPool, hList, &cur);
    
    while(cur != 0) {
        curAlgo = (RvSigCompAlgorithm *)cur;
        if(!RvStrcasecmp(curAlgo->algorithmName, algoName)) {
            break;
        }
        RLIST_GetNext(hListPool, hList, cur, &cur);
    }
    
    RvMutexUnlock(self->pLock, self->pLogMgr);
    *ppAlgorithm = (RvSigCompAlgorithm *)cur;
    return RV_OK;
}

/*************************************************************************
* SigCompAlgorithmSetDefault
* ------------------------------------------------------------------------
* General:
*   Sets an algorithm as default algorithm 
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompMgr - A handle to the sigComp manager
*			algoName    - The searched algorithm name
*************************************************************************/
RvStatus SigCompAlgorithmSetDefault(IN SigCompMgr  *pSigCompMgr, 
									IN RvChar	   *algoName)
{
    SigCompMgr		   *self = pSigCompMgr;
    RvSigCompAlgorithm *algo = 0;
    RvStatus			s;
	
    RvMutexLock(self->pLock, self->pLogMgr);
    s = SigCompAlgorithmFind(pSigCompMgr, algoName, &algo);
    if(s != RV_OK || algo == 0)
    {
        RvMutexUnlock(self->pLock, self->pLogMgr);
        return RV_ERROR_NOT_FOUND;
    }
	
    self->defaultAlgorithm = algo;
    RvMutexUnlock(self->pLock, self->pLogMgr);
    return RV_OK;
}
