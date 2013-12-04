/********************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

**********************************************************************************************************************/



/************************************************************************************************************************
 *                                              RA.c                                                                    *
 *                                                                                                                      *
 * The RA module provides basic memory allocation services to the rest of the stack moduels.                            *
 * RA gets requests to allocate X records of the size Y. It make the allocation on behalf of the module that makes the  *
 * request , returns a handle to this allocation and later it will provide services to allocate and deallocate these    *
 * records from its pool.                                                                                               *
 * RA is resposible to manage the free and used lists of each chain of records.                                         *
 *                                                                                                                      *
 * RA is used by the stack module in order to make real memory allocation and startup time ( and not at run time )      *
 * and to manage this memory for them.                                                                                  *
 * RA is used by both high level abstact data structures modules ( like hash , rlist and rpool ) and by the stack       *
 * module themselfs ( like RAS ,H.245 etc ).                                                                            *
 *                                                                                                                      *
 *                                                                                                                      *
 *                Written by                    Version & Date                            Change                        *
 *               ------------                   ---------------                          --------                       *
 *               Ron Shpilman                        28/1/96                             Fisrt Version                  *
 *               Ayelet Back                         10/11/99                          Adaptation for NG                *
 *                                                                                                                      *
 ************************************************************************************************************************/


/* The structure of an RA element is the following :


  +----------+------------+----------------+--------------------+
  |  header  | bit vector | array of data  | marked bits vector |
  +----------+------------+----------------+--------------------+
  (RAI_Header)               (sizeofElement*maxNodes)


   Header :
     Contains general data on the array like : Number of element , Element size , Beginning of the free list ,
     end of the free list etc
   bit vactor :
     This is an array of bits each bit represents one element in the Array, if the bit = 1 then the elemnt is used
     if it is = 0 , the elemnt is free
   array of data :
     The actual array of element. if a node is used it contains the user data , if it is free it contains a pointer(id)
     of the next free element , in this way the nodes themself are used in order to save the free list with no additional
     data. Since Data node can be used to point to the next element its minimum size is set to 4. ( If the user asks for
     elemnt that is smaller then 4 bytes , 4 bytes will be allocated per each element.
   marked bits vector :
     This appears only in RA structures that contain support for callbacks. It is used to mark elements as being
     handled inside a callback. In such a state, these elements will not be deallocated fully by RA_Delloc - they will be
     removed from the bits vector, but will not be added to the free list of elements until released.

*/



