/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                rpool.c
 *
 * The RPOOL package implements a data structure that provides an illusion of contiguous
 * memory that can increase according the the user's wishes.
 * In practice, the memory is built on top of RA, as a linked list of RA elements. RPOOL
 * hides the internal structure and allows the user to state parts of this memory as
 * physically contiguous and not just virtually contiguous, to allow faster access with
 * minimal needs of memory copies.
 *
 * RPOOL is mainly used as the data structure that holds ASN.1 messages, which are dynamic
 * by nature.
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  26-Jul-2000
 *
 ********************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include "RV_ADS_DEF.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "rvconfig.h"
#include "AdsRpool.h"
#include "rvmemory.h"
#include "rvansi.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include <stdio.h>
#endif
/* SPIRENT_END */

/*-------------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                              */
/*-------------------------------------------------------------------------*/

#ifdef RV_ADS_DEBUG
#define RPOOL_DEBUG_VALUE (0xaaaaaaaa)
#endif

/* Indicates the maximum number of bytes that user can store in a single
   RPOOL element.
   RPOOL element represents RA element, at the begining of which info about
   list element is stored. This piece of info is called Header.
   The rest space is available for user. Size of this space is defined
   by the RPOOL_MAX_USER_BYTES constant.
   Header is defined by RPOOL_FirstElem structure for the first element in
   RPOOL list, and - by RPOOL_Elem structure for the not first element.
   Number of bytes, stored by user in the RPOOL element is hold in
   the "usedBytes" field of the Header.
   Note the most left bit of the "usedBytes" field is used in order to differ
   header of the first element from header of the not first element.
 */
#define RPOOL_MAX_USER_BYTES (0x7fffffff)

/* Marks RPOOL element as the first element of RPOOL list */
#define RPOOL_SET_FIRST_ELEM_BIT(elem)  ((elem)->usedBytes |= 0x80000000)

/* Return RV_TRUE if the element is the first element in the RPOOL list*/
#define RPOOL_IS_FIRST_ELEM(elem) \
        ( (((elem)->usedBytes & 0x80000000) == 0) ? RV_FALSE : RV_TRUE )

/* Difference of size between header of the first element and all the others */
#define RPOOL_DIFF (sizeof(RPOOL_FirstElem) - sizeof(RPOOL_Elem))

/* Returns the number of bytes stored in the RPOOL block by user */
#define RPOOL_USED(elem) ((elem)->usedBytes & 0x7fffffff)

/* The number of characters which are used to represent an hexa-decimal numeric value */
#define HEXA_CHARS_NO 2
	 
/*-------------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                               */
/*-------------------------------------------------------------------------*/

/********************************************************************************************
 * RPOOL object structure
 * hra                 - RA handle. Holds the actual memory space used
 * blockSize           - Size of each block in RPOOL
 * numOfBlocks         - Number of blocks allocated for this pool
 * log                 - LOG handle
 * allocEmptyBlocks    - Indicate if new allocated blocks should be emptied or left trashed
 ********************************************************************************************/
typedef struct
{
    HRA                 hra;
    RvUint32           blockSize;
    RvUint32           numOfBlocks;
    RvLogSource         logSource;
    RvLogSource*        pLogSource;

} RPOOL;




/********************************************************************************************
 * RPOOL_Elem structure
 * This structure is the beginning of each RA element. It is used to create the
 * variable-length allocations of RPOOL using a linked-list structure.
 *
 * debugSpace   - Holds the same value for debug purposes
 * next         - Pointer to the next RA element
 * usedBytes    - Number of used bytes in the current element
 *                Each RPOOL block can use less bytes than it holds to allow parts of the
 *                RPOOL allocations to use unfragmented memory space.
 *                First block is marked by the most significant bit of usedBytes
 * size         - The total size of the allocation (may be fragmented between several RA
 *                elements)
 *                This value is only valid for the first allocation - it is here and not part
 *                of RPOOL_FirstElem since it can be packed into this 32bit structure
 * UsersCounter        - How many users do we have for this page.
 ********************************************************************************************/
typedef struct RPOOL_Elem_tag RPOOL_Elem;
/* CAUTION: the following struct size must a multiple of  */
/* 64 bit, otherwise a bus error would occur on unixware  */
/* due to alignment problem */
struct RPOOL_Elem_tag
{
#ifdef RV_ADS_DEBUG
    RvUint32       debugSpace;
    RvUint32       debugAlignment; /* Dummy member for 64 alignment */
#endif
    /* The order of the members is important, the bigger members has to come first */
    RPOOL_Elem*    next;
    RvInt32        size;
    RvInt32        usedBytes;
    RvUint32       UsersCounter;
    
};


/********************************************************************************************
 * RPOOL_FirstElem structure
 * This structure is the beginning of the first element of an RPOOL allocation. It holds some
 * additional information needed only for the first element of an allocation.
 * header   - The regular header of an RPOOL block
 * last     - Pointer to the last RA element of the allocation
 ********************************************************************************************/
typedef struct
{
    RPOOL_Elem  header;
    RPOOL_Elem* last;
#if (RV_ARCH_BITS == RV_ARCH_BITS_32)
    RvInt32        bits32Align; /* Dummy align. for 32bit OS (because of the pointer) */
#endif
} RPOOL_FirstElem;

/********************************************************************************************
 * RPOOL_HexaIndication enum
 * This enum is used for comparing 2 strings when characters are referred as equivalent to 
 * their ""%" HEX HEX" encoding. This indication tells if only one element of the compared
 * pair contains an escaped character.
 * ADS_RPOOL_HEXA_UNDEFINED   - Means that both elements includes regular characters (unescaped)
 * ADS_RPOOL_HEXA_ONLY_FIRST  - Only the first element includes an hexa character 
 * ADS_RPOOL_HEXA_ONLY_SECOND - Only the second element includes an hexa character 
 ********************************************************************************************/
typedef enum 
{
	ADS_RPOOL_HEXA_UNDEFINED,
	ADS_RPOOL_HEXA_ONLY_FIRST, 
	ADS_RPOOL_HEXA_ONLY_SECOND
} RPOOL_HexaIndication; 


/********************************************************************************************
 *                                          Internal functions
 ********************************************************************************************/


/********************************************************************************************
 * rpool_FreeBlocks
 * purpose : Free several blocks from a given one
 * input   : rpool          - Handle to the RPOOL used
 *           rpoolBlock     - RPOOL block to free from
 * output  : none
 * return  : none
 ********************************************************************************************/
static void RVCALLCONV rpool_FreeBlocks(IN RPOOL*         rpool,
                                 IN RPOOL_Elem*    rpoolBlock)
{
    RvStatus eStat= RV_OK;
    RPOOL_Elem* nextBlock;
    RPOOL_Elem* curBlock;

    curBlock = rpoolBlock;

    while (curBlock != NULL)
    {
        nextBlock = curBlock->next;

#ifdef RV_ADS_DEBUG
        if (curBlock->debugSpace != RPOOL_DEBUG_VALUE)
        {
            RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                      "FreeBlocks: Write beyond end of element"));
        }
#endif

        eStat = RA_DeAlloc(rpool->hra, (RA_Element)curBlock);
        if (eStat != RV_OK)
            break;

        curBlock = nextBlock;
    }
}


/********************************************************************************************
 * rpool_FreeBytes
 * purpose : Calculate the number of free bytes we've got for the current RPOOL block
 * input   : rpool          - Handle to the RPOOL used
 *           rpoolBlock     - RPOOL block to check
 * output  : none
 * return  : Number of free bytes in the RPOOL block
 ********************************************************************************************/
static RvUint32 RVCALLCONV rpool_FreeBytes(IN RPOOL*         rpool,
                                           IN RPOOL_Elem*    rpoolBlock)
{
    if (RV_TRUE == RPOOL_IS_FIRST_ELEM(rpoolBlock))
    {
        return (rpool->blockSize -
                (RPOOL_USED(rpoolBlock)));
    }
    else
    {
        return (rpool->blockSize +
                RPOOL_DIFF -
                rpoolBlock->usedBytes);
    }
}


/********************************************************************************************
 * rpool_GetPointer
 * purpose : Calculate the position of a pointer inside a single RPOOL block
 * input   : rpoolBlock     - RPOOL block to check
 *           blockOffset    - Offset in bytes inside the block
 * output  : none
 * return  : Pointer inside the RPOOL block
 ********************************************************************************************/
#ifndef RV_USE_MACROS
static RvChar* RVCALLCONV rpool_GetPointer(IN RPOOL_Elem*       rpoolBlock,
                                           IN RvInt32         blockOffset)
{
    if (RV_TRUE == RPOOL_IS_FIRST_ELEM(rpoolBlock))
    {
        return ((RvChar *)rpoolBlock + sizeof(RPOOL_FirstElem) + blockOffset);
    }
    else
    {
        return ((RvChar*)rpoolBlock + sizeof(RPOOL_Elem) + blockOffset);
    }
}
#else
#define rpool_GetPointer(rpoolBlock, blockOffset) \
            ( \
            (RV_TRUE == RPOOL_IS_FIRST_ELEM(rpoolBlock)) ? \
            ((RvChar *)rpoolBlock + sizeof(RPOOL_FirstElem) + blockOffset) : \
            ((RvChar*)rpoolBlock + sizeof(RPOOL_Elem) + blockOffset) \
            )

#endif /* #ifndef RV_USE_MACROS */


/********************************************************************************************
 * rpool_AllocBlocks
 * purpose : Allocate several blocks to be used later on.
 *           If the allocation fails, then no blocks are allocated.
 * input   : rpool          - Handle to the RPOOL used
 *           isFirst        - TRUE iff the first block allocated will also be the first block
 *                            in the RPOOL allocation
 *           numBytes       - Number of bytes to allocate
 *           bytesInBlock   - Number of bytes to use in each block
 *                            If bytesInBlock==0, then the maximum will be allocated
 * output  : firstElem      - First allocated RPOOL block
 *           lastElem       - Last allocated RPOOL block
 *                            Used to "fix" the RPOOL element's header information
 * return  : RV_OK on success
 *           RV_ERROR_OUTOFRESOURCES on failure
 ********************************************************************************************/
static RvStatus RVCALLCONV rpool_AllocBlocks(IN  RPOOL*         rpool,
                                             IN  RvBool         isFirst,
                                             IN  RvUint32       numBytes,
                                             IN  RvUint32       bytesInBlock,
                                             OUT RPOOL_Elem**   firstElem,
                                             OUT RPOOL_Elem**   lastElem)
{
    RvStatus       status;
    RPOOL_Elem*    newElemPtr;
    RPOOL_Elem*    prevElemPtr;
    RvUint32       bytesLeft, bytes;
    RvBool         firstAllocation;

    firstAllocation = RV_TRUE;
    prevElemPtr     = NULL;
    bytesLeft       = numBytes;
    status          = RV_OK;
    *lastElem       = NULL;
    *firstElem      = NULL;

    if (bytesInBlock == 0)
    {
        bytes = rpool->blockSize + RPOOL_DIFF;
    }
    else
    {
        bytes = bytesInBlock;
    }

    if (bytesLeft == 0)
    {
        /* We need to allocate a single block and leave it empty */
        status = RA_Alloc(rpool->hra, (RA_Element *)firstElem);
        if (status != RV_OK)
        {
            return status;
        }

#ifdef RV_ADS_DEBUG
        (*firstElem)->debugSpace = RPOOL_DEBUG_VALUE;
#endif
        (*firstElem)->next      = NULL;
        (*firstElem)->usedBytes = 0;
        *lastElem               = *firstElem;
        return RV_OK;
    }

    while ((bytesLeft > 0) && (status == RV_OK))
    {
        /* Allocate a new RA element */
        status = RA_Alloc(rpool->hra, (RA_Element *)&newElemPtr);

        if (status == RV_OK)
        {
            /* Clear the block if necessary */
            memset(newElemPtr, 0, rpool->blockSize + sizeof(RPOOL_FirstElem));

            /* Link to previous element */
            if (prevElemPtr != NULL)
            {
                prevElemPtr->next = newElemPtr;
            }

            /* Set the header of the block */
#ifdef RV_ADS_DEBUG
            newElemPtr->debugSpace = RPOOL_DEBUG_VALUE;
#endif

            if (firstAllocation)
            {
                if (isFirst && (bytesInBlock == 0))
                {
                    newElemPtr->usedBytes = RvMin(rpool->blockSize, bytesLeft);
                }
                else
                {
                    newElemPtr->usedBytes = RvMin(bytes, bytesLeft);
                }

                *firstElem      = newElemPtr;
                firstAllocation = RV_FALSE;
            }
            else
            {
                (*lastElem)->next     = newElemPtr;
                newElemPtr->usedBytes = RvMin(bytes, bytesLeft);
            }

            bytesLeft -= newElemPtr->usedBytes;
            *lastElem = newElemPtr;
        }
    }

    if ((*lastElem) != NULL)
        (*lastElem)->next = NULL;

    if (status != RV_OK)
    {
        /* Clean the allocations - we've got an error somewhere... */
        prevElemPtr = *firstElem;
        while (prevElemPtr != NULL)
        {
            /* Point to the next element */
            newElemPtr = prevElemPtr->next;

            /* Free current element */
            RA_DeAlloc(rpool->hra, (RA_Element)prevElemPtr);

            /* Move to the next element */
            prevElemPtr = newElemPtr;
        }

        return status;
    }

    return RV_OK;
}


