/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                rpool.h
 *
 * The RPOOL package implements a data structure that provides an illusion of contiguous
 * memory that can increase according the the user's wishes.
 * In practive, the memory is built on top of RA, as a linked list of RA elements. RPOOL
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
 *     Ayelet Back                        Nov 2000                       Divide header file
 *                                                                       for rpool.h and rpool_extern,h
 *                                                                       for the purpose of exporting
 *                                                                       the extern file as part
 *                                                                       of the SipExpress API.
 *
 ***********************************************************************************************/


#ifndef RPOOL_H
#define RPOOL_H


#ifdef __cplusplus
extern "C" {
#endif

#include "RV_ADS_DEF.h"
#include "AdsRa.h"
#include "rpool_API.h"
#include "rvlog.h"

/* this definition assumes that elementId returned from RA_Alloc is not negative */
#define RPOOL_NULL ( -1 )

/* Declaration of an handle that point to the data */
RV_DECLARE_HANDLE (RPOOL_ITEM_HANDLE);

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
#define RPOOL_CoreConstruct(elemSize,maxNumOfBlocks,logHandle,allocEmptyBlocks,name) RPOOL_CoreConstruct_trace(elemSize,maxNumOfBlocks,logHandle,allocEmptyBlocks,name,__FILE__,__LINE__)

HRPOOL RVAPI RVCALLCONV RPOOL_CoreConstruct_trace(IN RvInt32         elemSize,
                                            IN RvInt32         maxNumOfBlocks,
                                            IN RvLogMgr*        pLogMgr,
                                            IN RvBool          allocEmptyBlocks,
                                            IN const char*      name,
                                            const char* file, int line);
#else
HRPOOL RVAPI RVCALLCONV RPOOL_CoreConstruct(IN RvInt32         elemSize,
                                            IN RvInt32         maxNumOfBlocks,
											IN RvLogMgr*        pLogMgr,
                                            IN RvBool          allocEmptyBlocks,
                                            IN const char*      name);
#endif
/* SPIRENT_END */

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
                                        IN  RvInt32         size,
                                        IN  RvBool           consecutiveMemory ,
                                        OUT RvInt32         *AllocationOffset);



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
                                       IN  HPAGE             element);


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
 * output  : AllocationOffset   - This is the position from where the append was done ( actually the
 *                                position ( pointer to the new element ).
 * return  : RV_OK if allocation succedded.
 *           RV_ERROR_OUTOFRESOURCES - if allocation failed (no resources)
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_AppendBlocks(IN  HRPOOL     pool,
                                              IN  HPAGE      element,
                                              IN  RvInt32  size,
                                              IN  RvUint32  count,
											  OUT RvBool    *newBlockAllocated);



/********************************************************************************************
 * RPOOL_GetPtr
 * purpose : Return a real pointer to an object inside the given element accoring to the
 *           offset
 * input   : pool       - RPOOL handle
 *           element    - RPOOL element
 *           offset     - Offset of an object inside the element
 * output  : none
 * return  : A real pointer to the object
 ********************************************************************************************/
void RVAPI * RVCALLCONV RPOOL_GetPtr(IN HRPOOL       pool,
                                     IN HPAGE        element,
                                     IN RvInt32    offset);




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
                                          IN RvInt32    size);

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
                                                  IN RvInt32    size);




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
 *           pItem              -
 *
 * return  : RV_OK          - If succeeded.
 *           RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *           RV_ERROR_UNKNOWN             - In case of a failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_AppendFromExternal(IN HRPOOL               pool,
                                                    IN  HPAGE               element,
                                                    IN  const void*         src,
                                                    IN  RvInt                 size,
                                                    IN  RvBool             consecutiveMemory,
                                                    OUT RvInt32           *AllocationOffset,
                                                    OUT RPOOL_ITEM_HANDLE   *pItem);


/********************************************************************************************
 * RPOOL_GetSize
 * purpose : Returns the number of bytes that are currently allocated in a given POOL element
 * input   : pool       - RPOOL handle
 *           element    - RPOOL element
 * output  : none
 * return  : The number of bytes in this element
 ********************************************************************************************/