#ifdef __cplusplus
extern "C" {
#endif


/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include "RV_ADS_DEF.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AdsCopybits.h"
#include "AdsRa.h"
#include "rvmemory.h"
#include "rvmutex.h"


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "abmem_pool.h"

typedef struct {

   char* BufferName;
   RA_PrintFunc     PrintFunc;

   ABHRA pool;

	 RvLogSource         logSource;
    RvLogSource*        pLogSource; /*pointer to the log source or null if log is disabled*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvBool             bMutexAllocated;
	 RvMutex            mutex;
#endif

} RAI_Header;

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
#define RA_LOCK(ra) \
    if (ra != NULL) \
		RvMutexLock(&(ra->mutex),NULL);
#else

#define RA_LOCK(ra)
#endif

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
#define RA_UNLOCK(ra) \
    if (ra != NULL) \
		RvMutexUnlock(&(ra->mutex),NULL);
#else

#define RA_UNLOCK(ra)
#endif

#else /* if defined(UPDATED_BY_SPIRENT) */

/*-------------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                               */
/*-------------------------------------------------------------------------*/

/* The following structure is used as an RA header contaning all needed information about the specific array.
   The structure parametrers :
   FirstVacantElement    : First free element in the array free list.
   LastVacantElement     : Last free element in the array free list
   MaxNumOfElements      : The number of element that were allocated in the array.
   ArrayLocation         : Pointer to start location of the data nodes themselfs in the array ( after the header and the
                           occupation bits area ).
   CurrNumOfElements     : The number of elemnts that are used/allocated in the present.
   SizeofElement         : The size of the allocated element.
   MaxUsage              : The maximum number elements that were allocated till now.
   PrintFunc             : Pointer to print function provided at allocation time.
   LogRegsiterId         : Registration number provided by the log module at construction
                           time.
   Name                  : The name of the RA instance.
   FreeAllocation        : RV_TRUE, if we want to free this allocation in the end
   Locks                - Locks instance used
*/
#define POOL_NAME_LEN 32

typedef struct {
    RA_Element			FirstVacantElement;
    RA_Element			LastVacantElement;
    RvUint32			MaxNumOfElements;
    RvChar*			ArrayLocation;
    RvUint32			CurrNumOfElements;
    RvUint32			SizeOfElement;
    RvUint32			MaxUsage;
    RA_PrintFunc        PrintFunc;
	RvLogSource         logSource;
    RvLogSource*        pLogSource; /*pointer to the log source or null if log is disabled*/
    RvChar             Name[POOL_NAME_LEN];
    RvBool             FreeAllocation;
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvBool             bMutexAllocated;
	RvMutex             mutex;
#endif
} RAI_Header;




/* The following structure represents the way the software sees free nodes ( in order to keep a free list ) */
typedef struct {
	RA_Element   nextVacant;
    RvInt32      nodeIndex;
} RAI_VacantNode;


/*-------------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                              */
/*-------------------------------------------------------------------------*/
/* The minumum size of an element is the array*/
#define RAI_ElementMinSize (sizeof(RAI_VacantNode))

/* The following macros provide quick access .calculation to RA elements */


/* Definition of pointer size */
#define RAI_ALIGN sizeof(void*)

/* Definition of the location of the occupation bit vector inside the RA */
#define RAI_BIT_VECTOR(ra)      ((char *)(ra) + sizeof(RAI_Header) )

/* Calculation of the amount of bytes needed for the occupatio bit vector */
#define RAI_BIT_VECTOR_SIZE(n)   ( (n+7)/8 + (RAI_ALIGN - ((n+7)/8)%RAI_ALIGN)%RAI_ALIGN)

/* Calculate the pointer to the location of element i in a given ra */
#define RAI_ELEM_DATA(ra, i)    ((char *) (((RAI_Header*)(ra))->ArrayLocation + (i)*(((RAI_Header*)(ra))->SizeOfElement)))

/* Calculate the next free element after element i in ra */
#define RAI_NEXT_VACANT(ra, i)  (((RAI_VacantNode *)ELEM_DATA(ra, i))->NextVacant)

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
#define RA_LOCK(ra, funcName) \
    if ((ra != NULL)) \
    {RvLogSync(ra->pLogSource, (ra->pLogSource," RA_LOCK- %s (raH=0x%p,mutex=%0x%p)",funcName, ra,&ra->mutex)); RvMutexLock(&(ra->mutex),NULL);}
#else
#define RA_LOCK(ra, funcName)
#endif

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
#define RA_UNLOCK(ra, funcName) \
    if ((ra != NULL)) \
    {RvLogSync(ra->pLogSource, (ra->pLogSource," RA_UNLOCK- %s (raH=0x%p,mutex=%0x%p)",funcName, ra,&ra->mutex));	RvMutexUnlock(&(ra->mutex),NULL);}
#else
#define RA_UNLOCK(ra, funcName)
#endif
#endif
/* SPIRENT_END */


/* temp params just to check the amount of memory we allocate */
#ifdef RV_ADS_DEBUG
static RvUint32   TotalMemory = 0;
#endif



/*-------------------------------------------------------------------------*/
/*                          FUNCTIONS IMPLEMENTATION                       */
/*-------------------------------------------------------------------------*/

/**************************************************************************************************************************
 * RA_ElementIsVacant
 * purpose : Check if a element inside array ( that its location is given ) if free or used.
 *           Checking is done according to the value of the bit that fits the element in the occupation bit vector.
 *           This function should be used for debugging purposes
 * input   : raH      : Pointer to the RA.
 *           Location : The  checked element location,
 * output  : None.
 * return  : RV_TRUE  : If the element is free.
 *           RV_FALSE : If not.
 *************************************************************************************************************************/
RvBool RVAPI RVCALLCONV RA_ElementIsVacant(IN HRA      raH,
                                            IN RvInt32 Location)
{
    RAI_Header *ra = (RAI_Header *)raH;
	RvBool		bIsBitVacant = RV_TRUE;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RA_LOCK(ra);	
   if(ra && ra->pool) bIsBitVacant=ABRA_ElementIsVacant(ra->pool,Location);
   RA_UNLOCK(ra);
   return bIsBitVacant;
#else
	
	if(ra == NULL)
	{
		return bIsBitVacant;
	}
    RA_LOCK(ra, "RA_ElementIsVacant");
    RvLogEnter(ra->pLogSource, (ra->pLogSource,
        "RA_ElementIsVacant - (raH=0x%p(%s),Location=%d)",ra,ra->Name,Location));

	bIsBitVacant = (BITS_GetBit(RAI_BIT_VECTOR(ra), Location)?(RV_FALSE):(RV_TRUE));
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_ElementIsVacant - (raH=0x%p(%s),Location=%d)=%d",ra,ra->Name,Location,bIsBitVacant));

    RA_UNLOCK(ra, "RA_ElementIsVacant");
    return bIsBitVacant;
#endif
/* SPIRENT_END */
}

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
                                            IN RA_Element  ptr)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RV_INT32 position;
   
   RV_BOOL  isVacant=RV_FALSE;
   
   RA_LOCK(ra);
   
   /* Calculate the element */
   isVacant = ABRA_ElemPtrIsVacant(ra->pool,(ABRA_Element)ptr);
   
   RA_UNLOCK(ra);
#else
    RvInt32 location;
    RvInt32 position;
    RvBool  isVacant;

	if(ra == NULL)
	{
		return RV_FALSE;
	}
	RA_LOCK(ra, "RA_ElemPtrIsVacant");

    RvLogEnter(ra->pLogSource, (ra->pLogSource,
        "RA_ElemPtrIsVacant - (raH=0x%p(%s),ptr=0x%p)",ra,ra->Name,ptr));

#ifdef RV_ADS_DEBUG
    /* make sure the given pointer is within array's bounds */
    if ((ra == NULL) ||
        ((RvChar *)ptr < (RvChar *)(ra->ArrayLocation)) ||
        ((RvChar *)ptr > (RvChar *)(ra->ArrayLocation) + ra->MaxNumOfElements * ra->SizeOfElement))
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
                 "(%s) RA_ElemPtrIsVacant: Bad pointer. Shouldn't be here", ra->Name));
		RA_UNLOCK(ra, "RA_ElemPtrIsVacant");
        return (RvBool)RA_NullValue;
    }
#endif

    /* Calculate the element */
    location = (RvInt32)((RvChar *)ptr - (RvChar *)ra->ArrayLocation);
    position =  location / ra->SizeOfElement;
    isVacant =  RA_ElementIsVacant(raH, position) ;

    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_ElemPtrIsVacant - (raH=0x%p(%s),ptr=0x%p)=%d",ra,ra->Name,ptr,isVacant));

    RA_UNLOCK(ra, "RA_ElemPtrIsVacant");
#endif
/* SPIRENT_END */

    return isVacant;
}