/********************************************************************************************
 * rpool_FindBlock
 * purpose : Find the RA block where offset is
 * input   : pool    - RPOOL element
 *           element - Pointer to the pool element
 *           offset  - The offset being searched
 * output  : offset  - The offset inside the RA block
 * return  : Pointer to the RA block or NULL if failed
 ********************************************************************************************/
#ifdef RV_USE_MACROS
RVSTATICINLINE RPOOL_Elem* rpool_FindBlock(IN    RPOOL*    rpool,
                                           IN    HPAGE     element,
                                           INOUT RvInt32*  offset)
#else
static         RPOOL_Elem* rpool_FindBlock(IN    RPOOL*    rpool,
                                           IN    HPAGE     element,
                                           INOUT RvInt32*  offset)
#endif /* #ifndef RV_USE_MACROS */
{
    RPOOL_Elem* curElem;
    RvInt32    count, bytes;
#ifdef RV_ADS_DEBUG
    RvBool     error = RV_FALSE;
#else
	RV_UNUSED_ARG(rpool)
#endif

    curElem = (RPOOL_Elem *)element;

    if (curElem == NULL)
    {
        return NULL;
    }

    /* See if offset is within bounds */
    if (( *offset == 0 ) && ( curElem->size == 0 ))
    {
         return curElem;
    }
    else if (((*offset) < 0) || (curElem->size <= (*offset)))
    {
          return NULL;
    }

    count = 0;
    bytes = RPOOL_USED(curElem);

#ifdef RV_ADS_DEBUG
    if (curElem->debugSpace != RPOOL_DEBUG_VALUE)
    {
        RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                   "FindBlock: Write beyond end of element #1, pool=0x%p, page=0x%p (%s)",
                   rpool,element,RA_GetName(rpool->hra)));
        error = RV_TRUE;
    }

    /* Skip RA elements until we find the needed one */
    while ((error != RV_TRUE) && (count + bytes <= *offset))
    {
        count += bytes;
        curElem = curElem->next;

        /* Make some debug checks */
        if (curElem == NULL)
        {
            RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                      "FindBlock: Error finding offset!"));
            error = RV_TRUE;
        }
        else
        {
            if (curElem->debugSpace != RPOOL_DEBUG_VALUE)
            {
                RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                          "FindBlock: Write beyond end of element #2, pool=0x%p, page=0x%p (%s)",
                          rpool,element,RA_GetName(rpool->hra)));
                error = RV_TRUE;
            }
            bytes = curElem->usedBytes;
        }
    }

    if (error == RV_FALSE)
    {
        *offset -= count;
        return curElem;
    }

    /* can't go on without a valid element pointer */
    RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
             "FindBlock: Invalid element pointer"));
    return NULL;
#else
    /* Skip RA elements until we find the needed one */
    while (count + bytes <= *offset)
    {
        count += bytes;
        curElem = curElem->next;
        if (curElem != NULL)
            bytes = RPOOL_USED(curElem);
    }

    *offset -= count;
    return curElem;
#endif
}

/********************************************************************************************
 * rpool_StrcmpInternal
 * purpose : Compare two NULL terminated string on RPOOL , the comparison is case sensitive
 *           or not according to a given parameter.
 * input   : FirstPool     - Handle to the pool of the first string
 *           FirstPage     - RPOOL page of the first string.
 *           FirstOffset   - Position of the string in the first page.
 *           SecPool       - Handle to the pool of the second string
 *           SecPage       - RPOOL page of the second string
 *           SecOffset     - Position of the string in the second page.
 *           CaseSensitive - Boolean flags that states if the comparison is case sensitive or
 *                           not ( True for case sensitive false for non case sensitive ).
 * output  : None.
 * return  : RV_TRUE  - If both strings are identical.
 *           RV_FALSE - If not.( or if there is a "bug" and one of the strings is not
 *                      null terminated.
 ***********************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
RV_INT8 rpool_StrcmpInternal (IN HRPOOL     FirstPool,
                              IN HPAGE      FirstPage,
                              IN RV_INT32  FirstOffset,
                              IN HRPOOL     SecPool,
                              IN HPAGE      SecPage,
                              IN RV_INT32  SecOffset,
                              IN RV_BOOL    CaseSensitive )
#else /* !defined(UPDATED_BY_SPIRENT) */
static RvBool rpool_StrcmpInternal (IN HRPOOL     FirstPool,
                              IN HPAGE      FirstPage,
                              IN RvInt32  FirstOffset,
                              IN HRPOOL     SecPool,
                              IN HPAGE      SecPage,
                              IN RvInt32  SecOffset,
                              IN RvBool    CaseSensitive )
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
{
    RPOOL*         rpool1;
    RPOOL*         rpool2;
    RPOOL_Elem     *FirstElem;
    RPOOL_Elem     *SecElem;
    RvInt32       CurrFirstOffset = FirstOffset;
    RvInt32       CurrSecOffset = SecOffset;
    RvInt32       CurrFirstOffsetInOnePage = 0;
    RvInt32       CurrSecOffsetInOnePage = 0;
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    RV_INT32       StringCmpResult;
#else
    RvBool        StringAreEquel;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
    RvBool        EndOfSearch;
    RvChar*       FirstPtr;
    RvChar*       SecPtr;
    RvInt32       SafeCounter;


    rpool1 = (RPOOL *)FirstPool;
    rpool2 = (RPOOL *)SecPool;


    /* Find the right RA element */
    FirstElem = rpool_FindBlock(rpool1, FirstPage, &CurrFirstOffset);
    SecElem = rpool_FindBlock(rpool2, SecPage, &CurrSecOffset);

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    if ((FirstElem == NULL) && (SecElem == NULL))
        return ((RV_INT8) 0);

    if (FirstElem == NULL)
        return ((RV_INT8) -1);
    else if ( SecElem == NULL )
        return ((RV_INT8) 1);

    StringCmpResult = 0;
    EndOfSearch     = RV_FALSE;
#else /* !defined(UPDATED_BY_SPIRENT) */
    if ((FirstElem == NULL) && (SecElem == NULL))
        return RV_TRUE;

    if ((FirstElem == NULL) || ( SecElem == NULL ))
        return RV_FALSE;

    StringAreEquel = RV_TRUE;
    EndOfSearch    = RV_FALSE;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    FirstPtr = rpool_GetPointer(FirstElem, CurrFirstOffset);
    SecPtr   = rpool_GetPointer(SecElem, CurrSecOffset);

    SafeCounter = 0;



/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    while (( StringCmpResult == 0 ) && (!EndOfSearch ))
#else /* !defined(UPDATED_BY_SPIRENT) */
    while (( StringAreEquel ) && (!EndOfSearch ))
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
    {
       SafeCounter++;
       if ( SafeCounter > 100000 )
       {

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
           StringCmpResult = 1;
           RvLogExcep(rpool1->pLogSource, (rpool1->pLogSource,
                      "In rpool_StrcmpInternal - infinite loop"));
#else /* !defined(UPDATED_BY_SPIRENT) */
           StringAreEquel = RV_FALSE;
           RvLogExcep(rpool1->pLogSource, (rpool1->pLogSource,
                      "In rpool_StrcmpInternal - infinite loop"));
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
       }

       if ( ( ( FirstPtr[CurrFirstOffsetInOnePage] == SecPtr[CurrSecOffsetInOnePage] ) &&
              ( CaseSensitive == RV_TRUE )) ||
            ( ( tolower((int)FirstPtr[CurrFirstOffsetInOnePage]) == tolower((int)SecPtr[CurrSecOffsetInOnePage])) &&
              ( CaseSensitive == RV_FALSE )))
       {
           if ( FirstPtr[CurrFirstOffsetInOnePage] == '\0' )
               EndOfSearch = RV_TRUE;
           else
           {
              if ( CurrFirstOffset == RPOOL_USED(FirstElem)-1 )
              {
                 FirstElem       = FirstElem->next;
                 CurrFirstOffset = 0;
                 CurrFirstOffsetInOnePage = 0;
                 if ( FirstElem == NULL )
                 {
                     EndOfSearch    = RV_TRUE;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
                     StringCmpResult = -1;
#else /* !defined(UPDATED_BY_SPIRENT) */
                     StringAreEquel = RV_FALSE;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
                 }
                 else
                     FirstPtr = rpool_GetPointer(FirstElem, CurrFirstOffset);
              }
              else
              {
                  CurrFirstOffset++;
                  CurrFirstOffsetInOnePage++;
              }

              if ( CurrSecOffset == RPOOL_USED(SecElem)-1 )
              {
                 SecElem       = SecElem->next;
                 CurrSecOffset = 0;
                 CurrSecOffsetInOnePage = 0;
                 if ( SecElem == NULL )
                 {
                     EndOfSearch = RV_TRUE;
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
                     StringCmpResult = 1;
#else /* !defined(UPDATED_BY_SPIRENT) */
                     StringAreEquel = RV_FALSE;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
                 }
                 else
                     SecPtr   = rpool_GetPointer(SecElem, CurrSecOffset);
              }
              else
              {
                  CurrSecOffset++;
                  CurrSecOffsetInOnePage++;
              }
           }
       }
       else
       {
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
           //printf("rpool_StrcmpInternal(): First unequal char: FirstPtr[CurrFirstOffsetInOnePage]=%c, SecPtr[CurrSecOffsetInOnePage]=%c\n",FirstPtr[CurrFirstOffsetInOnePage], SecPtr[CurrSecOffsetInOnePage]);
           if ( CaseSensitive == RV_TRUE )
               StringCmpResult = FirstPtr[CurrFirstOffsetInOnePage] -
                   SecPtr[CurrSecOffsetInOnePage];
           else // not case sensitive
               StringCmpResult = tolower(FirstPtr[CurrFirstOffsetInOnePage]) -
                   tolower(SecPtr[CurrSecOffsetInOnePage]);

           // abs(StringCmpResult) should fit into 7 bits
           if (StringCmpResult > 0x7F)
               StringCmpResult = 0x7F;
           else if (StringCmpResult < -0x7F)
               StringCmpResult = -0x7F;
#else /* !defined(UPDATED_BY_SPIRENT) */
           StringAreEquel = RV_FALSE;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
       }
    }

    

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    return ((RV_INT8)StringCmpResult);
#else /* !defined(UPDATED_BY_SPIRENT) */
    return ( StringAreEquel);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
}

/********************************************************************************************
 * rpool_StrcmpHexaInternal
 * purpose : Compare two NULL terminated string on RPOOL , the comparison is case sensitive
 *           or not according to a given parameter. 
 * IMPORTANT: The function handles characters equivalent to their ""%" HEX HEX" encoding
 *			  according to RFC 2396. 
 *
 * input   : FirstPool     - Handle to the pool of the first string
 *           FirstPage     - RPOOL page of the first string.
 *           FirstOffset   - Position of the string in the first page.
 *           SecPool       - Handle to the pool of the second string
 *           SecPage       - RPOOL page of the second string
 *           SecOffset     - Position of the string in the second page.
 *           CaseSensitive - Boolean flags that states if the comparison is case sensitive or
 *                           not ( True for case sensitive false for non case sensitive ).
 * output  : None.
 * return  : RV_TRUE  - If both strings are identical.
 *           RV_FALSE - If not, or if there is a "bug" and one of the strings is not
 *                      null terminated.
 ***********************************************************************************************/
