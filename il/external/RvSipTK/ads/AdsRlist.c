
/************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************/


/************************************************************************************************
*                                              RLIST
*
* RLIST provide the functionality of two layers of two ways link lists.:
* 1. Level one - Pool of lists. All data items in all lists will have the same size and all memory
*                allocation ( for all items ) will be done at intialization over RA.
*                In addition the max number of potential lists over this pool will be determined
*                during the initialization. However no connection between potentail data items and
*                potential lists will be done at this stage.
* 2. Level Two - Single List object. The user will able to allocate a single list object over the pool
*                and later to insert elements to it.
* 3. Level Three - Contains the actual list element, A list element will be composed from the user data
*                  along with management record to keep links between the list elements.
*
*
*       Written by            Version & Date               Change
*       ------------          ---------------             --------
*       Ayelet Back                                       11/2000
*
************************************************************************************************/




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
#include "rvstdio.h"
#include "AdsRlist.h"
#include "rvmemory.h"
#include "rvlog.h"

/*-------------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                              */
/*-------------------------------------------------------------------------*/
#define LIST_NullValue RA_NullValue

/***********************************************************************************************************************
 * GetListLogSource
 * purpose : get the log source from a list
 * input   : _p : list pool
 *           _l : list
 * return  : log source
 *************************************************************************************************************************/
#define GetListLogSource(_p,_l) (NULL == _p) ? (((RPoolListHeaderType*)(_l))->pLogSource) : (((PoolOfListsType*)(hPoolList))->pLogSource)

/*-------------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                               */
/*-------------------------------------------------------------------------*/

/* This is the type that keep track on each pool of lists.
   It contains :
   ListHeaderRA             - Pointer to the RA that keeps the list headers.
   ElementRA                - Pointer to the RA that keeps the user element records.
   LogHandle                - The handle to the log module.
   LogRegisterId            - The identification ID returned from the LOG module at registration time.*/

typedef struct
{
    HRA                ListsHeaderRA;
    HRA                ElementsRA;
    RvLogMgr           *pLogMgr;
    RvLogSource        *pLogSource;
} PoolOfListsType;


/* This is the header type for each single list.
   It contains the following fields :
   FirstElementId : The pointer to the first element of the list. ( pointer to element in the elements RA )
   LastElementId  : The pointer to the last element in the list (  pointer to element in the elements RA ). */
typedef struct SingleListHeaderType
{
   RA_Element FirstElementPtr;
   RA_Element LastElementPtr;
} SingleListHeaderType;


/* This is the header type for a single dinamic list, which is built withing an rpool.
   It contains the following fields :
   FirstElementId : The pointer to the first element of the list.
                    ( pointer to element in the elements RA. This RA is on the memory pool.
                      It is not defined by core RA!).
   LastElementId  : The pointer to the last element in the list
                    ( pointer to element in the elements RA. This RA is on the memory pool.
                      It is not defined by core RA!).
   hPool          : Memory pool.
   hPage          : Memory page.
   elementSize    : The size of element in the list.
   LogHandle      : The handle to the log module.
   LogRegisterId  : The identification ID returned from the LOG module at registration time. */
typedef struct RPoolListHeaderType
{
   SingleListHeaderType  listHeaderType;
   HRPOOL                hPool;
   HPAGE                 hPage;
   RvInt32              elementSize;
   RvLogSource*          pLogSource;
} RPoolListHeaderType;


/* This is the type  that is attached per each single element to point to its next and prev elements
   ( and actually to manage the list ).
   It conatins the following fields :
   NextElement : The pointer to the next element in the list ( pointer in the elements RA ).
   PrevElement : The pointer to the previous element in the list ( pointer in the elements RA ).*/
typedef struct
{
     RA_Element NextElement;
     RA_Element PrevElement;
}ElementHeaderType;

/*-------------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                              */
/*-------------------------------------------------------------------------*/

/* Static functions */
static RvStatus AllocateNewElementFromRPool(IN  RPoolListHeaderType  *pList,
                                             OUT RA_Element           *phElement);

/*-------------------------------------------------------------------------*/
/*                          FUNCTIONS IMPLEMENTATION                       */
/*-------------------------------------------------------------------------*/

/***********************************************************************************************************************
 * RLIST_PoolListConstruct
 * purpose : This routine is used to allocate a pool of lists elements.
 *           It receives all pool list specifications ( as described in the parameters list below )
 *           and used RA to allocates memory for the list headers and list elements
 *           Note - No partial allocation is done , if there is no enough memory to do all allocations
 *           nothing is allocated and a NULL pointer is returned.
 * input   : TotalNumberOfUserElements : The number of user elements in all lists.
 *           TotalNumberOfLists        : The max number of seperate lists that will be allocated over the pool.
 *           UserDataRecSize           : The size of the user data record.
 *           pLogMgr                   : pointer to a log manager.
 *           name                      : The name of the module to be passed to the RA
 *                                       for the purpose of detailed printing.
 *
 * output  : None.
 * return  : Handle to the list ( NULL value if the allocation has failed ).
 *************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
RLIST_POOL_HANDLE RVAPI RVCALLCONV RLIST_PoolListConstruct_trace ( IN RvUint        TotalNumberOfUserElements,
                                                             IN RvUint        TotalNumberOfLists,
                                                             IN RvUint        UserDataRecSize ,
                                                             IN RvLogMgr*      pLogMgr,
                                                             IN const char*    Name,
                                                             const char* file, int line)
#else
RLIST_POOL_HANDLE RVAPI RVCALLCONV RLIST_PoolListConstruct ( IN RvUint        TotalNumberOfUserElements,
                                                             IN RvUint        TotalNumberOfLists,
                                                             IN RvUint        UserDataRecSize ,
                                                             IN RvLogMgr*      pLogMgr,
                                                             IN const char*    Name)
#endif
/* SPIRENT_END */

{
    PoolOfListsType *pListPool;

    if (RV_OK != RvMemoryAlloc(NULL, sizeof(PoolOfListsType), NULL, (void*)&pListPool))
        return NULL;

    pListPool->pLogMgr = pLogMgr;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pListPool->pLogMgr != NULL)
    {
        RvStatus crv = RV_OK;
        crv = RvMemoryAlloc(NULL,sizeof(RvLogSource),pLogMgr,(void*)&pListPool->pLogSource);
        if(crv != RV_OK)
        {
            RvMemoryFree(pListPool,pLogMgr);
            return NULL;
        }

        crv = RvLogSourceConstruct(pLogMgr,pListPool->pLogSource, ADS_RLIST, Name);
        if(crv != RV_OK)
        {
            RvMemoryFree(pListPool->pLogSource,pLogMgr);
            RvMemoryFree(pListPool,pLogMgr);
            return NULL;
        }
        RvLogInfo(pListPool->pLogSource, (pListPool->pLogSource,
            "RLIST_PoolListConstruct - (TotalNumberOfUserElements=%u,TotalNumberOfLists=%u,UserDataRecSize=%u,name=%s)",
            TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,NAMEWRAP(Name)));
    }
    else
    {
        pListPool->pLogSource = NULL;
    }