/************************************************************************************************************************
* RA_Clear
* purpose : DeAllocate all elements of a given ra.
*           After the routine is done the state of the array is as in initialization time.
* input   : raH : Pointer to the array.
* output  : None.
* return  : RV_OK          - If the allocation succeeded.
*           RV_ERROR_UNKNOWN             - Other failure ( bug )
************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_Clear(IN HRA raH)
{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RAI_Header *ra = (RAI_Header *)raH;
   
   ABRA_Clear(ra->pool);
#else
    RAI_Header *ra = (RAI_Header *)raH;
    RvUint32 i = 0;
	RAI_VacantNode* lastNode  = NULL;
    RAI_VacantNode* curNode = NULL;
	RvChar       * nextNode = NULL;

    if (NULL == raH)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvLogInfo(ra->pLogSource, (ra->pLogSource,
        "RA_Clear - (raH=0x%p(%s))",ra,ra->Name));

    ra->CurrNumOfElements = 0;
    ra->MaxUsage = 0;

    /* -- free list */
    curNode = (RAI_VacantNode *)RAI_ELEM_DATA(ra, 0);
	lastNode = (RAI_VacantNode *)RAI_ELEM_DATA(ra, ra->MaxNumOfElements-1);

    /* Set the first and last vacant element pointers to first and last elements in array */
    if (ra->MaxNumOfElements > 0)
        ra->FirstVacantElement = (RA_Element)curNode;
    else
        ra->FirstVacantElement = NULL;

    ra->LastVacantElement = (RA_Element)lastNode;

    /* Set each element to point at the next element as the next free element */
    for (i = 0; i < ra->MaxNumOfElements; i++)
    {
        nextNode = ((RvChar *)curNode) + ra->SizeOfElement;
        curNode->nextVacant = (RA_Element)nextNode;
        curNode->nodeIndex = i;
        curNode = (RAI_VacantNode *)nextNode;
    }

    /* Make sure last element points to NULL */
    lastNode->nextVacant = NULL;

    /* Mark all occupation bits as free */
    memset(RAI_BIT_VECTOR(ra), 0, RAI_BIT_VECTOR_SIZE(ra->MaxNumOfElements));
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_Clear - (raH=0x%p(%s))",ra,ra->Name));
#endif
/* SPIRENT_END */

    return RV_OK;
}


/*************************************************************************************************************************
 * RA_AllocationSize
 * purpose : The routine returns the allocation size needed in order to contain an RA instance
 *           of a given element size and element count.
 * input   : ElementSize   : The size of each element in the allocatiom.
 *           NumOfElements : The number of elements that should be allocate
 * output  : None.
 * return  : Number of bytes needed for allocation.
 *************************************************************************************************************************/
RvUint32 RVAPI RVCALLCONV RA_AllocationSize(IN RvUint32   ElementSize,
                                             IN RvUint32   NumOfElements)
{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   return sizeof(RAI_Header);
#else
    return
        sizeof(RAI_Header)                 +
        RAI_BIT_VECTOR_SIZE(NumOfElements) +
        ADS_ALIGN_NUMBER                   +  /* For the alignment of the beginning of the array */
        (NumOfElements * RvRoundToSize(RvMax(RAI_ElementMinSize, ElementSize),ADS_ALIGN_NUMBER));
#endif
/* SPIRENT_END */
}



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
#if defined(UPDATED_BY_SPIRENT)
HRA RVAPI RVCALLCONV RA_ConstructFrom_trace(IN RvUint32      ElementSize,
                                      IN RvUint32      NumOfElements,
                                      IN RvChar*       Ptr,
                                      IN RA_PrintFunc   PrintFunc,
                                      IN RvLogMgr	    *pLogMgr,
                                      IN const RvChar* Name,
                                      const char* file, int line)
#else
HRA RVAPI RVCALLCONV RA_ConstructFrom(IN RvUint32      ElementSize,
                                      IN RvUint32      NumOfElements,
                                      IN RvChar*       Ptr,
                                      IN RA_PrintFunc  PrintFunc,
 					             	  IN RvLogMgr	  *pLogMgr,
                                      IN const RvChar* Name)
#endif
/* SPIRENT_END */
{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RAI_Header* ra = NULL;
	 RvStatus crv = RV_OK;

    if(Ptr) {
       printf("Parameter 'Ptr' in the 'RA_ConstructFrom' is deprecated !\n");
        RvMemoryFree(Ptr,NULL);
    }

    if(!Name) Name="";


    if (RV_OK != RvMemoryAlloc(NULL, sizeof(RAI_Header), NULL, (void*)&ra))
    {
       return NULL;
    }
    
    memset(ra, 0, sizeof(*ra));
    
    ra->BufferName=strdup(Name);
    ra->PrintFunc        = PrintFunc;

    ra->pool=ABRA_ConstructFrom_trace(ElementSize,
       NumOfElements,Name,file,line);
#else
    RAI_Header *ra                = (RAI_Header *)Ptr;
    RvUint32    ActualElementSize = 
        RvRoundToSize(RvMax(RAI_ElementMinSize, ElementSize),ADS_ALIGN_NUMBER);
	RvStatus crv = RV_OK;
    RvInt32  nameLen;

	/* RA must contain elements... */
    if (NumOfElements == 0) return NULL;


    ra->MaxNumOfElements = NumOfElements;
    ra->ArrayLocation    = (RvChar*)RvAlignTo( (RvChar *)ra + 
                                               RAI_BIT_VECTOR_SIZE(NumOfElements) + 
                                               sizeof(RAI_Header),
                                               ADS_ALIGN_NUMBER);
    ra->SizeOfElement    = ActualElementSize;
    ra->PrintFunc        = PrintFunc;
    ra->FreeAllocation   = RV_FALSE;
    if (NULL != Name)
    {
        nameLen = (RvInt32)strlen(Name);
        if(nameLen >= POOL_NAME_LEN)
        {
            nameLen = POOL_NAME_LEN - 1;
        }
        memcpy((RvChar *)ra->Name, Name, nameLen);
    }
    else
    {
        nameLen = 0;
    }
    ra->Name[nameLen] = '\0';
#endif
/* SPIRENT_END */

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pLogMgr != NULL)
	{
        crv = RvLogSourceConstruct(pLogMgr,&ra->logSource,ADS_RA,"ra");
		if(crv != RV_OK)
		{
			return NULL;
		}
		ra->pLogSource = &ra->logSource;
    }
	else
	{
        ra->pLogSource = NULL;
	}