static RvBool rpool_StrcmpHexaInternal (IN HRPOOL     FirstPool,
                              IN HPAGE      FirstPage,
                              IN RvInt32  FirstOffset,
                              IN HRPOOL     SecPool,
                              IN HPAGE      SecPage,
                              IN RvInt32  SecOffset,
                              IN RvBool    CaseSensitive )
{
    RPOOL*         rpool1;
    RPOOL*         rpool2;
    RPOOL_Elem     *FirstElem;
    RPOOL_Elem     *SecElem;
    RvInt32       CurrFirstOffset = FirstOffset;
    RvInt32       CurrSecOffset = SecOffset;
    RvInt32       CurrFirstOffsetInOnePage = 0;
    RvInt32       CurrSecOffsetInOnePage = 0;
    RvBool        StringAreEquel;
    RvBool        EndOfSearch;
    RvChar*       FirstPtr;
    RvChar*       SecPtr;
    RvInt32       SafeCounter;
	/* An escaped character is represented by 2 hexadecimal chars+'\0' */
	RvChar		  hexaChars[HEXA_CHARS_NO+1]; 
	RvInt32		  hexaOffset    = 0;
	RvChar		  firstCompChar = '\0'; 
	RvChar		  secCompChar   = '\0';
	RPOOL_HexaIndication currHIndication = ADS_RPOOL_HEXA_UNDEFINED;  

	/* Initialize the last character of hexaChars to be '\0' ALWAYS */
	/* The last character MUSTN'T be changed */
	hexaChars[HEXA_CHARS_NO] = '\0'; 

    rpool1 = (RPOOL *)FirstPool;
    rpool2 = (RPOOL *)SecPool;


    /* Find the right RA element */
    FirstElem = rpool_FindBlock(rpool1, FirstPage, &CurrFirstOffset);
    SecElem = rpool_FindBlock(rpool2, SecPage, &CurrSecOffset);

    if ((FirstElem == NULL) && (SecElem == NULL))
        return RV_TRUE;

    if ((FirstElem == NULL) || ( SecElem == NULL ))
        return RV_FALSE;

    StringAreEquel = RV_TRUE;
    EndOfSearch    = RV_FALSE;

    FirstPtr = rpool_GetPointer(FirstElem, CurrFirstOffset);
    SecPtr   = rpool_GetPointer(SecElem, CurrSecOffset);

    SafeCounter = 0;

    while (( StringAreEquel ) && (!EndOfSearch ))
    {
       SafeCounter++;
       if ( SafeCounter > 100000 )
       {
           StringAreEquel = RV_FALSE;
           RvLogExcep(rpool1->pLogSource, (rpool1->pLogSource,
                      "In rpool_StrcmpHexaInternal - infinite loop"));
       }

	   switch (currHIndication) 
	   {
	   case ADS_RPOOL_HEXA_ONLY_FIRST:
		   hexaChars[hexaOffset] = FirstPtr[CurrFirstOffsetInOnePage];
		   /* The hexadecimal value is completed, thus the first char is found */ 
		   if (hexaOffset == HEXA_CHARS_NO-1 /* -1 since we start the offset from 0 */) 
		   {
			   currHIndication = ADS_RPOOL_HEXA_UNDEFINED;
			   RvSscanf(hexaChars,"%x",(RvUint32*)&firstCompChar);
		   }
		   else
		   {
			   hexaOffset++; /* Look for the next hexadecimal character */
		   }
		   break;
	   case ADS_RPOOL_HEXA_ONLY_SECOND:
		   hexaChars[hexaOffset] = SecPtr[CurrSecOffsetInOnePage]; 
		   /* The hexadecimal value is completed, thus the second char is found */ 
		   if (hexaOffset == HEXA_CHARS_NO-1 /* -1 since we start the offset from 0 */) 
		   {
			   currHIndication = ADS_RPOOL_HEXA_UNDEFINED;
			   RvSscanf(hexaChars,"%x",(RvUint32*)&secCompChar);
		   }
		   else
		   {
			   hexaOffset++; /* Look for the next hexadecimal character */
		   }
		   break;
	   case ADS_RPOOL_HEXA_UNDEFINED:
		   firstCompChar = FirstPtr[CurrFirstOffsetInOnePage]; 
		   secCompChar   = SecPtr[CurrSecOffsetInOnePage]; 

		   /* If one of the characters are '%', it's following 2 characters are */ 
		   /* referred to a single escaped character (according to RFC 2396)    */ 
		   if (firstCompChar != secCompChar)
		   {
			   if (firstCompChar == '%')
			   {
					currHIndication = ADS_RPOOL_HEXA_ONLY_FIRST; 
			   }
			   else if (secCompChar == '%')
			   {
				   currHIndication = ADS_RPOOL_HEXA_ONLY_SECOND; 
			   }
			   hexaOffset  = 0;
		   }
		   break; 
	   }	   

       if ((currHIndication != ADS_RPOOL_HEXA_UNDEFINED) || /* In this case only one of the elements should be promoted */
		   (firstCompChar == secCompChar && CaseSensitive == RV_TRUE ) ||
           (tolower((int)firstCompChar) == tolower((int)secCompChar) && CaseSensitive == RV_FALSE))
       {
           if ( firstCompChar == '\0' )
               EndOfSearch = RV_TRUE;
           else
           {
			  /* Only if the second element DOESN'T include an hexadecimal the pointer is promoted */
			  if (currHIndication != ADS_RPOOL_HEXA_ONLY_SECOND)
			  {
				  if ( CurrFirstOffset == RPOOL_USED(FirstElem)-1 ) 
				  {
					  FirstElem       = FirstElem->next;
					  CurrFirstOffset = 0;
					  CurrFirstOffsetInOnePage = 0;
					  if ( FirstElem == NULL )
					  {
						  EndOfSearch    = RV_TRUE;
						  StringAreEquel = RV_FALSE;
					  }
					  else
						  FirstPtr = rpool_GetPointer(FirstElem, CurrFirstOffset);
				  }
				  else
				  {
					  CurrFirstOffset++;
					  CurrFirstOffsetInOnePage++;
				  }
				  
			  }
              
			  /* Only if the first element DOESN'T include an hexadecimal the pointer is promoted */
			  if (currHIndication != ADS_RPOOL_HEXA_ONLY_FIRST)
			  {
				  if ( CurrSecOffset == RPOOL_USED(SecElem)-1 )
				  {
					 SecElem       = SecElem->next;
					 CurrSecOffset = 0;
					 CurrSecOffsetInOnePage = 0;
					 if ( SecElem == NULL )
					 {
						 EndOfSearch = RV_TRUE;
						 StringAreEquel = RV_FALSE;
					 }
					 else
						 SecPtr   = rpool_GetPointer(SecElem, CurrSecOffset);
				  }
				  else
				  {
					  CurrSecOffset++;
					  CurrSecOffsetInOnePage++;
				  }
			  }
		   }
       }
       else
       {
           StringAreEquel = RV_FALSE;
       }
    }


    return ( StringAreEquel);
}



/*-------------------------------------------------------------------------*/
/*                          FUNCTIONS IMPLEMENTATION                       */
/*-------------------------------------------------------------------------*/

/********************************************************************************************
 * RPOOL_Construct
 * purpose : Allocates memory for a new pool. A pool contains a set of memory pages.
 *           The function receives parameters that indicate the number of pages and
 *           the size of each page inside the pool.
 * input   : pageSize           - The size of each page in the requested pool.
 *           maxNumOfPages      - The number of pages that should be allocated.
 *           logHandle          - LOG handle to use for log messages. You can use
 *                                the RADVISION SIP Stack Manager functions to get
 *                                the log handle. If logHandle is NULL, no
 *                                messages are printed to the log.
 *           allocEmptyPages    - Indicates whether or not the content of new
 *                                allocated pages is initialized to zero.
 *           name               - Name of the RPOOL instance (used for log messages)
 * output  : none
 * return  : Returns a handle to the RPOOL instance when the function
 *           is successful. Otherwise, the function returns NULL.
 ********************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
HRPOOL RVAPI RVCALLCONV RPOOL_Construct_trace(IN RvInt32         pageSize,
                                        IN RvInt32         maxNumOfPages,
                                        IN RV_LOG_Handle    logHandle,
                                        IN RvBool          allocEmptyPagess,
                                        IN const char*      name,
                                        const char* file, int line)
{
    return RPOOL_CoreConstruct_trace(pageSize,
                               maxNumOfPages,
                               (RvLogMgr*)logHandle,
                               allocEmptyPagess,
                               name, file, line);
}
#else
HRPOOL RVAPI RVCALLCONV RPOOL_Construct(IN RvInt32         pageSize,
                                        IN RvInt32         maxNumOfPages,
                                        IN RV_LOG_Handle    logHandle,
                                        IN RvBool          allocEmptyPagess,
                                        IN const RvChar*      name)
{
    return RPOOL_CoreConstruct(pageSize,
                               maxNumOfPages,
                               (RvLogMgr*)logHandle,
                               allocEmptyPagess,
                               name);
}
#endif
/* SPIRENT_END */

/********************************************************************************************
 * RPOOL_CoreConstruct
 * purpose : Allocate memory for a new pool.
 * input   : elemSize           - The size of each element (block) in the requested pool
 *                                This is not the exact element size that will be used. For
 *                                performance improvements, the size will increase on blocks
 *                                different than the first one on the same element.
 *           maxNumOfBlocks     - The number of records that should be allocated
 *           pLogMgr            - pointer to the log manager.
 *           allocEmptyBlocks   - Do we open new allocated blocks with empty buffers
 *           name               - Name of the RPOOL instance (used for log messages)
 * output  : none
 * return  : A handle to the RPOOL instance on success.
 *           NULL on failure
 ********************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
HRPOOL RVAPI RVCALLCONV RPOOL_CoreConstruct_trace(IN RvInt32         elemSize,
                                            IN RvInt32         maxNumOfBlocks,
                                            IN RvLogMgr*        pLogMgr,
                                            IN RvBool          allocEmptyBlocks,
                                            IN const char*      name,
                                            const char* file, int line)
#else
HRPOOL RVAPI RVCALLCONV RPOOL_CoreConstruct(IN RvInt32         elemSize,
                                            IN RvInt32         maxNumOfBlocks,
                                            IN RvLogMgr*       pLogMgr,
                                            IN RvBool          allocEmptyBlocks,
                                            IN const RvChar*   name)
#endif
/* SPIRENT_END */
{
    RPOOL* rpool;
    RvInt32 allocSize;
    RvStatus  crv;
    
    crv = RV_OK;


    /*check validity*/
    if(elemSize<=0 || maxNumOfBlocks<=0)
    {
        return NULL;
    }
	if (elemSize >= RPOOL_MAX_USER_BYTES)
	{
		return NULL;
	}

    /* Use a single allocation for both RPOOL and RA */
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RPOOL), NULL, (void*)&rpool))
        return NULL;

    /* Initialize RA */
    rpool->hra = RA_ConstructFrom_trace(elemSize + sizeof(RPOOL_FirstElem),
                                  maxNumOfBlocks,
                                  NULL,
                                  NULL,
                                  pLogMgr, name, file, line);
#else

    allocSize =
        RA_AllocationSize(elemSize + sizeof(RPOOL_FirstElem), maxNumOfBlocks) +
        sizeof(RPOOL);

    if(allocSize / elemSize < maxNumOfBlocks)
    {
        return NULL;
    }
    if (RV_OK != RvMemoryAlloc(NULL, allocSize, NULL, (void*)&rpool))
        return NULL;

    /* Initialize RA */
    rpool->hra = RA_ConstructFrom(elemSize + sizeof(RPOOL_FirstElem),
                                  maxNumOfBlocks,
                                  (RvChar *)rpool + sizeof(RPOOL),
                                  NULL,
                                  pLogMgr, name);
#endif
/* SPIRENT_END */

    if (rpool->hra == NULL)
    {
        RvMemoryFree(rpool,NULL);
        return NULL;
    }

    /* Set RPOOL information */

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    /*
     * we have constructed this pool using ABACUS internal memory management which has its own
     * memory alignment technique and indexes; so, we don't need to roundup this blocksize to
     * avoid accidentally overriding ABACUS memory management variables (indexes)    
     */
    rpool->blockSize   = elemSize;
#else
    rpool->blockSize   = (RvUint32)RvRoundToSize(elemSize,ADS_ALIGN_NUMBER);
#endif
/* SPIRENT_END */

    rpool->numOfBlocks = maxNumOfBlocks;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    /* Set LOG filter */
    if(pLogMgr != NULL)
    {
        crv = RvLogSourceConstruct(pLogMgr,&rpool->logSource,ADS_RPOOL,name);
        if(crv != RV_OK)
        {
            RvMemoryFree(rpool,pLogMgr);
            return NULL;
        }
        rpool->pLogSource = &rpool->logSource;
        RvLogInfo(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_CoreConstruct - (elemSize=%d,maxNumOfBlocks=%d,allocEmptyBlocks=%d,name=%s)=0x%p",
            elemSize,maxNumOfBlocks,allocEmptyBlocks,NAMEWRAP(name),rpool));
    }
    else
    {
        rpool->pLogSource = NULL;
    }
#else
    rpool->pLogSource = NULL;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CoreConstruct - (elemSize=%d,maxNumOfBlocks=%d,allocEmptyBlocks=%d,name=%s)=0x%p",
        elemSize,maxNumOfBlocks,allocEmptyBlocks,NAMEWRAP(name),rpool));
    RV_UNUSED_ARG(allocEmptyBlocks);
    return (HRPOOL)rpool;
}

/********************************************************************************************
 * RPOOL_Destruct
 * purpose : Deallocate memory for a given pool.
 * input   : pool   - Handle for the pool that should be freed
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_Destruct(IN HRPOOL pool)
{
    RPOOL* rpool = (RPOOL*)pool;
    if (NULL == pool)
        return;

    RvLogInfo(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Destruct - (pool=0x%p)",rpool));

    if (rpool->hra != NULL)
    {
        RA_Destruct(rpool->hra);

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
        rpool->hra = NULL;
#endif
/* SPIRENT_END */

    }
    if(rpool->pLogSource != NULL)
    {
        RvLogSourceDestruct(rpool->pLogSource);
    }
    RvMemoryFree((void *)pool,NULL);
}



