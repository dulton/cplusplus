/************************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/************************************************************************************************************************
 *                                              RA.H                                                                    *
 *                                                                                                                      *
 * The RA module provides basic memory allocation services to the rest of the stack moduels.                            *
 * RA gets requests to allocate X records of the size Y. It make the allocation on behalf of the module that makes the  *
 * request , returned a handle to this allocation and later it will provide services to allocate and deallocate these   *
 * records from its pool.                                                                                               *
 * RA is resposible to manage the free and used lists of each chain of records.                                         *
 *                                                                                                                      *
 * RA is used by the stack module in order to make real memory allocation and startup time ( and not at run time )      *
 * and to manage this memory for them.                                                                                  *
 * RA is used by both high level abstact data structures modules ( like hash , rlist and rpool ) and by the stack       *
 * module themselfs ( like RAS ,H.245 etc ).                                                                            *
 *                                                                                                                      *
 *                Written by                    Version & Date                            Change                        *
 *               ------------                   ---------------                          --------                       *
 *               Ron Shpilman                        28/1/96                                                            *
 *               Ayelet Back                         10/11/99                            Adaptation for NG              *
 *                                                                                                                      *
 ************************************************************************************************************************/


#ifndef RA_H
#define RA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_ADS_DEF.h"
#include "rvlog.h"

/* Define pointer to NULL ( in turns of location ) */
#define RA_NullValue (-1)

/* Define the type of the handle that is returned by RA package */
RV_DECLARE_HANDLE (HRA);



/*********** element prototypes ************/
RV_DECLARE_HANDLE(RA_Element);

typedef void (*RA_PrintFunc)(RA_Element Elem, void *Param);



/*************************************************************************************************************************
 * RA_Construct
 * purpose : The routine is used in order to create a new allocation according to the given parameters.
 *           This is a place where REAL memory allocation ( from the OS ) is done.
 *           The routine return a pointer to the application.
 *           After the allocation is done all allocated records are marked as unused.
 *           Note - No partial allocation is done , if the routine cannot allocate all required memory , nothing will be
 *                  allocated and a NULL handle will be returned.
 * input   : ElementSize   : The size of each element in the allocatiom.
 *           NumOfElements : The number of elements that should be allocated.
 *           SupportCb     : Support callback deallocations
 *           PrintFunc     : Pointer to a line printing function.
 *           pLogMgr       : pointer to a log manager.
 *           Name          : The name of the allocation.
 * output  : None.
 * return  : Pointer to the new allocation , NULL in case of a failure.
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
#define RA_Construct(ElementSize,NumOfElements,PrintFunc,LogHandle,Name) RA_Construct_trace(ElementSize,NumOfElements,PrintFunc,LogHandle,Name,__FILE__,__LINE__)

HRA RVAPI RVCALLCONV RA_Construct_trace(IN RvUint32      ElementSize,
                                  IN RvUint32      NumOfElements,
                                  IN RA_PrintFunc   PrintFunc,
                                  IN RvLogMgr      *pLogMgr,
                                  IN const RvChar*    Name,
                                  const char* file, int line);
#else
HRA RVAPI RVCALLCONV RA_Construct(IN RvUint32      ElementSize,
                                  IN RvUint32      NumOfElements,
                                  IN RA_PrintFunc   PrintFunc,
							      IN RvLogMgr      *pLogMgr,
                                  IN const RvChar*    Name);
#endif
/* SPIRENT_END */


/*************************************************************************************************************************
 * RA_Destruct
 * purpose : Free the memory of a given allocation.
 *           This is the place where the REAL memory deallocation is done.
 *           This routine will be called by other functions when a stack instance is closed.
 * input   : raH : Pointer to the allocation that should be freed.
 * output  : None.
 **************************************************************************************************************************/
void RVAPI RVCALLCONV RA_Destruct(IN HRA raH);