#else
	ra->pLogSource = NULL;
    RV_UNUSED_ARG(pLogMgr);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    RvLogInfo(ra->pLogSource, (ra->pLogSource,
        "RA_ConstructFrom - (ElementSize=%u,NumOfElements=%u,pLogMgr=0x%p,name=%s)",
        ElementSize,NumOfElements,pLogMgr,NAMEWRAP(Name)));

    RA_Clear((HRA)ra);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
	crv = RvMutexConstruct(NULL,&ra->mutex);
	if(crv != RV_OK)
	{
		if(ra->pLogSource != NULL)
		{
			RvLogSourceDestruct(ra->pLogSource);
			ra->pLogSource = NULL;
		}
		return NULL;
	}
	ra->bMutexAllocated = RV_TRUE;
#endif

#ifdef RV_ADS_DEBUG
    TotalMemory += (NumOfElements*ElementSize);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RvLogDebug(ra->pLogSource, (ra->pLogSource,
               "RA_Construct for %s allocate %d bytes. Total %d",
               ra->BufferName, NumOfElements*ElementSize, TotalMemory));
#else
    RvLogDebug(ra->pLogSource, (ra->pLogSource,
               "RA_Construct for %s allocate %d bytes. Total %d",
               ra->Name, NumOfElements*ElementSize, TotalMemory));
#endif
/* SPIRENT_END */