/********************************************************************************************
 * RPOOL_GetPage
 * purpose : Allocate an element/page in the pool with a specific size in bytes.
 *           If there's not enough memory to allocate the full block, no allocation will be
 *           done.
 *           In case the requests are for zero bytes ( only allocation for reserve is done ) failure
 *           will be returned in case that no more free blocks exist on the pool.
 * input   : pool           - Handle to the RPOOL used
 *           size           - Size in bytes to allocate , It is allowed to ask for zero bytes.
 *                            In this case one element/page will be allocated and will be reserved
 *                            to the caller.
 * output  : newRpoolElem   - The handle to the allocated element
 * return  : RV_OK if allocation succedded.
 *           RV_ERROR_OUTOFRESOURCES - if allocation failed (no resources)
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_GetPage(IN  HRPOOL           pool,
                                         IN  RvInt32        size,
                                         OUT HPAGE*          newRpoolElem)
{
    RPOOL*              rpool= (RPOOL *)pool;
    RPOOL_FirstElem*    firstElem;
    RPOOL_Elem*         lastElem;
    RvStatus        status;

    if(pool == NULL)
        return RV_ERROR_UNKNOWN;

    *newRpoolElem = NULL;
    
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_GetPage - (pool=0x%p,size=%d,*newRpoolElem=0x%p) (%s)",rpool,size,*newRpoolElem,RA_GetName(rpool->hra)));

    /* Allocate RPOOL elements */
    status = rpool_AllocBlocks(rpool, RV_TRUE, size, 0 /*Use Maximum Available*/, (RPOOL_Elem**)&firstElem, &lastElem);
    if (status == RV_OK)
    {
        /* Set first element information */
        firstElem->last              = lastElem;
        firstElem->header.size       = size;
        RPOOL_SET_FIRST_ELEM_BIT(&firstElem->header);
        firstElem->header.UsersCounter      = 1;

        *newRpoolElem = firstElem;
    }
    if(status == RV_OK)
    {
        RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_GetPage - (pool=0x%p,size=%d,*newRpoolElem=0x%p)=%d (%s)",
            rpool,size,*newRpoolElem,RV_OK, RA_GetName(rpool->hra)));
        RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_GetPage - (pool=0x%p,size=%d,*newRpoolElem=0x%p)=%d (%s)",
            rpool,size,*newRpoolElem,RV_OK,RA_GetName(rpool->hra)));
    }
    else
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_GetPage - (pool=0x%p,size=%d,*newRpoolElem=0x%p)=%d, failed to allocate (%s)",
            rpool,size,*newRpoolElem,status,RA_GetName(rpool->hra)));
    }
    return status;
}



/********************************************************************************************
 * RPOOL_Append
 * purpose : Append memory allocation to a given element
 *           If there's not enough memory to allocate the additional bytes, no additional
 *           allocation will be done.
 * input   : pool               - Handle to the RPOOL used
 *           element            - RPOOL Allocation to use
 *           size               - Size in bytes to append
 *           consecutiveMemory  - TRUE if memory appended must be consecutive or not
 * output  : AllocationOffset   - The offset ( in the page ) of the new allocation.
 * return  : RV_OK if allocation succedded.
 *           RV_ERROR_OUTOFRESOURCES - if allocation failed (no resources)
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_Append(IN  HRPOOL            pool,
                                        IN  HPAGE             element,
                                        IN  RvInt32          size,
                                        IN  RvBool           consecutiveMemory ,
                                        OUT RvInt32         *AllocationOffset)

{
    RPOOL*   rpool;
    RvBool newBlockAllocated = RV_FALSE;
    RvStatus rv;

    if(pool == NULL || element == NULL)
        return RV_ERROR_UNKNOWN;

    rpool= (RPOOL *)pool;
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Append - (pool=0x%p,element=0x%p,size=%d,consecutiveMemory=%d,AllocationOffset=0x%p (%s)",
        rpool,element,size,consecutiveMemory,AllocationOffset,RA_GetName(rpool->hra)));

    *AllocationOffset  = RPOOL_GetSize(pool,element);

    if (consecutiveMemory == RV_TRUE)
    {
        RvInt32 elementMaxSize = 0;

        /* make sure we have an unfragmented allocation unit in the requested size */
        RPOOL_GetPoolParams(pool,&elementMaxSize,NULL);
        if (elementMaxSize<size)
        {
            RvLogError(rpool->pLogSource, (rpool->pLogSource,
                "RPOOL_Append - (pool=0x%p,element=0x%p,size=%d,consecutiveMemory=%d,AllocationOffset=0x%p)=%d. max consecutive mem is %d",
                rpool,element,size,consecutiveMemory,AllocationOffset,RV_ERROR_BADPARAM,elementMaxSize));
            return RV_ERROR_BADPARAM;
        }
        /* Allocate one sequential block */
        rv =  RPOOL_AppendBlocks(pool,element, size, 1 ,&newBlockAllocated);
    }
    else
    {
        /* Allocate many blocks each one of the size one*/
        rv =  RPOOL_AppendBlocks(pool,element, 1, size ,&newBlockAllocated);
    }

    if(rv != RV_OK)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_Append - (pool=0x%p,element=0x%p,size=%d,consecutiveMemory=%d,AllocationOffset=0x%p)=%d",
            rpool,element,size,consecutiveMemory,AllocationOffset,rv));
        return rv;
    }
    if(newBlockAllocated == RV_TRUE)
    {
        RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_Append - (pool=0x%p,element=0x%p,size=%d,consecutiveMemory=%d,*AllocationOffset=0x%p) new block appended (%s)",
            rpool,element,size,consecutiveMemory,*AllocationOffset,RA_GetName(rpool->hra)));
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Append - (pool=0x%p,element=0x%p,size=%d,consecutiveMemory=%d,*AllocationOffset=0x%p)=%d (%s)",
        rpool,element,size,consecutiveMemory,*AllocationOffset,RV_OK,RA_GetName(rpool->hra)));
    return rv;
}


/********************************************************************************************
 * RPOOL_Align
 * purpose : This function is used before you want to allocate struct from the rpool in order
 *           to be sure that the address of the struct will be aligned.
 * input   : pool               - Handle to the RPOOL used
 *           element            - RPOOL Allocation to use
 * return  : RV_OK if allocation succedded.
 *           RV_ERROR_OUTOFRESOURCES - if allocation failed (no resources)
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_Align(IN  HRPOOL            pool,
                                      IN  HPAGE             element)

{
    RvStatus           rv               = RV_OK;
    RvInt32            addBytes         = 0;
    RvInt32            AllocationOffset = 0;
    RPOOL_FirstElem*   firstElem        = (RPOOL_FirstElem *)element;
    RPOOL_Elem*        curElem          = firstElem->last;
    RvInt32            blockOffset      = 0;
    RPOOL*             rpool;
	RvInt32			   remainingBytes;

    if (curElem == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    rpool            = (RPOOL *)pool;
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Append - (pool=0x%p,element=0x%p)",rpool,element));

	remainingBytes = (RvInt32)rpool_FreeBytes(rpool, curElem);

    blockOffset = RPOOL_USED(curElem);
    /* we have to check if the allocation will be done in a real address which is an
     alignment to 4 or 8 (to prevent bus error on Solaris)*/

    addBytes = (RvInt32)RvRoundToSize(blockOffset,ADS_ALIGN_NUMBER) - blockOffset;
	
	/*If the addBytes is greater then the remainingBytes on current element we need to allign on with the remainingBytes since
	  The next element will be alligned*/
	if(addBytes>remainingBytes)
    {
       addBytes = remainingBytes;
    }

    if((addBytes > 0) && (addBytes < ADS_ALIGN_NUMBER))
    {
        rv = RPOOL_Append(pool, element, addBytes, RV_TRUE,&AllocationOffset);
    }
    if (RV_OK == rv)
    {
        RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_Append - (pool=0x%p,element=0x%p)=%d",rpool,element,RV_OK));
    }
    else
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_Append - (pool=0x%p,element=0x%p)=%d, failed to append (%s)",rpool,element,rv,RA_GetName(rpool->hra)));
    }

    return rv;
}

/********************************************************************************************
 * RPOOL_AppendBlocks
 * purpose : Append memory allocation to a given element
 *           If there's not enough memory to allocate the additional bytes, no additional
 *           allocation will be done.
 *           This function assures that blocks are not fragmented on their own, but there
 *           can be fragmentation between some of the blocks.
 * input   : pool       - Handle to the pool.
 *           element    - RPOOL Allocation to use
 *           size       - Size in bytes to append in each block
 *           count      - Number of blocks to append
 * output  : none.
 * return  : RV_OK if allocation succedded.
 *           RV_ERROR_OUTOFRESOURCES - if allocation failed (no resources)
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_AppendBlocks(IN  HRPOOL     pool,
                                              IN  HPAGE      element,
                                              IN  RvInt32  size,
                                              IN  RvUint32  count,
                                              OUT RvBool    *newBlockAllocated)

{

    RPOOL*              rpool= (RPOOL *)pool;
    RPOOL_FirstElem*    firstElem= (RPOOL_FirstElem *)element;
    RPOOL_Elem*         lastElem= firstElem->last;
    RPOOL_Elem*         newElem;
    RPOOL_Elem*         lastNewElem;
    RvUint32           freeBytes;
    RvUint32           blocksLeft= count;
    RvUint32           blocks;
    RvStatus           status;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AppendBlocks - (pool=0x%p,element=0x%p,size=%d,count=%u,newBlockAllocated=0x%p)",
        rpool,element,size,count,newBlockAllocated));

    *newBlockAllocated = RV_FALSE;

    if ((size == 0) || (count == 0))
        return RV_OK;

    /* Put as many blocks as we can in the last element allocated */
    freeBytes = rpool_FreeBytes(rpool, lastElem);
    blocks = RvMin(freeBytes / size, count);
    blocksLeft -= blocks;

    /* Allocate all the necessary elements */
    if (blocksLeft > 0)
    {
        status = rpool_AllocBlocks(rpool, RV_FALSE, blocksLeft * size,
                                   ((rpool->blockSize + RPOOL_DIFF) / size) * size,
                                   &newElem,
                                   &lastNewElem);
        *newBlockAllocated = RV_TRUE;
    }
    else
    {
        lastNewElem = lastElem;
        newElem = NULL;
        status = RV_OK;
    }

    if (status != RV_OK)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendBlocks (1) - (pool=0x%p,element=0x%p,size=%d,count=%u,newBlockAllocated=0x%p)=%d",
            rpool,element,size,count,newBlockAllocated,status));
        return status;
    }

    /* Fix information of the last element we had previously */
    lastElem->next = newElem;
    lastElem->usedBytes += (blocks * size);

    /* Fix header of the RPOOL allocation */
    firstElem->last = lastNewElem;
    firstElem->header.size += (count * size);

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AppendBlocks - (pool=0x%p,element=0x%p,size=%d,count=%u,*newBlockAllocated=%d)=%d",
        rpool,element,size,count,*newBlockAllocated,status));
    return status;
}


/********************************************************************************************
 * RPOOL_FreePage
 * purpose : Free a given RPOOL allocation back to the global pool
 * input   : pool       - Handle to the RPOOL used
 *           element    - Pointer to the pool element that should be deallocated
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_FreePage(IN HRPOOL    pool,
                                     IN HPAGE     element)
{
    RPOOL*      rpool= (RPOOL *)pool;
    RPOOL_Elem * pElement = (RPOOL_Elem*) element;

    if(pool == NULL || element == NULL)
    {
        return;
    }
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_FreePage - (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));

    if(pElement->UsersCounter > 1)
    {
        RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_FreePage - more than 1 user! do not free the page! (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));
        -- pElement->UsersCounter;
        
    }
    else
    {
        rpool_FreeBlocks(rpool,pElement);
        
        RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_FreePage - (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));
        
    }
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_FreePage - (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));
}

/******************************************************************************
 * RPOOL_ResetPage
 * purpose : Fills a given page with zeroes.
 *           The function is required for defense of data, stored on page,
 *           against memory dump attack.
 * input   : hPool  - Handle to the RPOOL used
 *           hPage  - Handle to the page to be reset
 *           offset - Offset of the page region to be reset.
 *           size   - Size of the page memory, to be reset. If zero, whole page
 *                    from offset will be reset.
 * output  : none
 * return  : none
 *****************************************************************************/
void RVAPI RVCALLCONV RPOOL_ResetPage(IN HRPOOL    hPool,
                                      IN HPAGE     hPage,
                                      IN RvInt32   offset,
                                      IN RvInt32   size)
{
    RPOOL*        rpool = (RPOOL *)hPool;
    RPOOL_Elem*   curElem;
    RvInt32       bytesToSkip;
    RvInt32       bytesToReset;
    RvInt32       bytesToResetInElement;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_ResetPage - (rpool=0x%p,element=0x%p,offset=%d,size=%d)",
        hPool, hPage, offset, size));

    /* Find the right RA element to begin with and offset of begining in it */
    bytesToSkip = offset;
    curElem = rpool_FindBlock(rpool, hPage, &bytesToSkip);
    if (curElem == NULL)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_ResetPage - (rpool=0x%p,element=0x%p,offset=%d,size=%d) - failed to find offset in the page",
            hPool, hPage, offset, size));
        return;
    }

    if (0 == size)
    {
        size = curElem->size - offset;
    }

    bytesToReset = size;
    while (bytesToReset > 0)
    {
        /* Calculate bytes to reset in this RA element */
        bytesToResetInElement = 
            RvMin(bytesToReset, RPOOL_USED(curElem) - bytesToSkip);
        /* Reset the bytes */
        memset(rpool_GetPointer(curElem, bytesToSkip),0,bytesToResetInElement);
        /* Update counter of reset bytes */
        bytesToReset -= bytesToResetInElement;
        /* Move to the next RA element */
        if (NULL != curElem->next)
        {
            curElem = curElem->next;
            bytesToSkip = 0;
        }
        /* Check: If no next RA element is available,
           check if all requested bytes were reset. */
        else
        {
            if (bytesToReset > 0)
            {
                RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                    "RPOOL_ResetPage - (rpool=0x%p,element=0x%p,offset=%d,size=%d) - failed to reset last %d bytes",
                    hPool, hPage, offset, size, bytesToReset));
                bytesToReset = 0;
            }
        }
    }
    
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_ResetPage - (rpool=0x%p,element=0x%p,offset=%d,size=%d)",
        hPool, hPage, offset, size));
}