/************************************************************************************************************************
 * RA_Alloc
 * purpose : Allocate one record from a specific allocation and return a pointer to it and its serial number in the allocated
 *           array.
 *           The allocated record will be marked as used.
 * input   : raH - Pointer to the allocation/array from which one record allocation should be done.
 * output  : ElementPtr - Pointer to the allocated element.
 *           ElementId  - The serial number of of the allocated element.
 * return  : RV_OK          - If the allocation succeeded.
 *           RV_ERROR_OUTOFRESOURCES   - In case all elements in the allocation are used.
 ************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
#define RA_Alloc(raH,ElementPtr) RA_Alloc_trace(raH,ElementPtr,__FILE__,__LINE__)
RvStatus RVAPI RVCALLCONV RA_Alloc_trace(IN  HRA         raH,
                                    OUT RA_Element* ElementPtr,
                                    const char* file, int line);
#else
RvStatus RVAPI RVCALLCONV RA_Alloc(IN  HRA         raH,
                                    OUT RA_Element* ElementPtr);
#endif
/* SPIRENT_END */


/*************************************************************************************************************************
 * RA_DeAllocByPtr
 * purpose : The routine is used in order to free an allocation/array element back to the pool.
 *           It gets the pointer tp the element that we want to release, mark it
 *           as free and enter it to the free list.
 * input   : raH        - Pointer to the allocation.
 *           ElementPtr - Pointer to the element.
 * output  : None.
 * return  : RV_OK - In case of success.
 *           RV_ERROR_UNKNOWN - In case of the failure ( bug state )
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
#define RA_DeAllocByPtr(raH,Element) RA_DeAllocByPtr_trace(raH,Element,__FILE__,__LINE__)
RvStatus  RVAPI RVCALLCONV RA_DeAllocByPtr_trace (IN HRA         raH,
                                            IN RA_Element  ElementPtr,
                                            const char* file, int line);
#else
RvStatus  RVAPI RVCALLCONV RA_DeAllocByPtr (IN HRA         raH,
                                             IN RA_Element  ElementPtr);
#endif
/* SPIRENT_END */

/*************************************************************************************************************************
 * RA_DeAlloc
 * purpose : The routine is used in order to free an allocation/array element back to the pool.
 *           It gets the allocation serial ID , mark it as free and enter it to the free list.
 * input   : raH       - Pointer to the allocation.
 *           ElementId - The element serial number ( location in the allocation list ).
 * output  : None.
 * return  : RV_OK - In case of success.
 *           RV_ERROR_UNKNOWN    - In case of the failure ( bug state )
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
#define RA_DeAlloc(raH,Element) RA_DeAlloc_trace(raH,Element,__FILE__,__LINE__)
RvStatus  RVAPI RVCALLCONV RA_DeAlloc_trace(IN HRA       raH,
									   IN RA_Element delElem,
                              const char* file, int line);
#else
RvStatus  RVAPI RVCALLCONV RA_DeAlloc(IN HRA       raH,
									   IN RA_Element delElem);
#endif
/* SPIRENT_END */


/************************************************************************************************************************
 * RA_GetByPointer
 * purpose : Return an element location in RA (it serial number/handle) by the pointer of the
 *           information it holds.
 * input   : raH      : pointer to the array
 *           ptr      : The pointer to the information in holds
 * output  : None
 * return  : The element location in RA.
 *           RA_NullValue on errors (only when debugging the code with RA_DEBUG)
 ***********************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_GetByPointer(IN HRA         raH,
                                          IN RA_Element  ptr);

/************************************************************************************************************************
 * RA_GetAnyByPointer
 * purpose : Return an element location in RA (it serial number/handle) by the
 *           pointer of the information it holds. Element can be vacant or busy
 * input   : raH      : pointer to the array
 *           ptr      : The pointer to the information in holds
 * output  : None
 * return  : The element location in RA.
 *           RA_NullValue on errors (only when debugging the code with RA_DEBUG)
 ***********************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_GetAnyByPointer(IN HRA         raH,
                                             IN RA_Element  ptr);

/*************************************************************************************************************************
 * RA_AllocationSize
 * purpose : The routine returns the allocation size needed in order to contain an RA instance
 *           of a given element size and element count.
 * input   : ElementSize   : The size of each element in the allocatiom.
 *           NumOfElements : The number of elements that should be allocated.
 * output  : None.
 * return  : Number of bytes needed for allocation.
 *************************************************************************************************************************/