#else /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    pListPool->pLogSource = NULL;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    if ( ! ( pListPool->ListsHeaderRA = RA_Construct_trace ( sizeof (SingleListHeaderType) ,
                                                         TotalNumberOfLists,
                                                         NULL,
                                                         pLogMgr,
                                                         Name, file, line)))
#else
    pListPool->ListsHeaderRA = RA_Construct ( sizeof (SingleListHeaderType) ,
                                                         TotalNumberOfLists,
                                                         NULL,
                                                         pLogMgr,
                                                         Name);
    if (pListPool->ListsHeaderRA == NULL)
#endif
/* SPIRENT_END */
    {

        RvLogError(pListPool->pLogSource, (pListPool->pLogSource,
            "RLIST_PoolListConstruct - (TotalNumberOfUserElements=%u,TotalNumberOfLists=%u,UserDataRecSize=%u,name=%s)=NULL, no memory for list header",
            TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,NAMEWRAP(Name)));

        if(pListPool->pLogSource != NULL)
        {
           RvLogSourceDestruct(pListPool->pLogSource);
           RvMemoryFree(pListPool->pLogSource,pLogMgr);
        }
        RvMemoryFree(pListPool,pLogMgr);

        return NULL;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    if ( ! ( pListPool->ElementsRA = RA_Construct_trace ( sizeof (ElementHeaderType) + UserDataRecSize,
                                                      TotalNumberOfUserElements,
                                                      NULL,
                                                      pLogMgr,
                                                      Name, file, line)))
#else
    pListPool->ElementsRA = RA_Construct ( sizeof (ElementHeaderType) + UserDataRecSize,
                                                      TotalNumberOfUserElements,
                                                      NULL,
                                                      pLogMgr,
                                                      Name);
    if (pListPool->ElementsRA == NULL)
#endif
/* SPIRENT_END */
    {
        RvLogError(pListPool->pLogSource, (pListPool->pLogSource,
            "RLIST_PoolListConstruct - (TotalNumberOfUserElements=%u,TotalNumberOfLists=%u,UserDataRecSize=%u,name=%s)=NULL, no memory for elements",
            TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,NAMEWRAP(Name)));

        RA_Destruct ( pListPool->ListsHeaderRA );
        if(pListPool->pLogSource != NULL)
        {
            RvLogSourceDestruct(pListPool->pLogSource);
            RvMemoryFree(pListPool->pLogSource,pLogMgr);
        }
        RvMemoryFree(pListPool,pLogMgr);
        return NULL;
    }

    RvLogInfo(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolListConstruct - (TotalNumberOfUserElements=%u,TotalNumberOfLists=%u,UserDataRecSize=%u,name=%s)=0x%p",
        TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,NAMEWRAP(Name),pListPool));
    RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolListConstruct - (TotalNumberOfUserElements=%u,TotalNumberOfLists=%u,UserDataRecSize=%u,name=%s)=0x%p",
        TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,NAMEWRAP(Name),pListPool));
    return ((RLIST_POOL_HANDLE)pListPool);
}



/*************************************************************************************************************************
 * RLIST_PoolListDestruct
 * purpose : Free all resources ( memory allocation ) that were allocated for a given pool list.
 * input   : hPoolList : Handle to the pool list.
 * output  : None,
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_PoolListDestruct ( IN RLIST_POOL_HANDLE hPoolList )
{
    PoolOfListsType *pListPool= (PoolOfListsType *)hPoolList;
    RvLogMgr        *pLogMgr= pListPool->pLogMgr;


    RvLogInfo(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolListDestruct - (pListPool=0x%p)",pListPool));

    RA_Destruct ( pListPool->ElementsRA );
    RA_Destruct ( pListPool->ListsHeaderRA );
    if(pListPool->pLogSource != NULL)
    {
        RvLogSourceDestruct(pListPool->pLogSource);
        RvMemoryFree(pListPool->pLogSource,pLogMgr);
    }
    RvMemoryFree(pListPool,pLogMgr);
}



/*************************************************************************************************************************
 * RLIST_ListConstruct
 * purpose : Allocate a new list on top of a given pool.
 *           The function actually searched for a free list element , mark it as used and returned it handle to the
 *           user.
 * input   : hPoolList : Handle to the pool of list from which the new list should be allocated.
 * output  : None.
 * return  : Handle to the new list. Null pointer will be returned in case there are no more free lists in the pool.
 *           in case there are no resources for the new list RLIST_Null will be returned.
 ***********************************************************************************************************************/
RLIST_HANDLE RVAPI RVCALLCONV RLIST_ListConstruct ( IN RLIST_POOL_HANDLE hPoolList )
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListElement;

    RvLogInfo(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_ListConstruct - (pListPool=0x%p)",pListPool));

    if (RV_ERROR_OUTOFRESOURCES == RA_Alloc(pListPool->ListsHeaderRA,(RA_Element *)&ListElement))
    {
        RvLogError(pListPool->pLogSource, (pListPool->pLogSource,
            "RLIST_PoolListDestruct - (pListPool=0x%p)",pListPool));
        return NULL;
    }

    ListElement->FirstElementPtr = NULL;
    ListElement->LastElementPtr  = NULL;

    RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_ListConstruct - (pListPool=0x%p)=0x%p",pListPool,ListElement));
    return ( (RLIST_HANDLE)ListElement );
}

/*************************************************************************************************************************
 * RLIST_ListDestruct
 * purpose : Free a list object back to the pool of lists. All users elements that belongs to this list will be
 *           freed back to the pool.
 *           Note - No actual memory deallocation is done using this funcation, Actually the resources are just freed
 *                  back to the pool for future allocation.
 * input   : hPoolList : handle to the lists pool.
 *           hList     : handle to the list that should be freed.
 * output  : None.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_ListDestruct ( IN RLIST_POOL_HANDLE hPoolList ,
                                           IN RLIST_HANDLE      hList )
{
   PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
   SingleListHeaderType *ListPtr= (SingleListHeaderType *)hList;
   ElementHeaderType    *ElemPtr;
   ElementHeaderType    *NextElemPtr;


   if (NULL == hPoolList || NULL == hList)
   {
       return;
   }

   ElemPtr = (ElementHeaderType    *)ListPtr->FirstElementPtr;

   RvLogInfo(pListPool->pLogSource, (pListPool->pLogSource,
       "RLIST_ListDestruct - (pListPool=0x%p,hList=0x%p)",pListPool,hList));

   while ( ElemPtr != NULL )
   {
       NextElemPtr =(ElementHeaderType    *) ElemPtr->NextElement;

       RA_DeAlloc ( pListPool->ElementsRA ,
                    (RA_Element)ElemPtr );

       ElemPtr = NextElemPtr;

   }

   RA_DeAlloc ( pListPool->ListsHeaderRA,
                (RA_Element)ListPtr);

   RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
       "RLIST_ListDestruct - (pListPool=0x%p,hList=0x%p)",pListPool,hList));
}


/*************************************************************************************************************************
 * RLIST_RPoolListConstruct
 * purpose : Allocate a new list on top of a given memory pool. This will be a dynamic list
 *           of elements over this memory pool.
 * input   : hPool : Handle to the memory pool.
 *           hPage : Handle to the memory page.
 *           elementSize : Size of list element.
 *           LogRegisterId : Log Id.
 * return  : Handle to the new list. NULL will be returned in case of low resources.
 ***********************************************************************************************************************/
