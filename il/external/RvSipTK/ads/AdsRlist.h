
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
*       Ayelet Back                                         11/2000
*
************************************************************************************************/


#ifndef RLIST_H
#define RLIST_H


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_ADS_DEF.h"
#include "AdsRa.h"
#include "AdsRpool.h"
#include "rvlog.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* Declaration of an handle to the pool of lists */
RV_DECLARE_HANDLE (RLIST_POOL_HANDLE);

/* Declaration of an handle one list */
RV_DECLARE_HANDLE (RLIST_HANDLE);

/* Declaration of an handle to list object item */
RV_DECLARE_HANDLE (RLIST_ITEM_HANDLE);


/*-----------------------------------------------------------------------*/
/*                              RLIST FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

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
#define RLIST_PoolListConstruct(TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,LogHandle,Name) RLIST_PoolListConstruct_trace(TotalNumberOfUserElements,TotalNumberOfLists,UserDataRecSize,LogHandle,Name,__FILE__,__LINE__)

RLIST_POOL_HANDLE RVAPI RVCALLCONV RLIST_PoolListConstruct_trace (IN RvUint        TotalNumberOfUserElements,
                                                            IN RvUint        TotalNumberOfLists,
                                                            IN RvUint        UserDataRecSize ,
									                	    IN RvLogMgr	 	 *pLogMgr,
                                                            IN const char*    Name,
                                                            const char* file, int line);
#else
RLIST_POOL_HANDLE RVAPI RVCALLCONV RLIST_PoolListConstruct (IN RvUint        TotalNumberOfUserElements,
                                                            IN RvUint        TotalNumberOfLists,
                                                            IN RvUint        UserDataRecSize ,
									                	    IN RvLogMgr	 	 *pLogMgr,
                                                            IN const char*    Name);
#endif
/* SPIRENT_END */


/*************************************************************************************************************************
 * RLIST_PoolListDestruct
 * purpose : Free all resources ( memory allocation ) that were allocated for a given pool list.
 * input   : hPoolList : Handle to the pool list.
 * output  : None,
 ************************************************************************************************************************/
void RVAPI RVCALLCONV RLIST_PoolListDestruct ( IN RLIST_POOL_HANDLE hPoolList );



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
RLIST_HANDLE RVAPI RVCALLCONV RLIST_ListConstruct ( IN RLIST_POOL_HANDLE hPoolList );



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
                                           IN RLIST_HANDLE      hList );


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
                                                        IN RvLogSource       *pLogSource);
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
                                OUT RLIST_ITEM_HANDLE  *ElemPtr );

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
                                OUT RLIST_ITEM_HANDLE  *NextElemPtr);

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
                                             OUT RLIST_ITEM_HANDLE  *ElementPtr);




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
                                             OUT RLIST_ITEM_HANDLE  *ElementPtr);


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
                                              OUT RLIST_ITEM_HANDLE  *ElementPtr);


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
                                               OUT RLIST_ITEM_HANDLE *ElementPtr);



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
                                     IN RLIST_ITEM_HANDLE   ElemPtr);



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
                                      OUT RLIST_ITEM_HANDLE      *NextElemPtr);


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
                                      OUT RLIST_ITEM_HANDLE  *PrevElemPtr );

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
                                      OUT RLIST_ITEM_HANDLE  *NextElemPtr );


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
                                      OUT RLIST_ITEM_HANDLE  *PrevElemPtr );

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
                                                IN  RLIST_ITEM_HANDLE  ElementPtr );


#ifdef RV_ADS_DEBUG
/*********************************************************************************************
 * RLIST_PrintList
 * purpose : Debug function that print directly to the screen the structure of a list.
 * input   : hPoolList , hList : Handles to the list pool and to the list itsef.
 * output  : None.
 ********************************************************************************************/
void RVAPI RVCALLCONV RLIST_PrintList ( IN  RLIST_POOL_HANDLE  hPoolList,
                                        IN  RLIST_HANDLE       hList );
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
                                                 OUT  RvUint32          *CurrNumOfUsedUserElement,
                                                 OUT  RvUint32          *MaxUsageInLists,
                                                 OUT  RvUint32          *MaxUsageInUserElement );

/************************************************************************************************************************
 * RLIST_ResetMaxUsageResourcesStatus
 * purpose : Reset the counter that counts the max number of nodes that were used ( at one time )
 *           until the call to this routine.
 * input   : hPoolList   - Pointer to the rlist.
 ************************************************************************************************************************/
void  RVAPI RVCALLCONV RLIST_ResetMaxUsageResourcesStatus (IN   RLIST_POOL_HANDLE  hPoolList);

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
                                      IN RLIST_ITEM_HANDLE   ElemPtr);
/************************************************************************************************
 * RLIST_GetNumOfElements
 * purpose : Returns number of elements in the list.
 * input   : hPoolList   : Handle to the pool of lists.
 *           hList       : Handle to the list.
 * output  : pNumOfElem  : Number of elements in the list
 * return  : True - If the element is vacant , False if not.
 ***********************************************************************************************************************/
RVAPI RvStatus RLIST_GetNumOfElements (IN  RLIST_POOL_HANDLE	hPoolList,
										IN  RLIST_HANDLE		hList,
										OUT RvUint32			*pNumOfElem);

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
                                             OUT HPAGE         *phPage);
#ifdef __cplusplus
}
#endif

#endif