RvUint32 RVAPI RVCALLCONV RA_AllocationSize(IN RvUint32   ElementSize,
                                             IN RvUint32   NumOfElements);


/*************************************************************************************************************************
 * RA_ConstructFrom
 * purpose : The routine is used in order to create a new RA according to the given parameters using a
 *           preallocated memory buffer.
 *           The routine return a pointer to the application.
 *           After the allocation is done all allocated records are marked as unused.
 * input   : ElementSize   : The size of each element in the allocatiom.
 *           NumOfElements : The number of elements that should be allocated.
 *           Ptr           : A pointer to a memory block where the RA instance is to be put.
 *                           It is assumed that Ptr points to a large enough block of memory.
 *                           The size needed for the RA instance is returned by the
 *                           RA_AllocationSize function.
 *           PrintFunc     : Pointer to a line printing function.
 *           pLogMgr       : pointer to a log manager.
 * output  : None.
 * return  : Pointer to the new allocation , NULL in case of a failure.
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
#define RA_ConstructFrom(ElementSize,NumOfElements,Ptr,PrintFunc,LogHandle,Name) RA_ConstructFrom_trace(ElementSize,NumOfElements,Ptr,PrintFunc,LogHandle,Name,__FILE__,__LINE__)

HRA RVAPI RVCALLCONV RA_ConstructFrom_trace(IN RvUint32      ElementSize,
                                      IN RvUint32      NumOfElements,
                                      IN RvChar*       Ptr,
                                      IN RA_PrintFunc   PrintFunc,
                                      IN RvLogMgr	    *pLogMgr,
                                      IN const RvChar* Name,
                                      const char* file, int line);
#else
HRA RVAPI RVCALLCONV RA_ConstructFrom(IN RvUint32      ElementSize,
                                      IN RvUint32      NumOfElements,
                                      IN RvChar*       Ptr,
                                      IN RA_PrintFunc   PrintFunc,
 					             	  IN RvLogMgr	    *pLogMgr,
                                      IN const RvChar* Name);
#endif
/* SPIRENT_END */
/************************************************************************************************************************
 * RA_GetElement
 * purpose : Return an element located in a given node in a given ra
 * input   : raH      : pointer to the array
 *           Location : The location of the requested element in the ra
 * output  : None
 * return  : The requested element ( NULL is retruned in case of Illegal parameters - DEBUG mode only ).
 ***********************************************************************************************************************/
RA_Element RVAPI RVCALLCONV RA_GetElement(IN HRA       raH,
                                          IN RvInt32  Location);

/************************************************************************************************************************
 * RA_GetAnyElement
 * purpose : Return an element located in a given node in a given ra
 *           The element can be vacant or busy
 * input   : raH      : pointer to the array
 *           Location : The location of the requested element in the ra
 * output  : None
 * return  : The requested element ( NULL is returned in case of Illegal parameters).
 ***********************************************************************************************************************/
RA_Element RVAPI RVCALLCONV RA_GetAnyElement(IN HRA       raH,
                                             IN RvInt32  Location);

/**************************************************************************************************************************
 * RA_ElementIsVacant
 * purpose : Check if a element inside array ( that its location is given ) if free or used.
 *           Checking is done according to the value of the bit that fits the element in the occupation bit vector.
 *           This function should be used for debugging purposes
 * input   : raH      : Pointer to the RA.
 *           Location : The  checked element location,
 * output  : None.
 * return  : True  : If the element is free.
 *           False : If not.
 *************************************************************************************************************************/
RvBool RVAPI RVCALLCONV RA_ElementIsVacant(IN HRA      raH,
                                            IN RvInt32 Location);

/************************************************************************************************************************
 * RA_ElemPtrIsVacant
 * purpose : Check if a element inside array ( that its ra-pointer is given ) if free or used.
 * input   : raH      : pointer to the array
 *           ptr      : The pointer to the information in holds
 * output  : None
 * return  : The element location in RA.
 *           RA_NullValue on errors (only when debugging the code with RV_ADS_DEBUG)
 ***********************************************************************************************************************/