#endif

    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_ConstructFrom - (ElementSize=%u,NumOfElements=%u,pLogMgr=0x%p,name=%s)=0x%p",
        ElementSize,NumOfElements,pLogMgr,NAMEWRAP(Name),ra));
    return (HRA)ra;
}


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
#if defined(UPDATED_BY_SPIRENT)
HRA RVAPI RVCALLCONV RA_Construct_trace(IN RvUint32      ElementSize,
                                  IN RvUint32      NumOfElements,
                                  IN RA_PrintFunc   PrintFunc,
                                  IN RvLogMgr      *pLogMgr,
                                  IN const RvChar*    Name,
                                  const char* file,int line)
{
    RAI_Header* ra = (RAI_Header *)RA_ConstructFrom_trace(ElementSize, NumOfElements, NULL, PrintFunc, pLogMgr, Name,file,line);

    return (HRA)ra;
}
#else
HRA RVAPI RVCALLCONV RA_Construct(IN RvUint32      ElementSize,
                                  IN RvUint32      NumOfElements,
                                  IN RA_PrintFunc   PrintFunc,
							      IN RvLogMgr      *pLogMgr,
                                  IN const RvChar*    Name)
{
    RAI_Header *ra;
    RvUint32    NumOfBytesToAlloc;
    char       *ptr = NULL;

    if (ElementSize< RAI_ElementMinSize)
    {
        return NULL;
    }

    NumOfBytesToAlloc = RA_AllocationSize(ElementSize, NumOfElements);

    if (RV_OK != RvMemoryAlloc(NULL, NumOfBytesToAlloc, NULL, (void*)&ptr))
    {
        return NULL;
    }

	memset(ptr, 1,NumOfBytesToAlloc);

    ra = (RAI_Header *)
        RA_ConstructFrom(ElementSize, NumOfElements, ptr, PrintFunc, pLogMgr, Name);

    if (ra != NULL)
	{
        ra->FreeAllocation = RV_TRUE;
	}
	else
	{
		RvMemoryFree(ptr,NULL);
        return NULL;
	}
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_Construct - (ElementSize=%u,NumOfElements=%u,pLogMgr=0x%p,name=%s)=0x%p",
        ElementSize,NumOfElements,pLogMgr,NAMEWRAP(Name),ra));

    return (HRA)ra;
}
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
void RVAPI RVCALLCONV RA_Destruct(IN HRA raH)
{
    RAI_Header *ra = (RAI_Header *)raH;


	if (ra != NULL)
	{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
        RvLogInfo(ra->pLogSource, (ra->pLogSource,
            "RA_Destruct - (raH=0x%p(%s))",ra,ra->BufferName));
#else
        RvLogInfo(ra->pLogSource, (ra->pLogSource,
            "RA_Destruct - (raH=0x%p(%s))",ra,ra->Name));
#endif
/* SPIRENT_END */

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
		if (ra->bMutexAllocated == RV_TRUE)
		{
			RvMutexDestruct(&(ra->mutex),NULL);
		}
#endif
        if(ra->pLogSource != NULL)
        {
           RvLogSourceDestruct(ra->pLogSource);
        }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       if(ra->BufferName) free(ra->BufferName);
       if(ra->pool) {
           ABRA_Destruct(ra->pool);
           ra->pool=NULL;
       }
       RvMemoryFree(ra,NULL);
#else
		if (ra->FreeAllocation)
		{
		    RvMemoryFree(ra,NULL);
		}
#endif
/* SPIRENT_END */
	}
}

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
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVAPI RVCALLCONV RA_Alloc_trace(IN  HRA         raH,
                                          OUT RA_Element* ElementPtr,
                                          const char* file,int line)
{
   RAI_Header *ra = (RAI_Header *)raH;
   RV_Status status=RV_Success;

   RA_LOCK(ra);
      
   if(ra->pool) {
      if(ABRA_Alloc_trace(ra->pool,(ABRA_Element*)ElementPtr,file,line)!=0) {
         status=RV_OutOfResources;
      }
   } else {
      status=RV_Failure;
   }

   RA_UNLOCK(ra);

   return status;
}
#else
RvStatus RVAPI RVCALLCONV RA_Alloc(IN  HRA         raH,
                                    OUT RA_Element* ElementPtr)
{
    RAI_Header *ra = (RAI_Header *)raH;
	RA_Element      allocatedElement;
	RvInt32         vLocation;
	
	if(ra==NULL || ElementPtr==NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	RA_LOCK(ra, "RA_Alloc");

    RvLogEnter(ra->pLogSource, (ra->pLogSource,
        "RA_Alloc - (raH=0x%p(%s),ElementPtr=0x%p)",ra,ra->Name,ElementPtr));

    if ( (*ElementPtr = ra->FirstVacantElement) == NULL )
    {
        RvLogError(ra->pLogSource, (ra->pLogSource,
                  "RA_Alloc - (raH=0x%p(%s),ElementPtr=0x%p)=%d: No more elements are available", 
                  ra,ra->Name,ElementPtr,RV_ERROR_OUTOFRESOURCES));
		RA_UNLOCK(ra, "RA_Alloc");
        return RV_ERROR_OUTOFRESOURCES;
    }

    ra->CurrNumOfElements++;
	allocatedElement = ra->FirstVacantElement;

	/* Get the element from list of vacant elements and fix that list */
	ra->FirstVacantElement = ((RAI_VacantNode *)allocatedElement)->nextVacant;
	if (ra->FirstVacantElement == NULL)
    {
		ra->LastVacantElement = NULL;
    }

		/* Get the index of this element and set in the bit vector */
	vLocation = ((RAI_VacantNode *)allocatedElement)->nodeIndex;

    BITS_SetBit(RAI_BIT_VECTOR(ra), vLocation, RV_TRUE); /* make it occupied */

    /* Set statistical information */
    if (ra->CurrNumOfElements > ra->MaxUsage)
    {
        ra->MaxUsage=ra->CurrNumOfElements;
    }

    *ElementPtr = allocatedElement;

    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_Alloc - (raH=0x%p(%s),*ElementPtr=0x%p)=%d",
        ra,ra->Name,((NULL!=ElementPtr) ? *ElementPtr : NULL), RV_OK));

	RA_UNLOCK(ra, "RA_Alloc");
    return RV_OK;
}
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
 *           RV_ERROR_UNKNOWN - In case of the failure ( bug state )
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus  RVAPI RVCALLCONV RA_DeAlloc_trace(IN HRA       raH,
                                             IN RA_Element delElem,
                                             const char* file,int line)
{
   RAI_Header *ra       = (RAI_Header *)raH;

   if (!ra || (delElem == NULL)) return RV_Failure;

   RA_LOCK(ra);
   
   if(ra->pool) ABRA_DeAlloc_trace(ra->pool,(ABRA_Element)delElem,file,line);
   
   RA_UNLOCK(ra);

   return RV_Success;
}
#else
RvStatus  RVAPI RVCALLCONV RA_DeAlloc(IN HRA       raH,
									   IN RA_Element delElem)
{
    RAI_Header *ra       = (RAI_Header *)raH;
	RvInt32    location = 0;
	if (!ra || (delElem == NULL)) return RV_ERROR_UNKNOWN;
	RA_LOCK(ra, "RA_DeAlloc");

    RvLogEnter(ra->pLogSource, (ra->pLogSource,
        "RA_DeAlloc - (raH=0x%p(%s),delElem=0x%p)",ra,ra->Name,delElem));


	location = RA_GetByPointer(raH, delElem);

    if (location == RA_NullValue)
    {
        RvLogExcep(ra->pLogSource, (ra->pLogSource,
            "RA_DeAlloc (%s): Can't locate element 0x%p", ra->Name,delElem));
        RA_UNLOCK(ra, "RA_DeAlloc");
        return RV_ERROR_UNKNOWN;
    }
	if (location < 0 || (RvUint32)location > ra->MaxNumOfElements)
	{
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
			"RA_DeAlloc (%s): Out of range. Shouldn't be here. delElem=0x%p, Location=0x%p", ra->Name,delElem,location));
		RA_UNLOCK(ra, "RA_DeAlloc");
        return RV_ERROR_UNKNOWN;
    }
    if (RA_ElementIsVacant(raH, location) == RV_TRUE)
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
            "RA_DeAlloc - (raH=0x%p(%s),delElem=0x%p)=%d, allready vacant shuoldnt be here",
            ra,ra->Name,delElem,RV_ERROR_UNKNOWN));
		RA_UNLOCK(ra, "RA_DeAlloc");
        return RV_ERROR_UNKNOWN;
    }

    ra->CurrNumOfElements--;
    ((RAI_VacantNode *)delElem)->nextVacant = NULL;
    ((RAI_VacantNode *)delElem)->nodeIndex = location;
    if (ra->LastVacantElement != NULL)
    {
        ((RAI_VacantNode *)ra->LastVacantElement)->nextVacant = delElem;
    }
    else
    {
        ra->FirstVacantElement = delElem;
    }

    ra->LastVacantElement = delElem;

    /* Set this element as a vacant one in the bits vector */
    BITS_SetBit(RAI_BIT_VECTOR(ra), location, RV_FALSE);
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_DeAlloc - (raH=0x%p(%s),delElem=0x%p)=%d",ra,ra->Name,delElem,RV_OK));

	RA_UNLOCK(ra, "RA_DeAlloc");
    return RV_OK;
}
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
 *           RA_NullValue on errors (only when debugging the code with RV_ADS_DEBUG)
 ***********************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_GetByPointer(IN HRA         raH,
                                          IN RA_Element  ptr)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RV_INT32 position=0;
   
   RA_LOCK(ra);
   
   position=ABRA_GetByPointer(((RAI_Header*)raH)->pool,(ABRA_Element)ptr);
   
   RA_UNLOCK(ra);

   return position;