RLIST_HANDLE RVAPI RVCALLCONV RLIST_RPoolListConstruct (IN HRPOOL             hPool,
                                                        IN HPAGE              hPage,
                                                        IN RvInt32           elementSize,
                                                        IN RvLogSource          *pLogSource)
{
    RPoolListHeaderType  *listElement = NULL;
    RvInt32             offset;
    /* Allocate list object */
    listElement = (RPoolListHeaderType  *)RPOOL_AlignAppend(hPool, hPage, sizeof(RPoolListHeaderType), &offset);

    if(listElement == NULL)
    {
        return NULL;
    }

    RvLogInfo(pLogSource, (pLogSource,
        "RLIST_RPoolListConstruct - (hPool=0x%p,hPage=0x%p,elementSize=%d)",hPool,hPage,elementSize));

    /* Initialize structure */
    listElement->listHeaderType.FirstElementPtr = NULL;
    listElement->listHeaderType.LastElementPtr  = NULL;
    listElement->pLogSource                     = pLogSource;
    listElement->elementSize                    = elementSize;
    listElement->hPool                          = hPool;
    listElement->hPage                          = hPage;

    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_RPoolListConstruct - (hPool=0x%p,hPage=0x%p,elementSize=%d)=0x%p",hPool,hPage,elementSize,listElement));

    return ((RLIST_HANDLE)listElement);
}

/******************************************************************************
 * RLIST_PoolGetFirstElem
 * purpose : The routine returns pointer to the first element in the Pool
 *           The element is of the list element format in the Pool
 *           It can be vacant element or busy
 * input   : hPoolList  : Handle to the pool of lists.
 * output  : ElemPtr    : A pointer to the head of the list.
 *                        In case that the pool is empty, NULL will be returned
 *****************************************************************************/
void RVAPI RVCALLCONV RLIST_PoolGetFirstElem (
                                IN  RLIST_POOL_HANDLE  hPoolList,
                                OUT RLIST_ITEM_HANDLE  *pElement)
{
    PoolOfListsType *pListPool = (PoolOfListsType *)hPoolList;
    if (NULL == pListPool)
    {
        return;
    }

    RvLogEnter(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolGetFirstElem - (hPoolList=0x%p,pElement=0x%p)",hPoolList,pElement));

    if (NULL == pElement)
    {
        RvLogError(pListPool->pLogSource, (pListPool->pLogSource,
            "RLIST_PoolGetFirstElem - (hPoolList=0x%p,pElement=0x%p)=%d",hPoolList,pElement,RV_ERROR_NULLPTR));
    }
    if (NULL != pElement)
    {
        *pElement = NULL;
        if (NULL != hPoolList)
        {
            RA_Element      raElement    = RA_GetAnyElement(pListPool->ElementsRA,0);
            if (NULL != raElement)
            {
                *pElement = (RLIST_ITEM_HANDLE)((char*)raElement+sizeof(ElementHeaderType));
            }
        }
		RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
			"RLIST_PoolGetFirstElem - (hPoolList=0x%p,*pElement=0x%p)",hPoolList,*pElement));
    }
	else
	{
		RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
			"RLIST_PoolGetFirstElem - (hPoolList=0x%p,pElement=NULL)",hPoolList));

	}
    return;
}

/******************************************************************************
 * RLIST_PoolGetNextElem
 * purpose : The routine returns pointer to the next element in the Pool
 *           The element is of the list element format in the Pool
 *           It can be vacant element or busy
 * input   : hPoolList  : Handle to the pool of lists.
 *           BaseElem   : Handle to the base element in the Pool.
 * output  : NextElemPtr: Pointer to the space, where the handle of the next
 *                        element will be stored.
 *****************************************************************************/
void RVAPI RVCALLCONV RLIST_PoolGetNextElem (
                                IN  RLIST_POOL_HANDLE  hPoolList,
                                IN  RLIST_ITEM_HANDLE  BaseElem,
                                OUT RLIST_ITEM_HANDLE  *pNextElem)
{
    PoolOfListsType  *pListPool = (PoolOfListsType *)hPoolList;

    if (NULL == pNextElem || NULL == pListPool)
    {
        return;
    }

    RvLogEnter(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolGetNextElem - (hPoolList=0x%p,BaseElem=0x%p,pNextElem=0x%p)",hPoolList,BaseElem,pNextElem));

    *pNextElem = NULL;
    if (NULL != hPoolList)
    {
        RvInt32    indexOfRaBaseElement;
        RA_Element  raBaseElement, raNextElement;

        raBaseElement = (RA_Element)((char*)BaseElem-sizeof(ElementHeaderType));
        indexOfRaBaseElement = RA_GetAnyByPointer(pListPool->ElementsRA,raBaseElement);
        raNextElement = RA_GetAnyElement(pListPool->ElementsRA,(indexOfRaBaseElement+1));
        if (NULL != raNextElement)
        {
            *pNextElem = (RLIST_ITEM_HANDLE)((char*)raNextElement+sizeof(ElementHeaderType));
        }
    }
    RvLogLeave(pListPool->pLogSource, (pListPool->pLogSource,
        "RLIST_PoolGetNextElem - (hPoolList=0x%p,BaseElem=0x%p,*pNextElem=0x%p)",hPoolList,BaseElem,*pNextElem));

    return;
}

/*************************************************************************************************************************
 * RLIST_InsertHead
 * purpose : The routine is used in order to enter a new user element to the head
 *           of a given list ( that belongs to a given pool of lists ).
 * input   : hPoolList        : Handle to the pool of lists ( This is needed since the pool handle
 *                              points to the pool of free user elements ).
 *           hList            : Handle to the list.
 * output  : ElementPtr       : Pointer to the allocated element in the list , The user can use this pointer
 *                              to copy the actual data to the list.
 * return  : RV_OK        - In case of insersion success.
 *           RV_ERROR_OUTOFRESOURCES - If there are no more available user element records.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RLIST_InsertHead (IN  RLIST_POOL_HANDLE  hPoolList,
                                             IN  RLIST_HANDLE       hList,
                                             OUT RLIST_ITEM_HANDLE  *ElementPtr)
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RPoolListHeaderType  *RPoolListPtr;
    RvStatus            Status;
    ElementHeaderType    *NewUserDataElement;
    ElementHeaderType    *UserDataElement1;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);
    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_InsertHead - (hPoolList=0x%p,hList=0x%p,ElementPtr=0x%p)",hPoolList,hList,ElementPtr));

    if (NULL == hPoolList)
    {
        /* The list is a dynamic list over rpool */
        RPoolListPtr  = (RPoolListHeaderType *)hList;
        ListPtr       = &(RPoolListPtr->listHeaderType);

        Status = AllocateNewElementFromRPool(RPoolListPtr,
            (RA_Element *)&NewUserDataElement);
    }
    else
    {
        /* The list is over RA */
        ListPtr       = (SingleListHeaderType *)hList;
        Status = RA_Alloc(pListPool->ElementsRA ,
            (RA_Element *)&NewUserDataElement);
    }

    if ( Status != RV_OK  )
    {
        RvLogError(pLogSource, (pLogSource,
            "RLIST_InsertHead - (hPoolList=0x%p)=%d",hPoolList,RV_ERROR_OUTOFRESOURCES));
        ElementPtr = NULL;
        return (RV_ERROR_OUTOFRESOURCES);

    }
    /* update pointers */
    if (ListPtr->FirstElementPtr ==  NULL )
    {
        ListPtr->FirstElementPtr=(RA_Element) NewUserDataElement;
        ListPtr->LastElementPtr= (RA_Element) NewUserDataElement;
        NewUserDataElement->NextElement = NULL;
        NewUserDataElement->PrevElement = NULL;
    }
    else
    {
        UserDataElement1 = (ElementHeaderType *) ListPtr->FirstElementPtr;
        UserDataElement1->PrevElement   = (RA_Element)NewUserDataElement;
        NewUserDataElement->NextElement = ListPtr->FirstElementPtr;
        NewUserDataElement->PrevElement = NULL;
        ListPtr->FirstElementPtr         =(RA_Element)NewUserDataElement;
    }
    *ElementPtr = (RLIST_ITEM_HANDLE)((char *)NewUserDataElement+sizeof(ElementHeaderType));

    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_InsertHead - (hPoolList=0x%p,hList=0x%p,*ElementPtr=0x%p)=%d",hPoolList,hList,*ElementPtr,RV_OK));
    return ( RV_OK );
}