/********************************************************************************************
 * RPOOL_AddPageUser
 * purpose : Add one user to the page users counter
 * input   : pool       - Handle to the RPOOL used
 *           element    - Pointer to the pool element that should be deallocated
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_AddPageUser(IN HRPOOL    pool,
                                        IN HPAGE     element)
{
    RPOOL*      rpool= (RPOOL *)pool;
    RPOOL_Elem * pElement = (RPOOL_Elem*) element;

    if(rpool == NULL || element == NULL)
    {
        return;
    }
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AddPageUser - (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));

    ++ pElement->UsersCounter;
    RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AddPageUser - UsersCounter=%d (pool=0x%p,element=0x%p) (%s)",
        pElement->UsersCounter,rpool,element,RA_GetName(rpool->hra)));
    
    
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AddPageUser - (pool=0x%p,element=0x%p) (%s)",rpool,element,RA_GetName(rpool->hra)));
}
/********************************************************************************************
 * RPOOL_FreeAllPages
 * purpose : Free all the pages of a given RPOOL
 * input   : pool       - Handle to the RPOOL used
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_FreeAllPages(IN HRPOOL    pool)
{
    RPOOL*      rpool= (RPOOL *)pool;

    if(pool == NULL)
    {
        return;
    }

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_FreeAllPages - (pool=0x%p) (%s) clearing all pages",rpool,RA_GetName(rpool->hra)));

    RA_Clear(rpool->hra);

    RvLogDebug(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_FreeAllPages - (pool=0x%p) (%s) all pages cleared",rpool,RA_GetName(rpool->hra)));
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_FreeAllPages - (pool=0x%p) (%s) all pages cleared",rpool,RA_GetName(rpool->hra)));
}



/********************************************************************************************
 * RPOOL_GetSize
 * purpose : Returns the number of bytes that are currently allocated in a given POOL element
 * input   : pool       - Handle to the pool
 *           element    - RPOOL element
 * output  : none
 * return  : The number of bytes in this element
 ********************************************************************************************/
RvInt32 RVAPI RVCALLCONV RPOOL_GetSize(IN HRPOOL pool,
                                        IN HPAGE  element)
{
    RV_UNUSED_ARG(pool);

    return ((RPOOL_Elem *)element)->size;
}




/********************************************************************************************
 * RPOOL_GetPtr
 * purpose : Return a real pointer to an object inside the given element accoring to the
 *           offset
 * input   : pool       - Handle to the pool
 *           element    - RPOOL element
 *           offset     - Offset of an object inside the element
 * output  : none
 * return  : A real pointer to the object
 ********************************************************************************************/
void RVAPI * RVCALLCONV RPOOL_GetPtr(IN HRPOOL     pool,
                                     IN HPAGE      element,
                                     IN RvInt32   offset)
{
    RPOOL_Elem* curElem;
    RvInt32    curOffset = offset;

    if(offset == -1 || pool == NULL || element == NULL)
        return NULL;
    /* Find the right RA element */
    curElem = (RPOOL_Elem *)rpool_FindBlock((RPOOL *)pool, element, &curOffset);

    if (curElem == NULL)
        return NULL;

    /* Calculate and return pointer to the object */
    return rpool_GetPointer(curElem, curOffset);
}




/********************************************************************************************
 * RPOOL_CopyFrom
 * purpose : Copy a given number of bytes from a given RPOOL element to a given position
 *           inside the destination RPOOL element.
 * input   : destPool    - Handle to the destination pool
 *           destElement - Destination RPOOL element
 *           destOffset  - Position to copy to
 *           srcPool     - Handle to the source pool.
 *           srcElement  - Source RPOOL element
 *           srcOffset   - Position to copy from
 *           size        - The number of bytes that should be copied.
 * output  : none
 * return  : RV_OK  - On success
 *           RV_ERROR_UNKNOWN     - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_CopyFrom(IN HRPOOL       destPool,
                                          IN HPAGE        destElement,
                                          IN RvInt32    destOffset,
                                          IN HRPOOL       srcPool,
                                          IN HPAGE        srcElement,
                                          IN RvInt32    srcOffset,
                                          IN RvInt32    size)
{
    RPOOL*      srcRpool        = (RPOOL *)srcPool;
    RPOOL*      destRpool       = (RPOOL *)destPool;
    RPOOL_Elem* srcElem         = (RPOOL_Elem *)srcElement;
    RPOOL_Elem* destElem        = (RPOOL_Elem *)destElement;
    RvInt32     bytesToCopy;
    RvInt32     leftBytes       = size;
    RvInt32     srcOfs          = srcOffset;
    RvInt32     destOfs         = destOffset;

    RvLogEnter(destRpool->pLogSource, (destRpool->pLogSource,
        "RPOOL_CopyFrom - (destPool=0x%p,destElement=0x%p,destOffset=%d,srcPool=0x%p,srcElement=0x%p,srcOffset=%d,size=%d)",
        destPool,destElement,destOffset,srcPool,srcElement,srcOffset,size));

    if (size == 0)
    {
        RvLogLeave(destRpool->pLogSource, (destRpool->pLogSource,
            "RPOOL_CopyFrom - (destPool=0x%p,destElement=0x%p,destOffset=%d,srcPool=0x%p,srcElement=0x%p,srcOffset=%d,size=%d)=%d, size is 0",
            destPool,destElement,destOffset,srcPool,srcElement,srcOffset,size,RV_OK));

        return RV_OK;
    }


    /* check if there is enough space in RPOOL elements */
    if ((srcOfs + size > srcElem->size) || (destOfs + size > destElem->size))
    {
        RvLogError(destRpool->pLogSource, (destRpool->pLogSource,
            "RPOOL_CopyFrom - (destPool=0x%p,destElement=0x%p,destOffset=%d,srcPool=0x%p,srcElement=0x%p,srcOffset=%d,size=%d)=%d, not enough space",
            destPool,destElement,destOffset,srcPool,srcElement,srcOffset,size,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* Find the right RA elements */
    srcElem = rpool_FindBlock(srcRpool, srcElement, &srcOfs);
    destElem = rpool_FindBlock(destRpool, destElement, &destOfs);
    if ((srcElem == NULL) || (destElem == NULL))
    {
        RvLogError(destRpool->pLogSource, (destRpool->pLogSource,
            "RPOOL_CopyFrom - (destPool=0x%p,destElement=0x%p,destOffset=%d)=%d, ra elem is NULL (srcElem=0x%p, destElem=0x%p)",
            destPool,destElement,destOffset,RV_ERROR_UNKNOWN,srcElem,destElem));
        return RV_ERROR_UNKNOWN;
    }

    while (leftBytes > 0)
    {
        /* Copy as many bytes as we can */
        bytesToCopy = RvMin(RPOOL_USED(srcElem) - srcOfs, RPOOL_USED(destElem) - destOfs);
        if (leftBytes < bytesToCopy) bytesToCopy = leftBytes;
        memmove(rpool_GetPointer(destElem, destOfs),
                rpool_GetPointer(srcElem, srcOfs),
                bytesToCopy);

        /* Fix positions */
        leftBytes -= bytesToCopy;
        size -= bytesToCopy;
        srcOfs += bytesToCopy;
        destOfs += bytesToCopy;

        /* Change pointers if we have to */
        if (leftBytes > 0)
        {
            if (srcOfs == RPOOL_USED(srcElem))
            {
                srcElem = srcElem->next;
                srcOfs = 0;
            }
            if (destOfs == RPOOL_USED(destElem))
            {
                destElem = destElem->next;
                destOfs = 0;
            }
        }
    }
    RvLogLeave(destRpool->pLogSource, (destRpool->pLogSource,
        "RPOOL_CopyFrom - (destPool=0x%p,destElement=0x%p,destOffset=%d,srcPool=0x%p,srcElement=0x%p,srcOffset=%d,size=%d)=%d",
        destPool,destElement,destOffset,srcPool,srcElement,srcOffset,size,RV_OK));

    return RV_OK;
}



/********************************************************************************************
 * RPOOL_CopyFromExternal
 * purpose : Copy a given number of bytes from a given source buffer to a given position
 *           inside the destination RPOOL element.
 * input   : pool    - Handle to the source pool.
 *           element - Pointer to the elemnt to which the copy should be done.
 *           offset  - Start point (offset from the begining of the element) to which the
 *                     copy should be done.
 *           src     - Pointer to the source buffer from which the copy should be done.
 *                     This buffer should be sequential
 *           size    - The number of bytes that should be copied.
 * output  : none
 * return  : RV_OK  - On success
 *           RV_ERROR_UNKNOWN     - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_CopyFromExternal(IN HRPOOL       pool,
                                                  IN HPAGE        element,
                                                  IN RvInt32    offset,
                                                  IN const void*  src,
                                                  IN RvInt32    size)
{
    RPOOL*        rpool         = (RPOOL *)pool;
    RPOOL_Elem*   curElem       = (RPOOL_Elem *)element;
    RvInt32       bytesToSkip;
    RvInt32       bytesToCopy;
    RvInt32       bytesInElement;
    RvInt32       curOffset     = offset;
    RvChar*       copyFrom      = (RvChar *)src;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CopyFrom - (rpool=0x%p,element=0x%p,offset=%d,src=0x%p,size=%d)",
        pool,element,offset,src,size));

    /* check if there is enough space in RPOOL element to copy buffer */
    if (offset + size > curElem->size)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_CopyFrom - (rpool=0x%p,element=0x%p,offset=%d,src=0x%p,size=%d)=%d, size to big",
            pool,element,offset,src,size,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* Find the right RA element */
    curElem = rpool_FindBlock(rpool, element, &curOffset);

    if (curElem == NULL)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_CopyFrom - (rpool=0x%p,element=0x%p,offset=%d,src=0x%p,size=%d)=%d, curElem==NULL",
            pool,element,offset,src,size,RV_ERROR_UNKNOWN));

        return RV_ERROR_UNKNOWN;
    }

    /* calculate bytes to copy in this RA element */
    bytesToCopy = size;
    bytesToSkip = curOffset;
    bytesInElement = RvMin(bytesToCopy, RPOOL_USED(curElem) - bytesToSkip);

    /* copy first block (partial block) */
    memcpy(rpool_GetPointer(curElem, bytesToSkip), copyFrom, bytesInElement);
    copyFrom += bytesInElement;

    bytesToCopy -= bytesInElement;

    while (bytesToCopy > 0)
    {
        /* advance to next element */
        curElem = curElem->next;

        /* calculate bytes to copy in this RA element */
        bytesInElement = RvMin(curElem->usedBytes, bytesToCopy);

        /* copy block */
        memcpy (rpool_GetPointer(curElem, 0), copyFrom, bytesInElement);

        /* decrease bytes left to copy */
        bytesToCopy -= bytesInElement;

        /* decrease bytes left to copy */
        copyFrom += bytesInElement;
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CopyFrom - (rpool=0x%p,element=0x%p,offset=%d,src=0x%p,size=%d)=%d",
        pool,element,offset,src,size,RV_OK));

    return RV_OK;
}