RvBool RVAPI RVCALLCONV RA_ElemPtrIsVacant(IN HRA         raH,
                                            IN RA_Element  ptr);

/************************************************************************************************************************
 * RA_GetFirstUsedElement
 * purpose : Return the first used element from the given location (include this one) in the given ra.
 *           This routine perfrom a O(n) search and should be called only at shutdown time.
 * input   : raH            : Pointer to the array
 *           StartLocation  : The start location to begin the rearch from.
 * output  : Element        : Pointer to the requested element.
 *           ElemLoc        : The element location.
 * return  : RV_OK     : In case we find one.
 *           RV_ERROR_UNKNOWN     : Otherwise (no used element after the given start location).
 ***********************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_GetFirstUsedElement(IN  HRA          raH,
                                                  IN  int          StartLocation,
                                                  OUT RA_Element*  Element,
                                                  OUT RvInt32*    ElemLoc);

/*************************************************************************************************************************
 * RA_NumOfUsedElements
 * purpose : The number of used elements in a given allocation.
 * input   : raH - Handle to the array allocation.
 * output  : None.
 * return  : The number of used elements.
 *************************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_NumOfUsedElements(IN HRA raH);


/************************************************************************************************************************
 * RA_NumOfFreeElements
 * purpose : Return the number of free elements in a given allocation.
 * input   : raH - Handle to the array allocation
 * output  : None.
 * return  : The number of free elements
 *************************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_NumOfFreeElements(IN HRA raH);

/*************************************************************************************************************************
 * RA_GetAllocationParams
 * purpose : The routine returned the parameters of a given allocation (elementSize and Number of elements).
 * input   : raH - Pointer to the allocation
 * output  : ElementSize   - The size of the basic record in this allocation.
 *           NumOfElements - The number of elements that were allocated for this allocation.
 *************************************************************************************************************************/

void RVAPI RVCALLCONV RA_GetAllocationParams(IN  HRA        raH,
                                             OUT RvUint32* ElementSize,
                                             OUT RvUint32* NumOfElements);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/*************************************************************************************************************************
 * RA_GetName
 * purpose : returns the RA name
 * input   : raH - Pointer to the RA.
 * output  : returns the RA name
 ************************************************************************************************************************/
RVAPI const RvChar*  RVCALLCONV RA_GetName(IN HRA   raH);

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/



/************************************************************************************************************************
 * RA_GetResourcesStatus
 * purpose : Return information about the resource allocation of this RA.
 * input   : raH : Pointer to the array.
 * output  : resources   - Includes the following information:
 * return  : RV_OK - In case of success.
 *           RV_ERROR_UNKNOWN    - In case of the failure ( bug state )
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_GetResourcesStatus(IN  HRA           raH,
                                                 OUT RV_Resource*  resources);



/************************************************************************************************************************
 * RA_ResetMaxUsageCounter
 * purpose : Reset the counter that counts the max number of nodes that were used ( at one time ) until the call to this
 *           routine.
 * input   : raH : Pointer to the array.
 * output  : None.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RA_ResetMaxUsageCounter(IN HRA raH);

/************************************************************************************************************************
* RA_Clear
* purpose : DeAllocate all elements of a given ra.
*           After the routine is done the state of the array is as in initialization time.
* input   : raH : Pointer to the array.
* output  : None.
* return  : RV_OK          - If the allocation succeeded.
*           RV_ERROR_UNKNOWN             - Other failure ( bug )
************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_Clear(IN HRA raH);


/************************************************************************************************************************
 * RA_ElemIsValid
 * purpose : Check if the given element pointer resides within a given array. 
 * input   : raH      : pointer to the array
 *           ptr      : The pointer to an RA element, to be checked 
 * output  : None
 * return  : RV_TRUE - If the input is valid, otherwise RV_FALSE
 ***********************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_ElemIsValid(IN HRA         raH,
                                        IN RA_Element  ptr); 


#ifdef __cplusplus
}
#endif

#endif