/*************************************************************************************************************************
 * RLIST_InsertTail
 * purpose : The routine is used in order to enter a new user element to the end
 *           of a given list ( that belongs to a given pool of lists ).
 * input   : hPoolList        : Handle to the pool of lists ( This is needed since the pool handle
 *                              points to the pool of free user elements ).
 *           hList            : Handle to the list.
 * output  : ElementPtr       : Pointer to the allocated element in the list , The user can use this pointer
 *                              to copy the actual data to the list.
 * return  : RV_OK        - In case of insersion success.
 *           RV_ERROR_OUTOFRESOURCES - If there are no more available user element records.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RLIST_InsertTail (IN  RLIST_POOL_HANDLE  hPoolList,
                                             IN  RLIST_HANDLE       hList,
                                             OUT RLIST_ITEM_HANDLE  *ElementPtr)
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RPoolListHeaderType  *RPoolListPtr;
    RvStatus            Status;
    ElementHeaderType    *NewUserDataElement;
    ElementHeaderType    *UserDataElement1;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);
    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_InsertTail - (hPoolList=0x%p,hList=0x%p,ElementPtr=0x%p)",hPoolList,hList,ElementPtr));

    if (NULL == hPoolList)
    {
        /* The list is a dynamic list over rpool */
        RPoolListPtr  = (RPoolListHeaderType *)hList;
        ListPtr       = &(RPoolListPtr->listHeaderType);

        Status = AllocateNewElementFromRPool(RPoolListPtr,
            (RA_Element *)&NewUserDataElement);
    }
    else
    {
        /* The list is over RA */
        ListPtr       = (SingleListHeaderType *)hList;
        Status = RA_Alloc( pListPool->ElementsRA ,
            (RA_Element *)&NewUserDataElement);
    }

    if ( Status != RV_OK  )
    {
        RvLogError(pLogSource, (pLogSource,
            "RLIST_InsertTail - (hPoolList=0x%p)=%d",hPoolList,RV_ERROR_OUTOFRESOURCES));
        ElementPtr = NULL;
        return ( RV_ERROR_OUTOFRESOURCES );

    }
    else
    {
        /* update pointers */
        if (ListPtr->FirstElementPtr ==  NULL )
        {
            ListPtr->FirstElementPtr = (RA_Element)NewUserDataElement;
            ListPtr->LastElementPtr = (RA_Element)NewUserDataElement;
            NewUserDataElement->NextElement = NULL;
            NewUserDataElement->PrevElement = NULL;
        }
        else
        {
            UserDataElement1 =(ElementHeaderType*)ListPtr->LastElementPtr;
            UserDataElement1->NextElement   = (RA_Element)NewUserDataElement;
            NewUserDataElement->NextElement = NULL;
            NewUserDataElement->PrevElement = ListPtr->LastElementPtr;
            ListPtr->LastElementPtr          = (RA_Element)NewUserDataElement;
        }
        *ElementPtr = (RLIST_ITEM_HANDLE)((char *)NewUserDataElement+sizeof(ElementHeaderType));
        RvLogLeave(pLogSource, (pLogSource,
            "RLIST_InsertTail - (hPoolList=0x%p,hList=0x%p,*ElementPtr=0x%p, NewElem->NextElem=0x%p)"
            ,hPoolList,hList,*ElementPtr,NewUserDataElement->NextElement));

        return ( RV_OK );
    }
}