#else
    RvInt32 location;
    RvInt32 position;
	if(ra == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	RA_LOCK(ra, "RA_GetByPointer");
#ifdef RV_ADS_DEBUG
    /* make sure the given pointer is within array's bounds */
    if ((ra == NULL) ||
        ((RvChar *)ptr < (RvChar *)(ra->ArrayLocation)) ||
        ((RvChar *)ptr > (RvChar *)(ra->ArrayLocation) + ra->MaxNumOfElements * ra->SizeOfElement))
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
                   "RA_GetByPointer - (raH=0x%p(%s),ptr=0x%p): Bad pointer. Shouldn't be here", ra,ra->Name,ptr));
		RA_UNLOCK(ra, "RA_GetByPointer");
        return RA_NullValue;
    }
#endif

    /* Calculate the element */
    location = (RvInt32)((RvChar *)ptr - (RvChar *)ra->ArrayLocation);

    position =  location / ra->SizeOfElement;

	if (RA_ElementIsVacant(raH, position) == RV_TRUE)
    /* Check if element is vacant or not */
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
                   "RA_GetByPointer - (raH=0x%p(%s),ptr=0x%p): Element is vacant. Shouldn't be here", ra,ra->Name,ptr));
		RA_UNLOCK(ra, "RA_GetByPointer");
        return RA_NullValue;
    }
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_GetByPointer - (raH=0x%p(%s),ptr=0x%p)=%d",ra,ra->Name,ptr,RV_OK));

	RA_UNLOCK(ra, "RA_GetByPointer");
    return position;
#endif
/* SPIRENT_END */
}

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
                                             IN RA_Element  ptr)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RV_INT32 position=0;
   
   RA_LOCK(ra);
   
   position=ABRA_GetByPointer(((RAI_Header*)raH)->pool,(ABRA_Element)ptr);
   
   RA_UNLOCK(ra);

   return position;
#else
    RvInt32 location;
    RvInt32 position;
	if(ra == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

    RA_LOCK(ra, "RA_GetAnyByPointer");
#ifdef RV_ADS_DEBUG
    /* make sure the given pointer is within array's bounds */
    if ((ra == NULL) ||
        ((RvChar *)ptr < (RvChar *)(ra->ArrayLocation)) ||
        ((RvChar *)ptr > (RvChar *)(ra->ArrayLocation) + ra->MaxNumOfElements * ra->SizeOfElement))
    {
        RvLogExcep(ra->pLogSource, (ra->pLogSource,
            "RA_GetAnyByPointer(%s) : Bad pointer. Shouldn't be here", ra->Name));
        RA_UNLOCK(ra, "RA_GetAnyByPointer");
        return RA_NullValue;
    }
#endif

    /* Calculate the element */
    location = (RvInt32)((RvChar *)ptr - (RvChar *)ra->ArrayLocation);

    position =  location / ra->SizeOfElement;
    RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_GetAnyByPointer - (raH=0x%p(%s),ptr=0x%p)=%d",ra,ra->Name,ptr,position));

    RA_UNLOCK(ra, "RA_GetAnyByPointer");
    return position;
#endif
/* SPIRENT_END */
}

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
#if defined(UPDATED_BY_SPIRENT)
RvStatus  RVAPI RVCALLCONV RA_DeAllocByPtr_trace (IN HRA         raH,
                                             IN RA_Element  ElementPtr,
                                             const char* file,int line)
{
    RvStatus   Status;
	RAI_Header *ra = (RAI_Header *)raH;
	
    Status   = RA_DeAlloc_trace (raH, ElementPtr, file, line);
    return (Status);

}
#else
RvStatus  RVAPI RVCALLCONV RA_DeAllocByPtr (IN HRA         raH,
                                             IN RA_Element  ElementPtr)
{
    RvStatus   Status;
	RAI_Header *ra ;

    ra = (RAI_Header *)raH;

	RA_LOCK(ra, "RA_DeAllocByPtr");
    Status   = RA_DeAlloc (raH, ElementPtr);
	RA_UNLOCK(ra, "RA_DeAllocByPtr");
    return (Status);
}
#endif
/* SPIRENT_END */

/************************************************************************************************************************
 * RA_GetElement
 * purpose : Return an element located in a given node in a given ra
 * input   : raH      : pointer to the array
 *           Location : The location of the requested element in the ra
 * output  : None
 * return  : The requested element ( NULL is returned in case of Illegal parameters - RV_ADS_DEBUG mode only ).
 ***********************************************************************************************************************/
RA_Element RVAPI RVCALLCONV RA_GetElement(IN HRA       raH,
                                          IN RvInt32  Location)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RA_Element elem = NULL;

    if(ra && ra->pool){ 
        RA_LOCK(ra);
        elem = (RA_Element)(ABRA_GetElement(ra->pool,Location));
        RA_UNLOCK(ra);
    }
   return elem;
#else
	if(ra == NULL)
	{
		return NULL;
	}

    if (((unsigned)Location>=(unsigned)ra->MaxNumOfElements))
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
            "RA_GetElement - (raH=0x%p(%s),Location=%d) out of range",ra,ra->Name,Location));
        return NULL;
    }

    if (RA_ElementIsVacant(raH, Location) == RV_TRUE)
    {
		RvLogExcep(ra->pLogSource, (ra->pLogSource,
            "RA_GetElement - (raH=0x%p(%s),Location=%d) vacant",ra,ra->Name,Location));
        return NULL;
    }

    return (RA_Element)RAI_ELEM_DATA(raH, Location);