/********************************************************************************************
 * RPOOL_CopyToExternal
 * purpose : The routine is used to copy a given number of bytes from a specific location
 *           inside a given pool to a given
 *           source.
 * input   : pool    - Handle to the destination pool.
 *           element - Pointer to the elemnt from which the copy should be done.
 *           offset  - Start point (offset from the beggining of the element) from which the
 *                     copy should be done.
 *           dest    - Pointer to the destination buffer to which the copy should be done.
 *           size    - The number of bytes that should be copied.
 * output  : none
 * return  : RV_OK  - If succeeded.
 *           RV_ERROR_UNKNOWN     - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_CopyToExternal(IN HRPOOL     pool,
                                                IN HPAGE      element,
                                                IN RvInt32  offset,
                                                IN void*      dest,
                                                IN RvInt32  size)
{
    RPOOL*         rpool= (RPOOL *)pool;
    RPOOL_Elem     *curElem= (RPOOL_Elem *)element;
    RvInt32       bytesToSkip;
    RvInt32       bytesToCopy;
    RvInt32       bytesInElement;
    RvInt32       curOffset = offset;
    RvChar*       destPtr = (RvChar *)dest;

    /*check the validity of the request*/
    if(dest==NULL || pool == NULL || element == NULL || size<0 || offset <0)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CopyToExternal - (rpool=0x%p,element=0x%p,offset=%d,dest=0x%p,size=%d)",
        pool,element,offset,dest,size));

    /* check if there is enough space to copy buffer */
    if (offset + size > curElem->size)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_CopyToExternal - (rpool=0x%p,element=0x%p,offset=%d,dest=0x%p,size=%d)=%d, size to big",
            pool,element,offset,dest,size,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* Find the right RA element */
    curElem = rpool_FindBlock(rpool, element, &curOffset);

    if (curElem == NULL)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_CopyToExternal - (rpool=0x%p,element=0x%p,offset=%d,dest=0x%p,size=%d)=%d, curElem=NULL",
            pool,element,offset,dest,size,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    /* calculate bytes to copy in this RA element */
    bytesToCopy = size;
    bytesToSkip = curOffset;
    bytesInElement = RvMin(RPOOL_USED(curElem) - bytesToSkip, bytesToCopy);

    /* copy first block (partial block) */
    memcpy(destPtr, rpool_GetPointer(curElem, curOffset), bytesInElement);

    destPtr += bytesInElement;
    bytesToCopy -= bytesInElement;

    while (bytesToCopy > 0)
    {
        /* advance to next element */
        curElem = curElem->next;

        /* calculate bytes to copy in this RA element */
        bytesInElement = RvMin(curElem->usedBytes, bytesToCopy);

        /* copy block */
        memcpy(destPtr, rpool_GetPointer(curElem, 0), bytesInElement);

        /* decrease bytes left to copy */
        bytesToCopy -= bytesInElement;

        /* increase bytes count */
        destPtr +=  bytesInElement;
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CopyToExternal - (rpool=0x%p,element=0x%p,offset=%d,dest=0x%p,size=%d)=%d, size to big",
        pool,element,offset,dest,size,RV_OK));

    return RV_OK;
}

/********************************************************************************************
 * RPOOL_AppendFromExternal
 * purpose : The routine is used to copy a given number of bytes from an external buffer to
 *           the end of a specific location inside a given RPOOL allocation
 * input   : pool               - Handle to the destination rpool.
 *           element            - Pointer to the elemnt from which the copy should be done.
 *           src                - Start point (offset from the beggining of the element) from
 *                                which the copy should be done.
 *           size               - Size in bytes to copy
 *           consecutiveMemory  - RV_TRUE if memory appended must be consecutive or not
 * output  : AllocationOffset   - This is the position from where the append was done ( actually the
 *                                position ( pointer to the new element ).

 * return  : RV_OK          - If succeeded.
 *           RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *           RV_ERROR_UNKNOWN          - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_AppendFromExternal(IN  HRPOOL              pool,
                                                    IN  HPAGE               element,
                                                    IN  const void*         src,
                                                    IN  RvInt                 size,
                                                    IN  RvBool             consecutiveMemory,
                                                    OUT RvInt32           *AllocationOffset,
                                                    OUT RPOOL_ITEM_HANDLE   *pItem)
{

    RvInt32    currentSize;
    RvStatus   status;
    RPOOL*     rpool;
    
    rpool = (RPOOL *)pool;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AppendFromExternal - (rpool=0x%p,element=0x%p,src=0x%p,size=%d,consecutiveMemory=%d,AllocationOffset=0x%p,pItem=0x%p)",
        pool,element,src,size,consecutiveMemory,AllocationOffset, pItem));

    /* Get current allocation size (= offset of the new allocated space) */
    currentSize = RPOOL_GetSize(pool,element);

    status = RPOOL_Append(pool,element, size, consecutiveMemory,AllocationOffset);

    if (status != RV_OK)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendFromExternal - (rpool=0x%p,element=0x%p,src=0x%p,size=%d)=%d, failed to append",
            pool,element,src,pItem,status));
        return status;
    }
	
	if(pItem != NULL)
	{
	    *pItem = (RPOOL_ITEM_HANDLE)RPOOL_GetPtr( pool ,
                                              element ,
                                              currentSize);
	}


    status = RPOOL_CopyFromExternal(pool,element, currentSize, src, size);
    if (RV_OK == status)
    {
        RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendFromExternal - (rpool=0x%p,element=0x%p,src=0x%p,size=%d,*AllocationOffset=%d)=%d,",
            pool,element,src,consecutiveMemory,*AllocationOffset,status));
    }
    else
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendFromExternal - (rpool=0x%p,element=0x%p,src=0x%p,size=%d)=%d, failed to copy",
            pool,element,src, size, status));
    }
    return status;
}

/********************************************************************************************
 * RPOOL_AppendFromExternalToPage
 * purpose : The routine is used to copy a given number of bytes from an external buffer to
 *           the end of a specific location inside a given RPOOL allocation
 * input   : pool               - Handle to the destination rpool.
 *           element            - Pointer to the elemnt from which the copy should be done.
 *           src                - Start point (offset from the beggining of the element) from
 *                                which the copy should be done.
 *           size               - Size in bytes to copy
 * output  : AllocationOffset   - This is the position from where the append was done ( actually the
 *                                position ( pointer to the new element ). if the append
 *                                fails then the output offset will be UNDEFINED.
 * return  : RV_OK          - If succeeded.
 *           RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *           RV_ERROR_UNKNOWN          - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_AppendFromExternalToPage(IN  HRPOOL              pool,
                                                          IN  HPAGE               element,
                                                          IN  const void*         src,
                                                          IN  RvInt                 size,
                                                          OUT RvInt32           *AllocationOffset)

{
    RvStatus            status;
    RPOOL_ITEM_HANDLE    ptr;
    RPOOL*              rpool;
    
    rpool = (RPOOL *)pool;

    if(pool == NULL || element == NULL ||
        src == NULL  || size <=0 || AllocationOffset==NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AppendFromExternalToPage - (rpool=0x%p,element=0x%p,src=0x%p,size=%d,AllocationOffset=0x%p)",
        pool,element,src,size, AllocationOffset));

    status = RPOOL_AppendFromExternal(pool,
                                      element,
                                      src,
                                      size,
                                      RV_FALSE,
                                      AllocationOffset,
                                      &ptr);


    if (status != RV_OK)
    {
        *AllocationOffset = -1;
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendFromExternalToPage - (rpool=0x%p,element=0x%p,src=0x%p,size=%d,*AllocationOffset=0x%p)=%d",
            pool,element,src,size, *AllocationOffset,status));
    }
    else
    {
        RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AppendFromExternalToPage - (rpool=0x%p,element=0x%p,src=0x%p,size=%d,*AllocationOffset=0x%p)=%d",
            pool,element,src,size, *AllocationOffset,status));
    }

    return status;
}


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***********************************************************************************************
 * RPOOL_Strcmp
 * purpose : Compare two null terminated strings , both reside on RPOOL.
 *           The length of the strings is unknown , the only thing that is promised is that these
 *           are NULL terminated strings.
 *           The comparison is CASE SENSITIVE comparison.
 * input   : FirstPool    - Handle to the pool of the first string
 *           FirstPage    - RPOOL page of the first string.
 *           FirstOffset  - Position of the string in the first page.
 *           SecPool      - Handle to the pool of the second string
 *           SecPage      - RPOOL page of the second string
 *           SecOffset    - Position of the string in the second page.
 * output  : none.
 * return  : 0        - If both strings are identical.
 *           < 0      - If the first string is shorter, or the leftmost non-equal character
 *                      is less in string1 (or if there is a "bug" and the second string
 *                      is not null terminated).
 *           > 0      - If the second string is shorter, or the leftmost non-equal character
 *                      is less in string2 (or if there is a "bug" and the first string
 *                      is not null terminated).
 ***********************************************************************************************/
RV_INT8 RVAPI RVCALLCONV SPIRENT_RPOOL_Strcmp ( IN HRPOOL     FirstPool,
                                                IN HPAGE      FirstPage,
                                                IN RV_INT32  FirstOffset,
                                                IN HRPOOL     SecPool,
                                                IN HPAGE      SecPage,
                                                IN RV_INT32  SecOffset )
{
   return ( rpool_StrcmpInternal ( FirstPool,
                                   FirstPage,
                                   FirstOffset,
                                   SecPool,
                                   SecPage,
                                   SecOffset,
                                   RV_TRUE ));
}

RV_BOOL RVAPI RVCALLCONV RPOOL_Strcmp ( IN HRPOOL     FirstPool,
                                        IN HPAGE      FirstPage,
                                        IN RV_INT32  FirstOffset,
                                        IN HRPOOL     SecPool,
                                        IN HPAGE      SecPage,
                                        IN RV_INT32  SecOffset )
{
   return ( rpool_StrcmpInternal ( FirstPool,
                                   FirstPage,
                                   FirstOffset,
                                   SecPool,
                                   SecPage,
                                   SecOffset,
                                   RV_TRUE ) == 0 ? RV_TRUE : RV_FALSE);

}

#else /* !defined(UPDATED_BY_SPIRENT) */
/***********************************************************************************************
 * RPOOL_Strcmp
 * purpose : Compare to null terminated string , both reside on RPOOL.
 *           The length of the strings is unknown , the only thing that is promised is that these
 *           are NULL terminated strings.
 *           The comparison is CASE SENSITIVE comparison.
 * input   : FirstPool    - Handle to the pool of the first string
 *           FirstPage    - RPOOL page of the first string.
 *           FirstOffset  - Position of the string in the first page.
 *           SecPool      - Handle to the pool of the second string
 *           SecPage      - RPOOL page of the second string
 *           SecOffset    - Position of the string in the second page.
 * output  : none.
 * return  : RV_TRUE  - If both strings are identical.
 *           RV_FALSE - If not.( or if there is a "bug" and one of the strings is not
 *                      null terminated.
 ***********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_Strcmp ( IN HRPOOL     FirstPool,
                                         IN HPAGE      FirstPage,
                                         IN RvInt32  FirstOffset,
                                         IN HRPOOL     SecPool,
                                         IN HPAGE      SecPage,
                                         IN RvInt32  SecOffset )
{
    RvBool bIdent = RV_FALSE;
    RPOOL* rpool;
    
    rpool = (RPOOL *)FirstPool;

    bIdent = rpool_StrcmpInternal(FirstPool,
                                FirstPage,
                                FirstOffset,
                                SecPool,
                                SecPage,
                                SecOffset,
                                RV_TRUE );
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Strcmp - (FirstPool=0x%p,FirstPage=0x%p,FirstOffset=%d,SecPool=0x%p,SecPage=0x%p,SecOffset=%d)=%d",
        FirstPool,FirstPage,FirstOffset,SecPool,SecPage,SecOffset,bIdent));

    return bIdent;
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***********************************************************************************************
 * RPOOL_Stricmp
 * purpose : Compare two null terminated strings , both reside on RPOOL.
 *           The length of the strings is unknown , the only thing that is promised is that these
 *           are NULL terminated strings.
 *           The comparison is NOT CASE SENSITIVE comparison.
 * input   : FirstPool    - Handle to the pool of the first string
 *           FirstPage    - RPOOL page of the first string.
 *           FirstOffset  - Position of the string in the first page.
 *           SecPool      - Handle to the pool of the second string
 *           SecPage      - RPOOL page of the second string
 *           SecOffset    - Position of the string in the second page.
 * output  : none.
 * return  : 0        - If both strings are identical.
 *           < 0      - If the first string is shorter, or the leftmost non-equal character
 *                      is less in string1 (or if there is a "bug" and the second string
 *                      is not null terminated).
 *           > 0      - If the second string is shorter, or the leftmost non-equal character
 *                      is less in string2 (or if there is a "bug" and the first string
 *                      is not null terminated).
 ***********************************************************************************************/
RV_INT8 RVAPI RVCALLCONV SPIRENT_RPOOL_Stricmp ( IN HRPOOL     FirstPool,
                                         IN HPAGE      FirstPage,
                                         IN RV_INT32  FirstOffset,
                                         IN HRPOOL     SecPool,
                                         IN HPAGE      SecPage,
                                         IN RV_INT32  SecOffset )
{
   return ( rpool_StrcmpInternal ( FirstPool,
                                   FirstPage,
                                   FirstOffset,
                                   SecPool,
                                   SecPage,
                                   SecOffset,
                                   RV_FALSE ));

}

RV_BOOL RVAPI RVCALLCONV RPOOL_Stricmp (IN HRPOOL     FirstPool,
                                        IN HPAGE      FirstPage,
                                        IN RV_INT32  FirstOffset,
                                        IN HRPOOL     SecPool,
                                        IN HPAGE      SecPage,
                                        IN RV_INT32  SecOffset )
{
   return ( rpool_StrcmpInternal ( FirstPool,
                                   FirstPage,
                                   FirstOffset,
                                   SecPool,
                                   SecPage,
                                   SecOffset,
                                   RV_FALSE ) == 0 ? RV_TRUE : RV_FALSE);

}