/*************************************************************************************************************************
 * RLIST_InsertAfter
 * purpose : The routine is used in order to enter a new user element to a given list ( that belongs
 *           to a given pool of lists ).
 *           The user provides its data and the element that it should be inserted after.
 * input   : hPoolList       : Handle to the pool of lists ( This is needed since the pool handle
 *                             points to the pool of free user elements ).
 *           hList           : Handle to the list.
 *           InsertAfterItem : The new element should be insert after the given element.
 *                             Note - NULL is illeagl value.
 * output  : ElementPtr      : Pointer to the allocated element in the list , The user can use this pointer
 *                             to copy the actual data to the list.
 * return  : RV_OK        - In case of insersion success.
 *           RV_ERROR_OUTOFRESOURCES - If there are no more available user element records.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RLIST_InsertAfter (IN  RLIST_POOL_HANDLE  hPoolList,
                                              IN  RLIST_HANDLE       hList,
                                              IN  RLIST_ITEM_HANDLE  InsertAfterItem,
                                              OUT RLIST_ITEM_HANDLE  *ElementPtr)
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RPoolListHeaderType  *RPoolListPtr;
    RvStatus            Status;
    ElementHeaderType    *NewUserDataElement;
    ElementHeaderType    *UserDataElement1;
    ElementHeaderType    *InsertAfterUserData;
    RvLogSource          *pLogSource;

    if(hList == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    pLogSource = GetListLogSource(hPoolList,hList);

    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_InsertAfter - (hPoolList=0x%p,hList=0x%p,InsertAfterItem=0x%p,ElementPtr=0x%p)",hPoolList,hList,InsertAfterItem,ElementPtr));

    InsertAfterUserData = (ElementHeaderType *)( (char *)InsertAfterItem - sizeof(ElementHeaderType));


    if (NULL == hPoolList)
    {
        /* The list is a dynamic list over rpool */
        RPoolListPtr  = (RPoolListHeaderType *)hList;
        ListPtr       = &(RPoolListPtr->listHeaderType);

        Status = AllocateNewElementFromRPool(RPoolListPtr,
            (RA_Element *)&NewUserDataElement);
    }
    else
    {
        /* The list is over RA */
        ListPtr       = (SingleListHeaderType *)hList;

        Status = RA_Alloc(pListPool->ElementsRA ,
            (RA_Element *)&NewUserDataElement);
    }

    if ( Status != RV_OK  )
    {
        RvLogError(pLogSource, (pLogSource,
            "RLIST_InsertAfter - (hPoolList=0x%p,hList=0x%p,InsertAfterItem=0x%p,ElementPtr=0x%p)=%d",hPoolList,hList,InsertAfterItem,ElementPtr,RV_ERROR_OUTOFRESOURCES));
        ElementPtr = NULL;
        return ( RV_ERROR_OUTOFRESOURCES );

    }
    else
    {
        /* update pointers */
        NewUserDataElement->NextElement  = InsertAfterUserData->NextElement;
        NewUserDataElement->PrevElement  = (RA_Element)InsertAfterUserData;
        InsertAfterUserData->NextElement = (RA_Element)NewUserDataElement;
        if ( NewUserDataElement->NextElement != NULL )
        {
            UserDataElement1  =(ElementHeaderType*)NewUserDataElement->NextElement;
            UserDataElement1->PrevElement =  (RA_Element)NewUserDataElement;
        }
        else
            ListPtr->LastElementPtr =(RA_Element)NewUserDataElement;

        *ElementPtr = (RLIST_ITEM_HANDLE)( (char *)NewUserDataElement + sizeof(ElementHeaderType));

        RvLogLeave(pLogSource, (pLogSource,
            "RLIST_InsertAfter - (hPoolList=0x%p,hList=0x%p,InsertAfterItem=0x%p,*ElementPtr=0x%p)=%d",hPoolList,hList,InsertAfterItem,*ElementPtr,RV_ERROR_OUTOFRESOURCES));
        return ( RV_OK );
    }
}

/*************************************************************************************************************************
 * RLIST_InsertBefore
 * purpose : The routine is used in order to enter a new user element to a given list ( that belongs
 *           to a given pool of lists ).
 *           The user provides its data and the element that it should be inserted before.
 * input   : hPoolList        : Handle to the pool of lists ( This is needed since the pool handle
 *                              points to the pool of free user elements ).
 *           hList            : Handle to the list.
 *           InsertBeforeItem : The new element should be insert before the given element.
 *                              Note - NULL is illeagl value.
 * output  : ElementPtr       : Pointer to the allocated element in the list , The user can use this pointer
 *                              to copy the actual data to the list.
 * return  : RV_OK        - In case of insersion success.
 *           RV_ERROR_OUTOFRESOURCES - If there are no more available user element records.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV RLIST_InsertBefore (IN  RLIST_POOL_HANDLE hPoolList,
                                               IN  RLIST_HANDLE      hList,
                                               IN  RLIST_ITEM_HANDLE InsertBeforeItem,
                                               OUT RLIST_ITEM_HANDLE *ElementPtr)
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RPoolListHeaderType  *RPoolListPtr;
    RvStatus            Status;
    ElementHeaderType    *NewUserDataElement;
    ElementHeaderType    *UserDataElement1;
    ElementHeaderType    *InsertBeforeUserData;
    RvLogSource          *pLogSource;

    if(hList == NULL)
    {
        return RV_ERROR_BADPARAM;
    }

    pLogSource = GetListLogSource(hPoolList,hList);
    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_InsertBefore - (hPoolList=0x%p,hList=0x%p,InsertBeforeItem=0x%p,ElementPtr=0x%p)",hPoolList,hList,InsertBeforeItem,ElementPtr));

    InsertBeforeUserData = (ElementHeaderType *)((char *)InsertBeforeItem-sizeof(ElementHeaderType));

    if (NULL == hPoolList)
    {
        /* The list is a dynamic list over rpool */
        RPoolListPtr  = (RPoolListHeaderType *)hList;
        ListPtr       = &(RPoolListPtr->listHeaderType);
        Status = AllocateNewElementFromRPool(RPoolListPtr,
            (RA_Element *)&NewUserDataElement);
    }
    else
    {
        /* The list is over RA */
        ListPtr       = (SingleListHeaderType *)hList;
        Status = RA_Alloc(pListPool->ElementsRA ,
            (RA_Element *)&NewUserDataElement);
    }

    if ( Status != RV_OK  )
    {
        RvLogError(pLogSource, (pLogSource,
            "RLIST_InsertBefore - (hPoolList=0x%p,hList=0x%p,InsertBeforeItem=0x%p,ElementPtr=0x%p)=%d",hPoolList,hList,InsertBeforeItem,ElementPtr,RV_ERROR_OUTOFRESOURCES));
        ElementPtr = NULL;
        return ( RV_ERROR_OUTOFRESOURCES );

    }
    else
    {
        /* update pointers */
        NewUserDataElement->NextElement   = (RA_Element)InsertBeforeUserData;
        NewUserDataElement->PrevElement   = InsertBeforeUserData->PrevElement;
        InsertBeforeUserData->PrevElement = (RA_Element)NewUserDataElement;
        if ( NewUserDataElement->PrevElement != NULL )
        {
            UserDataElement1  =(ElementHeaderType *)NewUserDataElement->PrevElement;
            UserDataElement1->NextElement =  (RA_Element)NewUserDataElement;
        }
        else
            ListPtr->FirstElementPtr =(RA_Element)NewUserDataElement;

        *ElementPtr = (RLIST_ITEM_HANDLE)( (char *)NewUserDataElement + sizeof(ElementHeaderType));
        RvLogLeave(pLogSource, (pLogSource,
            "RLIST_InsertBefore - (hPoolList=0x%p,hList=0x%p,InsertBeforeItem=0x%p,*ElementPtr=0x%p)=%d",hPoolList,hList,InsertBeforeItem,*ElementPtr,RV_ERROR_OUTOFRESOURCES));
        return ( RV_OK );
    }
}