RvInt32 RVAPI RVCALLCONV RPOOL_GetSize(IN HRPOOL  pool,
                                        IN HPAGE   element);


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***********************************************************************************************
 * SPIRENT_RPOOL_Strcmp
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
RvInt8 RVAPI RVCALLCONV SPIRENT_RPOOL_Strcmp ( IN HRPOOL     FirstPool,
                                        IN HPAGE      FirstPage,
                                        IN RvInt32  FirstOffset,
                                        IN HRPOOL     SecPool,
                                        IN HPAGE      SecPage,
                                        IN RvInt32  SecOffset );
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
RvInt8 RVAPI RVCALLCONV SPIRENT_RPOOL_Stricmp ( IN HRPOOL     FirstPool,
                                         IN HPAGE      FirstPage,
                                         IN RvInt32  FirstOffset,
                                         IN HRPOOL     SecPool,
                                         IN HPAGE      SecPage,
                                         IN RvInt32  SecOffset );

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


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
                                         IN RvInt32   FirstOffset,
                                         IN HRPOOL     SecPool,
                                         IN HPAGE      SecPage,
                                         IN RvInt32   SecOffset);

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
                                        IN RvInt32  SecOffset );



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
										 IN RvBool		CaseSensitive); 

/*********************************************************************************************
 * RPOOL_CmpToExternal
 * purpose : The routine perfroms case sensitive comparison between a NULL terminated string
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
                                               IN RvBool    bIgnoreCase);

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
RvBool RVAPI RVCALLCONV RPOOL_CmpPrefixToExternal ( IN HRPOOL     Pool,
                                                    IN HPAGE      Page,
                                                    IN RvInt32  Offset,
                                                    IN char       *ExtString );
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
                                                    IN char       *ExtString );




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
RvInt32 RVAPI RVCALLCONV RPOOL_GetUnfragmentedSize(IN HRPOOL     pool,
                                                    IN HPAGE      element,
                                                    IN RvInt32  offset);

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
HRA RVAPI RVCALLCONV RPOOL_GetRA(IN HRPOOL  pool);

#endif

/**********************************************************************************************
 * RPOOL_GetResources
 * purpose : Return statistics resources of rpool
 * input   : pool - Handle to the rpool.
 * output  : NumOfAlloc    - Number of allocated blocks in rpool.
 *           CurrNumOfUsed - The currently number of used blocks in rpool.
 *           MaxUsage      - The maximum number of used block at the same time , until now.
 **********************************************************************************************/
/*void RVAPI RVCALLCONV RPOOL_GetResources ( IN  HRPOOL    pool,
                                           OUT RvUint32 *NumOfAlloc,
                                           OUT RvUint32 *CurrNumOfUsed,
                                           OUT RvUint32 *MaxUsage );*/


#ifdef RV_ADS_DEBUG
/************************************************************************************************************************
 * RPOOL_Validate
 * purpose : Make sure RPOOL elements didn't write on top of each other.
 * input   : pool   - Handle to the RPOOL
 * output  : RV_OK if RPOOL elements are fine
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_Validate(IN HRPOOL pool);


/************************************************************************************************************************
 * RPOOL_ValidateElement
 * purpose : Make sure RPOOL element hold correct size information
 * input   : pool    - Handle to the RPOOL
 *           element - Pointer to the pool element
 * output  : RV_OK if RPOOL elements are fine
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RPOOL_ValidateElement(IN HRPOOL  pool,
                                                 IN HPAGE   element);

#endif

/********************************************************************************************
 * RPOOL_IsValidPtr
 * purpose : Returns RV_TRUE is an RPOOL pointer is valid i.e. points to a valid
 *           rpool position.
 * input   : ptr               - rpool pointer
 * return  : RV_TRUE if the pointer is valid. RV_FALSE otherwise
 ********************************************************************************************/
RVAPI RvBool  RVCALLCONV RPOOL_IsValidPtr(
                              RPOOL_Ptr *ptr);

/********************************************************************************************
 * RPOOL_AddPageUser
 * purpose : Add one user to the page users counter
 * input   : pool       - Handle to the RPOOL used
 *           element    - Pointer to the pool element that should be deallocated
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV RPOOL_AddPageUser(IN HRPOOL    pool,
                                        IN HPAGE     element);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
void RPool_MemShow( HRPOOL pool );
#endif
/* SPIRENT_END */

#ifdef __cplusplus
}
#endif


#endif