#else /* !defined(UPDATED_BY_SPIRENT) */
/***********************************************************************************************
 * RPOOL_Stricmp
 * purpose : Compare to null terminated string , both reside on RPOOL.
 *           The length of the strings is unknown , the only thing that is promised is that these
 *           are NULL terminated strings.
 *           The comparison is NOT CASE SENSITIVE comparison.
 * input   : FirstPool    - Handle to the pool of the first string
 *           FirstPage    - RPOOL page of the first string.
 *           FirstOffset  - Position of the string in the first page.
 *           SecPool      - Handle to the pool of the second string
 *           SecPage      - RPOOL page of the second string
 *           SecOffset    - Position of the string in the second page.
 * output  : none.
 * return  : RV_TRUE  - If both strings are identical.
 *           RV_FALSE - If not.( or if there is a "bug" and one of the strings is not
 *                      null terminated.
 ***********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_Stricmp (IN HRPOOL     FirstPool,
                                        IN HPAGE      FirstPage,
                                        IN RvInt32  FirstOffset,
                                        IN HRPOOL     SecPool,
                                        IN HPAGE      SecPage,
                                        IN RvInt32  SecOffset )
{
    RvBool bIdent = RV_FALSE;
    RPOOL* rpool;
    
    rpool = (RPOOL *)FirstPool;

    bIdent =  rpool_StrcmpInternal ( FirstPool,
                                   FirstPage,
                                   FirstOffset,
                                   SecPool,
                                   SecPage,
                                   SecOffset,
                                   RV_FALSE );
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Stricmp - (FirstPool=0x%p,FirstPage=0x%p,FirstOffset=%d,SecPool=0x%p,SecPage=0x%p,SecOffset=%d)=%d",
        FirstPool,FirstPage,FirstOffset,SecPool,SecPage,SecOffset,bIdent));
    return bIdent;
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***********************************************************************************************
 * RPOOL_StrcmpHexa
 * purpose : Compare to null terminated string , both reside on RPOOL.
 *           The length of the strings is unknown , the only thing that is promised is that these
 *           are NULL terminated strings.
 *           
 * IMPORTANT: The function handles characters equivalent to their ""%" HEX HEX" encoding
 *			  according to RFC 2396.Because the percent "%" character always has the reserved 
 *			  purpose of being the escape indicator, it must be escaped as "%25" in order to
 *			  be used as data within a URI (See RFC2396 2.4.2). 
 *
 * input   : FirstPool    - Handle to the pool of the first string
 *           FirstPage    - RPOOL page of the first string.
 *           FirstOffset  - Position of the string in the first page.
 *           SecPool      - Handle to the pool of the second string
 *           SecPage      - RPOOL page of the second string
 *           SecOffset    - Position of the string in the second page.
 *			 CaseSensitive - Boolean flags that states if the comparison is case sensitive or
 *                           not ( True for case sensitive false for non case sensitive ).
 * output  : none.
 * return  : RV_TRUE  - If both strings are identical.
 *           RV_FALSE - If not.( or if there is a "bug" and one of the strings is not
 *                      null terminated.
 ***********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_StrcmpHexa ( IN HRPOOL    FirstPool,
                                         IN HPAGE		FirstPage,
                                         IN RvInt32		FirstOffset,
                                         IN HRPOOL		SecPool,
                                         IN HPAGE		SecPage,
                                         IN RvInt32		SecOffset,
										 IN RvBool		CaseSensitive)
{
    RvBool bIdent = RV_FALSE;
    RPOOL* rpool;
    
    rpool = (RPOOL *)FirstPool;

    bIdent = rpool_StrcmpHexaInternal(
								FirstPool,
                                FirstPage,
                                FirstOffset,
                                SecPool,
                                SecPage,
                                SecOffset,
                                CaseSensitive );
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_StrcmpHexa - (FirstPool=0x%p,FirstPage=0x%p,FirstOffset=%d,SecPool=0x%p,SecPage=0x%p,SecOffset=%d,Case=%d)=%d",
        FirstPool,FirstPage,FirstOffset,SecPool,SecPage,SecOffset,CaseSensitive,bIdent));

    return bIdent;
}

/*********************************************************************************************
 * RPOOL_CmpToExternal
 * purpose : The routine perfroms comparison between a NULL terminated string
 *           that reside on RPOOL to a given NULL terminated string that is given as a parameter.
 * input   : Pool      - Handle to the pool of the string that resides on RPOOL
 *           Page      - RPOOL page of the RPOOL string
 *           Offset    - Position in RPOOL page of the rpool string.
 *           ExtString - Pointer to a buffer in which the external string resides.
 *           bIgnoreCase - compare case insensitive
 * output  : None.
 * return  : RV_TRUE  - If both string are equel.
 *           RV_FALSE - If the strings are not equel.
 *********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_CmpToExternal ( IN HRPOOL     Pool,
                                               IN HPAGE     Page,
                                               IN RvInt32   Offset,
                                               IN RvChar*   ExtString,
                                               IN RvBool    bIgnoreCase)
{
    RPOOL*         rpool = (RPOOL *)Pool;
    RPOOL_Elem     *Elem;
    RvInt32       CurrOffset = Offset;
    RvInt32       CurrOffsetInOnePage = 0;
    RvBool        StringAreEquel;
    RvBool        EndOfSearch;
    RvChar*       RpoolStrPtr;
    RvInt32       ExtStringLength;
    RvInt32       ExtStringIndex;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Stricmp - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p,bIgnoreCase=%d)",
        Pool,Page,Offset,ExtString,bIgnoreCase));

    /* Find the right RA element */
    Elem = rpool_FindBlock(rpool, Page, &CurrOffset);

    if (Elem == NULL)
        return RV_FALSE;

    StringAreEquel = RV_TRUE;
    EndOfSearch    = RV_FALSE;

    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);

    ExtStringLength = (RvInt32)strlen(ExtString );
    ExtStringIndex  = 0;


    while (( StringAreEquel ) && (!EndOfSearch ))
    {
       if ( (RpoolStrPtr[CurrOffsetInOnePage] == ExtString[ExtStringIndex]) ||
            (RpoolStrPtr[CurrOffsetInOnePage] >= 'A' &&
             RpoolStrPtr[CurrOffsetInOnePage] <= 'Z' &&
             bIgnoreCase == RV_TRUE      &&
             ((RpoolStrPtr[CurrOffsetInOnePage] - ('A' - 'a')) == ExtString[ExtStringIndex])) ||
            (RpoolStrPtr[CurrOffsetInOnePage] >= 'a' &&
             RpoolStrPtr[CurrOffsetInOnePage] <= 'z' &&
             bIgnoreCase == RV_TRUE      &&
             ((RpoolStrPtr[CurrOffsetInOnePage] + ('A' - 'a')) == ExtString[ExtStringIndex])) )

       {
           if ( RpoolStrPtr[CurrOffsetInOnePage] == '\0' )
           {
               EndOfSearch = RV_TRUE;
               if (ExtStringIndex !=  ExtStringLength  )
               {
                   StringAreEquel = RV_FALSE;
                   break;
               }
           }
           else
           {
              if ( CurrOffset == RPOOL_USED(Elem)-1 )
              {
                 Elem       = Elem->next;
                 CurrOffset = 0;
                 CurrOffsetInOnePage = 0;
                 if ( Elem == NULL )
                 {
                     EndOfSearch    = RV_TRUE;
                     StringAreEquel = RV_FALSE;
                     break;
                 }
                 else
                    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);
              }
              else
              {
                  CurrOffset++;
                  CurrOffsetInOnePage++;
              }

              if ( ExtStringIndex == ExtStringLength )
              {
                  EndOfSearch    = RV_TRUE;
                  StringAreEquel = RV_FALSE;
                  break;
              }
              else
                  ExtStringIndex++;
           }
       }
       else
       {
           StringAreEquel = RV_FALSE;
           break;
       }
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Stricmp - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p,bIgnoreCase=%d)=%d",
        Pool,Page,Offset,ExtString,bIgnoreCase,StringAreEquel));

    return ( StringAreEquel);
}

/*********************************************************************************************
 * RPOOL_CmpPrefixToExternal
 * purpose : The routine perfroms case sensitive comparison between a prefix of a string
 *           that reside on RPOOL to a given NULL terminated string that is given as a parameter.
 * input   : Pool      - Handle to the pool of the string that resides on RPOOL
 *           Page      - RPOOL page of the RPOOL string
 *           Offset    - Position in RPOOL page of the rpool string.
 *           ExtString - Pointer to a buffer in which the external string resides.
 * output  : None.
 * return  : RV_TRUE  - If both string are equel.
 *           RV_FALSE - If the strings are not equel.
 *********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_CmpPrefixToExternal (IN HRPOOL     Pool,
                                                    IN HPAGE      Page,
                                                    IN RvInt32  Offset,
                                                    IN RvChar       *ExtString )
{
    RPOOL*         rpool= (RPOOL *)Pool;
    RPOOL_Elem     *Elem;
    RvInt32       CurrOffset = Offset;
    RvInt32       CurrOffsetInOnePage = 0;
    RvBool        StringAreEquel;
    RvBool        EndOfSearch;
    RvChar*          RpoolStrPtr;
    RvInt32       ExtStringLength;
    RvInt32       ExtStringIndex;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CmpPrefixToExternal - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p)",
        Pool,Page,Offset,ExtString));

    /* Find the right RA element */
    Elem = rpool_FindBlock(rpool, Page, &CurrOffset);

    if (Elem == NULL)
        return RV_FALSE;

    StringAreEquel = RV_TRUE;
    EndOfSearch    = RV_FALSE;

    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);

    ExtStringLength = (RvInt32)strlen(ExtString) - 1;
    ExtStringIndex  = 0;


    while (( StringAreEquel ) && (!EndOfSearch ))
    {
       if ( RpoolStrPtr[CurrOffsetInOnePage] == ExtString[ExtStringIndex] )

       {
           if ( RpoolStrPtr[CurrOffsetInOnePage] == '\0' )
           {
               EndOfSearch = RV_TRUE;
               if (ExtStringIndex !=  ExtStringLength  )
               {
                   StringAreEquel = RV_FALSE;
                   break;
               }
           }
           else
           {
              if ( CurrOffset == RPOOL_USED(Elem)-1 )
              {
                 Elem       = Elem->next;
                 CurrOffset = 0;
                 CurrOffsetInOnePage = 0;
                 if ( Elem == NULL )
                 {
                     EndOfSearch    = RV_TRUE;
                     StringAreEquel = RV_FALSE;
                     break;
                 }
                 else
                    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);
              }
              else
              {
                  CurrOffset++;
                  CurrOffsetInOnePage++;
              }

              if ( ExtStringIndex == ExtStringLength )
              {
                  EndOfSearch    = RV_TRUE;
                  StringAreEquel = RV_TRUE;
                  break;
              }
              else
                  ExtStringIndex++;
           }
       }
       else
       {
           StringAreEquel = RV_FALSE;
           break;
       }
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CmpPrefixToExternal - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p)=%d",
        Pool,Page,Offset,ExtString,StringAreEquel));

    return ( StringAreEquel);
}

/*********************************************************************************************
 * RPOOL_CmpiPrefixToExternal
 * purpose : The routine perfroms case insensitive comparison between a prefix of a string
 *           that reside on RPOOL to a given NULL terminated string that is given as a parameter.
 * input   : Pool      - Handle to the pool of the string that resides on RPOOL
 *           Page      - RPOOL page of the RPOOL string
 *           Offset    - Position in RPOOL page of the rpool string.
 *           ExtString - Pointer to a buffer in which the external string resides.
 * output  : None.
 * return  : RV_TRUE  - If both string are equal.
 *           RV_FALSE - If the strings are not equal.
 *********************************************************************************************/
RvBool RVAPI RVCALLCONV RPOOL_CmpiPrefixToExternal (IN HRPOOL     Pool,
                                                    IN HPAGE      Page,
                                                    IN RvInt32  Offset,
                                                    IN RvChar       *ExtString )
{
    RPOOL*         rpool= (RPOOL *)Pool;
    RPOOL_Elem     *Elem;
    RvInt32       CurrOffset = Offset;
    RvInt32       CurrOffsetInOnePage = 0;
    RvBool        StringAreEquel;
    RvBool        EndOfSearch;
    RvChar*          RpoolStrPtr;
    RvInt32       ExtStringLength;
    RvInt32       ExtStringIndex;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CmpiPrefixToExternal - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p)",
        Pool,Page,Offset,ExtString));

    /* Find the right RA element */
    Elem = rpool_FindBlock(rpool, Page, &CurrOffset);

    if (Elem == NULL)
        return RV_FALSE;

    StringAreEquel = RV_TRUE;
    EndOfSearch    = RV_FALSE;

    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);

    ExtStringLength = (RvInt32)strlen(ExtString) - 1;
    ExtStringIndex  = 0;


    while (( StringAreEquel ) && (!EndOfSearch ))
    {
       if ( RpoolStrPtr[CurrOffsetInOnePage] == ExtString[ExtStringIndex] ||
            ((RpoolStrPtr[CurrOffsetInOnePage]-ExtString[ExtStringIndex])%('a'-'A') == 0))

       {
           if ( RpoolStrPtr[CurrOffsetInOnePage] == '\0' )
           {
               EndOfSearch = RV_TRUE;
               if (ExtStringIndex !=  ExtStringLength  )
               {
                   StringAreEquel = RV_FALSE;
                   break;
               }
           }
           else
           {
              if ( CurrOffset == RPOOL_USED(Elem)-1 )
              {
                 Elem       = Elem->next;
                 CurrOffset = 0;
                 CurrOffsetInOnePage = 0;
                 if ( Elem == NULL )
                 {
                     EndOfSearch    = RV_TRUE;
                     StringAreEquel = RV_FALSE;
                     break;
                 }
                 else
                    RpoolStrPtr = rpool_GetPointer(Elem, CurrOffset);
              }
              else
              {
                  CurrOffset++;
                  CurrOffsetInOnePage++;
              }

              if ( ExtStringIndex == ExtStringLength )
              {
                  EndOfSearch    = RV_TRUE;
                  StringAreEquel = RV_TRUE;
                  break;
              }
              else
                  ExtStringIndex++;
           }
       }
       else
       {
           StringAreEquel = RV_FALSE;
           break;
       }
    }

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_CmpPrefixToExternal - (Pool=0x%p,Page=0x%p,Offset=%d,ExtString=0x%p)=%d",
        Pool,Page,Offset,ExtString,StringAreEquel));

    return ( StringAreEquel);
}