#endif
/* SPIRENT_END */
}

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
                                             IN RvInt32  Location)
{
    RAI_Header *ra = (RAI_Header *)raH;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    RA_Element elem = NULL;

    if(ra && ra->pool){ 
        RA_LOCK(ra);
        elem = (RA_Element)(ABRA_GetElementInc(ra->pool,Location));
        RA_UNLOCK(ra);
    }
   return elem;

#else
    if (!ra)
    {
        return NULL;
    }

    if ((unsigned)Location>=(unsigned)ra->MaxNumOfElements)
    {
        return NULL;
    }

    return (RA_Element)RAI_ELEM_DATA(raH, Location);
#endif
/* SPIRENT_END */
}

/************************************************************************************************************************
 * RA_GetFirstUsedElement
 * purpose : Return the first used element from the given location (include this one) in the given ra.
 *           This routine perform a O(n) search and should be called only at shutdown time.
 * input   : raH            : Pointer to the array
 *           StartLocation  : The start location to begin the reach from.
 * output  : Element        : Pointer to the requested element.
 *           ElemLoc        : The element location.
 * return  : RV_OK     : In case we find one.
 *           RV_ERROR_UNKNOWN     : Otherwise (no used element after the given start location).
 ***********************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_GetFirstUsedElement(IN  HRA          raH,
                                                  IN  RvInt       StartLocation,
                                                  OUT RA_Element*  Element,
                                                  OUT RvInt32*    ElemLoc)
{
    RAI_Header  *ra = (RAI_Header*)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   RvStatus status=RV_Failure;

   RA_LOCK(ra);

   if(ra->pool) status=ABRA_GetFirstUsedElement(ra->pool,StartLocation,(ABRA_Element*)Element,ElemLoc);

   RA_UNLOCK(ra);

   return status;
#else
    RvBool     Found = RV_FALSE;
    RvUint32   i;

	if(ra == NULL)
	{
		return RV_ERROR_UNKNOWN;
	}

	RA_LOCK(ra, "RA_GetFirstUsedElement");
    for ( i = StartLocation ; (( i < ra->MaxNumOfElements ) && ( !Found )) ; i++ )
    {
        if ( ! RA_ElementIsVacant ( raH , i ))
        {
            Found = RV_TRUE;
            *ElemLoc = i;
            *Element = (RA_Element)RAI_ELEM_DATA(raH, i);
        }
    }
	RA_UNLOCK(ra, "RA_GetFirstUsedElement");

	if(ra == NULL) /*this one is for lint - don't remove*/
	{
		return RV_ERROR_UNKNOWN;
	}

    if ( Found)
    {
		RvLogLeave(ra->pLogSource, (ra->pLogSource,
            "RA_GetFirstUsedElement - (raH=0x%p(%s),*ElemLoc=%d)=0",ra,ra->Name,*ElemLoc));
        return RV_OK;
    }
    else
    {
        RvLogError(ra->pLogSource, (ra->pLogSource,
            "RA_GetFirstUsedElement - (raH=0x%p(%s))=%d",ra,ra->Name,RV_ERROR_UNKNOWN));
        return (RV_ERROR_UNKNOWN);
    }
#endif
/* SPIRENT_END */
}

/*************************************************************************************************************************
 * RA_NumOfUsedElements
 * purpose : The number of used elements in a given allocation.
 * input   : raH - Handle to the array allocation.
 * output  : None.
 * return  : The number of used elements.
 *************************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_NumOfUsedElements(IN HRA raH)
{
    RAI_Header *ra = (RAI_Header *)raH;
    RvInt32     usedElem = 0;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

    if(ra && ra->pool) {
        RA_LOCK(ra);
        usedElem = ABRA_NumOfUsedElements(ra->pool);
        RA_UNLOCK(ra);
    }

    return usedElem;
#else
    if (ra == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

	RA_LOCK(ra, "RA_NumOfUsedElements");
    usedElem = ra->CurrNumOfElements;
	RA_UNLOCK(ra, "RA_NumOfUsedElements");
    return usedElem;
#endif
/* SPIRENT_END */
}

/************************************************************************************************************************
 * RA_NumOfFreeElements
 * purpose : Return the number of free elements in a given allocation.
 * input   : raH - Handle to the array allocation
 * output  : None.
 * return  : The number of free elements
 *************************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_NumOfFreeElements(IN HRA raH)
{
    RAI_Header *ra       = (RAI_Header *)raH;
    RvInt32     freeElem = 0;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

    if(ra && ra->pool){ 
        RA_LOCK(ra);
        freeElem = ABRA_NumOfFreeElements(ra->pool);
        RA_UNLOCK(ra);
    }

    return freeElem;
#else

    if (ra == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    
    RA_LOCK(ra, "RA_NumOfFreeElements");
    freeElem = ra->MaxNumOfElements - ra->CurrNumOfElements;
	RA_UNLOCK(ra, "RA_NumOfFreeElements");
    
    return freeElem;
#endif
/* SPIRENT_END */
}

/*************************************************************************************************************************
 * RA_GetAllocationParams
 * purpose : The routine returned the parameters of a givne allocation ( elementSize and Number of elements ).
 * input   : raH - Pointer to the allocation
 * output  : ElementSize   - The size of the basic record in this allocation.
 *           NumOfElements - The number of elements that were allocated for this allocation.
 *************************************************************************************************************************/
void RVAPI RVCALLCONV RA_GetAllocationParams(IN  HRA        raH,
                                             OUT RvUint32* ElementSize,
                                             OUT RvUint32* NumOfElements)
{
    RAI_Header *ra = (RAI_Header *)raH;

    if (!raH)
    {
        if (ElementSize != NULL) *ElementSize = 0;
        if (NumOfElements != NULL) *NumOfElements = 0;
        return;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(ra && ra->pool) ABRA_GetAllocationParams(ra->pool,
       ElementSize,NumOfElements);
#else
    if (ElementSize != NULL)
        *ElementSize = ra->SizeOfElement;
    if (NumOfElements != NULL)
        *NumOfElements = ra->MaxNumOfElements;
#endif
/* SPIRENT_END */
}


/************************************************************************************************************************
 * RA_GetResourcesStatus
 * purpose : Return information about the resource allocation of this RA.
 * input   : raH : Pointer to the array.
 * output  : resources  - Includes the following information:
 * return  : RV_OK - In case of success.
 *           RV_ERROR_UNKNOWN    - In case of the failure ( bug state )
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RA_GetResourcesStatus(IN  HRA           raH,
                                                 OUT RV_Resource*  resources)
{
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
   ABRA_Resource res;
   RAI_Header *ra = (RAI_Header *)raH;

   if (ra == NULL)
   {
       return RV_ERROR_UNKNOWN;
   }

   RA_LOCK(ra);
   RvStatus status=ABRA_GetResourcesStatus(ra->pool,&res);
   RA_UNLOCK(ra);

   resources->numOfUsed        = res.numOfUsed;
   resources->maxUsage         = res.maxUsage;
   resources->maxNumOfElements = res.maxNumOfElements;

   return status;
#else
    RAI_Header *ra = (RAI_Header *)raH;

    if (ra == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RA_LOCK(ra, "RA_GetResourcesStatus");
    resources->numOfUsed        = ra->CurrNumOfElements;
    resources->maxUsage         = ra->MaxUsage;
    resources->maxNumOfElements = ra->MaxNumOfElements;
    RA_UNLOCK(ra, "RA_GetResourcesStatus");
    return RV_OK;
#endif
/* SPIRENT_END */
}


/************************************************************************************************************************
 * RA_ResetMaxUsageCounter
 * purpose : Reset the counter that counts the max number of nodes that were used ( at one time ) until the call to this
 *           routine.
 *           No sanity check is done for a debug routine
 * input   : raH : Pointer to the array.
 * output  : None.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RA_ResetMaxUsageCounter(IN HRA raH)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(ra && ra->pool) ABRA_ResetMaxUsageCounter(ra->pool);
#else
    ra->MaxUsage = 0;
#endif
/* SPIRENT_END */
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/*************************************************************************************************************************
 * RA_GetName
 * purpose : returns the ra name
 * input   : raH - Pointer to the ra.
 * output  : returns the ra name
 ************************************************************************************************************************/
RVAPI const RvChar*  RVCALLCONV RA_GetName(IN HRA   raH)
{
    RAI_Header *ra = (RAI_Header *)raH;
    if (!ra)
    {
        return "";
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    return NAMEWRAP(ra->BufferName);
#else
    return NAMEWRAP(ra->Name);
#endif
/* SPIRENT_END */
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef RV_ADS_DEBUG
/*************************************************************************************************************************
 * RA_Print
 * purpose : Print a given allocation ( array ).
 *           The routine uses the print routine that was given as parameter at construction time.
 * input   : raH - Pointer to the allocation.
 *           param : Parameters to the print routine.
 * output  : None.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RA_Print(IN HRA   raH,
                               IN void* Param)
{
    RAI_Header *ra = (RAI_Header *)raH;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(ra && ra->pool) ABRA_Print(ra->pool,Param,(ABRA_PrintFunc)(ra->PrintFunc));
#else
    RvUint32 i;


    if (!ra)
    {
        return;
    }

    for (i=0; i < ra->MaxNumOfElements; i++)
        if (!RA_ElementIsVacant( raH, i) && ra->PrintFunc )
            ra->PrintFunc( (RA_Element) RAI_ELEM_DATA(ra, i), Param );
#endif
/* SPIRENT_END */
}

#endif /* RV_ADS_DEBUG */

/************************************************************************************************************************
 * RA_ElemIsValid
 * purpose : Check if the given element pointer resides within a given array. 
 * input   : raH      : pointer to the array
 *           ptr      : The pointer to an RA element, to be checked 
 * output  : None
 * return  : RV_TRUE - If the input is valid, otherwise RV_FALSE
 ***********************************************************************************************************************/
RvInt32 RVAPI RVCALLCONV RA_ElemIsValid(IN HRA         raH,
                                        IN RA_Element  ptr)
{
	RvBool		bValid = RV_TRUE;
    RAI_Header *ra	   = (RAI_Header *)raH;
	RvUint32	location;
     

	if(ra == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    return RV_TRUE;
#else
	RA_LOCK(ra, "RA_ElemIsValid");

	RvLogEnter(ra->pLogSource, (ra->pLogSource,
        "RA_ElemIsValid - (raH=0x%p(%s),ptr=0x%p)",ra,ra->Name,ptr));

    /* Check if the given pointer is within array's bounds */
    if (((RvChar *)ptr < (RvChar *)(ra->ArrayLocation)) ||
        ((RvChar *)ptr > (RvChar *)(ra->ArrayLocation) + ra->MaxNumOfElements * ra->SizeOfElement))
    {
		bValid = RV_FALSE; 
    }
	else 
	{
		location = (RvUint32)((RvChar *)ptr - (RvChar *)ra->ArrayLocation);
		bValid   = ((RvUint32)(location / ra->SizeOfElement))* ra->SizeOfElement == location ? RV_TRUE : RV_FALSE;
	}
	
	RvLogLeave(ra->pLogSource, (ra->pLogSource,
        "RA_ElemIsValid - (raH=0x%p(%s),ptr=0x%p,valid=%d)=%d",ra,ra->Name,ptr,bValid,RV_OK));
	
	RA_UNLOCK(ra, "RA_ElemIsValid");
    return bValid;
#endif
/* SPIRENT_END */
}

#ifdef __cplusplus
}
#endif