/***********************************************************************************************************************
 * RLIST_Remove
 * purpose : Remove a given user element from a given list. ( back to the pool ).
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 *           ElemId      : Pointer to the element that should be removed.
 * output  : None.
 ***********************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_Remove ( IN RLIST_POOL_HANDLE   hPoolList ,
                                     IN RLIST_HANDLE        hList,
                                     IN RLIST_ITEM_HANDLE   ElemPtr)
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    ElementHeaderType    *RemovedItemUserData;
    ElementHeaderType    *UserDataRec;
    RA_Element           PrevRecPtr;
    RA_Element           NextRecPtr;
    RvLogSource          *pLogSource;

    RemovedItemUserData = (ElementHeaderType *)((char *)ElemPtr - sizeof(ElementHeaderType));
    pLogSource = GetListLogSource(hPoolList,hList);

    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_Remove - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p)",hPoolList,hList,ElemPtr));

    if (NULL == pListPool)
    {
        /* The list is a dynamic list over rpool */
        ListPtr   = &((RPoolListHeaderType *)hList)->listHeaderType;
    }
    else
    {
        /* The list is over RA */
        ListPtr   = (SingleListHeaderType *)hList;
    }

    NextRecPtr = RemovedItemUserData->NextElement;
    PrevRecPtr = RemovedItemUserData->PrevElement;

    if ( NextRecPtr != NULL )
    {
        UserDataRec  =(ElementHeaderType*)NextRecPtr;
        UserDataRec->PrevElement = PrevRecPtr;

        /* Reset pointer to next and prev elements. They are no longer available from
		   a removed element.
        This enables detection of removal based on comparison of next before
        and after block of operations, one of that can be the removal of this
        element.
           See NotifyConnUnsentMessagesStatus for such logic example. */
        RemovedItemUserData->NextElement = NULL;
		RemovedItemUserData->PrevElement = NULL;
    }
    else
    {
        ListPtr->LastElementPtr = PrevRecPtr;
    }

    if ( PrevRecPtr != NULL )
    {
        UserDataRec  =(ElementHeaderType*)PrevRecPtr;
        UserDataRec->NextElement = NextRecPtr;
    }
    else
    {
        ListPtr->FirstElementPtr = NextRecPtr;
    }

    if (NULL != pListPool)
    {
        /* The list is over RA. deallocate RA element. */
        RA_DeAlloc( pListPool->ElementsRA ,(RA_Element)RemovedItemUserData );
    }
    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_Remove - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p)",hPoolList,hList,ElemPtr));
}

/*************************************************************************************************************************
 * RLIST_GetNext
 * purpose : The routine returnd pointer to a next element in the user list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 *           ElementPtr  : Pointer to the element that we are looking for its follower.
 * output  : NextElemPtr : A pointer to the next elements user data, In case there is no next
 *                         element a NULL pointer will be returned.
 *                         The user can use this pointer directly to get/set data.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_GetNext ( IN  RLIST_POOL_HANDLE      hPoolList ,
                                      IN  RLIST_HANDLE           hList,
                                      IN  RLIST_ITEM_HANDLE      ElemPtr,
                                      OUT RLIST_ITEM_HANDLE      *NextElemPtr)
{
    ElementHeaderType    *UserDataRec;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);
    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_GetNext - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p,NextElemPtr=0x%p)",hPoolList,hList,ElemPtr,NextElemPtr));


    UserDataRec         = (ElementHeaderType *)((char *)ElemPtr - sizeof(ElementHeaderType));

    if ( UserDataRec->NextElement == NULL )
    {
        *NextElemPtr = NULL;
    }
    else
    {
        *NextElemPtr = (RLIST_ITEM_HANDLE)((RvChar *) (UserDataRec->NextElement) +
                                            sizeof(ElementHeaderType));
    }
    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_GetNext - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p,*NextElemPtr=0x%p)",hPoolList,hList,ElemPtr,*NextElemPtr));

}


/*************************************************************************************************************************
 * RLIST_GetPrev
 * purpose : The routine returned pointer to a previous element in the user list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 *           ElementPtr  : Pointer to the element that we are looking for its predesesor.
 * output  : PrevElemPtr : A pointer to the prev elements user data , In case there is no prev
 *                         element a NULL pointer will be returned.
 *                         The user can use this pointer directly to get/set data.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_GetPrev ( IN  RLIST_POOL_HANDLE  hPoolList ,
                                      IN  RLIST_HANDLE       hList,
                                      IN  RLIST_ITEM_HANDLE  ElemPtr,
                                      OUT RLIST_ITEM_HANDLE  *PrevElemPtr )
{
    ElementHeaderType    *UserDataRec;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);

    UserDataRec         = (ElementHeaderType *)((char *)ElemPtr - sizeof(ElementHeaderType));

    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_GetPrev - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p,PrevElemPtr=0x%p)",hPoolList,hList,ElemPtr,PrevElemPtr));

    if ( UserDataRec->PrevElement == NULL )
    {
        *PrevElemPtr = NULL;
    }
    else
    {
        *PrevElemPtr = (RLIST_ITEM_HANDLE)((RvChar *) (UserDataRec->PrevElement) +
                                           sizeof(ElementHeaderType));
    }
    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_GetPrev - (hPoolList=0x%p,hList=0x%p,ElemPtr=0x%p,*PrevElemPtr=0x%p)",hPoolList,hList,ElemPtr,*PrevElemPtr));
}

/*************************************************************************************************************************
 * RLIST_GetHead
 * purpose : The routine returned pointer to the head of the given list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 * output  : PrevElemPtr : A pointer to the head of the list.
 *                         In case that the list is empty NULL pointer will be returned.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_GetHead ( IN  RLIST_POOL_HANDLE  hPoolList ,
                                      IN  RLIST_HANDLE       hList,
                                      OUT RLIST_ITEM_HANDLE  *NextElemPtr )
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);

    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_GetHead - (hPoolList=0x%p,hList=0x%p,NextElemPtr=0x%p)",hPoolList,hList,NextElemPtr));

    if(hList == NULL)
    {
        RvLogError(pLogSource, (pLogSource,
        "RLIST_GetHead - illegal parameter (hPoolList=0x%p,hList=0x%p,NextElemPtr=0x%p)",hPoolList,hList,NextElemPtr));
        return;
    }

    if (NULL == pListPool)
    {
        /* The list is a dynamic list over rpool */
        ListPtr   = &((RPoolListHeaderType *)hList)->listHeaderType;
    }
    else
    {
        /* The list is over RA */
        ListPtr   = (SingleListHeaderType *)hList;
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    if ( (ListPtr == NULL) || (ListPtr->FirstElementPtr  == NULL ))        
#else /* !defined(UPDATED_BY_SPIRENT_ABACUS) */
    if ( ListPtr->FirstElementPtr  == NULL )
#endif
/* SPIRENT_END */
    {
        *NextElemPtr = NULL;
    }
    else
    {
        *NextElemPtr = (RLIST_ITEM_HANDLE)((RvChar *) (ListPtr->FirstElementPtr) +
                                           sizeof(ElementHeaderType));
    }

    RvLogLeave(pLogSource, (pLogSource,
            "RLIST_GetHead - (hPoolList=0x%p,hList=0x%p,*NextElemPtr=0x%p)",hPoolList,hList,*NextElemPtr));

}

/*************************************************************************************************************************
 * RLIST_GetTail
 * purpose : The routine returned pointer to the last element of the given list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 * output  : PrevElemPtr : A pointer to the tail of the list.
 *                         In case that the list is empty NULL pointer will be returned.
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_GetTail ( IN  RLIST_POOL_HANDLE  hPoolList ,
                                      IN  RLIST_HANDLE       hList,
                                      OUT RLIST_ITEM_HANDLE  *PrevElemPtr )
{
    PoolOfListsType      *pListPool= (PoolOfListsType *)hPoolList;
    SingleListHeaderType *ListPtr;
    RvLogSource          *pLogSource;

    pLogSource = GetListLogSource(hPoolList,hList);

    RvLogEnter(pLogSource, (pLogSource,
        "RLIST_GetTail - (hPoolList=0x%p,hList=0x%p,PrevElemPtr=0x%p)",hPoolList,hList,PrevElemPtr));
    if (NULL == pListPool)
    {
        /* The list is a dynamic list over rpool */
        ListPtr   = &((RPoolListHeaderType *)hList)->listHeaderType;
    }
    else
    {
        /* The list is over RA */
        ListPtr   = (SingleListHeaderType *)hList;
    }

    if ( ListPtr->LastElementPtr  == NULL )
        *PrevElemPtr = NULL;
    else
        *PrevElemPtr = (RLIST_ITEM_HANDLE)((RvChar *) (ListPtr->LastElementPtr) +
                                       sizeof(ElementHeaderType));
    RvLogLeave(pLogSource, (pLogSource,
        "RLIST_GetTail - (hPoolList=0x%p,hList=0x%p,*PrevElemPtr=0x%p)",hPoolList,hList,*PrevElemPtr));
}

/**********************************************************************************************
 * RLIST_GetElementId
 * purpose : The routine gets a pointer to a user element in a given RLIST pool and return the
 *           index ( RA index for user element pool ) of this element.
 * input   : hPoolList    : Handle to the pool of lists.
 *           ElementPtr   : pointer to the user element.
 * output  : None.
 * return  : The index of the elemnt.
 ***********************************************************************************************/
RvUint32 RVAPI RVCALLCONV RLIST_GetElementId ( IN  RLIST_POOL_HANDLE  hPoolList ,
                                                IN  RLIST_ITEM_HANDLE  ElementPtr )
{
   PoolOfListsType      *pListPool;


   pListPool          = (PoolOfListsType *)hPoolList;

   if (NULL == pListPool)
   {
       return 0;
   }

   return ( RA_GetByPointer( pListPool->ElementsRA,
                             (RA_Element)((char *)ElementPtr - sizeof(ElementHeaderType))));

}


#ifdef RV_ADS_DEBUG
/*********************************************************************************************
 * RLIST_PrintList
 * purpose : Debug function that print directly to the screen the structure of a list.
 * input   : hPoolList , hList : Handles to the list pool and to the list itsef.
 * output  : None.
 ********************************************************************************************/
void RVAPI RVCALLCONV RLIST_PrintList ( IN  RLIST_POOL_HANDLE  hPoolList ,
                                        IN  RLIST_HANDLE       hList )
{
   PoolOfListsType      *pListPool;
   SingleListHeaderType *ListPtr;
   RA_Element            Element;
   ElementHeaderType    *ElementPtr;

   pListPool         = (PoolOfListsType *)hPoolList;

   if (NULL == pListPool)
   {
       /* The list is a dynamic list over rpool */
        ListPtr  = &((RPoolListHeaderType *)hList)->listHeaderType;
   }
   else
   {
       /* The list is over RA */
        ListPtr  = (SingleListHeaderType *)hList;
   }

   Element = ListPtr->FirstElementPtr;

   while ( Element != NULL )
   {
       ElementPtr = (ElementHeaderType *)Element;

       RvPrintf ("Element = %p , Next = %p . Prev = %p\n",Element,
                                                          ElementPtr->NextElement,
                                                          ElementPtr->PrevElement );
       Element = ElementPtr->NextElement;

   }


}
#endif

/************************************************************************************************************************
 * RLIST_GetResourcesStatus
 * purpose : Return information about the resource allocation of this RLIST pool.
 * input   : hPoolList   - Pointer to the rlist.
 * out     : AllocNumOfLists          - The number of lists allocated at construct time.
 *           AllocMaxNumOfUserElement - The number of user element allocated at construct time.
 *           CurrNumOfUsedLists       - The number of list that are currently allocated.
 *           CurrNumOfUsedUsreElement - The number of user records that are currently allocated.
 *           MaxUsageInLists          - The max number of lists that were allocated at the same time.
 *           MaxUsageInUserElement    - The max number of user element that were allocated at the same time.
 ************************************************************************************************************************/
void  RVAPI RVCALLCONV RLIST_GetResourcesStatus (IN   RLIST_POOL_HANDLE  hPoolList ,
                                                 OUT  RvUint32          *AllocNumOfLists,
                                                 OUT  RvUint32          *AllocMaxNumOfUserElement,
                                                 OUT  RvUint32          *CurrNumOfUsedLists,
                                                 OUT  RvUint32          *CurrNumOfUsedUsreElement,
                                                 OUT  RvUint32          *MaxUsageInLists,
                                                 OUT  RvUint32          *MaxUsageInUserElement )
{

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
   PoolOfListsType *PoolListPtr;
   RV_Resource     ListsResources;
   RV_Resource     ElementsResources;
   
   PoolListPtr  = (PoolOfListsType *)hPoolList;

   if (NULL == PoolListPtr)
   {
        *AllocNumOfLists            = 0;
        *AllocMaxNumOfUserElement   = 0;
        *CurrNumOfUsedLists         = 0;
        *CurrNumOfUsedUsreElement   = 0;
        *MaxUsageInLists            = 0;
        *MaxUsageInUserElement      = 0;

        return;
   }

   RA_GetResourcesStatus( PoolListPtr->ListsHeaderRA ,
                          &ListsResources );

   RA_GetResourcesStatus( PoolListPtr->ElementsRA ,
                          &ElementsResources );

   *AllocNumOfLists            = ListsResources.maxNumOfElements;
   *AllocMaxNumOfUserElement   = ElementsResources.maxNumOfElements;
   *CurrNumOfUsedLists         = ListsResources.numOfUsed;
   *CurrNumOfUsedUsreElement   = ElementsResources.numOfUsed;
   *MaxUsageInLists            = ListsResources.maxUsage;
   *MaxUsageInUserElement      = ElementsResources.maxUsage;
#else /* !defined(UPDATED_BY_SPIRENT_ABACUS) */
   PoolOfListsType *pListPool;
   RV_Resource     ListsResources;
   RV_Resource     ElementsResources;

   pListPool  = (PoolOfListsType *)hPoolList;

   if (NULL == pListPool)
   {
        *AllocNumOfLists            = 0;
        *AllocMaxNumOfUserElement   = 0;
        *CurrNumOfUsedLists         = 0;
        *CurrNumOfUsedUsreElement   = 0;
        *MaxUsageInLists            = 0;
        *MaxUsageInUserElement      = 0;

        return;
   }

   RA_GetResourcesStatus( pListPool->ListsHeaderRA ,
                          &ListsResources );

   RA_GetResourcesStatus( pListPool->ElementsRA ,
                          &ElementsResources );

   *AllocNumOfLists            = ListsResources.maxNumOfElements;
   *AllocMaxNumOfUserElement   = ElementsResources.maxNumOfElements;
   *CurrNumOfUsedLists         = ListsResources.numOfUsed;
   *CurrNumOfUsedUsreElement   = ElementsResources.numOfUsed;
   *MaxUsageInLists            = ListsResources.maxUsage;
   *MaxUsageInUserElement      = ElementsResources.maxUsage;
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */
}

/************************************************************************************************************************
 * RLIST_ResetMaxUsageResourcesStatus
 * purpose : Reset the counter that counts the max number of nodes that were used ( at one time )
 *           until the call to this routine.
 * input   : hPoolList   - Pointer to the rlist.
 ************************************************************************************************************************/

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
#include "AdsRa.h"
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */

void  RVAPI RVCALLCONV RLIST_ResetMaxUsageResourcesStatus (IN   RLIST_POOL_HANDLE  hPoolList)
{
   PoolOfListsType *pListPool;

   pListPool  = (PoolOfListsType *)hPoolList;

   if (NULL == pListPool)
   {
        return;
   }

   RA_ResetMaxUsageCounter(pListPool->ElementsRA);
}

/************************************************************************************************
 * RLIST_IsElementVacant
 * purpose : Check if a given element in a given list is free or allocated.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 *           ElemId      : Pointer to the element that should be removed.
 * output  : None.
 * return  : True - If the element is vacant , False if not.
 ***********************************************************************************************************************/
RVAPI RvBool RLIST_IsElementVacant ( IN RLIST_POOL_HANDLE   hPoolList ,
                                      IN RLIST_HANDLE        hList,
                                      IN RLIST_ITEM_HANDLE   ElemPtr)
{
   PoolOfListsType      *pListPool;
  /* RvInt32             ElementId;*/

   pListPool = (PoolOfListsType *)hPoolList;


   if (NULL == pListPool)
   {
       return RV_TRUE;
   }

/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT)
   return RA_ElemPtrIsVacant(pListPool->ElementsRA,
                            (RA_Element)(((char *)ElemPtr) - sizeof(ElementHeaderType)));

#else
   return RA_ElemPtrIsVacant(pListPool->ElementsRA,
                            (RA_Element)ElemPtr);
#endif
/* SPIRENT_END */

}

/************************************************************************************************
 * RLIST_GetNumOfElements
 * purpose : Returns number of elements in the list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 * output  : pNumOfElem  : Number of elements in the list
 * return  : RV_OK or RV_ERROR_BADPARAM.
 ***********************************************************************************************************************/
RVAPI RvStatus RLIST_GetNumOfElements (IN  RLIST_POOL_HANDLE   hPoolList,
                                       IN  RLIST_HANDLE        hList,
                                       OUT RvUint32           *pNumOfElem)
{
    RLIST_ITEM_HANDLE           listElem;

    /* Check input */
    if (pNumOfElem == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    if (hList == NULL)
    {
        *pNumOfElem = 0;
        return RV_OK;
    }

    /* Initiate output */
    *pNumOfElem = 0;
    
    RLIST_GetHead (hPoolList,hList,&listElem);
    while (listElem != NULL)
    {
        (*pNumOfElem)++;
        RLIST_GetNext(hPoolList,hList,listElem,&listElem);
    }

    return RV_OK;
}

/******************************************************************************
 * RLIST_RPoolListGetPoolAndPage
 * purpose : exposes pool and page, used by the RPool list.
 * input   : hList       : Handle to the list.
 * output  : phPool      : Handle to the pool, used by the list.
 *           phPage      : Handle to the page, used by the list.
 * return  : RV_OK on success, error code otherwise.
 *****************************************************************************/
RVAPI RvStatus RLIST_RPoolListGetPoolAndPage(IN  RLIST_HANDLE  hList,
                                             OUT HRPOOL        *phPool,
                                             OUT HPAGE         *phPage)
{
    RPoolListHeaderType  *pListElement = (RPoolListHeaderType*)hList;

    if (NULL==hList  ||  NULL==phPage  ||  NULL==phPool)
    {
        return RV_ERROR_BADPARAM;
    }

    RvLogEnter(pListElement->pLogSource, (pListElement->pLogSource,
        "RLIST_RPoolListGetPoolAndPage - (hList=0x%p)",hList));

    *phPool = pListElement->hPool;
    *phPage = pListElement->hPage;

    RvLogLeave(pListElement->pLogSource, (pListElement->pLogSource,
        "RLIST_RPoolListGetPoolAndPage - (hList=0x%p,*phPage=0x%p)",hList,*phPage));

    return RV_OK;
}

/*-------------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                              */
/*-------------------------------------------------------------------------*/
/************************************************************************************************
 * AllocateNewElementFromRPool
 * purpose : Allocate new element on RPOOL memory. Used for dinamic list.
 * input   : pList       : Handle to the list.
 * output  : phElement   : New allocated element.
 * return  : RV_OK - success. RV_ERROR_OUTOFRESOURCES - Failure.
 ************************************************************************************************/
static RvStatus AllocateNewElementFromRPool(IN  RPoolListHeaderType  *pList,
                                             OUT RA_Element           *phElement)
{
    RvInt32  offset;
    
    if(NULL == pList->hPage)
    {
        RvLogExcep(pList->pLogSource, (pList->pLogSource,
            "AllocateNewElementFromRPool: pList 0x%p holds a NULL page", pList));
        return RV_ERROR_UNKNOWN;
    }

    /* Allocate new consecutive element on the memory page of the list */
    *phElement = (RA_Element )RPOOL_AlignAppend(pList->hPool, pList->hPage,
                      sizeof(ElementHeaderType) + pList->elementSize,
                      &offset);
    if(*phElement == NULL)
    {
        return RV_ERROR_OUTOFRESOURCES;
    }

    return RV_OK;
}


#ifdef __cplusplus
}
#endif