/***********************************************************************************************
 * RPOOL_Strlen
 * purpose : Return the length of a NULL terminated string that is located in RPOOL.
 *           The size of the string ( that is returned by the routine ) is WITHOUT the Null termination
 *           character.
 *           This string may reside on more the one page element.
 * input   : Pool   : Handle to the pool.
 *           Page   : Handle to the page.
 *           Offset : The start location of the string in the page..
 * output  : None.
 * return  : The string length. ( -1 is returned if it is a not NULL terminated string or if
 *           the given location is not legal ).
 *********************************************************************************************/
RvInt32 RVAPI RVCALLCONV RPOOL_Strlen ( IN HRPOOL     Pool,
                                          IN HPAGE      Page,
                                          IN RvInt32  Offset )
{
    RPOOL*         rpool= (RPOOL *)Pool;
    RPOOL_Elem     *Elem;
    RvInt32       CurrOffset = Offset;
    RvInt32       CurrOffsetInOnePage = 0;
    RvBool        EndOfSearch;
    RvChar           *StrPtr;
    RvInt32       Answer;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Strlen - (Pool=0x%p,Page=0x%p,Offset=%d)",
        Pool,Page,Offset));


    /* Find the right RA element */
    Elem = rpool_FindBlock(rpool, Page, &CurrOffset);


    if ( Elem == NULL )
        return(-1);

    StrPtr = rpool_GetPointer(Elem, CurrOffset);

    Answer       = 0;
    EndOfSearch  = RV_FALSE;


    while ( EndOfSearch == RV_FALSE )
    {
       if ( StrPtr[CurrOffsetInOnePage] != '\0' )
       {
           Answer++;
           if ( CurrOffset == RPOOL_USED(Elem)-1 )
           {
              Elem       = Elem->next;
              CurrOffset = 0;
              CurrOffsetInOnePage = 0;
              if ( Elem == NULL )
              {
                  EndOfSearch = RV_TRUE;
                  Answer = -1;
              }
              else
                 StrPtr = rpool_GetPointer(Elem, CurrOffset);
           }
           else
           {
               CurrOffset++;
               CurrOffsetInOnePage++;
           }
       }
       else
       {
           EndOfSearch = RV_TRUE;
       }

       /* Safety for non-finite loops */
       if ( Answer >100000 )
       {
           Answer      = -1;
           EndOfSearch = RV_TRUE;
           RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                      "In RPOOL_Strlen - infinite loop"));
       }
    }
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Strlen - (Pool=0x%p,Page=0x%p,Offset=%d)=%d",
        Pool,Page,Offset,Answer));

    return ( Answer );
}

/********************************************************************************************
 * RPOOL_GetUnfragmentedSize
 * purpose : Returns the number of bytes that are unfragmented from the given offset
 * input   : pool    - RPOOL handle
 *           element - Pointer to the pool element.
 *           offset  - Offset inside the element
 * output  : none
 * return  : The number of bytes unfragmented from the given offset
 *           0 is returned if the offset is beyond the size of the allocated buffer
 ********************************************************************************************/
RvInt32 RVAPI RVCALLCONV RPOOL_GetUnfragmentedSize(HRPOOL     pool,
                                                    HPAGE      element,
                                                    RvInt32  offset)
{
    RPOOL*      rpool = (RPOOL *)pool;
    RPOOL_Elem* curElem;
    RvInt32       curOffset = offset;

    /* Find the right RA element */
    curElem = rpool_FindBlock(rpool, element, &curOffset);

    if (curElem == NULL)
        return 0;

    return (RPOOL_USED(curElem) - curOffset);
}


#ifdef RV_ADS_DEBUG
/************************************************************************************************************************
 * RPOOL_GetRA
 * purpose : Get the handle to the RA instance used by RPOOL
 *           We allow this to get statistics from RA (free elements etc.)
 *           Mostly used for debugging
 * input   : pool   - Handle to the RPOOL
 * output  : none
 * return  : Handle to RA
 ************************************************************************************************************************/
HRA RVAPI RVCALLCONV RPOOL_GetRA(IN HRPOOL  pool)
{
    return ((RPOOL *)pool)->hra;
}
#endif

/************************************************************************************************************************
 * RPOOL_GetPoolParams
 * purpose : The routine is to return the paremeters used to construct the pool.
 *           NULL pointers are ignored when this function is called.
 * input   : pool   - handle to the pool.
 * output  : blockSize      - the block size used in this pool.
 *           numOfBlocks    - the number of elements in the pool.
 * return  : none
 *************************************************************************************************************************/
void RVAPI RVCALLCONV RPOOL_GetPoolParams (IN HRPOOL      pool,
                                           IN RvInt32*   elementSize,
                                           IN RvInt32*   numOfElements)
{
    if (elementSize != NULL)
        *elementSize = ((RPOOL *)pool)->blockSize;

    if (numOfElements != NULL)
        *numOfElements = ((RPOOL *)pool)->numOfBlocks;
}


/**********************************************************************************************
 * RPOOL_GetResources
 * purpose : Return statistics resources of rpool
 * input   : pool - Handle to the rpool.
 * output  : NumOfAlloc    - Number of allocated blocks in rpool.
 *           CurrNumOfUsed - The currently number of used blocks in rpool.
 *           MaxUsage      - The maximum number of used block at the same time , until now.
 **********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_GetResources ( IN  HRPOOL    pool,
                                           OUT RvUint32 *NumOfAlloc,
                                           OUT RvUint32 *CurrNumOfUsed,
                                           OUT RvUint32 *MaxUsage )
{
    RV_Resource raResources;

    RA_GetResourcesStatus((HRA)((RPOOL*)pool)->hra ,
                           &raResources );
    *NumOfAlloc     = raResources.maxNumOfElements;
    *CurrNumOfUsed  = raResources.numOfUsed;
    *MaxUsage       = raResources.maxUsage;

}


#ifdef RV_ADS_DEBUG
/************************************************************************************************************************
 * RPOOL_Validate
 * purpose : Make sure RPOOL elements didn't write on top of each other.
 * input   : pool   - Handle to the RPOOL
 * output  : RV_OK if RPOOL elements are fine
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_Validate(IN HRPOOL pool)
{
    RPOOL*      rpool= (RPOOL *)pool;
    RPOOL_Elem* curElem;
    RvInt32    i;

    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Validate - (pool=0x%p)",pool));

    for (i = 0; i < (int)rpool->numOfBlocks; i++)
    {
        curElem = (RPOOL_Elem *)RA_GetElement(rpool->hra, i);
        if (curElem != NULL)
            if (curElem->debugSpace != RPOOL_DEBUG_VALUE)
            {
                RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
                    "RPOOL_Validate - (pool=0x%p)=%d",
                    pool,RV_ERROR_UNKNOWN));
                return RV_ERROR_UNKNOWN;
            }
    }
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_Validate - (pool=0x%p)=%d",
        pool,RV_OK));

    return RV_OK;
}


/************************************************************************************************************************
 * RPOOL_ValidateElement
 * purpose : Make sure RPOOL element hold correct size information
 * input   : pool    - Handle to the RPOOL
 *           element - Pointer to the pool element
 * output  : RV_OK if RPOOL elements are fine
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_ValidateElement(IN HRPOOL  pool,
                                                 IN HPAGE   element)
{
    RPOOL*      rpool;
    RPOOL_Elem* curElem= (RPOOL_Elem *)element;
    RvInt32    count, size;

    rpool= (RPOOL *)pool;
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_ValidateElement - (pool=0x%p,element=0x%p)",
        pool,element));

    if (curElem == NULL)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_ValidateElement - (pool=0x%p,element=0x%p)=%d, element=NULL",
            pool,element,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    if (curElem->debugSpace != RPOOL_DEBUG_VALUE)
    {
        RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_ValidateElement - (pool=0x%p,element=0x%p)=%d, Write beyond end of element",
            pool,element,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }

    size = curElem->size;
    count = 0;

    while (curElem != NULL)
    {
        count += RPOOL_USED(curElem);
        curElem = curElem->next;
    }

    if (size != count)
    {
        RvLogExcep(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_ValidateElement - (pool=0x%p,element=0x%p)=%d, Allocation size mismatch",
            pool,element,RV_ERROR_UNKNOWN));
        return RV_ERROR_UNKNOWN;
    }
    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_ValidateElement - (pool=0x%p,element=0x%p)=%d",
        pool,element,RV_OK));

    return RV_OK;
}
#endif


/********************************************************************************************
 * RPOOL_AlignAppend
 * purpose : This function is used when we want to allocate struct from the rpool in order
 *           to be sure that the address of the struct will be aligned.
 * input   : pool               - Handle to the RPOOL used
 *           element            - RPOOL Allocation to use
 *           size               - Size in bytes to append
 * output  : AllocationOffset   - The offset ( in the page ) of the new allocation.
 * return  : AllocationPtr      - A real pointer to the object
 ********************************************************************************************/
RvChar RVAPI * RVCALLCONV RPOOL_AlignAppend(
                                        IN  HRPOOL           pool,
                                        IN  HPAGE            element,
                                        IN  RvInt32          size,
                                        OUT RvInt32         *AllocationOffset)

{
    RPOOL           *rpool;          
    RvStatus         eStat          = RV_OK;
    RPOOL_FirstElem *firstElem      = (RPOOL_FirstElem *)element;
    RPOOL_Elem      *curElem        = firstElem->last;
    RvChar          *AllocationPtr = NULL;

    rpool = (RPOOL *)pool;
    RvLogEnter(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AlignAppend - (pool=0x%p,element=0x%p,size=%d,AllocationOffset=0x%p)",
        pool,element,size,AllocationOffset));

    if (curElem == NULL)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AlignAppend - (pool=0x%p,element=0x%p,size=%d,*AllocationOffset=%d)=0x%p, curElem=NULL",
            pool,element,size,*AllocationOffset,AllocationPtr));

        return NULL;
    }
    eStat = RPOOL_Align(pool,element);
    if (eStat != RV_OK)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AlignAppend - (pool=0x%p,element=0x%p,size=%d,*AllocationOffset=%d)=0x%p, error in RPOOL_Align()",
            pool,element,size,*AllocationOffset,AllocationPtr));
        return NULL;
    }
    eStat =  RPOOL_Append(pool, element, size, RV_TRUE, AllocationOffset);
    if (eStat != RV_OK)
    {
        RvLogError(rpool->pLogSource, (rpool->pLogSource,
            "RPOOL_AlignAppend - (pool=0x%p,element=0x%p,size=%d,*AllocationOffset=%d)=0x%p, error in RPOOL_Append()",
            pool,element,size,*AllocationOffset,AllocationPtr));
        return NULL;
    }
    AllocationPtr = (RvChar *)RPOOL_GetPtr(pool,element,*AllocationOffset) ;

    RvLogLeave(rpool->pLogSource, (rpool->pLogSource,
        "RPOOL_AlignAppend - (pool=0x%p,element=0x%p,size=%d,*AllocationOffset=%d)=0x%p",
        pool,element,size,*AllocationOffset,AllocationPtr));

    return AllocationPtr;
}

/********************************************************************************************
 * RPOOL_IsValidPtr
 * purpose : Returns RV_TRUE is an RPOOL pointer is valid i.e. points to a valid
 *           rpool position.
 * input   : ptr               - rpool pointer
 * return  : RV_TRUE if the pointer is valid. RV_FALSE otherwise
 ********************************************************************************************/
RVAPI RvBool  RVCALLCONV RPOOL_IsValidPtr(
                              RPOOL_Ptr *ptr)
{
    if(ptr == NULL)
    {
        return RV_FALSE;
    }
    if(ptr->hPage == NULL_PAGE ||
        ptr->hPage == NULL      ||
        ptr->offset < 0)
    {
        return RV_FALSE;
    }
    return RV_TRUE;
}
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

void RPool_MemShow( HRPOOL pool )
{
   // RPool memory statistics.

   RPOOL* rpool = (RPOOL*)pool;

   if( !rpool )
   {
      printf( "%s: Null rpool\n", __FUNCTION__ );
      return;
   }

   printf( "      blockSize = %d, numOfBlocks = %d\n", rpool->blockSize, rpool->numOfBlocks );
}

#endif
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif



